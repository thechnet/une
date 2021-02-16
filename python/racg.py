# racg.py – CMDRScript Python Random Arithmetic Calculation Generator
# Modified 2021-02-16

#region Init
from eval import _eval
from random import randint, choice
from math import floor, ceil
from subprocess import run
from os import chdir
USE_C_FOR_VERIFICATION = False
C_IMMEDIATE_RUN = "c.cmd"
VERBOSE = False
#endregion Init

# Python sometimes behaves differently than expected:
# 1//2 = 0 ✓, however, -1//2 = -1 ╳
# USE_C_FOR_VERIFICATION = True

FILE_FAILED = "U:/Desktop/CMDRScript_racg.py_failed.txt"
# VERBOSE = True

OPS = ["+", "-", "*", "/", "%"]
NUM_MIN = -10
NUM_MAX = 10
PARTS_PER_SEQUENCE = 20
SEQUENCES = 10000
MAX_PAREN_LVL = 5

print("\33[90mnum: {}-{}, seq: {}, parts per seq: {}, max paren lvl: {}\33[0m".format(NUM_MIN, NUM_MAX, SEQUENCES, PARTS_PER_SEQUENCE, MAX_PAREN_LVL))
if USE_C_FOR_VERIFICATION: print("\33[93m(using c verification.)\33[0m")
if not VERBOSE:
  print("\33[s", end="")
success = fail = skip = 0
for i, seq in enumerate(range(0, SEQUENCES, 1)):
  sequence = str(randint(NUM_MIN, NUM_MAX))
  parenLvl = 0
  for part in range(0, PARTS_PER_SEQUENCE-1, 1):
    num = randint(NUM_MIN, NUM_MAX)
    op = choice(OPS)
    if op == "/" and num == 0: num = 1 # Prevent division by zero.
    paren = randint(0, 4)
    sequence += op
    if paren == 3 and parenLvl < MAX_PAREN_LVL:
      sequence += "("
      parenLvl += 1
      paren = False
    sequence += str(num)
    if paren == 4 and parenLvl > 0:
      sequence += ")"
      parenLvl -= 1
  for j in range(0, parenLvl, 1):
    parenLvl -=1
    sequence += ")"
  if not VERBOSE and not i%(SEQUENCES//10):
    print("\33[u{}%".format(round(i/SEQUENCES*100)))
  try:
    _evalres = _eval(sequence)
  except ZeroDivisionError:
    if VERBOSE: print("{}/{} \33[90mdivision by zero detected in _eval, skipping...\33[0m".format(i+1, SEQUENCES))
    skip += 1
    continue
  if USE_C_FOR_VERIFICATION:
    evalres = int(run(
        C_IMMEDIATE_RUN + " printf(\"\%d\",{});".format(
          sequence.replace("--", "+")
        ), capture_output=True
      ).stdout)
  else:
    try:
      evalres = eval(sequence.replace("/", "//")) # Integer division.
    except ZeroDivisionError:
      if VERBOSE: print("{}/{} \33[90mdivision by zero detected in eval, skipping...\33[0m".format(i+1, SEQUENCES))
      skip += 1
      continue
    if evalres >= 0: evalres = floor(evalres)
    else: evalres = ceil(evalres)
  if VERBOSE:
    print("{}/{} \33[94m'{}': ".format(i+1, SEQUENCES, sequence), end="")
  if _evalres == evalres:
    success += 1
    if VERBOSE: print("\33[92mpassed. ({} == {})\33[0m".format(_evalres, evalres))
  else:
    fail += 1
    if VERBOSE: print("\33[91mfailed. ({} != {})\33[0m".format(_evalres, evalres))
    f = open(FILE_FAILED, "a")
    f.write("{}\n_evalres: {}\nevalres: {}\n\n".format(sequence, _evalres, evalres))
    f.close()

if not VERBOSE:
  print("\33[u", end="")
print("\33[92m{} passed\33[0m, \33[{}m{} failed\33[0m, \33[92m{} skipped\33[0m".format(success, 91 if fail else 92, fail, skip))