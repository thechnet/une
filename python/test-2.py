# test-2.py – CMDRScript Python Interpreter Test 2
# Modified 2021-02-16

#region Init
from os import chdir
import pprint

DIR = "U:/Documents/C/Workspaces/CMDRScript/python"
FILE_SCRIPT = "test-2.cmdr"

chdir(DIR)

TKPRINT_PRINT_VALUE = False
PPRINT = False

chars = """
set a=1
if (a==1)
"""
#endregion Init

commandDefined = False
command = ""
buf = ""

i = -1
while i + 1 < len(chars):
  
  i += 1
  c = chars[i]
  
  if c == " ":
    if not commandDefined:
      commandDefined = True
      command = buf
      continue
  
  
  
  buf += c