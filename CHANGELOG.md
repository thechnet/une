# Changelog

## Unreleased

### Fixed
- Multiplying a string or list with a negative number causes a crash.
- Functions do not persist in the CLI.

## [0.9.0]

### Added
- New built-in function `append()`.  
  Exactly the same as `write()` but does not discard the content previously found in the target file.
- New `exit` statement to immediately exit the running script.  
  Similar to the return statement, the exit statement also supports an optional exit code; however, since this code is returned to the operating system, it **must** be an integer. If the exit code is omitted, it defaults to `0`.
- To implement the exit statement, the interpreter state was given a new flag `should_exit` to signal when the script should exit. The state of this flag can be retrieved after the execution through a new optional parameter in the `une_run` function.
- New test cases for:
  - Return statement without return value.
  - New `exit` statement.
  - Changed `script()` behavior.
  - New `append()` command.

### Changed
- The built-in `script` command now executes in the same context as the host script, allowing the caller and callee to exchange information.
- Updated README.
- Updated documentation to include the exit statement (6.3).
- The command line interface now shows version information.

### Fixed
- Return statement without return value has no effect.

## [0.8.0] - 2022-05-29

### Added
- Added CLI.  
  To access the interface, use `une -c`.  
  To exit a session, use <kbd>CTRL</kbd>+<kbd>C</kbd>.
- Test case to check that the last evaluated result is implicitly returned when exiting a script.

### Changed
- The interpreter now implicitly returns the final evaluated result when exiting a script.  
  This is useful for implementing a CLI, as it allows the user to quickly check the contents of variables or evaluate expressions without `print`.
- `une_run` now accepts an optional external context. When this context is supplied, the function uses *it* for the interpretation instead of creating a new context.

## [0.7.3] - 2021-11-22

### Added
- Test case for latest bugfix.

### Fixed
- Fixed stale pointer in *for* loop implementation causing nested loops to break.

## [0.7.2] - 2021-10-24

### Fixed
- Fixed built-in functions not verifying the number of arguments supplied to them.
- Fixed crash when checking equality of function, built-in function, or 'Void' against any value.

## [0.7.1] - 2021-10-21

### Added
- `error.une` example to demonstrate error messages.

### Fixed
- Fixed an early deallocation of the definition file in functions, before they were used for the traceback.

## [0.7.0] - 2021-10-19

### Added
- Build argument lists.  
  To simplify the build process, this commit introduces the "win", "unix", "win-dbg", and "unix-dbg" build configurations, which refer to argument lists in res/build.  
  To build Une using one of these configurations, use `<cc> @<file_name>`.
- Test case for *Changed · 3*.

### Changed
- Changed error message formatting.
- Refactored test.py to improve robustness.
- Calling an index of a list no longer requires enclosing the list access in parentheses.

### Fixed
- test.py: Not appending ".exe" to executable path.
- Warnings when compiling with Clang on Windows.

### Removed
- make.sh is no longer the preferred build method. Refer to *Added · 1*.

## [0.6.1] - 2021-09-28

### Added
- New test cases.
- Clang support.  
  - `make.sh` now suppresses warnings for zero variadic macro arguments.
  - On Windows:
    - `une_file_exists` uses `GetFileAttributesA()` to ensure the path is not a directory.
    - Sleeping for a given amount of miliseconds is handled via `Sleep()`.
- `object_orientation.une` example.

### Changed
- Renamed `src/builtin.c` and `builtin.h` to `builtin_functions.c` and `builtin_functions.h` to avoid confusion with `src/datatypes/builtin.c` and `builtin.h`.
- Some other small changes.
- Since the introduction of memdbg, allocations were left unchecked in non-debug mode. This has now been adressed with a new macro `verify()`, which, if the pointer provided to it is `NULL`, prints an error message and aborts the program.
- On Windows builds, memdbg no longer prints signal messages.
- Functions are no longer scoped per context, instead they are all part of a main function buffer in the interpreter state.  
  This allows users to, for example, return new functions as return value of other functions.

