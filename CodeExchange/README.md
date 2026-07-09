# CodeExchange (cx) — Version 2

CodeExchange (`cx`) is a lightweight, zero-dependency C utility designed to package multiple text files into a single, compact archive format optimized for clean text exchange, LLM prompts, and chat-based transfers. 

Unlike raw copy-pasting which can be mangled by platform text-formatting engines, `cx` implements a transparent, human-readable layout reinforced by dual-mode Hex Run-Length Encoding (RLE) and strict per-line validation checkpoints.

---

## Key Features

* **Human-Transparent Compression:** Uses custom `/x` and `/z` text markers for compression. The output remains 100% human-readable without requiring extraction tools just to see what the code does.
* **Smart RLE Engine:** Dynamically calculates overhead costs. It skips encoding entirely if raw text takes fewer bytes than compression markers.
* **Dual-Mode Sizing Hex:** Uses `/xXX` (byte hex) for tight control of small repetitions (up to 255) and automatically scales to `/zXXXX` (word hex) for massive formatting rows (up to 65535).
* **Anti-Mangle Bumper System:** Embeds an explicit pipe delimiter (`|`) to anchor lines, preventing aggressive forum filters from stripping trailing spaces or tabs via `rtrim()`.
* **Two-Layer Data Integrity:** Protects snippets using a fast, localized 1-byte XOR checksum per line (`|XX`) along with a global file checksum (`FCS:XX`) inside the container header.

---

## Command Line Switches

### Encoding Files
```bash
cx -e [-l line_length] <file1> [file2 ...]
