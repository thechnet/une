# The Une Programming Language

## 1. General Concepts

Statements occupy an entire line, and are concluded by a newline at their end:

```
statement
```

To combine multiple statements onto one line, join them using a semicolon (`;`):

```
statement; statement
```

Comments can be placed at the end of a line using a hashtag symbol (`#`), followed by the comment. Comments span the entire rest of the line, and can therefore not be inserted between tokens.

```
# Comment
statement # Comment
```

Multiple statements can be grouped into a block by surrounding them with curly brackets (`{`, `}`):

```
{
	statement
	statement
}
```

If a keyword requires being followed by *statement(s)*, you can immediately place either a single statement or a block after it:

```
keyword statement
```
```
keyword {
	statement
	statement
}
```

Finally, whitespace, as in spaces or newlines, is optional (except to conclude statements):

```
keyword{statement;statement}
```
```
keyword
{
	statement
	statement
}
```

## 2. Types

There are eight types of data.

### 2.1 Integers

For example `-46`, `0`, `346`.

For verbose booleans, use `True` and `False`. They are identical to `1` and `0`.

### 2.2 Floating-point numbers

For example `-4.6`, `0.0`, `3.46E2` (= `346.0`).

### 2.3 Strings

For example `"This is a string."`.

Strings can contain embedded expressions:

```
"2 * 2 = {2 * 2}" == "2 * 2 = 4"
```

