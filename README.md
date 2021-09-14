# Une

[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/thechnet/une)

## What is Une?

Une */juːn/* is an **unfinished, unpolished, and *very* simple** interpreted programming language written in C as part of my Gymnasium Matura paper.

## What can Une do?

Une supports most basic language capabilities one expects from an interpreted programming language, but it doesn't go much further than that.
Some demo programs can be found in [examples](examples).

## How do I run Une?

### 1. Building the executable

#### Windows

> A video guide for the Windows installation can be found [here](https://www.youtube.com/watch?v=Irjglwouq7s).

- Download and install [MSYS2](https://www.msys2.org/).
- From the **MSYS2 terminal**, install GCC using `pacman -S gcc`.
- Add `MSYS2\usr\bin` to your `PATH` environment variable, **where `MSYS2` is your MSYS2 *installation directory***.

- Download and extract this repository.
- Open a Command Prompt **inside the innermost "une-main" directory**.
- Build Une using `sh make.sh`.

#### macOS

> A video guide for the macOS installation can be found [here](https://www.youtube.com/watch?v=Hm5mQRtN44w).

> ❕ Depending on your environment, some parts of this installation may produce errors. If you encounter an error, follow the instructions given in the error message and retry the step that failed.

- Install [Homebrew](https://brew.sh/).
- In Terminal, install GCC using `brew install gcc`.
- Create a symbolic link to GCC using the following commands:
  1. `cd /usr/local/bin`
  2. `sudo rm gcc`
  3. `ln -s gcc-11 gcc` *(Ensure the `-11` suffix matches your GCC major version.)*
- Download this repository, unzipping it if necessary.
- Open Terminal **inside the innermost "une-main" directory**.
- Build Une using `./make.sh`.
  > If this fails, ensure the build script is executable using `chmod 700 make.sh`, and repeat.

### 2. Running Une

Running Une without any arguments will give you its usage:

```
$ une
une {<script>|-s <string>}
```
> Note: On macOS, write `./une` instead of just `une`.

Run the "arithmetic_interpreter" example. It should print "7".

```
$ une examples/arithmetic_interpreter.une
7
```

Directly pass commands to Une using the `-s` flag:

```
$ une -s "print(\"Hello, Une\")"
Hello, Une
```

---

## Visual Studio Code Language Support

If you're using Visual Studio Code, make sure to install [the Une extension](https://marketplace.visualstudio.com/items?itemName=chnet.une) to get syntax highlighting and general language support. The repository for the extension can be found [here](https://github.com/thechnet/une-vscode).

---

<img src="res/icon.png" width=10%>