### Fixed
- Spelling mistake in usage message.
- `make.sh` does not append ".exe" to the output binary.
- Some warnings when compiling with `-Wextra`.
- Negations are parsed *after* powers, indices, and calls, creating an unintuitive order of operations, where, for example, `-2**2` is evaluated to `4` when one would expect to get `-4`. To fix this, negations are now parsed *before* each of these structures.
- When copying a function node, the program crashes because it tries to duplicate a string as if it was a node.

## [0.6.0] - 2021-08-13

### Added
- Warning when `UNE_DEBUG_MEMDBG` is enabled.
- Option to skip ahead in `testing/test.py`.

### Changed
- A statement immediately following a closing brace is no longer considered valid syntax.
- Some refactoring.
- Functions and built-in functions are now datatypes.  
  The fundamental change in this is that a call is now a seperate operation, performed *on* a datatype. When not called, a function or built-in function is just *data*, which can represent itself or be checked for truth (both types are *always* true).

  A literal representation of a "square" function might look as follows:  
  `function(number) { return number**2 }`  
  To "define" a function, we simply bind it to a variable...  
  `square = function(number) { return number**2 }`  
  ... which can then be referenced to call the function:  
  `square(4)` (would return `16`)

  Internally, this changes a bunch of things:
  - The lexer now differentiates between built-in functions and variables, creating a new `UNE_TT_BUILTIN` token holding a `une_builtin_function` when encountering the name of a built-in function.
  - The parser creates two new nodes, `UNE_NT_FUNCTION` or `UNE_NT_BUILTIN`. For user-defined functions, the node closely resembles `UNE_NT_DEF`. For built-in functions, the node simply holds the `une_builtin_function` value from the token.
  - The interpreter now has seperate methods for interpreting a function or built-in function. They return a `une_result`:
    - For functions, the result holds a pointer to the function in the function buffer of the context. To allow this, the function buffers no longer hold actual `une_function`s but instead pointers to standalone allocations.
    - For built-in functions, the result holds a `une_builtin_function`, which can be used to access the function pointer, get the amount of required parameters, and more.

  One problem this change created is that functions are no longer named. Therefore, when creating the traceback on an error, we instead use the file and line where the function was defined. To access this information, every `une_context` now holds a pointer to the `une_function` that created the context, which in turn holds information on its point of definition.
- Updated examples.

### Removed
- Old function implementation.

## [0.5.9] - 2021-08-08

### Changed
- The help message on invalid command line arguments now shows an environment-specific path to the executable by using the implicit zeroth path argument.
- Built-in functions no longer each have a custom declaration.
- Data result types are now organized in `src/datatypes`.  
  - Each data type defines a number of routines in its corresponding `.c` file, with all types being united in `datatypes.c`.
  - When applying an operation, the required routine is found through the `une_datatypes` lookup table.

### Fixed
- The error line is always `1`.
- The position of an expression in parentheses does not include the surrounding parentheses.
- If there are non-printable but legal characters on the line of an error, the underscore in the error display gets misaligned.
- `ostream` buffers do not grow if assertions are disabled.
- The creation of `UNE_TT_INT` and `UNE_TT_FLT` tokens does not work if assertions are disabled.
- Missing error on conversion of integer into invalid character.

### Removed
- `LOGFREE()` macros.
- Data result type helpers from `result.c` – See *Changed*.

## [0.5.8] - 2021-07-30

### Added
- Warnings when running with lexing, parsing, or interpreting disabled.
- Test case for redefining a built-in function.

### Changed
- Main error message now shows the usage instead of an error message.
- Changed the default context name from "\<stdin\>" to "\<args\>".
- `memdbg.c` is no longer linked if not building in debug mode.
- Minor refactoring.

### Fixed
- Undefined errors when running with lexing, parsing, or interpreting disabled.
- Segmentation fault when freeing a `NULL` pointer while memdbg is enabled.

