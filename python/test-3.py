# test-3.py – CMDRScript Python Interpreter Test 3
# Modified 2021-02-18

# Similar to test-1, but instead of testing the properties of a character, tests if it's contained in a list of predefined characters.
# This approach is more flexible and easier to expand.

import os
os.system("clear")

f = '''
a="str";
if(a==1);
'''

TKS = {
  "num": {"chars":"0123456789","exact":False},
  "id": {"chars":"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_","exact":False},
  "end": {"chars":";","exact":True},
  "lpar": {"chars":"(","exact":True},
  "rpar": {"chars":")","exact":True},
  #"cmpequ": {"chars":"==","exact":True},
  "equ": {"chars":"=","exact":True}
}

cmds = []
tks = []

def printtk(tk, buf):
  global tks, cmds
  print("\33[92m{}\33[0m'{}' ".format(tk.upper(), buf), end="")
  tks.append({"type":tk,"str":buf})
  if tk == "end":
    print("")
    cmds.append(tks)
    tks = []

tk = ""
buf = ""

# get exact tokens
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
  if c=="\n": continue
  if tk and not c in TKS[tk]["chars"]:
    printtk(tk, buf)
    tk = ""
    buf = ""
  if not tk:
    for k in TKS:
      if c in TKS[k]["chars"]:
        tk = k
        break
    if not tk:
      print("unknown " + c)
      break
  buf += c
  if TKS[tk]["exact"]:
    if buf == TKS[tk]["chars"]:
      printtk(tk, buf)
      tk = ""
      buf = ""
      continue
    else:
      for k in TKS:
        if buf[:-0] == TKS[k]["chars"][:len(buf)]:
          tk = k
          break

# group tokens
print("")
for cmd in cmds:
  if cmd[0]["type"] == "id":
    if cmd[0]["str"] == "if":
      print("if command")
      continue
  if cmd[1]["type"] == "equ":
    print("variable declaration")
    continue
    
    
  else:
    print("unknown command")
    break

print("")