# Une

Une (as in *une*-iverse) is a simple interpreted programming language inspired by C, JavaScript, and Python.

It was originally created as part of my 2021 Matura paper on creating a programming language, but has since turned into a personal playground for experimenting with software design and development tools.

Pre-built binaries can be found under [releases](https://github.com/thechnet/une/releases).

## Building

- Install [Clang](https://clang.llvm.org), [CMake](https://cmake.org), and a build system of your choice (for example, *make*).
- For the release version:
  - Create a "release" directory in the repository.
  - Inside this directory, run `cmake .. -G <gen>` (where `<gen>` is the generator for your build system).
- For the debug version:
  - Create a "debug" directory in the repository.
  - Inside this directory, run the same command as above but append `-DCMAKE_BUILD_TYPE=Debug`.
- In the created directory, compile the binary using your build system.

To run the test suite:

- Install [Python 3](https://www.python.org).
- Build the debug version.
- Run `test.py` from within the "testing" directory.

## Getting Started

Run Une without any arguments to get its usage:

- Run a script by passing a filename.
- Provide a script as a string using `-s`.
- Enter the interactive console using `-i`.

From there, check out some of the [examples](https://github.com/thechnet/une/tree/main/examples) or refer to the [documentation](DOCUMENTATION.md).

## Language Support

Visual Studio Code provides basic language support, including syntax highlighting, via the [Une extension](https://marketplace.visualstudio.com/items?itemName=chnet.une).

<br/>

<img src="res/icon.png" width=5%>