The following characters must be escaped using a backslash (`\`) to be used inside strings:
- `\`
- `"`
- `{`
- `}`

Also, the following sequences have special meaning:
- `\n` – Insert a new line.
- `\r` – Insert a carriage return character.
- `\t` – Insert a tab character.
- `\e` – Inserts the *ESC* character.
- `\a` – Inserts the *BEL* character.
- `\oOO` – Insert the character with octal value `OO`.
- `\xXX` – Insert the character with hexadecimal value `XX`.
- `\` new line – Does not include the new line in the string.

For information on how to access or modify the characters making up a string, see 2.9.

### 2.4 Functions

For example:

```
(parameter1, parameter2) -> {
	statement1
	statement2
}
````

> If your function only accepts one parameter, you may also omit the parenthesis around it

Functions are called by appending the arguments in parentheses (`(`, `)`) after their value:

```
(parameter1, parameter2) -> {
	statement1
	statement2
}(argument1, argument2)
```

### 2.5 Native Functions

For example `print`.

These values are simply references to the underlying native functions, to allow passing them around.

To call a native function, use the same syntax as for user-defined functions:

```
print(argument)
```

### 2.6 Void

`Void`, used to communicate the absence of a value (see 6.2).

### 2.7 Lists

Lists can hold any number of any kind of data, including other lists. The items are separated using commas (`,`):

```
[46, 4.6, "string", () -> return, print, [46]]
```

For information on how to access or modify the elements making up a list, see 2.9.

### 2.8 Objects

Much like lists, objects can hold any number of any kind of data, including objects, with the difference being that objects hold *members*, accessed using names instead of indices:

```
person = {
	name: "Mia",
	age: Void
}
print(person.name) # Prints 'Mia'.
```

To add new members to objects, just assign them a value:

```
person.surname = "Wallace"
```

When a function is stored in an object member, that function becomes a *method* of the object. During a method call, the enclosing object is available via the `this` keyword.

```
person.set_age = (new_age) -> {
	this.age = new_age
}
person.set_age(24)
print(person.age) # Prints '24'.
```

### 2.9 Indices and slices

To access an element of a list or string, follow it by the offset of the item from the first item in square brackets (`[`, `]`):

```
[1, 2, 3][0] # Retrieves '1'
"string"[1] # Retrieves '"t"'.
```

To retrieve multiple elements (called a *slice*) at once, provide a range:

```
[1, 2, 3, 4, 5][1..4] # Retrieves '[2, 3, 4]'. The range end is excluded from the slice.
```

> You can also use `Void` as the range end to represent the *end* of the list or string.

You can also use negative indices to represent an offset from the end of the list or string:

```
"string"[1..-1] # Retrieves '"trin"'.
```

> Negative indices can also be used when retrieving a *single* element.

Finally, you can omit either boundary. If the range start is omitted, it defaults to `0`. If the range end is omitted, it defaults to `Void`.

```
"string"[3..] # Retrieves '"ing"'.
```

All of the above rules also apply when *assigning* to a list or string. As long as the right-hand value matches the pattern defined in the left-hand slice, the values will be modified.

```
a = [1, 2, 3, 4, 5]
a[2..4] = [23, 46] # 'a' is now '[1, 2, 23, 46, 5]'.
```

## 3. Conditional Expressions

### 3.1 Truthiness

A conditional expression always evaluates to either `0`, being *false*, or `1`, being *true*.

Attributes required for each type to be considered *true* are:

- Integers: not being of value `0`.
- Floating-point numbers: not being of value `0.0`.
- Strings: not being empty.
- Lists: containing at least one element.
- Objects: having at least one member.
- Functions: **always** *true*.
- Native functions: **always** *true*.
- Void: **never** *true*.

Attributes required for each type to be considered *false* are the opposites of the ones required for it to be *true*.

### 3.2 Negation

The truthiness of a value can be inverted by prefixing the value with an exclamation mark (`!`):

```
!0 # Evaluates to 'true'.
```

### 3.3 Comparisons

The following binary operators can be used between two values to compare them.

#### 3.3.1 Equality and Inequality

Types can be compared for equality using `==`. Each comparison evaluates to *true* if the type and the value are the same.

Exceptions are comparisons between integers and floating-point numbers with the same numerical value:

```
46.0==46 # Evaluates to 'true'.
```

The same types can be compared for *in*equality using `!=`. Each comparison evaluates to *true* if the requirements for equality are not (exactly) met.

#### 3.3.2 Size

Integers, floating-point numbers, strings, and lists can be compared in terms of size using `>`, `>=`, `<=`, and `<`.

For a value *A* to be considered greater than a value *B* of the same type, the following requirements must be met:

- Integers and floating-point numbers: the numerical value of *A* must be greater than that of value *B*.
- Strings: the number of characters in *A* must be larger than the number of characters in *B*.
- Lists: the number of items in *A* must be larger than the number of items in *B*.

#### 3.3.3 Chaining

Conditions can be chained together.

Using `&&`, the expression only evaluates to *true* if both conditions are *true*.

```
1 && 1 # 'true'
0 && 1 # 'false'
```

Using `||`, the expression evaluates to the left-hand side if the left condition is *true*; otherwise, it evaluates to the right-hand side.

```
1 || 0 # 'true'
0 || 0 # 'false'
```

Using `??`, the expression evaluates to the left-hand side if the left condition is not `Void`; otherwise, it evaluates to the right-hand side.

```
1 ?? 2 # 1
print("Something") ?? 2 # 2
```

#### 3.3.4 Iterable Comparison

Sometimes we want to perform a comparison on all elements within an iterable. This is achieved using the `any` and `all` operators.

When placed in front of iterable values in comparisons, the comparison is performed once per element in the iterable. Using `any`, the expression yields true if the comparison applies to *any* element in the iterable.

```
"foo" == any ["foo", "bar"]
```

Using `all`, it yields true if the comparison applies to *all* elements in the iterable.

```
3 >= all [1, 2, 3]
```

Operators can be used on both sides to perform complex checks in a single expression:

```
# Check if any input is not an accepted value
inputs = [1, 2, 3]
accepted = [2, 3, 4]
any inputs != all accepted
```

## 4. Operations

All common arithmetic operations (addition, subtraction, multiplication, division, modulus, power) are supported. The operator for modulus is `%`, the operator for power is `**`. An additional operation, floor division, behaves like division but always rounds the result down to the next smallest integer. Its operator is `//`.

For integers and floating-point numbers, the operations behave as expected. Some of them can, however, also be used for strings and lists: to combine two strings or lists into one, use the addition operator:

```
"foo" + "bar" == "foobar"
["A", "B"] + ["C"] == ["A", "B", "C"]
```

Using the multiplication operator between a string or list and an integer repeats the string or list the given number of times:

```
"piece" * 6 == "piecepiecepiecepiecepiecepiece"
3 * [46] == [46, 46, 46]
```

An additional operator, `cover`, can be used to provide an alternative value for an expression that results in an error:

```
10/5 cover "Zero division" == 2
10/0 cover "Zero division" == "Zero division"
```

## 5. Variables

### 5.1 Assignment

Variable names can consist of any alphanumeric characters plus underscores (`_`). They are defined as follows:

```
variable_name = value
```

> This is also how you define functions — as variables:
> ```
> add = (left, right) -> {
> 	return left + right
> }
> add(40, 6) # Evaluates to '46'
> ```

All variables are mutable; to change their contents, simply assign them a new value.

Elements at a specific index in a list or string can be modified as follows:

```
list[index] = value
```

### 5.2 Scope

By default, variables defined within functions are limited to the context of the function, meaning they will be destroyed after the function concludes. The prevent this, or to change the value of a variable previously defined outside the function, add the `global` keyword before the statement:
```
global variable_name = value
```

### 5.3 In-Place Operations

All of the arithmetic operations (see 4.) can be performed on variables in-place as follows:

```
number = 4
number **= 2 # 'number' now holds 16.
```

## 6. Control Flow

### 6.1 Normal Control Flow

Under normal circumstances, statement(s) are interpreted sequentally, from left to right, from top to bottom:

```
first; second
third
```

### 6.2 Return Statement

Any execution context can be concluded early using the `return` keyword, optionally followed by a return value. If the return value is omitted, `Void` is returned (see 2.6). Return statements work inside functions, where they conclude the currently running function, or in the main script, where they conclude the entire execution:

```
() -> {
	statement1
	return some_value
	statement2 # Not executed.
}

return # The return value can be omitted.
```

If there is no `return` to conclude a function, it will simply return the last evaluated result.

### 6.3 Exit Statement

A script can be aborted early using the `exit` keyword, optionally followed by an integer-only exit code to be returned to the operating system. If the exit code is omitted, it defaults to 0.

```
return () -> {
	return () -> {
		exit 46 # Immediately ends the execution with exit code 46.
		return 1 # Does not get returned.
	}()
}()
```

### 6.4 Selective Control Flow

Conditional execution is achieved using the `if` keyword:

```
if conditional_expression {
	# conditional_expression evaluated to 'true'.
}
```

Optionally, one can add separate branches using the `elif` keyword.

```
if conditional_expression {
	# conditional_expression evaluated to 'true'.
} elif other_conditional_expression {
	# conditional_expression evaluated to 'false'.
	# other_conditional_expression evaluated to 'true'.
}
```

Finally, alternative statement(s) can be included using the `else` keyword.

```
if conditional_expression {
	# The conditional expression evaluated to 'true'.
} else {
	# The conditional expression evaluated to 'false'.
}
```

### 6.5 Iterative Control Flow

Statement(s) can be repeated.

Using the `for` keyword, statement(s) can be repeated a given number of times:

```
for iterator_variable from starting_integer till concluding_integer {
	statement
}
```

Here, `iterator_variable` is the name for the variable that will hold the iteration inside the body. It can be accessed like any other variable. `starting_integer` is the integer the iterator has at the beginning of the loop. `concluding_integer` is the integer that, when reached, will conclude the loop; it is important to note here that once the concluding integer was reached, the body is **not** executed one last time. The operation applied to the iterator for every iteration depends on the starting and concluding integers: if the concluding integer is smaller than the starting integer, the iterator is decremented by `1` every loop — otherwise, it is incremented by `1` every loop.

Alternatively, the `for` loop also accepts the `in` keyword in place of `from-till`. This simplifies the act of iterating over items in a list or characters in a string; instead of having to access the current element *via* the iterator variable, the current element is instead placed directly *into* the iterator variable:

```
for element in list_or_string {
	statement
}
```

Using the `while` keyword, statement(s) can be repeated until a conditional expression no longer evaluates to 'true':

```
i = 10
while i > 0 {
	i = i-1
}
```

Both of these iterative control flows can be controlled using keywords.

To conclude a loop early, use `break`:

```
i = 10
while 1 {
	i = i-1
	if i <= 0
		break
}
```

To skip to the next iteration in a loop, use `continue`:

```
odd_numbers = 0
for i from 0 till 11 {
	if i%2
		continue
	odd_numbers = odd_numbers+1
}
```

### 6.6 Assert Statement

The `assert` statement is used to ensure assertions are met at runtime.

```
assert 2 > 1 # Passes.
assert 1 > 2 # Fails.
```

## 7. Native Functions

- `put(value)` – Prints `value` to the standard output pipe:  
  ```
  > une -s "put(\"Hello, World!\")"
  Hello, World!> 
  ```
- `print(value)` – Same as `put`, but automatically appends a newline to the output:
  ```
  > une -s "put(\"Hello, World!\")"
  Hello, World!
  > 
  ```
- `int(value)` – Returns `value` as an integer:
  ```
  int("46") == 46
  ```
- `flt(value)` – Returns `value` as a floating-point number:
  ```
  flt(46) == 46.0
  ```
- `str(value)` – Returns `value` as a string:
  ```
  str(46) == "46"
  ```
- `len(value)` – Returns the number of characters or items in `value`:
  ```
  len("Une") == 3
  len([1, 2, 3]) == 3
  ```
- `sleep(time)` – Halts execution for `time` milliseconds:
  ```
  [16:20:46] > une -s "sleep(10000); print(\"Done.\")"
  [16:20:56] Done.
  ```
- `ord(character)` – Returns the ordinal character code of `character`:
  ```
  ord("A") == 65
  ```
- `chr(code)` – Returns the character represented by the ordinal character code `code`:
  ```
  chr(65) == "A"
  ```
- `write(path, text)` – Writes the text `text` to a file at `path`:
  ```
  > une -s "write(\"test.txt\", \"return 40\")"
  ```
  test.txt:
  ```
  return 40
  ```
- `append(path, text)` – Same as `write`, but does not overwrite existing content:
  ```
  > une -s "append(\"test.txt\", \"+6\")"
  ```
  test.txt:
  ```
  return 40+6
  ```
- `read(path)` – Returns the text contents of a file at `path`:
  ```
  read("test.txt") == "return 40+6"
  ```
- `script(path)` – Calls a script at `path` into the host context:
  ```
  script("test.txt") == 46
  ```
- `eval(string)` – Same as `script()`, but accepts a string instead:
  ```
  eval("23*2") == 46
  ```
- `input(prompt)` – Prints `prompt` and requests input from the user, returning the entered string:
  ```
  > une -s "print(\"You wrote '\" + input(\"Write something: \") + \"'\")"
  Write something: test
  You wrote 'test'
  ```
- `exist(path)` – Returns `1` if a file `path` exists, otherwise `0`:
  ```
  exist("test.txt") == 1
  ```
- `split(text, delimiters)` – Returns a list of slices created by separating the string `text` at every delimiter contained in the list `delimiters`:
  ```
  split("1,2;3", [",", ";"]) == ["1", "2", "3"]
  ```
- `join(list, seperator)` – Joins the strings in `list` into a single string, seperated by `seperator`:
  ```
  join(["1", "2", "3"], ";") == "1;2;3"
  ```
- `replace(search, replace, subject)` – Replace string `search` with string `replace` in string `subject`:
  ```
  replace("+", "*", "2+23") == "2*23"
  ```
- `sort(subject, comparator)` – Return a sorted copy of the list `subject`, using the `comparator` function to compare elements:
  ```
  sort([3, 1, 2], (a, b) -> return a-b) == [1, 2, 3]
  ```
- `setwd()` – Set the working directory.
  ```
  setwd("C:\Directory")
  ```
- `getwd()` – Get the current working directory.
  ```
  getwd() == "C:\Directory"
  ```
- `playwav()` – Play a WAV file. (Windows only)
  ```
  playwav("sound.wav")
  ```

---

<img src="res/icon.png" width=10%>

