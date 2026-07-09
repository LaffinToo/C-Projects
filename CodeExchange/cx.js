#!/usr/bin/env node
/**
 * CodeExchange (cx) v3.0.0 — Node.js Implementation
 * High-performance streaming architecture using file descriptor blocks.
 */

const fs = require('fs');
const path = require('path');

const HEX_DIGITS = "0123456789ABCDEF";
const IO_BUFFER_SIZE = 16384;

function emitRawBuffer(outputStream, buffer) {
    if (buffer.length === 0) return;
    let crc = 0;
    for (let i = 0; i < buffer.length; i++) {
        crc ^= buffer[i];
    }
    const hexCrc = HEX_DIGITS[(crc >> 4) & 0x0F] + HEX_DIGITS[crc & 0x0F];
    outputStream.write(buffer.toString('ascii') + `|${hexCrc}\n`);
}

function flushRun(buffer, charByte, count, outputStream, lineLength) {
    if (count === 0) return buffer;

    let normalSeq;
    switch (charByte) {
        case 10: normalSeq = Buffer.from('\\n'); break;
        case 9:  normalSeq = Buffer.from('\\t'); break;
        case 13: normalSeq = Buffer.from('\\r'); break;
        case 92: normalSeq = Buffer.from('\\\\'); break;
        default: normalSeq = Buffer.from([charByte]);
    }
    
    const normalWidth = normalSeq.length;
    const maxAllowed = lineLength > 4 ? lineLength - 4 : lineLength;

    while (count > 0) {
        const chunk = Math.min(count, 65535);
        const rawCost = normalWidth * chunk;
        const rleCost = chunk > 255 ? (6 + normalWidth) : (4 + normalWidth);

        if (lineLength > 0 && (buffer.length + rleCost > maxAllowed)) {
            if (buffer.length > 0) {
                emitRawBuffer(outputStream, buffer);
                buffer = Buffer.alloc(0);
                continue;
            }
        }

        if (rleCost < rawCost) {
            const markerStr = chunk > 255 
                ? `/z${chunk.toString(16).toUpperCase().padStart(4, '0')}`
                : `/x${chunk.toString(16).toUpperCase().padStart(2, '0')}`;
            buffer = Buffer.concat([buffer, Buffer.from(markerStr), normalSeq]);
            count -= chunk;
        } else {
            let processNow = chunk;
            if (lineLength > 0) {
                const spaceLeft = Math.max(0, maxAllowed - buffer.length);
                const maxRaw = Math.floor(spaceLeft / normalWidth);
                if (maxRaw === 0) {
                    emitRawBuffer(outputStream, buffer);
                    buffer = Buffer.alloc(0);
                    continue;
                }
                processNow = Math.min(processNow, maxRaw);
            }

            const repeated = Buffer.alloc(processNow * normalWidth);
            for (let i = 0; i < processNow; i++) {
                normalSeq.copy(repeated, i * normalWidth);
            }
            buffer = Buffer.concat([buffer, repeated]);
            count -= processNow;
        }

        if (lineLength > 0 && buffer.length >= maxAllowed) {
            emitRawBuffer(outputStream, buffer);
            buffer = Buffer.alloc(0);
        }
    }
    return buffer;
}

function encodeFile(outputStream, filePath, lineLength) {
    if (!fs.existsSync(filePath)) {
        process.stderr.write(`Error: File not found: ${filePath}\n`);
        return;
    }

    outputStream.write(`===== FILE: ${filePath} - LL:${lineLength} ====\n`);
    
    let globalCrc = 0;
    let lineBuffer = Buffer.alloc(0);
    let currentByte = null;
    let runCount = 0;

    const fd = fs.openSync(filePath, 'r');
    const ioBuf = Buffer.alloc(IO_BUFFER_SIZE);
    let bytesRead = 0;

    while ((bytesRead = fs.readSync(fd, ioBuf, 0, IO_BUFFER_SIZE, null)) > 0) {
        for (let i = 0; i < bytesRead; i++) {
            const b = ioBuf[i];
            globalCrc ^= b;

            if (runCount === 0) {
                currentByte = b;
                runCount = 1;
            } else if (b === currentByte) {
                runCount++;
            } else {
                lineBuffer = flushRun(lineBuffer, currentByte, runCount, outputStream, lineLength);
                currentByte = b;
                runCount = 1;
            }
        }
    }
    fs.closeSync(fd);

    if (runCount > 0) {
        lineBuffer = flushRun(lineBuffer, currentByte, runCount, outputStream, lineLength);
    }
    if (lineBuffer.length > 0) {
        emitRawBuffer(outputStream, lineBuffer);
    }

    const hexGlobal = globalCrc.toString(16).toUpperCase().padStart(2, '0');
    outputStream.write(`===== ENDFILE: ${filePath} - FCS:${hexGlobal} ====\n`);
}

// CLI Command Route Dispatcher
const args = process.argv.slice(2);
if (args[0] === '-e') {
    let lineLimit = 80;
    let fileIdx = 1;
    if (args[1] === '-l') {
        lineLimit = parseInt(args[2], 10) || 80;
        fileIdx = 3;
    }
    process.stdout.write("CodeExchange format - xc format v3 (Single-Pass Stream Architecture)\n\n");
    for (let i = fileIdx; i < args.length; i++) {
        encodeFile(process.stdout, args[i], lineLimit);
    }
    process.stdout.write("===== EOF\n");
}
