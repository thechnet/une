# test-1.py – CMDRScript Python Interpreter Test 1
# Modified 2021-02-16

#region Init
from os import chdir
import pprint

DIR = "U:/Documents/C/Workspaces/CMDRScript/python"
FILE_SCRIPT = "test-1.cmdr"

chdir(DIR)

TKPRINT_PRINT_VALUE = False
PPRINT = False

chars = """
a=1 1"string"1;
b="string""string";
if (a == b);
"""
#endregion Init

# TKPRINT_PRINT_VALUE = True
# PPRINT = True

TOKEN_COLORS = {
  "str": 93,
  "equals": 37,
  "end": 91,
  "num": 94,
  "name": 97,
  "exp": 35,
  "cmpequ": 92
}

#region item_display
def item_display(item):
  if item["type"] in TOKEN_COLORS:
    print("\33[" + str(TOKEN_COLORS[item["type"]]) + "m", end="")
  else:
    print("\33[91m", end="")
  if TKPRINT_PRINT_VALUE:
    print(item["value"], end="")
  else:
    print(item["type"].upper() + " ", end="")
  print("\33[0m", end="")
  if(item["type"] == "end"):
    print("")
#endregion item_display

#region struct_display
def struct_display(struct):
  for thing in struct:
    if thing["type"] == "exp":
      print("\33[35m(", end="")
      if not TKPRINT_PRINT_VALUE:
        print(" ", end="")
      for item in thing["value"]:
        item_display(item)
      print("\33[35m)\33[0m", end="")
      if not TKPRINT_PRINT_VALUE:
        print(" ", end="")
    else:
      item_display(thing)
#endregion struct_display

#region item_append
def item_append(struct, type, value):
  struct.append({ "type": type, "value": value })
#endregion item_append

tktype = "none"
tkbuf = ""
struct = []
grouptype = "none"
groupbuf = []

i = -1
while i + 1 < len(chars):
  i += 1
  c = chars[i]
  if tktype == "str":
    if c == "\"":
      item_append(groupbuf, tktype, tkbuf)
      tktype = "none"
      tkbuf = ""
      continue
    tkbuf += c
    continue
  if c == "\"": # (we know tktype != str because of above)
    if tktype != "none":
      if tktype != "none":
        if grouptype == "none":
          item_append(struct, tktype, tkbuf)
        else:
          item_append(groupbuf, tktype, tkbuf)
    tktype = "str"
    continue
  if c.isnumeric():
    if tktype == "none" or tktype == "num":
      tktype = "num"
      tkbuf += c
      continue
    if tktype == "name": # we already had a char that set it to name, so it can't be the first one
      tkbuf += c
      continue
    print("Unexpected numeric character!")
    exit()
  if c.isalpha() or c == "_":
    if tktype == "none" or tktype == "num" or tktype == "name":
      tktype = "name"
      tkbuf += c
      continue
    print("Unexpected alpha character!")
    exit()
  if c == "=":
    if i+1 < len(chars) and chars[i+1] == '=':
      if grouptype == "none":
        if tktype != "none":
          item_append(struct, tktype, tkbuf)
      else:
        print("Unexpected token '==' in grouptype '" + grouptype + "'!")
        exit()
      i += 1
      tktype = "cmpequ"
      tkbuf += "="
      item_append(struct, tktype, tkbuf)
      tktype = "none"
      tkbuf = ""
      continue
    else:
      if grouptype != "none":
        print("Unexpected character '=' in grouptype '" + grouptype + "'!")
        exit()
        continue
      if tktype != "none":
        item_append(struct, tktype, tkbuf)
      item_append(struct, "equals", "=")
      grouptype = "exp"
      groupbuf = []
      tktype = "none"
      tkbuf = ""
    continue
  if c == ";":
    if tktype != "none":
      if grouptype == "none":
        item_append(struct, tktype, tkbuf)
      else:
        item_append(groupbuf, tktype, tkbuf)
    if grouptype != "none":
      item_append(struct, grouptype, groupbuf)
      grouptype = "none"
      groupbuf = []
    item_append(struct, "end", ";")
    tktype = "none"
    tkbuf = ""
    continue
  if c == "\n" or c == "\r" or c == " ":
    if tktype != "none":
      if grouptype == "none":
        item_append(struct, tktype, tkbuf)
      else:
        item_append(groupbuf, tktype, tkbuf)
    tktype = "none"
    tkbuf = ""
    continue
  print("Unexpected character '" + c + "'!")

if PPRINT:
  pp = pprint.PrettyPrinter()
  pp.pprint(struct)
else:
  struct_display(struct)