## [0.5.7] - 2021-07-21

### Changed
- Various small things.

### Fixed
- Missing error on redefinition of built-in function.

### Removed
- Removed `make.cmd`.

## [0.5.6] - 2021-07-17

### Added
- memdbg: Added signal handling, `fopen()` and `fclose()` wrappers, and other general improvements.
- Added debug report – If enabled, prints the state of Une into `une_report_return.txt` and `une_report_status.txt` after successful execution.

### Changed
- Some refactoring.

### Fixed
- `int("str")` does not work.
- `GET_IDX` on a non-indexable type defines an error but does not return it.
- Lexer treats directories like empty files.

## [0.5.5] - 2021-07-15

### Added
- tools: Added `une_file_exists()` and `une_file_or_folder_exists()`.

### Changed
- tools: Reworked implementations of `une_wcs_to_une_int()` and `une_wcs_to_une_flt()`.
- Other minor changes.

### Fixed
- Some built-in commands don't check if a path is a directory instead of a file.
- Aborting without setting error on unclosed index during `SET_IDX` parsing.

## [0.5.4] - 2021-07-15

### Added
- Invalid input to `main.c` now displays an error.
- Added debug option to define (almost) all sizes as `1`.
- Added debug option to return the error type as integer.

### Changed
- Promoted `UNE_RT_VOID` to a data result type. `VOID` is always falsy and never equal to itself.
- Further simplified errors:
  - Replaced `UNE_ET_UNEXPECTED_CHARACTER` with `UNE_ET_SYNTAX`.
  - Combined `UNE_ET_VARIABLE_NOT_DEFINED` and `UNE_ET_FUNCTION_NOT_DEFINED` into `UNE_ET_SYMBOL_NOT_DEFINED`.
  - Renamed `UNE_ET_FUNCTION_ARGC` to `UNE_ET_FUNCTION_ARG_COUNT`.
  - Tweaked wording of error descriptions.
- Renamed `float()` to `flt()` to align with internal naming.
- Made `une_node_to_wcs()` and `une_token_to_wcs()` a little bit safer for testing.
- Reworked `une_str_to_wcs()` implementation.
- Other small refactorings and changes.

### Fixed
- Some functions using `une_ostream`s hold on to stale pointers.
  (`une_lex()`, `une_error_display()`, `une_builtin_split()`)
- `une_context_free()` frees all parents.
  This behavior is now achieved through `une_context_free_children()`.
- `une_result_list_mul()` allocates too much memory for the new lists.
- `input()` does not allocate enough memory for the input string.
- `UNE_NT_COP` does not include the condition in its position.

### Removed
- Removed `UNE_D` macro because it caused semantic highlighting issues.

## [0.5.3] - 2021-07-12

### Added
- Added simple traceback to error.
- Added `escseq.h` – Macros for ANSI escape sequences.
- Minor changes to codebase to fully conform to C standard.
- `une_node_to_wcs()` for `UNE_NT_SET` and `UNE_NT_SET_IDX` now shows whether the assigments are `global` or not.
- memdbg: Added manual `ARR(array, index)` array checker.
- Escape sequences are now globally toggleable from `escseq.h`.

### Fixed
- Fixed a bug in `stream.c` where `une_istream_wfile_reset()` would not reset the streams last character, causing subsequent calls to `pull()` not to increment the stream index.

## [0.5.2] - 2021-07-09

### Changed
- memdbg now helps prevent buffer overflows by adding padding behind each memory allocation.
  All allocations' paddings are tested whenever a memdbg wrapper function is called.
  Wrapper functions for other allocators and/or memory-modifying functions can be created in `memdbg.c`.
- Other minor changes and refactorings.

### Fixed
- Fixed a bug where certain lexer errors at the end of the input caused a crash.

### Removed
- Removed `une_strdup()` and `une_wcsdup()` as these functions are now wrapped in memdbg.

