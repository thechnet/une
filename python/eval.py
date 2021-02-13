# eval.py – CMDRScript Python Recursive Arithmetic Evaluation
# Modified 2021-02-13

EXP = "5+4*3+1"

# res = 0
#([+] addSubtract = "+")
# [5] buf = "5"
# [+] addSubtract? -> res += int(buf), buf = ""; holdBuf = buf = "5", addSubtract = "+"
# [4] buf = "4"
# [*] multDivide? -> multBuf _= int(buf) ELSE multBuf = int(buf); buf = "", multDivide = "*"
# [3] buf = "3"
# [+] addSubtract? -> 

def _eval(exp):
  res = 0
  buf = 0
  bufDefined = False
  bufOp = ""
  num = ""
  for c in exp:
    if c == "+":
      if bufDefined:
        pass
      res += int(num)
      num = ""
      continue
    if c == "-":
      res -= int(num)
      num = ""
      continue
    if c == "*":
      if bufDefined:
        if bufOp == "*":
          buf *= int(num)
        elif bufOp == "/":
          buf /= int(num)
      bufDefined = True
      buf = int(num)
      bufOp = "*"
      num = ""
      continue
    num += c

# def _eval(exp):
#   tks = []
#   buf = ""
#   inPar = False
#   for c in exp:
#     if c == ")":
#       tks.append(_eval(buf))
#       continue
#     if inPar:
#       buf += c
#       continue
#     if c == "(":
#       inPar = True
#       continue
#     if c == "+" or c == "-" or c == "*" or c == "/":
#       tks.append(int(buf))
#       tks.append(c)
#       continue
#     if not c.isalpha():
#       print("Invalid character '" + c + "'.")
#       exit()
#     buf += c
#   tks.append(buf)
  
#   tks1 = []
#   for i, tk in enumerate(tks):
#     if c == "*":
#       if i > 0 and i+1 < len(tks):
        

print(_eval(EXP))