# Building SAD

This document describes how to build SAD using CMake on Linux, macOS, and Windows.

## Prerequisites

- **CMake** 3.15 or higher
- A C99-compatible C compiler
- A C++11-compatible C++ compiler

### Platform-Specific Compilers

#### Linux
- GCC 4.8+ or Clang 3.4+
- Install via package manager:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install build-essential cmake

  # Fedora/RHEL
  sudo dnf install gcc gcc-c++ cmake

  # Arch Linux
  sudo pacman -S base-devel cmake
  ```

#### macOS
- Xcode Command Line Tools or Xcode
- Install via:
  ```bash
  xcode-select --install
  brew install cmake  # if using Homebrew
  ```

#### Windows
Supported compilers:
- Visual Studio 2015 or higher (MSVC)
- Watcom C/C++ (if `WATCOM` environment variable is set)
- Borland C++ (legacy support)

## Quick Start

### Basic Build

```bash
# Configure the build
cmake -B build

# Build the project
cmake --build build

# The executable will be in build/bin/sad
```

### Build with Release Optimizations

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Build Options

### Build Types

Specify the build type with `-DCMAKE_BUILD_TYPE=<type>`:

- **Release** (default) - Optimized build with no debug symbols
- **Debug** - No optimization, includes debug symbols
- **RelWithDebInfo** - Optimized with debug symbols
- **MinSizeRel** - Optimized for minimal size

Example:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

### Custom Installation Prefix

By default, SAD installs to:
- Linux/macOS: `/usr/local/bin`
- Windows: `C:\Program Files\sad\bin`

To specify a custom installation directory:

```bash
# Configure with custom prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/sad

# Build
cmake --build build

# Install to the custom location
cmake --install build
```

### Parallel Builds

Speed up compilation using multiple CPU cores:

```bash
# Use all available cores
cmake --build build --parallel

# Or specify number of cores
cmake --build build -j 4
```

## Platform-Specific Instructions

### Linux

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Install (may require sudo)
sudo cmake --install build

# Or install to user directory
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build
cmake --install build
```

### macOS

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Install (may require sudo)
sudo cmake --install build

# Or install to user directory
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/bin
cmake --build build
cmake --install build
```

### Windows

#### Using Visual Studio

```cmd
REM Configure for Visual Studio
cmake -B build -G "Visual Studio 17 2022"

REM Build
cmake --build build --config Release

REM Install
cmake --install build --config Release
```

#### Using Watcom

Ensure the `WATCOM` environment variable is set, then:

```cmd
REM Configure
cmake -B build -G "Watcom WMake"

REM Build
cmake --build build

REM Install
cmake --install build
```

#### Using NMake (MSVC command-line)

```cmd
REM Open Visual Studio Developer Command Prompt, then:
cmake -B build -G "NMake Makefiles"
cmake --build build
cmake --install build
```

## Advanced Options

### Out-of-Source Builds

CMake supports multiple build directories from the same source:

```bash
# Create different build configurations
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake -B build-release -DCMAKE_BUILD_TYPE=Release

# Build specific configurations
cmake --build build-debug
cmake --build build-release
```

### Verbose Build Output

To see the actual compiler commands being executed:

```bash
cmake --build build --verbose
```

Or set verbose makefiles during configuration:

```bash
cmake -B build -DCMAKE_VERBOSE_MAKEFILE=ON
```

### Specifying a Compiler

To use a specific compiler:

```bash
# GCC
cmake -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++

# Clang
cmake -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# Specific version
cmake -B build -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11
```

## Cleaning the Build

To clean and rebuild from scratch:

```bash
# Remove the build directory
rm -rf build

# Reconfigure and rebuild
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Testing the Build

After building, test the executable:

```bash
# Run the built executable
./build/bin/sad --help

# Or on Windows
build\bin\sad.exe --help
```

## Common Issues

### CMake Not Found

**Error:** `cmake: command not found`

**Solution:** Install CMake from your package manager or download from https://cmake.org/download/

### Compiler Not Found

**Error:** `No CMAKE_C_COMPILER could be found`

**Solution:** Install a C/C++ compiler for your platform (see Prerequisites above)

### Permission Denied During Install

**Error:** `Permission denied` when installing

**Solution:**
- Use `sudo` on Linux/macOS: `sudo cmake --install build`
- Or install to a user directory: `cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local`

### Old CMake Version

**Error:** `CMake 3.15 or higher is required`

**Solution:** Update CMake:
```bash
# Ubuntu/Debian (may need to add Kitware's repository)
sudo apt-get update
sudo apt-get install cmake

# macOS
brew upgrade cmake

# Or download from https://cmake.org/download/
```

## Migration from Old Build System

If you previously used the Perl-based configure script:

**Old way:**
```bash
perl configure.pl
cd build
make
make install
```

**New way:**
```bash
cmake -B build
cmake --build build
cmake --install build
```

The new CMake system automatically detects your platform and compiler - no configuration script needed!

## Automated Release Builds

The repository includes a GitHub Actions workflow that automatically builds binaries for macOS, Windows, and Linux when you create a release tag.

### Creating a Release

To trigger an automated release build:

```bash
# 1. Ensure all changes are committed
git add .
git commit -m "Prepare for release v2.6.1"
git push

# 2. Create an annotated tag
git tag -a v2.6.1 -m "Release version 2.6.1"

# 3. Push the tag to GitHub
git push origin v2.6.1
```

### What Happens Automatically

When you push a tag (e.g., `v2.6.1`), GitHub Actions will:

1. **Build on three platforms in parallel:**
   - macOS (x64) → `sad-v2.6.1-macos-x64`
   - Windows (x64) → `sad-v2.6.1-windows-x64.exe`
   - Ubuntu Linux (x64) → `sad-v2.6.1-linux-x64`

2. **Create a GitHub Release** with:
   - All three binaries attached and ready to download
   - Auto-generated release notes from commit messages
   - The tag as the release version

### Monitoring Builds

To watch the automated builds:

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. You'll see the "Release Binaries" workflow running
4. Click on the workflow run to see real-time build progress for each platform

### Downloading Binaries

Once the workflow completes:

1. Go to your repository on GitHub
2. Click the **Releases** section (right sidebar)
3. Find your version (e.g., `v2.6.1`)
4. Download the binary for your platform

Users can download pre-built binaries instead of building from source.

### Managing Tags

List all tags:
```bash
git tag
```

View tag details:
```bash
git show v2.6.1
```

Delete a tag (if needed):
```bash
# Delete locally
git tag -d v2.6.1

# Delete from GitHub
git push origin :refs/tags/v2.6.1
```

**Note:** Deleting a tag from GitHub will not automatically delete the associated release. You'll need to delete the release manually from the GitHub web interface if desired.

### Workflow Location

The automated build workflow is defined in `.github/workflows/release.yml`. The workflow:

- Uses GitHub's hosted runners (no setup required)
- Builds with CMake Release configuration
- Includes full git history for version information
- Sets proper executable permissions on Unix binaries
- Names binaries with the version tag automatically

## Getting Help

For CMake-specific help:
```bash
cmake --help
cmake --help-full
```

For build issues, please check:
1. CMake version: `cmake --version` (must be 3.15+)
2. Compiler availability: `gcc --version` or `clang --version`
3. Build output for specific error messages