## [0.5.1] - 2021-07-08

### Added
- memdbg – A debugging module that keeps track of memory allocations. (Replaces `alloc_counter`.)
- logging – A debugging module to simply create verbose log messages. (Replaces `LOGX` and `ERR`.)

### Fixed
- Fixed `__une_static`.

### Removed
- Removed `une_malloc()`, `une_realloc()`, and `une_free()` – See memdbg.

## [0.5.0] - 2021-07-05

### Added
- If passed "`-s`" as first argument, Une will now interpret the string following it.
- Added `put()` – This function behaves how `print()` behaved previously.

### Changed
- The ternary operation syntax has been changed to `PREDICATE?CONSEQUENT:ALTERNATE`.
- `print()` now automatically appends a newline to the end of the output.
- Array input streams now require a parameter `__cast_type` which holds the dereferenced type the `void*` should be casted to.
  This was implemented to address a problem where a `void*` casted to a `wint_t*` returns values unable to be compared to character literals.
- Other minor changes.

## [0.4.0] - 2021-07-05

### Added
- Added `script(path)` built-in command – Execute external Une scripts.
- Added `exist(path)` built-in command – Check if a file or folder exists.
- Added `split(str, delims)` – Using the provided delimiters, split a string into a list of substrings.

### Changed
- Changed variable definition scopes: By default, all variable definitions now only affect the current context. To modify variables from parent contexts, a new "`global VAR=EXPR`" syntax has been introduced.
- Changed `UNE_TT_NOT`, `UNE_TT_AND`, and `UNE_TT_OR` from "`not`", "`and`", and "`or`" to "`!`", "`&&`", and "`||`" respectively.
- Replaced `UNE_ET_OPERATION_NOT_SUPPORTED`, `UNE_ET_COMPARISON_NOT_SUPPORTED`, `UNE_ET_INDEX_CANNOT_GET`, `UNE_ET_INDEX_CANNOT_SET`, `UNE_ET_INDEX_TYPE_NOT_SUPPORTED`, and `UNE_ET_FUNCTION_UNEXPECTED_ARG_TYPE` with `UNE_ET_TYPE`.
- Renamed `wcs_to_une_int()`, `wcs_to_une_flt()`, `wcs_to_str()`, `str_to_wcs()`, `str_dup()`, and `wcs_dup()` to `une_wcs_to_une_int()`, `une_wcs_to_une_flt()`, `une_str_to_wcs()`, `une_wcs_to_str()`, `une_strdup()`, and `une_wcsdup()` respectively.

### Fixed
- Various.

## [0.3.0] - 2021-06-26

### Added
- New built-in functions:
  - `chr(int)` – Convert an integer to a character.
  - `ord(char)` – Convert a character to an integer.
  - `read(path)` – Read from a file.
  - `write(path, str)` – Write to a file.
  - `input(str)` – Get text from stdin.

### Fixed
- Fixed a problem with `streams.h`.
- Added more specific size defines.

## [0.2.1] - 2021-06-13

### Added
- Added `streams.c/h`: These files contain tools to simplify interfacing with arrays or files and to prevent common bugs. They are not yet used everywhere and might still need more work.
- All macros and functions now have a description above their definition.
- Added `une_free()` as a wrapper to `free()`.

### Changed
- All-around cosmetic refactoring.
- Some frequently used methods have been defined as functions or macros to avoid rewriting code.
- Errors have been significantly simplified: In most cases, detailed errors are not needed to identify the problem and only clutter the source.
- Tokens are now sometimes created using `une_token_create()`.
- The lexer has been split up into subroutines.
- Multiple places in the code now use lookup tables, which are easier to maintain.
- Some parts of the code have previously already used enum "regions" to describe a range of allowed values for a function. These regions are now defined right in the enums as `UNE_R_x` defines, as an attempt to minimize the risk of corrupting regions on accident when reordering enum values.
- Replaced `une_result_to_wcs()` with `une_result_represent()`. This change has yet to be carried over into other `x_to_wcs()` functions.
- Renamed `rmalloc()` to `une_malloc()`.
- Renamed `rrealloc()` to `une_realloc()`.
- Renamed `UNE_STATIC` to `__une_static`.
- Renamed `UNE_BIF_NONE` to `__UNE_BIF_none__`.
- Renamed `malloc_counter` to `une_alloc_count`.
- *Many* other small changes.

