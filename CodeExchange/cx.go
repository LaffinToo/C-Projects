package main

// CodeExchange (cx) v3.0.0 — Go Implementation
// High-performance streaming layout utilizing native io.Writer constructs.

import (
	"fmt"
	"io"
	"os"
	"strconv"
)

const ioBufferSize = 16384
const hexDigits = "0123456789ABCDEF"

func emitRawBuffer(output io.Writer, buffer []byte) {
	if len(buffer) == 0 {
		return
	}
	var crc byte = 0
	for _, b := range buffer {
		crc ^= b
	}
	output.Write(buffer)
	fmt.Fprintf(output, "|%c%c\n", hexDigits[(crc>>4)&0x0F], hexDigits[crc&0x0F])
}

func flushRun(buffer []byte, charByte byte, count int, output io.Writer, lineLength int) []byte {
	if count == 0 {
		return buffer
	}

	var normalSeq []byte
	switch charByte {
	case '\n': normalSeq = []byte("\\n")
	case '\t': normalSeq = []byte("\\t")
	case '\r': normalSeq = []byte("\\r")
	case '\\': normalSeq = []byte("\\\\")
	default:   normalSeq = []byte{charByte}
	}

	normalWidth := len(normalSeq)
	maxAllowed := lineLength - 4
	if lineLength <= 4 {
		maxAllowed = lineLength
	}

	for count > 0 {
		chunk := count
		if chunk > 65535 {
			chunk = 65535
		}
		rawCost := normalWidth * chunk
		rleCost := 4 + normalWidth
		if chunk > 255 {
			rleCost = 6 + normalWidth
		}

		if lineLength > 0 && (len(buffer)+rleCost > maxAllowed) {
			if len(buffer) > 0 {
				emitRawBuffer(output, buffer)
				buffer = buffer[:0]
				continue
			}
		}

		if rleCost < rawCost {
			var marker string
			if chunk > 255 {
				marker = fmt.Sprintf("/z%04X", chunk)
			} else {
				marker = fmt.Sprintf("/x%02X", chunk)
			}
			buffer = append(buffer, []byte(marker)...)
			buffer = append(buffer, normalSeq...)
			count -= chunk
		} else {
			processNow := chunk
			if lineLength > 0 {
				spaceLeft := maxAllowed - len(buffer)
				if spaceLeft < 0 {
					spaceLeft = 0
				}
				maxRaw := spaceLeft / normalWidth
				if maxRaw == 0 {
					emitRawBuffer(output, buffer)
					buffer = buffer[:0]
					continue
				}
				if processNow > maxRaw {
					processNow = maxRaw
				}
			}
			for i := 0; i < processNow; i++ {
				buffer = append(buffer, normalSeq...)
			}
			count -= processNow
		}

		if lineLength > 0 && len(buffer) >= maxAllowed {
			emitRawBuffer(output, buffer)
			buffer = buffer[:0]
		}
	}
	return buffer
}

func encodeFile(output io.Writer, filePath string, lineLength int) {
	file, err := os.Open(filePath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening file: %v\n", err)
		return
	}
	defer file.Close()

	fmt.Fprintf(output, "===== FILE: %s - LL:%d ====\n", filePath, lineLength)

	var globalCrc byte = 0
	var currentByte byte = 0
	runCount := 0
	lineBuffer := make([]byte, 0, 1024)
	ioBuf := make([]byte, ioBufferSize)

	for {
		bytesRead, err := file.Read(ioBuf)
		if bytesRead > 0 {
			for i := 0; i < bytesRead; i++ {
				b := ioBuf[i]
				globalCrc ^= b

				if runCount == 0 {
					currentByte = b
					runCount = 1
				} else if b == currentByte {
					runCount++
				} else {
					lineBuffer = flushRun(lineBuffer, currentByte, runCount, output, lineLength)
					currentByte = b
					runCount = 1
				}
			}
		}
		if err == io.EOF {
			break
		}
	}

	if runCount > 0 {
		lineBuffer = flushRun(lineBuffer, currentByte, runCount, output, lineLength)
	}
	if len(lineBuffer) > 0 {
		emitRawBuffer(output, lineBuffer)
	}

	fmt.Fprintf(output, "===== ENDFILE: %s - FCS:%02X ====\n", filePath, globalCrc)
}

func main() {
	args := os.Args
	if len(args) < 2 {
		return
	}

	if args[1] == "-e" {
		lineLimit := 80
		fileIdx := 2
		if len(args) > 3 && args[2] == "-l" {
			if val, err := strconv.Atoi(args[3]); err == nil {
				lineLimit = val
			}
			fileIdx = 4
		}
		fmt.Printf("CodeExchange format - xc format v3 (Single-Pass Stream Architecture)\n\n")
		for i := fileIdx; i < len(args); i++ {
			encodeFile(os.Stdout, args[i], lineLimit)
		}
		fmt.Printf("===== EOF\n")
	}
}
