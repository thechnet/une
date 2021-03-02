# test-6.py – CMDRScript Python Interpreter Test 6
# Modified 2021-02-25

# 2021-20-20: This is an expanded rebuild of test-5, focusing less on Python-specific
# language features and with an eventual C implementation in mind.
# It also implements variable assigments and references, strings,
# some string operations, the seperation of commands using ';',
# and multiline input.

# 2021-02-21: This implementation now also supports:
# - if, elif, else statements (including NOT, EQU, NEQ, GTR,
#   LSS, GEQ, LEQ - still missing is AND, OR)
# - while loops (including BREAK, CONTINUE)
# - functions (including context switching, RETURN)
# As it currently stands, some parts of this code are very bug-prone
# and not very robust. Before moving on to the C implementation, I want to solve
# these problems and also find a finalized design for syntax.

# 2021-02-22: Added C-style inline comments and Python-style line comments.

# 2021-02-25: This implementation now also supports:
# - AND, OR in logical statements
# - Power ('^')
# - For loops

# 2021-02-26: Added Position class.
# The Token class now holds a position object. This position can later be used
# to show precise error locations. The function 'show_error' shows a line of code,
# with arrows indicating the location of the error.

# 2021-03-02:
# - Turned data classes into regular classes to support Python <3.7.
# - All tokens now include a position object when created.
# - Illegal Character errors are now handled using the new system.
# (This test is getting very messy, I should move to a new recode soon.)

#region Constants
DIGITS = '0123456789'
LETTERS = '_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
WHITESPACE = '\n\r\t '

STATIC_TOKENS = {
  '{': 'LBRC',
  '}': 'RBRC',
  ';': 'END',
  '&': 'AND',
  '|': 'OR',
  '+': 'ADD',
  '-': 'SUB',
  '*': 'MUL',
  '%': 'MOD',
  '(': 'LPAR',
  ')': 'RPAR',
  ',': 'SEP',
  '^': 'POW'
}
#endregion Constants

#region Classes and Functions

class Position:
  def __init__(self, file: str, text: str, ln_idx: int, ln: int, col: int):
    self.file = file
    self.text = text
    self.ln_idx = ln_idx
    self.ln = ln
    self.col = col

class Token:
  def __init__(self, type: str, value: any, pos: Position):
    self.type = type
    self.value = value
    self.pos = pos
  
  def __repr__(self):
    return f'\33[93m{self.type}\33[97m:\33[92m{self.value}\33[97m' if self.value != None else f'\33[93m{self.type}\33[97m'
  
  def detail(self):
    ret = f'\33[90m{self.pos.file}:{self.pos.ln_idx}:{self.pos.ln}:{self.pos.col}:\33[93m{self.type}'
    if self.value != None:
      ret += f'\33[97m:\33[92m{self.value}\33[97m'
    return ret

class Node:
  def __init__(self, type: str, token: Token, subnodes: list = None):
    self.type = type
    self.token = token
    self.subnodes = subnodes
  
  def __repr__(self):
    if self.subnodes:
      ret = f'\33[97m(\33[94m{self.type}\33[97m'
      if self.token.value != None:
        ret += f'\33[96m {self.token.value}\33[97m'
      ret += f'\33[97m, {self.subnodes}\33[97m)'
    else:
      ret = f'\33[92m{self.token.value}\33[97m'
    return ret

class Context:
  def __init__(self, symboltable: dict, parent: any):
    self.symboltable = symboltable
    self.parent = parent
  
  def __repr__(self):
    return f'{self.symboltable}'

def advance(list, index):
  index += 1
  return (list[index], index) if index < len(list) else (None, -1)

def symboltable_get(ctx, key):
  while True:
    if key in ctx.symboltable:
      return ctx.symboltable[key]
    if ctx.parent == None:
      return None
    ctx = ctx.parent

def show_error(begin, end, message):
  print(
    f'File {begin.file}, Line {begin.ln}:\n  ' +
    begin.text[begin.ln_idx:].split('\n')[0] +
    f'\n\33[{begin.col+2}C' + '^'*(end.col-begin.col) +
    f'\n{message}'
  )
#endregion Classes and Functions

#region Built-in Functions
def builtin_print(value):
  print(value)
  
def builtin_str(value):
  return str(value)

def builtin_int(value):
  return int(value)

def builtin_var_set_str(key, value):
  context.symboltable[key] = value
  return 1

def builtin_var_get_str(key):
  return symboltable_get(context, key)