### Removed
- Removed `file_read()`, as it is no longer used.

## [0.2.0] - 2021-06-04

### Changed
- This release reverts the additions of [0.1.4]. As such, the codebase now follows the following principles:
  - There is no top-level "omni" data structure.
  - The `une_x_state` structs are only used to transport commonly used data (this does **not** include `une_error`).

### Added
- Added `sleep()` built-in command – Pauses execution for a given amount of miliseconds.

## [0.1.5] - 2021-05-24

### Added
- Added `une_lexer_x()` functions to enable Changed · 1.
- Added `une_lex_x()` functions to break up `une_lex()`.

### Changed
- `une_lex()` (formerly `une_lex_wcs()`) can now lex both strings and files.

### Removed
- Removed `une_lex_file()`.

## [0.1.4] - 2021-05-22

### Added
- This release introduces `une_instance`, a new top-level data structure. It holds the following data:
  - `une_error`
  - `une_lexer_state`: A new struct that holds information for the lexer.
  - `une_parser_state`: A new struct that holds information for the parser.
  - `une_interpreter_state`: A new struct that holds information for the interpreter, among it the redefined `une_context`, which now *only* holds contextual information for the interpretation. In many functions, a `une_instance*` replaces multiple parameters.

### Changed
- Parser now uses a group of `une_p_x()` functions to simulate a token stream. I'm unsure if I like this approach, so I might remove it again in a later commit.
- Deprecated `une_lex_file()` – An upcoming, refactored lexer will reintroduce this functionality.
- Added `une_error_create()`.
- Cosmetic changes.

### Fixed
- Various.

## [0.1.3] - 2021-05-21

### Changes
- All `x_type_to_wcs()` functions are now based on lookup tables, defined in the corresponding `x.c` files as `une_x_table`.
- Identifiers are no longer interpreted and `une_interpret()` no longer contains a case for `UNE_NT_ID`.
- Restructured some `x_to_wcs()` and `x_free()` functions.

### Fixed
- Various.

## [0.1.2] - 2021-05-11

### Changed
- Most debugging functionality is no longer compiled if `UNE_DEBUG` is not defined.

### Fixed
- `une_lex_wcs()` doesn't free the tokens it has already created on an error.
- `une_tokens_free()` now calls a new function `une_token_free()` for every token.

## [0.1.1] - 2021-05-03

### Changed
- Refactoring  
  This release continues the ongoing refactoring efforts. As of this commit, all of `parser.c` has been refactored.  
  Notable changes are:
  - Binary operations like addition, multiplication, etc. are now handled by `une_parse_binary_operation()`.
  - Unary operations like negation are now handled by `une_parse_unary_operation()`.
  - `une_parse_sequence()` now also handles consumption of the enclosing tokens in a sequence, and instead of returning a `void**` to a `une_node*` array, it returns a proper `une_node*` like every other parse function.
  - `une_parse_block()` has been merged into `une_parse_stmt()`.
  - Removed `UNE_DEBUG_SOFT_ERROR`.
  - Some expression-related functions have been renamed. The top level function for expressions is now always called `une_parse_expression()`.
  - All parse functions are now static. The public interface for the parser is `une_parse()`.
  - Enums now have a `__none__` and `__max__` value to determine their size and to represent a missing value.

## [0.1.0] - 2021-04-29

### Added
- Built-in functions  
  This release adds basic support for built-in functions. Both the support for these functions and the functions themselves require some more work in the future.

