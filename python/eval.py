# eval.py – CMDRScript Python Recursive Arithmetic Evaluation
# Modified 2021-02-16

#region Init
DBG = False
#endregion Init

#region itosc
def itosc(num):
  out = ""
  for c in str(num): out += chr(8320+ord(c)-48)
  return out
#endregion itosc

#region _eval
def _eval(exp):
  if exp == "": return 0 # hmmm
  result = 0
  sumBufferStr = ""
  additiveOp = ""
  multiplicativeOp = ""
  productBuffer = 0
  lastOpPosition = -1
  parenLvl = 0
  parenBuffer = ""
  
  
  if DBG: print("\33[34mRESET ('{}')".format(exp))
  
  for i, c in enumerate(exp + "+"):
    
    if DBG: print("\33[0m[" + c + "] ", end="")
    
    
    
    if c == ")":
      if DBG: print("\33[35mparenLvl {} -> {}".format(parenLvl, parenLvl-1))
      parenLvl -= 1
      if not parenLvl:
        if DBG: print("    \33[35mcalling self with parenBuffer")
        sumBufferStr = _eval(parenBuffer)
        parenBuffer = ""
      else:
        parenBuffer += ")"
      continue
    
    
    
    if c == "(":
      if DBG: print("\33[35mparenLvl {} -> {}".format(parenLvl, parenLvl+1))
      if parenLvl: parenBuffer += "("
      parenLvl += 1
      continue
    
    
    
    if parenLvl:
      if DBG: print("\33[95m-> parenBuffer")
      parenBuffer += c
      continue
    
    
    
    if c == "+" or (c == "-" and lastOpPosition != i-1): # and i > 0
      
      lastOpPosition = i
      if DBG: print("\33[93m\33[7mADDITIVE OPERATION\33[27m")
      
      if DBG: print("    multiplicativeOp is '{}'".format(multiplicativeOp))
      
      if multiplicativeOp == "*":
        if DBG: print("      sumBufferStr{} = str(int(sumBufferStr){} * productBuffer{})".format(itosc(int(sumBufferStr)), itosc(int(sumBufferStr)), itosc(productBuffer)), end="")
        sumBufferStr = str(productBuffer*int(sumBufferStr))
        if DBG: print(" = {}".format(sumBufferStr))
        
      elif multiplicativeOp == "/":
        if int(sumBufferStr) == 0: raise ZeroDivisionError
        if DBG: print("      sumBufferStr{} = str(productBuffer{} / int(sumBufferStr){})".format(itosc(int(sumBufferStr)), itosc(productBuffer), itosc(int(sumBufferStr))), end="")
        sumBufferStr = str(productBuffer//int(sumBufferStr))
        if DBG: print(" = {}".format(sumBufferStr))
      
      elif multiplicativeOp == "%":
        if DBG: print("      sumBufferStr{} = str(int(sumBufferStr){} % productBuffer{})".format(itosc(int(sumBufferStr)), itosc(int(sumBufferStr)), itosc(productBuffer)), end="")
        sumBufferStr = str(productBuffer%int(sumBufferStr))
        if DBG: print(" = {}".format(sumBufferStr))
        
      multiplicativeOp = ""
      
      if DBG: print("\33[93m    additiveOp is '{}'".format(additiveOp))
      if DBG: print("      result{} {}= int(sumBufferStr){}".format(itosc(result), additiveOp if additiveOp else "+", itosc(int(sumBufferStr))), end="")
      
      if additiveOp == "-":
        result -= int(sumBufferStr)
        
      else:
        result += int(sumBufferStr)
        
      if DBG: print(" = {}".format(result))
      additiveOp = c
      if DBG: print("    additiveOp is now " + c)
      sumBufferStr = ""
      if DBG: print("    sumBufferStr cleared\33[0m")
      continue
    
    
    
    if c == "*" or c == "/" or c == "%":
      lastOpPosition = i
      if DBG: print("\33[96m\33[7mMULTIPLICATIVE OPERATION\33[27m")
      
      if DBG: print("    multiplicativeOp is '{}'".format(multiplicativeOp))
      if DBG: print("      productBuffer{} {}= int(sumBufferStr){}".format(itosc(productBuffer), multiplicativeOp, itosc(int(sumBufferStr))), end="")
      
      if multiplicativeOp == "*":
        productBuffer *= int(sumBufferStr)
      
      elif multiplicativeOp == "/":
        if int(sumBufferStr) == 0: raise ZeroDivisionError
        productBuffer //= int(sumBufferStr)
        
      elif multiplicativeOp == "%":
        productBuffer %= int(sumBufferStr)
      
      else:
        productBuffer = int(sumBufferStr)
      
      if DBG: print(" = {}".format(productBuffer))
      
      sumBufferStr = ""
      if DBG: print("    sumBufferStr cleared")
      if DBG: print("    multiplicativeOp is now " + c + "\33[0m")
      multiplicativeOp = c
      continue
    
    
    
    if DBG: print("\33[90m-> sumBufferStr\33[0m")
    sumBufferStr += c
    
  if DBG: print("\33[34mRETURN ({})".format(result))
  
  return result
#endregion _eval

if __name__ == "__main__":
  
  DBG = True
  
  EXPS = [ "5%2"]
  amt = len(EXPS)
  for i, exp in enumerate(EXPS):
    print("\33[7m\33[97m{}/{}\33[0m\n".format(i+1, amt, exp), end="")
    _evalres = _eval(exp)
    evalres = int(eval(exp))
    if _evalres == evalres:
      print("\33[92mpassed. (\33[7m{}\33[27m == {})".format(_evalres, evalres))
    else:
      print("\33[91mfailed. (\33[7m{}\33[27m != {})".format(_evalres, evalres))
  print("\33[0m", end="")