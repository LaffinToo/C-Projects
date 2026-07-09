# CodeExchange (cx) — Version 3

CodeExchange (`cx`) is a high-performance, zero-dependency C utility designed to package multiple text files into a single, compact archive format optimized for clean text exchange, LLM prompts, and chat-based transfers. 

Unlike raw copy-pasting which can be mangled by platform text-formatting engines, `cx` implements a transparent, human-readable layout reinforced by dual-mode Hex Run-Length Encoding (RLE) and strict per-line validation checkpoints.

Version 3 introduces a **Single-Pass Stream Architecture**, leveraging block-buffered file I/O and interactive, on-the-fly checksumming. By eliminating file rewinds, Version 3 allows `cx` to handle direct standard input/output (`stdin`/`stdout`) data pipelines flawlessly.

---

## Key Features

* **Single-Pass Streaming Engine:** Processes data in high-speed blocks (16KB caches) without file rewinds, providing native support for shell pipes and live streams.
* **Human-Transparent Compression:** Uses custom `/x` and `/z` text markers for compression. The output remains 100% human-readable without requiring extraction tools just to see what the code does.
* **Smart RLE Engine:** Dynamically calculates overhead costs. It skips encoding entirely if raw text takes fewer bytes than compression markers.
* **Dual-Mode Sizing Hex:** Uses `/xXX` (byte hex) for tight control of small repetitions (up to 255) and automatically scales to `/zXXXX` (word hex) for massive formatting rows (up to 65535).
* **Anti-Mangle Bumper System:** Embeds an explicit pipe delimiter (`|`) to anchor lines, preventing aggressive forum filters from stripping trailing spaces or tabs via `rtrim()`.
* **Two-Layer Data Integrity:** Protects snippets using a fast, localized 1-byte XOR checksum calculated interactively per line (`|XX`) along with a global file checksum (`FCS:XX`) appended seamlessly in the trailer footer.

---

## Command Line Switches

### Encoding Files
```bash
cx -e [-l line_length] <file1> [file2 ...]
```
* `-e` : Triggers the encoding routine. Output is sent directly to `stdout` (can be piped to a file, clipboard manager, or another command).
* `-l <number>` : *(Optional)* Sets the maximum character width limit per line before forcing a safe formatting break. Defaults to **80**.

### Decoding Archives
```bash
cx -d <archive_file>
```
* `-d` : Triggers the decoding routine to extract payload items.
* `<archive_file>` : Path to the targeting source text file structure containing the CodeExchange content stream.

---

## Archive Structural Format

Every archived object inside a `cx` stream is bounded by explicit metadata container frames:

```text
CodeExchange format - xc format v3 (Single-Pass Stream Architecture)

===== FILE: <filename> - LL:<line_limit> ====
<payload_line_1>|<line_checksum>
<payload_line_2>|<line_checksum>
===== ENDFILE: <filename> - FCS:<global_file_checksum> ====
===== EOF
```

### Formatting Syntax Markers
* `/xXX[char]` : Repetition run up to 255 elements where `XX` is a 2-digit uppercase Hex value.
* `/zXXXX[char]` : Repetition run up to 65535 elements where `XXXX` is a 4-digit uppercase Hex value.
* `\n`, `\t`, `\r`, `\\` : Legacy structural escape codes used to keep strings safely flat during transmission.

---

## Translation Examples

### Example 1: Indentation & Short Runs
**Original Source (`test.py`):**
```python
def main():
    print("CodeExchange Active")
```

**Encoded Archive Output:**
```text
===== FILE: test.py - LL:80 ====
def main():\n|6B
/x04print("CodeExchange Active")\n|2E
===== ENDFILE: test.py - FCS:4C ====
```
* *Notice:* The 4 leading indentation spaces are cleanly optimized into `/x04 `, while the line checksum (`|2E`) validates the content string integrity.

### Example 2: Massive Multi-Element Divider
**Original Source (`banner.txt`):**
```text
====================================================================================================
```
*(100 consecutive `=` characters)*

**Encoded Archive Output:**
```text
===== FILE: banner.txt - LL:80 ====
/x64=\n|8B
===== ENDFILE: banner.txt - FCS:2A ====
```
* *Notice:* The 100 raw byte columns automatically condense down to 8 safe characters (`/x64=\n|8B`), saving substantial layout real estate inside text boundaries.

### Example 3: Forced Width Wrapping (`-l 40`)
If an input file contains an exceptionally long string of text without newlines, the encoder uses the `-l` constraint as a hard ceiling. It automatically subtracts 4 columns to account for the trailing `|XX` envelope marker, safely fracturing the stream into discrete blocks.

**Original Source (`long_string.txt`):**
```text
AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRRSSSSTTTTUUUUVVVVWWWWXXXXYYYYZZZZ
```
*(104 characters of continuous un-broken text)*

**Encoded Archive Output using `cx -e -l 40`:**
```text
===== FILE: long_string.txt - LL:40 ====
AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIII|19
JJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRR|0E
SSSSTTTTUUUUVVVVWWWWXXXXYYYYZZZZ\n|51
===== ENDFILE: long_string.txt - FCS:1E ====
```
* *Notice:* The lines wrap cleanly right at **36 characters of payload**, leaving exactly enough room for the trailing `|XX` boundaries without overflowing your strict 40-character target terminal limit. Each fractured slice receives its own standalone row hash!

---

## Multi-Language Implementations

While the core utility is written in optimized C, CodeExchange Version 3 is built for cross-platform pipelines. Independent implementations conforming to the single-pass streaming architecture specification are available in the repository:

* **Node.js (`cx.js`):** Ideal for cloud-native backend environments, GitHub Actions pipelines, or web-based CLI tooling. Leverages non-blocking buffers for low-memory processing.
* **Go (`cx.go`):** Built for cross-compiling into tiny, ultra-fast standalone binaries. Ideal for DevOps engineers and bare-metal streaming speed across systems.
* **C# / .NET Core (`cx.cs`):** Perfect for modern enterprise applications or cross-platform desktop integration, utilizing high-performance memory spans to achieve zero-allocation data passes.

---

## Local Compilation

Compile using any standard C99 compatible compiler with zero external configuration requirements:

```bash
gcc -O2 cx.c -o cx
```
