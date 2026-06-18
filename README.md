# SAD - Slicer's Awesome Dump

A powerful, cross-platform hex dump utility with multiple output formats and extensive customization options.

## Overview

SAD writes various representations of files or streams to standard output. Unlike basic hex dump tools, SAD offers multiple output formats including hexadecimal, decimal, octal, binary, ASCII, floating point, and more. It's designed to be highly configurable and works seamlessly across Linux, macOS, and Windows platforms.

## Features

- **Multiple Output Formats**: Hex, decimal, octal, binary, ASCII, control characters, floating point (single/double)
- **Flexible Data Width**: Display data as bytes (8-bit), words (16-bit), dwords (32-bit), or qwords (64-bit)
- **Endianness Support**: Switch between little-endian and big-endian byte ordering
- **Range Selection**: Dump specific byte ranges using start/end offsets or counts
- **Duplicate Compression**: Optionally compress repeated lines for cleaner output
- **Pager Integration**: Built-in support for piping to `less`, `more`, or custom pagers
- **Configuration**: Save preferences in config files or environment variables
- **Cross-Platform**: Works on Linux, macOS, and Windows

## Building

SAD uses CMake for building across all platforms. For detailed build instructions, see [BUILD.md](BUILD.md).

### Quick Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# The executable will be in build/bin/sad
```

### Installation

```bash
# Install to default location (/usr/local/bin on Unix)
sudo cmake --install build

# Or install to custom location
cmake --install build --prefix=$HOME/.local
```

## Usage

### Basic Syntax

```bash
sad [options] [file(s)]
```

If no file is specified, SAD reads from standard input.

### Common Examples

#### Basic hex dump
```bash
sad myfile.bin
```
Output:
```
00000000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 0D 0A Hello World..
```

#### View file in ASCII mode
```bash
sad --ascii textfile.txt
```
Output:
```
Hello<cr><nl>
How are you?<cr><nl>
*eof*
```

#### Dump as binary text
```bash
sad --binary myfile.bin
```
Output:
```
00000000: 01001000 01100101 01101100 01101100 01101111 00001101 Hello.
00000006: 00001010 00001001 01001000 01101111 01110111 00100000 ..How
```

#### Display as 64-bit hex values, one per line
```bash
sad --hex --qwords --columns=1 --no-ascii test.bin
# Or using short options:
sad -x -8 -n1 -I test.bin
```
Output:
```
00000000: 090A0D6F6C6C6548
00000008: 2065726120776F48
00000010: 0A0D0A0D3F756F79
```

#### Dump specific range (bytes 100-200)
```bash
sad --start-offset=100 --end-offset=200 myfile.bin
```

#### Dump as floating point numbers
```bash
sad --float --columns=1 datafile.bin
# Or:
sad -f -n1 datafile.bin
```

#### View with pager
```bash
sad --pager largefile.bin
# Or:
sad -p largefile.bin
```

#### Decimal output with signed values
```bash
sad --decimal --signed myfile.bin
# Or:
sad -d -s myfile.bin
```

#### Pipe input from other commands
```bash
echo "Hello World" | sad
cat myfile.bin | sad --hex --words
```

## Output Formats

| Format | Option | Description |
|--------|--------|-------------|
| **ASCII** | `-a, --ascii` | Printable ASCII with control character names (`<cr>`, `<nl>`, etc.) |
| **Hexadecimal** | `-x, --hex` | Hexadecimal bytes (default for binary files) |
| **Decimal** | `-d, --decimal` | Decimal numbers (unsigned by default) |
| **Octal** | `-o, --octal` | Octal numbers |
| **Binary** | `-b, --binary` | Binary text representation (0s and 1s) |
| **Control Chars** | `-c, --control` | Shows control characters as `<^M>`, `<^J>`, etc. |
| **Float** | `-f, --float` | Single-precision floating point |
| **Double** | `-l, --double` | Double-precision floating point |

## Key Options

### Data Width
- `-1, --bytes, --8bit` - Display as 8-bit bytes (default)
- `-2, --words, --16bit` - Display as 16-bit words
- `-4, --dwords, --32bit` - Display as 32-bit double words
- `-8, --qwords, --64bit` - Display as 64-bit quad words

### Display Options
- `-n, --columns=<n>` - Number of columns per line
- `-F, --no-offset` - Hide byte offset column
- `-I, --no-ascii` - Hide ASCII representation column
- `-E, --no-eof-marker` - Don't show end-of-file marker
- `-H, --no-highlight` - Disable pager highlighting
- `-U, --show-duplicate` - Show duplicate lines (don't compress)
- `-e, --show-spaces` - Show visible space characters in ASCII mode

### Offset Options
- `-q, --offset-oct` - Show offsets in octal
- `-z, --offset-dec` - Show offsets in decimal

### Range Selection
- `-j, --start-count=<n>` - Start at element count (based on width)
- `-J, --start-offset=<n>` - Start at byte offset
- `-k, --count=<n>` - Dump N elements (based on width)
- `-K, --end-offset=<n>` - End at byte offset

### Byte Order
- `-B, --big-endian` - Use big-endian byte order (default is little-endian)

### Floating Point Options
- `-W, --float-width=<n>` - Total width of floating point numbers
- `-P, --float-precision=<n>` - Decimal precision (default: 6)

### Other
- `-p, --pager[=PAGER]` - Send output through a pager program
- `-?, --help` - Show help message
- `-v, --version` - Show version information

## Configuration

### Configuration File

SAD can read default options from a configuration file named `sad.cfg` in:
1. Your `$HOME` directory
2. The `%ST32%\config` directory (Windows)

Example `sad.cfg`:
```bash
# SAD configuration file
--no-highlight       # Disable pager highlighting
--show-duplicates    # Show all lines
--float-precision=10 # Set float precision
```

### Environment Variable

Set default options via the `SAD` environment variable:

```bash
export SAD="--hex --no-highlight"
sad myfile.bin
```

**Note**: Command-line options always override config file and environment variable settings.

## Use Cases

### Analyzing Binary Files
```bash
# Examine executable headers
sad --hex --qwords --columns=2 /bin/ls | head -20

# Look for text strings in binary
sad --ascii suspicious.exe | grep -i "password"
```

### Debugging File Formats
```bash
# View file structure with decimal offsets
sad --hex --offset-dec datafile.bin

# Examine floating-point data
sad --float --double --columns=4 scientific_data.bin
```

### Data Comparison
```bash
# Compare two files side by side (with diff tools)
sad file1.bin > /tmp/dump1.txt
sad file2.bin > /tmp/dump2.txt
diff /tmp/dump1.txt /tmp/dump2.txt
```

### Network Protocol Analysis
```bash
# Dump packet captures
tcpdump -r capture.pcap -w - | sad --hex --words
```

## Platform Support

- **Linux**: GCC, Clang
- **macOS**: Apple Clang, GCC
- **Windows**: MSVC, Watcom, Borland

## Version

Current version: 2.5.x

See [version.h](src/version.h) for detailed version information.

## License

SAD is free for non-commercial use.

Commercial use requires a license. For licensing information, contact:
[sad-license@yellowsocks.com](mailto:sad-license@yellowsocks.com)

## Author

John Kiernan <[john@yellowsocks.com](mailto:john@yellowsocks.com)>

Copyright © 1997, 2002, 2003

## Documentation

For complete documentation, see [doc/sad.html](doc/sad.html).

## Contributing

This is a modernized version of SAD with updated build system using CMake. The old Perl-based configuration system has been replaced with native CMake configuration.
