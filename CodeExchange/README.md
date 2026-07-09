# CodeExchange (cx)

CodeExchange (`cx`) is a lightweight, zero-dependency C utility designed to package multiple text files into a single, compact archive format optimized for clean text exchange, LLM prompts, and chat-based transfers. 

Unlike traditional binaries that require specific extraction tools on the receiving end, `cx` supports **Polyglot Archives**—files that act as valid, self-extracting shell scripts (`.sh`) in a Linux/Unix terminal while remaining perfectly readable and parseable by the `cx` binary itself.

---

## Key Features

*   **Zero Dependencies:** Standard C library compilation—no external compression libraries needed.
*   **Chat & LLM Friendly:** Replaces binary control characters with clear, explicit literal escape sequences (`\n`, `\t`, `\r`, `\\`).
*   **Polyglot Compatibility:** The `cx` extraction routine automatically skips execution wrappers, hashbangs, or fallback shell script logic at the top of an archive, cleanly jumping straight to the data payload.
*   **Line-Wrapping (Formatting Control):** Enforces a maximum line length rule during encoding to prevent data truncation or distortion in strict text boxes.

---

## Core Use Cases

### 1. Feeding Source Code to LLMs & Chatbots
When sharing a multi-file coding project with an AI assistant or a colleague over a chat platform, copy-pasting raw text often breaks indentation or triggers layout formatting issues. Encoding the files with `cx` structures the entire codebase into a single, predictable block that is easy to copy, paste, and parse.

### 2. Universal Self-Extracting Script Distribution
You can prepend a standard shell script header to a generated `.cx` archive. When a Linux user receives the file, they can execute it directly in their terminal using `bash` to extract the files. If they have the `cx` binary installed, they can pass that exact same file directly to `cx -d` for a faster, robust native extraction.

---

## Installation & Compilation

Compile the utility using any standard C compiler (like `gcc` or `clang`):

```bash
gcc -O3 cx.c -o cx
