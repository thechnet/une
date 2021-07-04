<img src="res/banner.png" width=40%>

# What is Une?

Une is an **unfinished, unpolished, and *very* simple** interpreted programming language written in pure C as part of my Gymnasium Matura paper.

# What can Une do?

Une supports most basic language capabilities one expects from an interpreted programming language, but it doesn't go much further than that.
Some demo programs can be found in [examples](examples).

# How do I run Une?

1. Build the executable:

- Download this repository.
- Edit either `make.cmd` or `make.sh` (depending on which system you're on) and make sure `compiler` points to an up-to-date C compiler (I recommend GCC or Clang).
- Run `make.cmd` or `make.sh` from within the repository.

2. Run a script:

```
./une <script>
```