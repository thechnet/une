# test-5.py – CMDRScript Python Interpreter Test 5
# Modified 2021-02-20

# Using an Abstract Syntax Tree
# While the previous approaches for lexing weren't bad, I had no idea how to parse the tokens.
# Turning to the internet for help, I found a video series by YouTuber 'CodePulse' (youtube.com/watch?v=Eythq9848Fg), showing an approach to create an Abstract Syntax Tree:
# - We first perform lexical analysis to create an array of tokens.
#   This is very similar to the ideas I've explored so far.
#   The only difference is that instead of moving to character index
#   in every loop, we only move it when we were able to properly
#   process the current character.
# - Next we build a syntax tree:
#   - We look at the first token, and from there decide what end object
#     we want to construct. In this first test we're only evaluating
#     arithmetic expressions, so we always know we want to construct
#     an expression.
#   - We construct objects by constructing their individual pieces,
#     which in turn will construct *their* individual pieces.
#     An arithmetic evaluation is built from individual terms, which in
#     turn are built from factors. Therefore, we get an expression by:
#     - constructing an expression by
#     - constructing a term by
#     - constructing a factor,
#     - returning the factor,
#     - returning the term,
#     - and finally, returning the expression.
# - Finally, we interpret the tree:
#   - We find out what action needs to be done by inspecting
#     the corresponding place in the data structure.
#     (e.g. the action in '1*1+1' would be add '1*1' and '1', however
#     in '1*(1+1)' it would be multiply '1' by '(1+1)'.)
#   - We execute this action by visiting all its children and executing
#     the actions they in turn require, until we have a final result, which
#     we then pass back up to the root action.

# This test is a first implementation of an arithmetic evaluator.
# It is heavily based on CodePulse's example.
# I recoded this program in test-6, focusing less on Python-specific
# language features and with an eventual C implementation in mind.

DIGITS = '0123456789'

f = '(1 + 2) * 3'

#region Lexer
tokens = []
index = -1
c = ""

def advance():
  global index, c
  index += 1
  c = f[index] if index < len(f) else None

advance()

while c != None:
  if c in ' \t\n':
    advance()
    continue
  elif c in DIGITS:
    buf = ''
    dotFound = False
    while c != None and c in DIGITS + '.':
      if c == '.':
        if dotFound: break
        buf += '.'
        dotFound = True
      else:
        buf += c
      advance()
    if dotFound:
      tokens.append(('FLOAT', float(buf)))
    else:
      tokens.append(('INT', int(buf)))
  elif c == '+':
    advance()
    tokens.append(('PLUS'))
  elif c == '-':
    advance()
    tokens.append(('MINUS'))
  elif c == '*':
    advance()
    tokens.append(('MUL'))
  elif c == '/':
    advance()
    tokens.append(('DIV'))
  elif c == '(':
    advance()
    tokens.append(('LPAREN'))
  elif c == ')':
    advance()
    tokens.append(('RPAREN'))
  else:
    print('Illegal Character ' + c)
    exit(0)
#endregion

#region Parser
tkindex = -1
tk = None

def tkadvance():
  global tkindex, tk
  tkindex += 1
  tk = tokens[tkindex] if tkindex < len(tokens) else None

tkadvance()

def factor():
  if tk != None and tk[0] in ('INT', 'FLOAT'):
    value = tk[1]
    tkadvance()
    return value
  elif tk == 'LPAREN':
    tkadvance()
    value = expression()
    if tk != 'RPAREN':
      print('Invalid syntax: Expected RPAREN')
      exit(0)
    tkadvance()
    return value
  else:
    print('Invalid syntax: Expected number')
    exit(0)

def term():
  left = factor()
  while tk != None and tk in ('MUL', 'DIV'):
    bin_op = tk
    tkadvance()
    left = (left, bin_op, factor())
  return left

def expression():
  left = term()
  while tk != None and tk in ('PLUS', 'MINUS'):
    bin_op = tk
    tkadvance()
    left = (left, bin_op, term())
  return left

ast = expression()
#endregion

#region Interpreter
def visit(node):
  if type(node) in (int, float):
    return node
  elif node[1] == 'MUL':
    return visit(node[0]) * visit(node[2])
  elif node[1] == 'DIV':
    rvalue = visit(node[2])
    if rvalue == 0:
      print('Runtime error: Division by zero')
      exit(0)
    return visit(node[0]) / rvalue
  elif node[1] == 'PLUS':
    return visit(node[0]) + visit(node[2])
  elif node[1] == 'MINUS':
    return visit(node[0]) - visit(node[2])

print(visit(ast))
#endregion