#endregion Built-in Functions

#region Lexer
def lex(file, text):
  global error, error_begin, error_end
  tokens = []
  ln = 0
  ln_idx = 0
  col = 0
  c, i = advance(text, -1)
  while c != None and not error:
    if c in WHITESPACE:
      if c == '\n':
        ln += 1
        col = 0
        ln_idx = i+1
      c, i = advance(text, i)
    elif c in DIGITS:
      # Integer token
      buf = c
      c, i = advance(text, i)
      while c != None and c in DIGITS:
        buf += c
        c, i = advance(text, i)
      tokens.append(Token('INT', int(buf), Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c in LETTERS:
      # Identifier token
      buf = c
      c, i = advance(text, i)
      while c != None and c in LETTERS + DIGITS:
        buf += c
        c, i = advance(text, i)
      tokens.append(Token('ID', buf, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '"':
      # String token
      buf = ''
      c, i = advance(text, i)
      escape = False
      while c != None:
        if c == '\\':
          c, i = advance(text, i)
          escape = True
          continue
        if escape:
          escape = False
          if c == 'n':
            buf += '\n'
          else:
            buf += c
          c, i = advance(text, i)
          continue
        if c == '"':
          c, i = advance(text, i)
          break
        buf += c
        c, i = advance(text, i)
      tokens.append(Token('STR', buf, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '=':
      # Assignment and equality
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('EQU', None, Position(file, text, ln_idx, ln, i-ln_idx)))
      else:
        tokens.append(Token('EQ', None, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '!':
      # Not or not equals
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('NEQ', None, Position(file, text, ln_idx, ln, i-ln_idx)))
      else:
        tokens.append(Token('NOT', None, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '>':
      # Greater than and greater than or equal to
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('GEQ', None, Position(file, text, ln_idx, ln, i-ln_idx)))
      else:
        tokens.append(Token('GTR', None, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '<':
      # Less than and less than or equal to
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('LEQ', None, Position(file, text, ln_idx, ln, i-ln_idx)))
      else:
        tokens.append(Token('LSS', None, Position(file, text, ln_idx, ln, i-ln_idx)))
    elif c == '/':
      # Division or inline comment
      c, i = advance(text, i) # /
      if c != '*':
        tokens.append(Token('DIV', None, Position(file, text, ln_idx, ln, i-ln_idx)))
      else:
        while c != None:
          if c == '*':
            c, i = advance(text, i) # *
            if c == '/':
              c, i = advance(text, i) # /
              break
            else:
              continue
          c, i = advance(text, i)
    elif c == '#':
      # Line comment
      while c != None and c != '\n':
        c, i = advance(text, i)
    elif c in STATIC_TOKENS:
      # Static single-character tokens
      tokens.append(Token(STATIC_TOKENS[c], None, Position(file, text, ln_idx, ln, i-ln_idx)))
      c, i = advance(text, i)
    else:
      # Unknown character
      error = f"Illegal Character: '{c}'"
      error_begin = Position(file, text, ln_idx, ln, i-ln_idx)
      error_end = Position(file, text, ln_idx, ln, i-ln_idx+1)
  if not error:
    tokens.append(Token('EOF', None, Position(file, text, ln_idx, ln, i-ln_idx)))
  return tokens
#endregion Lexer

#region Parser

#region atom
def atom():
  global tk, i
  this = tk
  
  if this.type == 'INT':
    tk, i = advance(tokens, i)
    return Node('NUMBER', this, None)
  
  if this.type == 'STR':
    tk, i = advance(tokens, i)
    return Node('STRING', this, None)
    
  if this.type == 'ID':
    tk, i = advance(tokens, i)
    if tk.type == 'LPAR':
      tk, i = advance(tokens, i) # (
      
      args = []
  
      if tk != None and tk.type != 'RPAR':
        args.append(expression())
      
      while tk != None and tk.type != 'RPAR':
        if tk.type != 'SEP':
          print('atom: id: Expected SEP.')
          exit(0)
        tk, i = advance(tokens, i) # ,
        args.append(expression())
      
      tk, i = advance(tokens, i) # )
      
      return Node(
        'FUNCEXE',
        this,
        [
          args
        ]
      )
    else:
      return Node('VARGET', this, None)
  
  elif this.type == 'LPAR':
    tk, i = advance(tokens, i)
    expr = expression()
    if expr == None:
      print('atom: lpar: Expected expression.')
      exit(0)
    if tk.type != 'RPAR':
      print('atom: rpar: Expected RPAR.')
      exit(0)
    tk, i = advance(tokens, i)
    return expr
#endregion atom

#region power
def power():
  global tk, i
  this = tk
  
  left = atom()
  
  while tk.type == 'POW':
    op = tk
    tk, i = advance(tokens, i) # ^
    left = Node('POW', op, [left, atom()])
  
  return left
#endregion

#region factor
def factor():
  global tk, i
  this = tk

  if this.type == 'SUB':
    tk, i = advance(tokens, i)
    fac = power()
    if fac == None or fac.type != 'NUMBER':
      print('factor: sub: Expected number.')
      exit(0)
    return Node('NEG', this, [fac])
  
  return power()
#endregion factor

#region term
def term():
  global tk, i
  this = tk
  
  left = factor()
  if left == None:
    print(f'term: Expected factor, got None')
    exit(0)
  
  if this.type == 'STR':
    if tk.type == 'MUL':
      while tk != None and tk.type in ('MUL', 'DIV', 'MOD'):
        op = tk
        tk, i = advance(tokens, i)
        left = Node('STRMUL', op, [left, factor()])
        if left == None:
          print('term: str: mul: Expected factor.')
          exit(0)
      return left
    elif tk.type in ('DIV', 'MOD'):
      print('term: str: div/mod: Expected *')
      exit(0)
    else:
      return left
  
  while tk != None and tk.type in ('MUL', 'DIV', 'MOD'):
    op = tk
    tk, i = advance(tokens, i)
    left = Node(op.type, op, [left, factor()])
    if left == None:
      print('term: mul/div/mod: Expected factor.')
      exit(0)
  
  return left
#endregion term

#region expression
def expression():
  global tk, i
  this = tk
  
  left = term()
  if left == None:
    print('expression: Expected term.')
    exit(0)
  
  if this.type == 'STR':
    if tk.type == 'ADD':
      while tk != None and tk.type == 'ADD':
        op = tk
        tk, i = advance(tokens, i)
        left = Node('STRCAT', op, [left, term()])
        if left == None:
          print('expression: str: add: Expected term.')
          exit(0)
      return left
    elif tk.type == 'SUB':
      print('expression: str: sub: Expected +')
      exit(0)
    else:
      return left
  
  while tk != None and tk.type in ('ADD', 'SUB'):
    op = tk
    tk, i = advance(tokens, i)
    left = Node(op.type, op, [left, term()])
    if left == None:
      print('expression: add/sub: Expected term.')
      exit(0)
  
  return left
#endregion expression

#region logicalexpression
def logicalexpression():
  global tk, i
  this = tk
  if this.type == 'NOT':
    tk, i = advance(tokens, i)
    return Node(this.type, this, [logicalexpression()])
  
  left = expression()
  if left == None:
    print('logicalexpression: Expected expression.')
    exit(0)
  
  if tk.type in ('EQU', 'NEQ', 'GTR', 'LSS', 'GEQ', 'LEQ'):
    op = tk
    tk, i = advance(tokens, i)
    left = Node(op.type, op, [left, expression()])
    if left == None:
      print('logicalexpression: Expected expression.')
      exit(0)
  
  if tk.type in ('AND', 'OR'):
    op = tk
    tk, i = advance(tokens, i)
    left = Node(op.type, op, [left, logicalexpression()])
    if left == None:
      print('logicalexpression: Expected logical expression.')
      exit(0)
  
  # while tk.type in ('EQU', 'NEQ', 'GTR', 'LSS', 'GEQ', 'LEQ'):
  #   op = tk
  #   tk, i = advance(tokens, i)
  #   left = Node(op.type, op, [left, expression()])
  #   if left == None:
  #     print('logicalexpression: Expected expression.')
  #     exit(0)
  
  return left
#endregion logicalexpression

#region codeblock
def codeblock():
  global tk, i
  this = tk
  
  if tk.type != 'LBRC':
    print('statement: if: Expected LBRC.')
    exit(0)
  
  tk, i = advance(tokens, i) # {
  
  block = []
  
  while tk != None and tk.type != 'RBRC':
    block.append(statement()) # statement;
  
  if tk.type != 'RBRC':
    print('statement: if: Expected RBRC.')
    exit(0)
  
  tk, i = advance(tokens, i) # }
  
  return block
#endregion codeblock

#region ifstatement
def ifstatement():
  global tk, i
  this = tk
  
  logexpr = logicalexpression()
  
  trueblock = []
  falseblock = []
  
  trueblock = codeblock()
  
  if tk.type == 'ID' and tk.value == 'elif':
    tk, i = advance(tokens, i) # elif
    falseblock = [ ifstatement() ]
  
  elif tk.type == 'ID' and tk.value == 'else':
    tk, i = advance(tokens, i) # else
    falseblock = codeblock()
  
  return Node(
    'IF',
    this,
    [
      logexpr,
      trueblock,
      falseblock
    ]
  )
#endregion ifstatement

#region whilestatement
def whilestatement():
  global tk, i
  this = tk
  
  logexpr = logicalexpression()
  
  trueblock = codeblock()
  
  return Node(
    'WHILE',
    this,
    [
      logexpr,
      trueblock
    ]
  )
#endregion whilestatement

#region forloop
def forloop():
  global tk, i
  this = tk
  
  if this.type != 'ID':
    print('forloop: id: Expected ID.')
    exit(0)
  name = this.value
  tk, i = advance(tokens, i) # ID

  if tk.type == 'ID' and tk.value == 'from':
    tk, i = advance(tokens, i) # 'from'
  else:
    print("forloop: from: Expected 'from'.")
    exit(0)
  
  begin = expression()
  
  if tk.type == 'ID' and tk.value == 'to':
    tk, i = advance(tokens, i) # 'to'
  else:
    print("forloop: to: Expected 'to'.")
    exit(0)
  
  end = expression()
  
  block = codeblock()
  
  return Node(
    'FOR',
    this,
    [
      name,
      begin,
      end,
      block
    ]
  )
#endregion forloop

#region functiondef
def functiondef():
  global tk, i
  this = tk
  
  if this.type != 'ID':
    print('functiondef: Expected ID.')
    exit(0)
  
  funcid = this.value
  
  tk, i = advance(tokens, i) # id
  
  if tk.type != 'LPAR':
    print('functiondef: Expected LPAR.')
    exit(0)
  
  tk, i = advance(tokens, i) # (
  
  params = []
  
  if tk != None and tk.type != 'RPAR':
    if tk.type != 'ID':
      print('functiondef: Expected ID.')
      exit(0)
    params.append(tk.value)
    tk, i = advance(tokens, i) # arg
  
  while tk != None and tk.type != 'RPAR':
    if tk.type != 'SEP':
      print('functiondef: Expected SEP.')
      exit(0)
    tk, i = advance(tokens, i) # ,
    if tk.type != 'ID':
      print('functiondef: Expected ID.')
      exit(0)
    params.append(tk.value)
    tk, i = advance(tokens, i) # arg
  
  tk, i = advance(tokens, i) # )
  
  funcblock = codeblock()
  
  return Node(
    'FUNCDEF',
    this,
    [
      params,
      funcblock
    ]
  )
#endregion functiondef

#region statement
def statement():
  global tk, i
  this = tk
    
  if this.type == 'ID':
    
    if this.value == 'if':
      tk, i = advance(tokens, i)
      return ifstatement()
    
    if this.value == 'while':
      tk, i = advance(tokens, i)
      return whilestatement()
    
    if this.value == 'for':
      tk, i = advance(tokens, i)
      return forloop()
    
    if this.value == 'def':
      tk, i = advance(tokens, i)
      return functiondef()
    
    if this.value == 'return':
      tk, i = advance(tokens, i) # id
      expr = expression()
      if tk.type != 'END':
        print('statement: id: return: Expected end of command.')
        exit(0)
      tk, i = advance(tokens, i) # end
      return Node('RETURN', this, [expr])
    
    if this.value == 'break':
      tk, i = advance(tokens, i) # id
      if tk.type != 'END':
        print('statement: id: break: Expected end of command.')
        exit(0)
      tk, i = advance(tokens, i) # end
      return Node('BREAK', this, [])
    
    if this.value == 'continue':
      tk, i = advance(tokens, i) # id
      if tk.type != 'END':
        print('statement: id: continue: Expected end of command.')
        exit(0)
      tk, i = advance(tokens, i) # end
      return Node('CONTINUE', this, [])
    
    _tk, _i = tk, i
    tk, i = advance(tokens, i)
    
    if tk.type == 'EQ':
      tk, i = advance(tokens, i)
      expr = expression()
      if expr == None:
        print('statement: id: eq: Expected expression.')
        exit(0)
      if tk.type != 'END':
        print('statement: id: eq: Expected end of command.')
        exit(0)
      tk, i = advance(tokens, i) # END
      return Node('VARSET', this, [expr])
    
    tk, i = _tk, _i # UGLY
  
  # Needs to stay to allow for freestanding function calls.
  expr = expression()
  if tk.type != 'END':
      print('statement: id: eq: Expected end of command.')
      exit(0)
  tk, i = advance(tokens, i)
  return expr
#endregion statement

#region parse
def parse(tokens):
  asts = []
  
  global tk, i
  
  while tk.type != 'EOF':
    
    asts.append(statement())
  
  return asts
#endregion parse

#endregion Parser

#region Interpreter
def interpret(node):
  global context, interpretState
  
  if interpretState != '': return # hmmmm
  
  if node.type in ('NUMBER', 'STRING'):
    return node.token.value
  elif node.type == 'NEG':
    return interpret(node.subnodes[0]) * -1
  if node.type == 'ADD':
    return interpret(node.subnodes[0]) + interpret(node.subnodes[1])
  elif node.type == 'SUB':
    return interpret(node.subnodes[0]) - interpret(node.subnodes[1])
  elif node.type == 'MUL':
    return interpret(node.subnodes[0]) * interpret(node.subnodes[1])
  elif node.type == 'DIV':
    right = interpret(node.subnodes[1])
    if right == 0:
      print('Division by zero.')
      exit(0)
    return int(interpret(node.subnodes[0]) / right)
  elif node.type == 'MOD':
    right = interpret(node.subnodes[1])
    if right == 0:
      print('Modulus by zero.')
      exit(0)
    return interpret(node.subnodes[0]) % right
  elif node.type == 'STRMUL':
    return interpret(node.subnodes[0]) * interpret(node.subnodes[1])
  elif node.type == 'STRCAT':
    return interpret(node.subnodes[0]) + str(interpret(node.subnodes[1]))
  elif node.type == 'VARSET':
    context.symboltable[node.token.value] = interpret(node.subnodes[0])
    return context.symboltable[node.token.value]
  elif node.type == 'VARGET':
    value = symboltable_get(context, node.token.value)
    if value != None:
      return value
    else:
      print('interpret: varget: Undefined variable.')
      exit(0)
  elif node.type == 'NOT':
    return 1 if not interpret(node.subnodes[0]) else 0
  elif node.type == 'EQU':
    return 1 if interpret(node.subnodes[0]) == interpret(node.subnodes[1]) else 0
  elif node.type == 'NEQ':
    return 1 if interpret(node.subnodes[0]) != interpret(node.subnodes[1]) else 0
  elif node.type == 'GTR':
    return 1 if interpret(node.subnodes[0]) > interpret(node.subnodes[1]) else 0
  elif node.type == 'LSS':
    return 1 if interpret(node.subnodes[0]) < interpret(node.subnodes[1]) else 0
  elif node.type == 'GEQ':
    return 1 if interpret(node.subnodes[0]) >= interpret(node.subnodes[1]) else 0
  elif node.type == 'LEQ':
    return 1 if interpret(node.subnodes[0]) <= interpret(node.subnodes[1]) else 0
  elif node.type == 'AND':
    return 1 if interpret(node.subnodes[0]) and interpret(node.subnodes[1]) else 0
  elif node.type == 'OR':
    return 1 if interpret(node.subnodes[0]) or interpret(node.subnodes[1]) else 0
  elif node.type == 'IF':
    if interpret(node.subnodes[0]):
      returnValue = 1
      for subnode in node.subnodes[1]:
        ret = interpret(subnode)
        if interpretState != '':
          if interpretState == 'RETURN':
            returnValue = ret
          break
        returnValue = ret
      return returnValue
    else:
      returnValue = 1
      for subnode in node.subnodes[2]:
        ret = interpret(subnode)
        if interpretState != '':
          if interpretState == 'RETURN':
            returnValue = ret
          break
        returnValue = ret
      return returnValue
  elif node.type == 'WHILE':
    returnValue = 1
    while interpret(node.subnodes[0]):
      for subnode in node.subnodes[1]:
        ret = interpret(subnode)
        if interpretState != '':
          if interpretState == 'RETURN':
            returnValue = ret
          break
        returnValue = ret
        
      if interpretState != '':
        if interpretState == 'CONTINUE':
          interpretState = ''
          continue
        if interpretState == 'BREAK':
          interpretState = ''
        break
    return returnValue
  elif node.type == 'FOR':
    returnValue = 1
    begin = interpret(node.subnodes[1])
    end = interpret(node.subnodes[2])
    if begin > end:
      op = -1
    else:
      op = 1
    for i in range(begin, end, op):
      context.symboltable[node.subnodes[0]] = i
      
      for subnode in node.subnodes[3]:
        ret = interpret(subnode)
        if interpretState != '':
          if interpretState == 'RETURN':
            returnValue = ret
          break
        returnValue = ret
        
      if interpretState != '':
        if interpretState == 'CONTINUE':
          interpretState = ''
          continue
        if interpretState == 'BREAK':
          interpretState = ''
        break
    return returnValue
  elif node.type == 'FUNCDEF':
    context.symboltable[node.token.value] = node.subnodes
    return 1
  elif node.type == 'FUNCEXE':
    func = symboltable_get(context, node.token.value)
    args = node.subnodes[0]
    if func == None:
      if node.token.value == 'print': # BAD!
        if len(args) != 1:
          print('Invalid number of arguments provided.')
          exit(0)
        builtin_print(interpret(args[0]))
        return 1
      if node.token.value == 'str': # BAD!
        if len(args) != 1:
          print('Invalid number of arguments provided.')
          exit(0)
        return builtin_str(interpret(args[0]))
      if node.token.value == 'int': # BAD!
        if len(args) != 1:
          print('Invalid number of arguments provided.')
          exit(0)
        return builtin_int(interpret(args[0]))
      if node.token.value == 'var_set_str': # BAD!
        if len(args) != 2:
          print('Invalid number of arguments provided.')
          exit(0)
        return builtin_var_set_str(interpret(args[0]), interpret(args[1]))
      if node.token.value == 'var_get_str': # BAD!
        if len(args) != 1:
          print('Invalid number of arguments provided.')
          exit(0)
        return builtin_var_get_str(interpret(args[0]))
      print('Undefined function.')
      exit(0)
    params = func[0]
    if len(params) != len(args):
      print('Invalid number of arguments provided.')
      exit(0)
    context = Context({}, context)
    for i, param in enumerate(params):
      context.symboltable[param] = interpret(args[i])
    returnValue = 1
    for subnode in func[1]:
      ret = interpret(subnode)
      if interpretState == 'RETURN':
        interpretState = ''
        returnValue = ret
        break
    context = context.parent
    return returnValue
  elif node.type == 'RETURN':
    if context.parent == None:
      print("Can't return from root program")
      exit(0)
    result = interpret(node.subnodes[0])
    interpretState = 'RETURN'
    return result
  elif node.type == 'BREAK':
    interpretState = 'BREAK' # HORRIBLE FIXME:
    return 1
  elif node.type == 'CONTINUE':
    interpretState = 'CONTINUE' # HORRIBLE FIXME:
    return 1
  elif node.type == 'POW':
    return interpret(node.subnodes[0]) ** interpret(node.subnodes[1])
#endregion

error = ''
error_begin = None
error_end = None

context = Context({}, None)
interpretState = ''

script = '''
$
'''

# script = '''
# a=10^0+2*3; # = 7
# def/*ine function*/ is_even(number)
# {
#   if number%2 == 0
#   {
#     return 1;
#   }
#   else
#   {
#     return 0;
#   }
# }
# while a>0
# {
#   a=a-1;
#   if !is_even(a)
#   {
#     continue;
#   }
#   print("number " + str(a) + " is even");
# }
# '''

tokens = lex('<hardcoded>', script)
if error:
  show_error(error_begin, error_end, error)
  exit(0)
tk, i = advance(tokens, -1)
asts = parse(tokens)
results = []
print('\33[35m', end='')
for ast in asts:
  results.append(interpret(ast))
print('\33[0m')

#region Print Results
print(
  'Input\n'
  f'\33[97m{script.strip()}\33[0m\n'
)
print('Lexer Result\n')
# print(f'\33[97m{tokens}')
for tok in tokens: print(tok.detail())
print('\33[0m')
print(
  'Parser Result\n'
  f'\33[97m{asts}\33[0m\n'
)
print(
  'Symboltable Result\n'
  f'\33[97m{context.symboltable}\33[0m\n'
)
print(
  'Interpreter Result\n'
  f'\33[97m{results}\33[0m'
)
#endregion Print Results