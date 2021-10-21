# Une

[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/thechnet/une)

## What is Une?

Une */ˈjun/* is an **unfinished, unpolished, and *very* simple** interpreted programming language written in C as part of my Gymnasium Matura paper.

## What can Une do?

Une supports most basic language capabilities one expects from an interpreted programming language, but it doesn't go much further than that.
Some demo programs can be found in [examples](examples).

## How do I run Une?

### 1. Building the executable

<details open>
<summary><b>Windows</b></summary>

- Download the latest prebuilt [LLVM-MinGW toolchain by GitHub user *mstorsjo*](https://github.com/mstorsjo/llvm-mingw/releases). The recommended configurations are `msvcrt-x86_64` for 64-bit, and `msvcrt-i686` for 32-bit systems.
- Extract the directory contained in the download, and move it to a permanent location (for example into the program files).
- Add the toolchain to your `PATH` environment variable:
  - Inside the directory, <kbd>Shift</kbd>-right-click on the `bin` folder and choose *Copy as path*.
  - Search Windows for "Edit the system environment variables", and open the first result.
  - Choose *Environment Variables…*.
  - Ensure that under *User variables*, "Path" is selected.
  - Choose *Edit* → *New*, and insert the copied path using <kbd>Ctrl</kbd>+<kbd>V</kbd>.
  - Choose *OK* 3x.
- Download and extract this repository.
- Open a *Command Prompt* instance inside the downloaded repository.  
  An easy way to do this is to navigate inside the repository, click on the adress bar, type `cmd`, and press <kbd>Enter</kbd>.
- Build `une.exe` using `clang @win`.  
  To build the debug version, use `clang @win-dbg`.


</details>

<details open>
<summary><b>macOS</b></summary>

- Download and extract this repository.
- Open a Terminal instance inside the repository.  
  An easy way to do this is to select the repository in *Finder*, and – in the menu bar – choose *Finder* → *Services* → *New Terminal at Folder*
- In *Terminal*, type `clang` and press <kbd>Enter</kbd>.
- In the dialog that appears, choose *Install* → *Agree*.
- Once the installation is complete, choose *Done*.
- Build `une` using `clang @unix`.  
  To build the debug version, use `clang @unix-dbg`.

</details>

### 2. Running Une

Running Une without any arguments will give you its usage:

```
> une
Usage: une {<script>|-s <string>}
```
> Note: On macOS, write `./une` instead of `une`.

Run the `arithmetic_interpreter.une` example. It should print `7`:

```
> une examples/arithmetic_interpreter.une
7
```

Directly pass commands to Une using the `-s` flag:

```
> une -s "print(\"Hello, Une\")"
Hello, Une
```

---

## Visual Studio Code Language Support

If you're using Visual Studio Code, make sure to install [the Une extension](https://marketplace.visualstudio.com/items?itemName=chnet.une) to get syntax highlighting and general language support. The repository for the extension can be found [here](https://github.com/thechnet/une-vscode).

---

<img src="res/icon.png" width=10%>
