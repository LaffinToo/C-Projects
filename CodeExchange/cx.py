#!/usr/bin/env python3
# CodeExchange (cx) v3.0.0 — Python Implementation
import sys
import os

HEX_DIGITS = "0123456789ABCDEF"

def emit_raw_buffer(output, buffer_bytes):
    if not buffer_bytes:
        return
    crc = 0
    for b in buffer_bytes:
        crc ^= b
    output.write(buffer_bytes.decode('ascii') + f"|{HEX_DIGITS[(crc >> 4) & 0x0F]}{HEX_DIGITS[crc & 0x0F]}\n")

def flush_run(buffer_bytes, char_byte, count, output, line_length):
    if count == 0:
        return buffer_bytes

    normal_seq = b'\\n' if char_byte == 10 else b'\\t' if char_byte == 9 else b'\\r' if char_byte == 13 else b'\\\\' if char_byte == 92 else bytes([char_byte])
    normal_width = len(normal_seq)
    max_allowed = max(0, line_length - 4) if line_length > 0 else 999999

    while count > 0:
        chunk = min(count, 65535)
        raw_cost = normal_width * chunk
        rle_cost = (6 + normal_width) if chunk > 255 else (4 + normal_width)

        if line_length > 0 and (len(buffer_bytes) + rle_cost > max_allowed):
            if buffer_bytes:
                emit_raw_buffer(output, buffer_bytes)
                buffer_bytes = b''
                continue

        if rle_cost < raw_cost:
            marker = f"/z{chunk:04X}".encode('ascii') if chunk > 255 else f"/x{chunk:02X}".encode('ascii')
            buffer_bytes += marker + normal_seq
            count -= chunk
        else:
            process_now = chunk
            if line_length > 0:
                space_left = max(0, max_allowed - len(buffer_bytes))
                max_raw = space_left // normal_width
                if max_raw == 0:
                    emit_raw_buffer(output, buffer_bytes)
                    buffer_bytes = b''
                    continue
                process_now = min(process_now, max_raw)
            
            buffer_bytes += normal_seq * process_now
            count -= process_now

        if line_length > 0 and len(buffer_bytes) >= max_allowed:
            emit_raw_buffer(output, buffer_bytes)
            buffer_bytes = b''
            
    return buffer_bytes

def encode_file(output, file_path, line_length):
    try:
        with open(file_path, "rb") as f:
            output.write(f"===== FILE: {file_path} - LL:{line_length} ====\n")
            global_crc = 0
            line_buf = b''
            current_byte = None
            run_count = 0
            
            while True:
                chunk = f.read(16384)
                if not chunk:
                    break
                for b in chunk:
                    global_crc ^= b
                    if run_count == 0:
                        current_byte = b
                        run_count = 1
                    elif b == current_byte:
                        run_count += 1
                    else:
                        line_buf = flush_run(line_buf, current_byte, run_count, output, line_length)
                        current_byte = b
                        run_count = 1
            
            if run_count > 0:
                line_buf = flush_run(line_buf, current_byte, run_count, output, line_length)
            if line_buf:
                emit_raw_buffer(output, line_buf)
                
            output.write(f"===== ENDFILE: {file_path} - FCS:{global_crc:02X} ====\n")
    except Exception as e:
        sys.stderr.write(f"Error processing {file_path}: {e}\n")

if __name__ == "__main__":
    # Standard CLI arg routing matches the original C spec
    if len(sys.argv) > 1 and sys.argv[1] == "-e":
        line_limit = 80
        args = sys.argv[2:]
        if args and args[0] == "-l":
            line_limit = int(args[1])
            args = args[2:]
        sys.stdout.write("CodeExchange format - xc format v3 (Single-Pass Stream Architecture)\n\n")
        for filepath in args:
            encode_file(sys.stdout, filepath, line_limit)
        sys.stdout.write("===== EOF\n")
