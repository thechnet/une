# test-6.py – CMDRScript Python Interpreter Test 6
# Modified 2021-02-20

# This is an expanded rebuild of test-5, focusing less on Python-specific
# language features and with an eventual C implementation in mind.
# It also implements variable assigments and references, strings,
# some string operations, the seperation of commands using ';',
# and multiline input.

from dataclasses import dataclass

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
  '/': 'DIV',
  '%': 'MOD',
  '(': 'LPAR',
  ')': 'RPAR'
}
#endregion Constants

#region Classes and Functions
@dataclass
class Token:
  type: str
  value: any = None
  
  def __repr__(self):
    return f'\33[93m{self.type}\33[97m:\33[92m{self.value}\33[97m' if self.value != None else f'\33[93m{self.type}\33[97m'

@dataclass
class Node:
  type: str
  token: Token
  subnodes: list = None
  
  def __repr__(self):
    if self.subnodes:
      ret = f'\33[97m(\33[94m{self.type}\33[97m'
      if self.token.value != None:
        ret += f'\33[96m {self.token.value}\33[97m'
      ret += f'\33[97m, {self.subnodes}\33[97m)'
    else:
      ret = f'\33[92m{self.token.value}\33[97m'
    return ret

def advance(list, index):
  index += 1
  return (list[index], index) if index < len(list) else (None, -1)
#endregion Classes and Functions

#region Lexer
def lex(text):
  tokens = []
  c, i = advance(text, -1)
  while c != None:
    if c in WHITESPACE:
      c, i = advance(text, i)
    elif c in DIGITS:
      # Integer token
      buf = c
      c, i = advance(text, i)
      while c != None and c in DIGITS:
        buf += c
        c, i = advance(text, i)
      tokens.append(Token('INT', int(buf)))
    elif c in LETTERS:
      # Identifier token
      buf = c
      c, i = advance(text, i)
      while c != None and c in LETTERS + DIGITS:
        buf += c
        c, i = advance(text, i)
      tokens.append(Token('ID', buf))
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
      tokens.append(Token('STR', buf))
    elif c == '=':
      # Assignment and equality
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('EQU'))
      else:
        tokens.append(Token('EQ'))
    elif c == '>':
      # Greater than and greater than or equal to
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('GEQ'))
      else:
        tokens.append(Token('GTR'))
    elif c == '<':
      # Less than or less than or equal to
      c, i = advance(text, i)
      if c == '=':
        c, i = advance(text, i)
        tokens.append(Token('LEQ'))
      else:
        tokens.append(Token('LSS'))
    elif c in STATIC_TOKENS:
      # Static single-character tokens
      tokens.append(Token(STATIC_TOKENS[c]))
      c, i = advance(text, i)
    else:
      # Unknown character
      print('Illegal Character: \'' + c + '\'')
      exit(0)
  tokens.append(Token('EOF'))
  return tokens
#endregion Lexer

#region Parser
def factor():
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
      tk, i = advance(tokens, i)
      pass
    else:
      return Node('VARGET', this, None)
  
  elif this.type == 'SUB':
    tk, i = advance(tokens, i)
    fac = factor()
    if fac == None or fac.type != 'NUMBER':
      print('factor: sub: Expected number.')
      exit(0)
    return Node('NEG', this, [fac])
  
  elif this.type == 'LPAR':
    tk, i = advance(tokens, i)
    expr = expression()
    if expr == None:
      print('factor: lpar: Expected expression.')
      exit(0)
    if tk.type != 'RPAR':
      print('factor: rpar: Expected RPAR.')
      exit(0)
    tk, i = advance(tokens, i)
    return expr

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

def parse(tokens):
  asts = []
  
  global tk, i
  
  while tk.type != 'EOF':
    
    this = tk
    
    if this.type == 'ID':
      _tk, _i = tk, i
      
      tk, i = advance(tokens, i)
      
      if tk.type == 'EQ':
        tk, i = advance(tokens, i)
        expr = expression()
        if expr == None:
          print('parse: id: eq: Expected expression.')
          exit(0)
        if tk.type != 'END':
          print('parse: id: eq: Expected end of command.')
          exit(0)
        tk, i = advance(tokens, i)
        asts.append(Node('VARASSIGN', this, [expr]))
        continue
      
      tk, i = _tk, _i # UGLY
    
    # else
    expr = expression()
    if tk.type != 'END':
        print('parse: id: eq: Expected end of command.')
        exit(0)
    tk, i = advance(tokens, i)
    asts.append(expr)
  
  return asts
#endregion Parser

#region Interpreter
def interpret(node):
  global symboltable
  
  if node.type in ('NUMBER', 'STRING'):
    return node.token.value
  if node.type == 'POS':
    return interpret(node.subnodes[0])
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
  elif node.type == 'VARASSIGN':
    symboltable[node.token.value] = interpret(node.subnodes[0])
    return symboltable[node.token.value]
  elif node.type == 'VARGET':
    if node.token.value in symboltable:
      return symboltable[node.token.value]
    else:
      print('interpret: varget: Undefined variable.')
      exit(0)
#endregion

symboltable = {}
script = '''
r="test";
2*3*r;
'''
tokens = lex(script)
tk, i = advance(tokens, -1)
asts = parse(tokens)
results = []
for ast in asts:
  results.append(interpret(ast))

#region Print Results
print("")
print(
  'Input\n'
  f'\33[97m{script.strip()}\33[0m\n'
)
print(
  'Lexer Result\n'
  f'\33[97m{tokens}\n\33[0m'
)
print(
  'Parser Result\n'
  f'\33[97m{asts}\33[0m\n'
)
print(
  'Symboltable Result\n'
  f'\33[97m{symboltable}\33[0m\n'
)
print(
  'Interpreter Result\n'
  f'\33[97m{results}\33[0m'
)
#endregion Print Results