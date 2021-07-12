# Une

[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/thechnet/une)

## What is Une?

Une is an **unfinished, unpolished, and *very* simple** interpreted programming language written in pure C as part of my Gymnasium Matura paper.

## What can Une do?

Une supports most basic language capabilities one expects from an interpreted programming language, but it doesn't go much further than that.
Some demo programs can be found in [examples](examples).

## How do I run Une?

1. Build the executable:

- Download this repository.
- Edit either `make.cmd` or `make.sh` (depending on which system you're on) and make sure `compiler` points to an up-to-date C toolchain (I recommend GCC or Clang).
- Run `make.cmd` or `make.sh` from within the repository.

2. Run a program:

```
une {<script>|-s <string>}
```

## Visual Studio Code Language Support

If you're using Visual Studio Code, make sure to install [the Une extension](https://marketplace.visualstudio.com/items?itemName=chnet.une) to get syntax highlighting and general language support. The repository for the extension can be found [here](https://github.com/thechnet/une-vscode).

---

<img src="res/icon.png" width=10%>