### Changed
- Refactoring  
  This release also continues the ongoing code refactoring efforts. As of this commit, all of `interpreter.c` has been refactored.

## [0.0.5] - 2021-04-28

### Changed
- Refactoring.

## [0.0.4] - 2021-04-27

### Fixed
- Various.

## [0.0.3] - 2021-04-25

### Added
- New logo.

### Changed
- Refactoring, style changes.

### Fixed
- Various.

## [0.0.2] - 2021-04-22

### Changed
- Seperated source into multiple files.

## [0.0.1] - 2021-04-17

### Added
- Initial release, based on [0.0.0-c.2.1].
- Une logo.

### Changed
- Changed name from "CMDRScript" to "Une".

## [0.0.0-c.2.1] - 2021-04-16

### Added
- Interpretation.

## [0.0.0-c.2] - 2021-04-06

### Added
- Second C test. Builds on [0.0.0-c.1.2], adding the structure for interpreting the parsed program.

## [0.0.0-c.1.2] - 2021-04-04

### Added
- Memory management.

### Changed
- Refactoring.

### Fixed
- Various.

## [0.0.0-c.1.1] - 2021-04-02

### Added
- `while` loops.
- `if` statements.
- Function definitions.
- Negation.

## [0.0.0-c.1] - 2021-04-01

### Added
- First C parser test, including most of the lexing and parsing features the latest Python test has, but missing the interpreter.

## [0.0.0-python.6.3] - 2021-03-02

### Added
- Token positions.

## [0.0.0-python.6.2] - 2021-02-25

### Added
- '`^`' (exponent) operator.
- Comments.
- `AND`, `OR`.
- `for` loops.
- Various other things.

## [0.0.0-python.6.1] - 2021-02-21

### Added
- `if`, `elif`, `else` statements (including `NOT`, `EQU`, `NEQ`, `GTR`, `LSS`, `GEQ`, `LEQ` – still missing is `AND`, `OR`).
- `while` loops (including `BREAK`, `CONTINUE`).
- functions (including context switching, `RETURN`).

## [0.0.0-python.6] - 2021-02-20

### Added
- Sixth Python test. This is a rebuild of [0.0.0-python.5], focusing less on Python-specific language features. It also implements variable assigments and references, strings, some string operations, the seperation of commands using '`;`', and multiline input.

## [0.0.0-python.5] - 2021-02-20

### Added
- Fifth Python test. This test completely abandons the structure of the previous tests and instead implements the concept of an abstract syntax tree. The most notable difference between this approach and the ones tried previously is that creating tokens, combining tokens to create statements, and verifying the syntax is done in seperate steps, while the previous tests tried to do all of these things at the same time.  
  This test is also the first to go beyond lexical analysis, adding a working parser and interpreter. The result is a simple arithmetic interpreter.

## [0.0.0-python.4] - 2021-02-19

### Added
- Fourth Python lexer test. Expands on previous tests, tries to identify parts of instructions by referencing knowledge gathered from previous characters.

## [0.0.0-python.3] - 2021-02-19

### Added
- Third Python lexer test. This test is similar to the first test, but instead of using built-in Python methods to categorize characters, we define our own sets and test if the character is contained in one of them.

## [0.0.0-python.2] - 2021-02-16

### Added
- Second, abandonded Python lexer test. The idea with this approach is that the first word of every statement describes what kind of instruction follows.

## [0.0.0-python.1] - 2021-02-13

### Added
- First Python lexer test. Its structure has similarities to a state machine:  
  The lexer keeps track of what type of token it is currently lexing. It decides what to do with the character based on the current token type and the type of the character. It handles one character per loop.

<!-- Unreleased -->
[Unreleased]: https://github.com/thechnet/une/compare/v0.9.0...HEAD

