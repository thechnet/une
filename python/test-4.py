# test-4.py – CMDRScript Python Interpreter Test 4
# Modified 2021-02-19

# Based on syntax-2, incorporating parts of test-4:
# We first identify the type of command we're working with:...

# f = """
# a=1;
# a="str";
# a=fun(1, 2);
# fun(1, 2);
# if a==1
# {
#   break;
# }
# for a, a<1, a++
# {
#   break;
# }
# def fun(a, b)
# {
#   break;
# }
# return;
# """

f = """
a=1;
def fun(a, b)
{
  break;
}
fun(1, 2);
"""

TKS = {
  "num": {"chars":"0123456789","exact":False},
  "id": {"chars":"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_","exact":False},
  "end": {"chars":";","exact":True},
  "lpar": {"chars":"(","exact":True},
  "rpar": {"chars":")","exact":True},
  "equ": {"chars":"=","exact":True},
  "sep": {"chars":",","exact":False},
  "ignore": {"chars": " \n","exact":False},
  "lcbr":{"chars":"{","exact":True},
  "rcbr":{"chars":"}","exact":True}
}

codeBlockWords = ["for", "if", "def"]

cmds = []
tks = []

def printtk(tk, buf):
  global tks, cmds
  if tk == "ignore": return
  print("\33[92m{}\33[0m'{}' ".format(tk.upper(), buf), end="")
  tks.append({"type":tk,"str":buf})
  if tk == "end" or tk == "rcbr":
    print("")
    cmds.append(tks)
    tks = []

def printword(word, wordType):
  print("\n\33[94m{}\33[0m'{}' ".format(wordType, word), end="")

def printexp(exp):
  print("\33[93m'{}'\33[0m ".format(exp), end="")

tk = ""
buf = ""
word = ""
wordType = ""

for c in f:
  if c=="\"":
    if tk == "str":
      printtk(tk, buf)
      tk = ""
    else:
      if tk: printtk(tk, buf)
      tk = "str"
    buf = ""
    continue
  if tk == "str":
    buf += c
    continue
  if c == "\n": continue
  if not word:
    if c == "=":
      word = buf
      wordType = "var"
      buf = ""
      printword(word, wordType)
      continue
    elif c == "(":
      word = buf
      wordType = "fun"
      buf = ""
      printword(word, wordType)
      continue
    elif c == " ":
      if buf in codeBlockWords:
        word = buf
        wordType = buf
        buf = ""
        printword(word, wordType)
        continue
      else:
        print("unknown code block word " + buf)
        exit()
    buf += c
  else:
    if wordType == "var":
      if c == ";":
        printexp(buf)
        buf = ""
        word = ""
        continue
      buf += c
      continue