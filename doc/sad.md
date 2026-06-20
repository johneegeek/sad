# SAD

Version 2.3.x
© 1997 - 2026
John Kiernan <john@yellowsocks.com>

## Table of Contents

- [SAD](#sad)
- [Table of Contents](#table-of-contents)
- [What is SAD?](#what-is-sad)
- [How do I use SAD?](#how-do-i-use-sad)
  - [Output Formats](#output-formats)
    - [ASCII](#ascii)
    - [Binary Text](#binary-text)
    - [Control Characters](#control-characters)
    - [Decimal](#decimal)
    - [Hexadecimal](#hexadecimal)
    - [Octal](#octal)
    - [Floating point](#floating-point)
  - [Alternate Option Passing](#alternate-option-passing)
    - [Configuration File](#configuration-file)
    - [Environment Variable](#environment-variable)
  - [Command Line Options](#command-line-options)
  - [Examples](#examples)

## What is SAD?

Sad writes a representation, hex bytes by default, of a file or stream to standard output. If no file is given on the command line, standard input is used. Different formats for the output can be specified using a command line options.

## How do I use SAD?

SAD is a command line (console app) that is invoked like:

```
sad [options] [file(s)]
```

SAD will also accept piped input, or input streamed from a file:

```
echo Hello | sad

sad < hello.txt
```

### Output Formats

#### ASCII

The ASCII format is the default output file for files that contain printable ASCII characters. Printable characters are displayed as is, non-printable characters are interpreted and displayed as their meaning implies. For example:

```
Hello<cr><nl>
<ht>How are you?
<cr><nl>
*eof*
```

Where `<cr>` refers to a "carriage return" and `<nl>` refers to a "new line." If the input file is binary, then non printable characters that can no be interpreted will be displayed as hex values enclosed in <>.

#### Binary Text

The Binary text format displays all output as represented by a binary string.
For example:

```
00000000: 01001000 01100101 01101100  01101100 01101111 00001101 Hello.
00000006: 00001010 00001001 01001000  01101111 01110111 00100000 ..How
0000000C: 01100001 01110010 01100101  00100000 01111001 01101111 are yo
00000012: 01110101 00111111 00001101  00001010 00001101 00001010 u?....
00000018: *eof*
```

#### Control Characters

The Control Character format is the similar to the ASCII format. Printable characters are displayed as is, non-printable characters are interpreted and displayed as their "control" key references. For example:

```
Hello<^M><^J>
<^I>How are you?
<^M><^J>
*eof*
```

Where `<^M>` refers to CTRL+M (or a "carriage return") and `<^J>` refers to CTRL+J (or a "new line"). If the input file is binary, then non printable characters that can not be interpreted will be displayed as hex values enclosed in <>.

#### Decimal

Decimal format displays all output as represented by a decimal number. The representation can be unsigned (default) or signed. (See [--signed](#--signed--s))
For example:

```
00000000: 072 101 108 108 111  013 010 009 072 111 Hello...Ho
0000000A: 119 032 097 114 101  032 121 111 117 063 w are you?
00000014: 013 010 013 010 *eof* ....
```

#### Hexadecimal

Hexadecimal format is the default output format for binary files or files that contain non-printable characters. The output is represented by a hexadecimal number.
For example:

```
00000000: 48 65 6C 6C 6F 0D 0A 09  48 6F 77 20 61 72 65 20 Hello...How are
00000010: 79 6F 75 3F 0D 0A 0D 0A  *eof* you?....
```

#### Octal

Octal format displays all output as represented by an octal number.
For example:

```
00000000: 110 145 154 154  157 015 012 011 Hello...
00000008: 110 157 167 040  141 162 145 040 How are
00000010: 171 157 165 077  015 012 015 012 you?....
00000018: *eof*
```

#### Floating point

Floating point format displays output as represented by single (default) or double (See [--double](#--double--l)) floating point format. For example:

```
00000000:     1.819e+009     1.517e+008 5.447e+008     5.435e+008 HoHa
00000010:     1.065e+009     1.686e+008  *eof* y.
```

For floating point output, the width and precision can be specified.
(See [--float-width](#--float-widthn--w) & [--float-precision](#--float-precisionn--p))

### Alternate Option Passing

#### Configuration File

Optionally, **sad** can parse options from a configuration file. The configuration file, named **sad.cfg**, will be searched for in two possible locations.

1. The users "**HOME**" directory, as specified by the **HOME** environment variable.
2. The `%WTDIR%/config` directory. (As distributed with WT tools)
3. The `$XDG_CONFIG_DIR/sad` directory. (All environments)

The configuration file options are parsed the same as though they were given on the command line, with the exception that *newline* characters and comments are ignored. Comments are delimited with a "#" character.
An example **sad.cfg** file:

```
# sad configuration file
#
--no-highlight     # Do not add "pager" highlighting
--show-duplicates
--float-precision=10
```

NOTE: *Command line options always override options in the configuration file.*

#### Environment Variable

Optionally, **sad** can parse options from an environment variable. The environment variable is the same as the executable filename minus any extension. (**SAD** in most cases.)

NOTE: *Command line options always override options set in environment variable.*

### Command Line Options

#### --ascii (-a)

Display input in ASCII format. (See [ASCII format](#ascii)) This is the default format for non-binary files.

#### --binary (-b)

Display input in `binary text` format. (See [Binary Text format](#binary-text)) This is the default format for non-binary files.

#### --formatted-ascii (-A)

Display input in "formatted" ASCII. This is the similar to ASCII except the output if formatted to fit into columns.

#### --control (-c)

Display input in text and control characters. (See [Control Characters](#control-characters) format)

#### --decimal (-d)

Display input in decimal format. (See [Decimal](#decimal) format)

#### --octal (-o)

Display input in octal format. (See [Octal](#octal) format)

#### --signed (-s)

Display input in signed decimal format.

#### --hex (-x)

Display input in hex format. (See [Hexadecimal](#hexadecimal) format)

#### --float (-f)

Display input in floating point format. (See [Floating Point](#floating-point) format)

#### --double (-l)

Display in double floating point format. (See [Floating Point](#floating-point) format)

#### --float-width=&lt;n&gt; (-W)

When using floating point numbers (and double) you can specify the total `width` of the number. (See [Floating Point](#floating-point) format)

#### --float-precision=&lt;n&gt; (-P)

When using floating point numbers (and double) you can specify the precision of the number. (The number of decimal places) The default is 6.
(See [Floating Point](#floating-point) format)

#### --big-endian (-B)

Use "big endian" byte ordering. The default is to represent everything in "little endian" or "Intel" format. This option will allow you to display output in "big endian" or "Motorola" format.

#### --no-highlight (-H)

Do not add highlight information. By default, SAD outputs special highlighting that is visible in programs like LESS. This may look strange in some pagers or text editors.

#### --no-offset (-F)

Do not show offset. By default, SAD displays the input offset in bytes for some output formats. The offset is shown in Hexadecimal by default.

#### --no-ascii (-I)

Do not show ascii representation of data. In some output formats, SAD displays an ASCII representation of the input data at the end of the line, this can be overridden with this option.

#### --no-eof-marker (-E)

Do not show end of file marker. SAD displays an end of file marker when it gets to the end of the input data. It is represented by **\*eof\*** or **@** depending on how much space is available on the line. If the end of file marker is not desired this option will prevent the eof marker from being displayed.

#### --show-duplicate (-U)

Show duplicate lines. SAD normally tries to compress the output data by representing duplicated lines with an asterisk (*). Use this option to display all lines.

#### --show-spaces (-e)

Show visible spaces in ascii mode. SAD will display a visual representation of a space instead of displaying an actual space.

#### --offset-oct (-q)

Show offsets in octal. By default SAD outputs offsets in hexadecimal.

#### --offset-dec (-z)

Show offsets in decimal. By default SAD outputs offsets in hexadecimal.

#### --pager[=PAGER] (-p)

Send output to pager. If a pager program is optionally supplied, SAD will attempt to send output to the pager specified, otherwise SAD will check for an environmental variable called PAGER. If an environmental variable does not exist, and a pager program was not specified, SAD will try to send output to "**more**." In Windows and UNIX environments SAD will open a pipe to the pager. In DOS environments SAD will try to open a temporary file and send it to the pager.

#### --columns=&lt;n&gt; (-n)

Number of columns that will display on a line. SAD will default to a number so each line can be displayed without wrapping. If a different number of columns are required, they can be specified here.

#### --width=&lt;n&gt; (-w)

Data width in bytes to interpret and display, default is 1.
Valid byte widths are (1, 2, 4, and 8).

#### --bytes, --8bit (-1)

Display in bytes or 8 bits. (Same as [--width](#--widthn--w)=1).

#### --words,--16bit (-2)

Display in words or 16 bits. (Same as [--width](#--widthn--w)=2).

#### --dwords,--32bit (-4)

Display in double words or 32 bits. (Same as [--width](#--widthn--w)=4).

#### --qwords,--64bit (-8)

Display in quad words or 64 bits. (Same as [--width](#--widthn--w)=8).

#### --start-count=&lt;n&gt; (-j)

Starting item to begin dump. (0 based index) This is a count of "elements" that are of size [width](#--widthn--w). Note: Since --width defaults to 1, this option must be after this the --width option on the command line or the count will be a count of bytes.

#### --start-offset=&lt;n&gt; (-J)

Starting offset in bytes to begin dump. An absolute offset in bytes is specified.

#### --count=&lt;n&gt; (-k)

Number of items to dump (of data width). This is a count of "elements" that are of size [width](#--widthn--w). Note: Since --width defaults to 1, this option must be after this the --width option on the command line or the count will be a count of bytes.

#### --end-offset=&lt;n&gt; (-K)

Ending offset in bytes. An absolute offset in bytes is specified.

#### --help (-?)

Show an option listing.

#### --version (-v)

Show version information

### Examples

```
sad --hex --qwords --columns=1 --no-ascii test.txt
sad –x -8 –n1 –I test.txt
```

Output:
```
00000000: 090A0D6F6C6C6548
00000008: 2065726120776F48
00000010: 0A0D0A0D3F756F79
00000018: *eof*
```

---

```
sad --hex –-qwords –-columns=1 –-start-count=2 –count=1 test.txt
sad -x -8 -n1 -j0 -k1 test.txt
```

Output:
```
00000008: 2065726120776F48 H
```

---

```
sad -fEFHn1 test.txt
```

Output:
```
    1.819e+009 H
    1.517e+008 o
    5.447e+008 H
    5.435e+008 a
    1.065e+009 y
    1.686e+008 .
```