<!-- Releases -->
[0.9.0]: https://github.com/thechnet/une/compare/v0.8.0...v0.9.0
[0.8.0]: https://github.com/thechnet/une/compare/v0.7.3...v0.8.0
[0.7.3]: https://github.com/thechnet/une/compare/v0.7.2...v0.7.3
[0.7.2]: https://github.com/thechnet/une/compare/v0.7.1...v0.7.2
[0.7.1]: https://github.com/thechnet/une/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/thechnet/une/compare/v0.6.1...v0.7.0
[0.6.1]: https://github.com/thechnet/une/compare/v0.6.0...v0.6.1
[0.6.0]: https://github.com/thechnet/une/compare/v0.5.9...v0.6.0
[0.5.9]: https://github.com/thechnet/une/compare/v0.5.8...v0.5.9
[0.5.8]: https://github.com/thechnet/une/compare/v0.5.7...v0.5.8
[0.5.7]: https://github.com/thechnet/une/compare/v0.5.6...v0.5.7
[0.5.6]: https://github.com/thechnet/une/compare/v0.5.5...v0.5.6
[0.5.5]: https://github.com/thechnet/une/compare/v0.5.4...v0.5.5
[0.5.4]: https://github.com/thechnet/une/compare/v0.5.3...v0.5.4
[0.5.3]: https://github.com/thechnet/une/compare/v0.5.2...v0.5.3
[0.5.2]: https://github.com/thechnet/une/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/thechnet/une/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/thechnet/une/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/thechnet/une/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/thechnet/une/compare/v0.2.1...v0.3.0
[0.2.1]: https://github.com/thechnet/une/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/thechnet/une/compare/v0.1.5...v0.2.0
[0.1.5]: https://github.com/thechnet/une/compare/v0.1.4...v0.1.5
[0.1.4]: https://github.com/thechnet/une/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/thechnet/une/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/thechnet/une/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/thechnet/une/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/thechnet/une/compare/v0.0.5...v0.1.0
[0.0.5]: https://github.com/thechnet/une/compare/v0.0.4...v0.0.5
[0.0.4]: https://github.com/thechnet/une/compare/v0.0.3...v0.0.4
[0.0.3]: https://github.com/thechnet/une/compare/v0.0.2...v0.0.3
[0.0.2]: https://github.com/thechnet/une/compare/v0.0.1...v0.0.2
[0.0.1]: https://github.com/thechnet/une/tree/v0.0.1

<!-- C Tests -->
[0.0.0-c.2.1]: https://github.com/thechnet/une/compare/v0.0.0-c.2...v0.0.0-c.2.1
[0.0.0-c.2]: https://github.com/thechnet/une/compare/v0.0.0-c.1.2...v0.0.0-c.2
[0.0.0-c.1.2]: https://github.com/thechnet/une/compare/v0.0.0-c.1.1...v0.0.0-c.1.2
[0.0.0-c.1.1]: https://github.com/thechnet/une/compare/v0.0.0-c.1...v0.0.0-c.1.1
[0.0.0-c.1]: https://github.com/thechnet/une/tree/v0.0.0-c.1

<!-- Python Tests -->
[0.0.0-python.6.3]: https://github.com/thechnet/une/compare/v0.0.0-python.6.2...v0.0.0-python.6.3
[0.0.0-python.6.2]: https://github.com/thechnet/une/compare/v0.0.0-python.6.1...v0.0.0-python.6.2
[0.0.0-python.6.1]: https://github.com/thechnet/une/compare/v0.0.0-python.6...v0.0.0-python.6.1
[0.0.0-python.6]: https://github.com/thechnet/une/tree/v0.0.0-python.6
[0.0.0-python.5]: https://github.com/thechnet/une/tree/v0.0.0-python.5
[0.0.0-python.4]: https://github.com/thechnet/une/tree/v0.0.0-python.4
[0.0.0-python.3]: https://github.com/thechnet/une/tree/v0.0.0-python.3
[0.0.0-python.2]: https://github.com/thechnet/une/tree/v0.0.0-python.2
[0.0.0-python.1]: https://github.com/thechnet/une/tree/v0.0.0-python.1
