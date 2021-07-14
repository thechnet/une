import os
from sys import stdout
dir = "../private/gcov"
bin = "une"

# Options
__HIDE_OUTPUT = True

# Error Values
UNE_C_INPUT = 1
UNE_ET_SYNTAX = 1
UNE_ET_BREAK_OUTSIDE_LOOP = 2
UNE_ET_CONTINUE_OUTSIDE_LOOP = 3
UNE_ET_ASSIGN_TO_LITERAL = 4
UNE_ET_SYMBOL_NOT_DEFINED = 5
UNE_ET_INDEX_OUT_OF_RANGE = 6
UNE_ET_ZERO_DIVISION = 7
UNE_ET_UNREAL_NUMBER = 8
UNE_ET_FUNCTION_ALREADY_DEFINED = 9
UNE_ET_FUNCTION_ARG_COUNT = 10
UNE_ET_FILE_NOT_FOUND = 11
UNE_ET_ENCODING = 12
UNE_ET_TYPE = 13

tests = [
  # builtin.c
  ['int([1])', UNE_ET_TYPE],
  ['flt([1])', UNE_ET_TYPE],
  ['str([1])', UNE_ET_TYPE],
  ['len(1)', UNE_ET_TYPE],
  ['sleep(1.1)', UNE_ET_TYPE],
  ['chr(1.1)', UNE_ET_TYPE],
  ['ord(1)', UNE_ET_TYPE],
  ['read(1)', UNE_ET_TYPE],
  ['read(\\"💥.txt\\")', UNE_C_INPUT],
  ['read(\\"1lkj23\\")', UNE_ET_FILE_NOT_FOUND],
  ['write(1, \\"test\\")', UNE_ET_TYPE],
  ['write(\\"test\\", 1)', UNE_ET_TYPE],
  ['write(\\"💥.txt\\", \\"test\\")', UNE_C_INPUT],
  ['write(\\"\\\\\\\\\\", \\"test\\")', UNE_ET_FILE_NOT_FOUND],
  ['input(1)', UNE_ET_TYPE],
  ['script(1)', UNE_ET_TYPE],
  ['script(\\"💥.une\\")', UNE_C_INPUT],
  ['exist(1)', UNE_ET_TYPE],
  ['exist(\\"💥.une\\")', UNE_C_INPUT],
  ['split(1, [\\".\\"])', UNE_ET_TYPE],
  ['split(\\"1.2.3\\", 1)', UNE_ET_TYPE],
  ['split(\\"1.2.3\\", [1])', UNE_ET_TYPE],
  
  # interpret_as
  ['for i from 1.1', UNE_ET_SYNTAX],
  
  # list
  ['[int([])]', UNE_ET_TYPE],
  
  # cop
  ['unknown ? 1 : 0', UNE_ET_SYMBOL_NOT_DEFINED],
  
  # not
  ['!unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  
  # equ
  ['unknown==1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1==unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  
  # neq
  ['unknown!=1', UNE_ET_SYMBOL_NOT_DEFINED],
  
  # gtr
  ['unknown>1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1>unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]>1', UNE_ET_TYPE],
  
  # geq
  ['unknown>=1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1>=unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]>=1', UNE_ET_TYPE],
  
  # lss
  ['unknown<1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1<unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]<1', UNE_ET_TYPE],
  
  # leq
  ['unknown<=1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1<=unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]<=1', UNE_ET_TYPE],
  
  # add
  ['unknown+1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1+unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1+[1]', UNE_ET_TYPE],
  
  # sub
  ['unknown-1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1-unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1-[1]', UNE_ET_TYPE],
  
  # mul
  ['unknown*1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1*unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]*[1]', UNE_ET_TYPE],
  
  # div
  ['unknown/1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1/unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1/[1]', UNE_ET_TYPE],
  
  # fdiv
  ['unknown//1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1//unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1//[1]', UNE_ET_TYPE],
  
  # mod
  ['unknown%1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1%unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1%[1]', UNE_ET_TYPE],
  
  # pow
  ['unknown**1', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1**unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['1**[1]', UNE_ET_TYPE],
  ['(-1)**1.1', UNE_ET_UNREAL_NUMBER],
  
  # neg
  ['-unknown', UNE_ET_SYMBOL_NOT_DEFINED],
  ['-[1]', UNE_ET_TYPE],
  
  # set
  ['a=[1]**2', UNE_ET_TYPE],
  
  # set_idx
  ['a[0]', UNE_ET_SYMBOL_NOT_DEFINED],
  ['a=1;a[0]', UNE_ET_TYPE],
  ['a=[1];a[[0]]', UNE_ET_TYPE],
  ['a=[1];a[-1]', UNE_ET_INDEX_OUT_OF_RANGE],
  ['a=[1];a[1]', UNE_ET_INDEX_OUT_OF_RANGE],
  ['a=[0];a[0]=[1]**2', UNE_ET_TYPE],
  
  # get
  ['a', UNE_ET_SYMBOL_NOT_DEFINED],
  
  # get_idx
  ['([1]**2)[0]', UNE_ET_TYPE],
  ['[0][[0]]', UNE_ET_TYPE],
  ['1[0]', UNE_ET_TYPE],
  ['[0][1]', UNE_ET_INDEX_OUT_OF_RANGE],
  
  # def
  ['def a(){return};def a(){return}', UNE_ET_FUNCTION_ALREADY_DEFINED],
  
  # call
  ['a()', UNE_ET_SYMBOL_NOT_DEFINED],
  ['def a(b){return b};a()', UNE_ET_FUNCTION_ARG_COUNT],
  ['def a(b){return b};a([1]**2)', UNE_ET_TYPE],
  
  # for
  ['for i from [1]**2 till 3 print(i)', UNE_ET_TYPE],
  ['for i from 0 till [1]**2 print(i)', UNE_ET_TYPE],
  ['for i from 0 till 3 print([i]**2)', UNE_ET_TYPE],
  
  # while
  ['i=3;while [i]**2 i=i-1', UNE_ET_TYPE],
  ['i=3;while i>0{[i]**2;i=i-1}', UNE_ET_TYPE],
  
  # if
  ['if [1]**2 1', UNE_ET_TYPE],
]

cd = os.getcwd()
os.chdir(dir)
print("\33[33m\33[1mEnsure UNE_DEBUG_RETURN_ERROR_TYPE is enabled.\33[0m")
for i, test in enumerate(tests):
  error_type = os.system(("1>nul 2>&1 " if __HIDE_OUTPUT else "") + bin + " -s \"" + test[0] + "\"")
  if error_type == test[1]:
    print("\33[32m[" + str(i+1) + "] Passed { "+test[0]+" }\33[0m")
  else:
    print("\33[31m[" + str(i+1) + "] Failed { "+test[0]+" }\33[0m")
os.chdir(cd)
