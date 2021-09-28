#!python3
import os
from os import system as cmd
from sys import platform

def is_win():
  return platform=='win32' or platform=='cygwin' or platform=='msys'

def is_cmd_exe():
  return platform=='win32'

# Options
SKIP_UNTIL = 0 # 0/2
HIDE_OUTPUT = True
CLEAR = True
STOP_AT_FAIL = True
ENUMERATE_TESTS = False
FILE_SCRIPT = 'test.py.une'
DIR = '.'
UNE = '..\\\\une' if is_win() else '../une'
FILE_RETURN = 'une_report_return.txt'
FILE_STATUS = 'une_report_status.txt'
UNE_R_END_DATA_RESULT_TYPES = 11

# UNCOMMENT THESE LINES WHEN USING GCOV:
# DIR = '..\\private\\gcov' if is_win() else '../private/gcov'
# UNE = 'une' if is_win() else './une'

##### CONSTANTS

ATTR_DIRECT_ARG = 1
ATTR_FILE_ONLY = 2
ATTR_NO_IMPLICIT_RETURN = 3
ATTR_NEVER_HIDE_OUTPUT = 4

# Result Types
UNE_RT_ERROR = 1
UNE_RT_VOID = 5
UNE_RT_INT = 6
UNE_RT_FLT = 7
UNE_RT_STR = 8
UNE_RT_LIST = 9
UNE_RT_FUNCTION = 10
UNE_RT_BUILTIN = 11
result_types = {
  UNE_RT_ERROR: 'UNE_RT_ERROR',
  UNE_RT_VOID: 'UNE_RT_VOID',
  UNE_RT_INT: 'UNE_RT_INT',
  UNE_RT_FLT: 'UNE_RT_FLT',
  UNE_RT_STR: 'UNE_RT_STR',
  UNE_RT_LIST: 'UNE_RT_LIST',
  UNE_RT_FUNCTION: 'UNE_RT_FUNCTION',
  UNE_RT_BUILTIN: 'UNE_RT_BUILTIN',
}

# Error Types
UNE_ERROR_INPUT = 1
UNE_ET_SYNTAX = UNE_R_END_DATA_RESULT_TYPES+1
UNE_ET_BREAK_OUTSIDE_LOOP = UNE_R_END_DATA_RESULT_TYPES+2
UNE_ET_CONTINUE_OUTSIDE_LOOP = UNE_R_END_DATA_RESULT_TYPES+3
UNE_ET_SYMBOL_NOT_DEFINED = UNE_R_END_DATA_RESULT_TYPES+4
UNE_ET_INDEX_OUT_OF_RANGE = UNE_R_END_DATA_RESULT_TYPES+5
UNE_ET_ZERO_DIVISION = UNE_R_END_DATA_RESULT_TYPES+6
UNE_ET_UNREAL_NUMBER = UNE_R_END_DATA_RESULT_TYPES+7
UNE_ET_CALLABLE_ARG_COUNT = UNE_R_END_DATA_RESULT_TYPES+8
UNE_ET_FILE_NOT_FOUND = UNE_R_END_DATA_RESULT_TYPES+9
UNE_ET_ENCODING = UNE_R_END_DATA_RESULT_TYPES+10
UNE_ET_TYPE = UNE_R_END_DATA_RESULT_TYPES+11
error_types = {
  UNE_ERROR_INPUT: 'UNE_ERROR_INPUT',
  UNE_ET_SYNTAX: 'UNE_ET_SYNTAX',
  UNE_ET_BREAK_OUTSIDE_LOOP: 'UNE_ET_BREAK_OUTSIDE_LOOP',
  UNE_ET_CONTINUE_OUTSIDE_LOOP: 'UNE_ET_CONTINUE_OUTSIDE_LOOP',
  UNE_ET_SYMBOL_NOT_DEFINED: 'UNE_ET_SYMBOL_NOT_DEFINED',
  UNE_ET_INDEX_OUT_OF_RANGE: 'UNE_ET_INDEX_OUT_OF_RANGE',
  UNE_ET_ZERO_DIVISION: 'UNE_ET_ZERO_DIVISION',
  UNE_ET_UNREAL_NUMBER: 'UNE_ET_UNREAL_NUMBER',
  UNE_ET_CALLABLE_ARG_COUNT: 'UNE_ET_CALLABLE_ARG_COUNT',
  UNE_ET_FILE_NOT_FOUND: 'UNE_ET_FILE_NOT_FOUND',
  UNE_ET_ENCODING: 'UNE_ET_ENCODING',
  UNE_ET_TYPE: 'UNE_ET_TYPE'
}

##### TESTS

tests = [
  
  # Built-in Commands
  
  ['input("Write \'test\' and press enter: ")', UNE_RT_STR, 'test', ATTR_NEVER_HIDE_OUTPUT],
  ['input(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['print(100.9)', UNE_RT_VOID, 'VOID'],
  ['print("str")', UNE_RT_VOID, 'VOID'],
  ['print([1])', UNE_RT_VOID, 'VOID'],
  ['print([1, "str"])', UNE_RT_VOID, 'VOID'],
  
  ['put(1)', UNE_RT_VOID, 'VOID'],
  
  ['int(100)', UNE_RT_INT, '100'],
  ['int(100.9)', UNE_RT_INT, '100'],
  ['int("100")', UNE_RT_INT, '100'],
  ['int([1])', UNE_RT_ERROR, UNE_ET_TYPE],
  ['int("100x")', UNE_RT_ERROR, UNE_ET_ENCODING],
  
  ['flt(100)', UNE_RT_FLT, '100.000'],
  ['flt(100.9)', UNE_RT_FLT, '100.900'],
  ['flt("100.9")', UNE_RT_FLT, '100.900'],
  ['flt([1])', UNE_RT_ERROR, UNE_ET_TYPE],
  ['flt("100.9x")', UNE_RT_ERROR, UNE_ET_ENCODING],
  
  ['str(1)', UNE_RT_STR, '1'],
  ['str(1.1)', UNE_RT_STR, '1.100'],
  ['str("str")', UNE_RT_STR, 'str'],
  ['str([1])', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['len("str")', UNE_RT_INT, '3'],
  ['len([1, 2, 3])', UNE_RT_INT, '3'],
  ['len(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['sleep(1)', UNE_RT_VOID, 'VOID'],
  ['sleep(1.1)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['chr(65)', UNE_RT_STR, 'A'],
  ['chr(1.1)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['chr(999999999999)', UNE_RT_ERROR, UNE_ET_ENCODING],
  
  ['ord("A")', UNE_RT_INT, '65'],
  ['ord(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['write("script.une", "return 46")', UNE_RT_VOID, 'VOID'],
  ['write(1, "test")', UNE_RT_ERROR, UNE_ET_TYPE],
  ['write("test", 1)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['write("/", "test")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND],
  
  ['read("script.une")', UNE_RT_STR, 'return 46'],
  ['read(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['read("1lkj23")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND],
  
  ['script("script.une")', UNE_RT_INT, '46'],
  ['script(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['script("/")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND],
  
  ['exist("script.une")', UNE_RT_INT, '1'],
  ['exist(1)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  ['split("1.2.3", ["."])', UNE_RT_LIST, '["1", "2", "3"]'],
  ['split(1, ["."])', UNE_RT_ERROR, UNE_ET_TYPE],
  ['split("1.2.3", 1)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['split("1.2.3", [1])', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # main
  ['unknown', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND, ATTR_DIRECT_ARG],
  ['', UNE_RT_ERROR, UNE_ERROR_INPUT, ATTR_DIRECT_ARG],
  ['-s', UNE_RT_ERROR, UNE_ERROR_INPUT, ATTR_DIRECT_ARG],
  
  # Syntax
  ['\r# comment', UNE_RT_VOID, 'VOID'],
  ['"\\\\\\"\\n\\\n"', UNE_RT_STR, '\\"\n', ATTR_FILE_ONLY],
  ['\r1 $', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['1.', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['"ab', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['"\\a"', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['def 1', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['1 ? +', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['1 ? 1,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['1 ? 1 : +', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['[1', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['a(', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['(1', UNE_RT_ERROR, UNE_ET_SYNTAX],
  [',', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['def a(', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['def a()', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i 1', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i from ,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i from 0 ,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i from 0 till', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i from 0 till 1 ,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['while 1 {+}', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['if 1 )', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['if 1 1 else', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['continue', UNE_RT_ERROR, UNE_ET_CONTINUE_OUTSIDE_LOOP],
  ['break', UNE_RT_ERROR, UNE_ET_BREAK_OUTSIDE_LOOP],
  ['a[', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['a=a[1,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['a[1,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['a=*', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['1+)', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['def a', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['[1,', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['[1, +]', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['[1 1]', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['for i from 1.1', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['a=function(a, 1){return a}', UNE_RT_ERROR, UNE_ET_SYNTAX],
  ['function()', UNE_RT_ERROR, UNE_ET_SYNTAX],
  
  # Data
  ['100', UNE_RT_INT, '100'],
  ['100.9', UNE_RT_FLT, '100.900'],
  ['[]', UNE_RT_LIST, '[]'],
  ['[1]', UNE_RT_LIST, '[1]'],
  ['[1, 2]', UNE_RT_LIST, '[1, 2]'],
  ['[int([])]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['"str"', UNE_RT_STR, 'str'],
  
  # COP
  ['1 ? 2 : 3', UNE_RT_INT, '2'],
  ['0.0 ? 1 : 2', UNE_RT_INT, '2'],
  ['"str" ? 1 : 2', UNE_RT_INT, '1'],
  ['[] ? 1 : 2', UNE_RT_INT, '2'],
  ['unknown ? 1 : 0', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # NOT
  ['!0', UNE_RT_INT, '1'],
  ['!unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # AND
  ['1&&1', UNE_RT_INT, '1'],
  ['1&&0', UNE_RT_INT, '0'],
  ['0&&1', UNE_RT_INT, '0'],
  ['unknown&&1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # OR
  ['0||0', UNE_RT_INT, '0'],
  ['1||0', UNE_RT_INT, '1'],
  ['0||1', UNE_RT_INT, '1'],
  ['unknown||1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # EQU
  ['1==1', UNE_RT_INT, '1'],
  ['1==1.0', UNE_RT_INT, '1'],
  ['1.0==1', UNE_RT_INT, '1'],
  ['1.0==1.0', UNE_RT_INT, '1'],
  ['"str"=="str"', UNE_RT_INT, '1'],
  ['[1]==[1]', UNE_RT_INT, '1'],
  ['unknown==1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1==unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # NEQ
  ['1!=1', UNE_RT_INT, '0'],
  ['unknown!=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # GTR
  ['2>1', UNE_RT_INT, '1'],
  ['2>1.0', UNE_RT_INT, '1'],
  ['2.0>1', UNE_RT_INT, '1'],
  ['2.0>1.0', UNE_RT_INT, '1'],
  ['"string">"str"', UNE_RT_INT, '1'],
  ['[1,2,3]>[1,2]', UNE_RT_INT, '1'],
  ['unknown>1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1>unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]>1', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # GEQ
  ['1>=1', UNE_RT_INT, '1'],
  ['1>=1.0', UNE_RT_INT, '1'],
  ['1.0>=1', UNE_RT_INT, '1'],
  ['1.0>=1.0', UNE_RT_INT, '1'],
  ['"str">="str"', UNE_RT_INT, '1'],
  ['[1,2]>=[1,2]', UNE_RT_INT, '1'],
  ['unknown>=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1>=unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]>=1', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # LSS
  ['2<1', UNE_RT_INT, '0'],
  ['2<1.0', UNE_RT_INT, '0'],
  ['2.0<1', UNE_RT_INT, '0'],
  ['2.0<1.0', UNE_RT_INT, '0'],
  ['"string"<"str"', UNE_RT_INT, '0'],
  ['[1,2,3]<[1,2]', UNE_RT_INT, '0'],
  ['unknown<1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1<unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]<1', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # LEQ
  ['1<=1', UNE_RT_INT, '1'],
  ['1<=1.0', UNE_RT_INT,'1'],
  ['1.0<=1', UNE_RT_INT, '1'],
  ['1.0<=1.0', UNE_RT_INT, '1'],
  ['"str"<="str"', UNE_RT_INT, '1'],
  ['[1,2]<=[1,2]', UNE_RT_INT, '1'],
  ['unknown<=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1<=unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]<=1', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # ADD
  ['1+1', UNE_RT_INT, '2'],
  ['1+1.0', UNE_RT_FLT, '2.000'],
  ['1.0+1', UNE_RT_FLT, '2.000'],
  ['1.0+1.0', UNE_RT_FLT, '2.000'],
  ['"str"+"str"', UNE_RT_STR, 'strstr'],
  ['[1]+[2]', UNE_RT_LIST, '[1, 2]'],
  ['unknown+1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1+unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1+[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1.0+"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  ['[1]+1', UNE_RT_ERROR, UNE_ET_TYPE],
  ['"str"+1', UNE_RT_ERROR, UNE_ET_TYPE],
  ['"str"*"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # SUB
  ['1-1', UNE_RT_INT, '0'],
  ['1-1.0', UNE_RT_FLT, '0.000'],
  ['1.0-1', UNE_RT_FLT, '0.000'],
  ['1.0-1.0', UNE_RT_FLT, '0.000'],
  ['unknown-1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1-unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1-[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1.0-"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # MUL
  ['2*2', UNE_RT_INT, '4'],
  ['2*2.5', UNE_RT_FLT, '5.000'],
  ['2.5*2', UNE_RT_FLT, '5.000'],
  ['2.5*2.0', UNE_RT_FLT, '5.000'],
  ['3*"str"', UNE_RT_STR, 'strstrstr'],
  ['"string"*3', UNE_RT_STR, 'stringstringstring'],
  ['3*[1]', UNE_RT_LIST, '[1, 1, 1]'],
  ['[2]*3', UNE_RT_LIST, '[2, 2, 2]'],
  ['unknown*1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1*unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['[1]*[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1.0*"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1*int', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # DIV
  ['6/2', UNE_RT_INT, '3'],
  ['1/2', UNE_RT_FLT, '0.500'],
  ['1/4.0', UNE_RT_FLT, '0.250'],
  ['4.0/1', UNE_RT_FLT, '4.000'],
  ['1.0/3.0', UNE_RT_FLT, '0.333'],
  ['unknown/1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1/unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1/[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1/0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['1/0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['1.0/"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # FDIV
  ['1//3', UNE_RT_INT, '0'],
  ['5//2.0', UNE_RT_INT, '2'],
  ['0.1//1', UNE_RT_INT, '0'],
  ['6.0//4.0', UNE_RT_INT, '1'],
  ['unknown//1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1//unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1//[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1//0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['1//0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['1.0//"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1.0/0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['1.0/0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  
  # MOD
  ['3%2', UNE_RT_INT, '1'],
  ['5.5%2', UNE_RT_FLT, '1.500'],
  ['5%3.5', UNE_RT_FLT, '1.500'],
  ['5.5%3.5', UNE_RT_FLT, '2.000'],
  ['unknown%1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1%unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1%[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1.0%"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # POW
  ['3**3', UNE_RT_INT, '27'],
  ['4**0.5', UNE_RT_FLT, '2.000'],
  ['0.8**3', UNE_RT_FLT, '0.512'],
  ['4.0**2.0', UNE_RT_FLT, '16.000'],
  ['unknown**1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1**unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['1**[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['(-1)**1.1', UNE_RT_ERROR, UNE_ET_UNREAL_NUMBER],
  ['1.0**"str"', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # NEG
  ['--100', UNE_RT_INT, '100'],
  ['--100.9', UNE_RT_FLT, '100.900'],
  ['-unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['-[1]', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # SET
  ['a=46;return a', UNE_RT_INT, '46', ATTR_NO_IMPLICIT_RETURN],
  ['global a=46;return a', UNE_RT_INT, '46', ATTR_NO_IMPLICIT_RETURN],
  ['a=[1]**2', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # SET_IDX
  ['a=[0];a[0]=1;return a', UNE_RT_LIST, '[1]', ATTR_NO_IMPLICIT_RETURN],
  ['a=[0];global a[0]=1;return a', UNE_RT_LIST, '[1]', ATTR_NO_IMPLICIT_RETURN],
  ['a=[0];a[0]=[1]**2', UNE_RT_ERROR, UNE_ET_TYPE],
  ['a[0]=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['a=1;a[0]=1', UNE_RT_ERROR, UNE_ET_TYPE],
  ['a=[0];a[[1]]=1', UNE_RT_ERROR, UNE_ET_TYPE],
  ['a=[0];a[-1]=1', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  ['a=[0];a[1]=1', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  
  # GET
  ['a=46;return a', UNE_RT_INT, '46', ATTR_NO_IMPLICIT_RETURN],
  ['a', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  
  # GET_IDX
  ['[46][0]', UNE_RT_INT, '46'],
  ['"str"[0]', UNE_RT_STR, 's'],
  ['a[0]', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['a="str";a[1/0]', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION],
  ['([1]**2)[0]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['[0][[0]]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1[0]', UNE_RT_ERROR, UNE_ET_TYPE],
  ['[0][-1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  ['[0][1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  ['"a"[-1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  ['"a"[1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE],
  
  # FOR
  ['a=0;for i from 0 till 3 a=a+i;return a', UNE_RT_INT, '3', ATTR_NO_IMPLICIT_RETURN],
  ['a=0;for i from 3 till 0{if i==2 continue;a=a+i};return a', UNE_RT_INT, '4', ATTR_NO_IMPLICIT_RETURN],
  ['a=0;for i from 0 till 0 a=1;return a', UNE_RT_INT, '0', ATTR_NO_IMPLICIT_RETURN],
  ['a=0;for i from 0 till 1{break;a=1};return a', UNE_RT_INT, '0', ATTR_NO_IMPLICIT_RETURN],
  ['for i from [1]**2 till 3 print(i)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['for i from [1] till 3 print(i)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['for i from 0 till [1]**2 print(i)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['for i from 0 till 3 print([i]**2)', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # WHILE
  ['i=3;while i>0 i=i-1;return i', UNE_RT_INT, '0', ATTR_NO_IMPLICIT_RETURN],
  ['i=0;while i>0{i=-1;break};return i', UNE_RT_INT, '0', ATTR_NO_IMPLICIT_RETURN],
  ['i=3;while [i]**2 i=i-1', UNE_RT_ERROR, UNE_ET_TYPE],
  ['i=3;while i>0{[i]**2;i=i-1}', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # IF
  ['if 1 return 1', UNE_RT_INT, '1', ATTR_NO_IMPLICIT_RETURN],
  ['if 0 return 0;return -1', UNE_RT_INT, '-1', ATTR_NO_IMPLICIT_RETURN],
  ['if 1 return 1 elif 0 return 0 else return 0', UNE_RT_INT, '1', ATTR_NO_IMPLICIT_RETURN],
  ['if 0 return 0 elif 2 return 2 else return 0', UNE_RT_INT, '2', ATTR_NO_IMPLICIT_RETURN],
  ['if 0 return 0 elif 0 return 0 else return 3', UNE_RT_INT, '3', ATTR_NO_IMPLICIT_RETURN],
  ['if 0 return 0 elif 0 return 0;return', UNE_RT_VOID, 'VOID', ATTR_NO_IMPLICIT_RETURN],
  ['if [1]**2 1', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # CALLABLES
  ['fn=function(arg){a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]}', UNE_RT_VOID, 'VOID', ATTR_NO_IMPLICIT_RETURN],
  ['a=function(){return};a=function(){return}', UNE_RT_VOID, 'VOID', ATTR_NO_IMPLICIT_RETURN],
  ['int=function(){return}', UNE_RT_ERROR, UNE_ET_SYNTAX],
  
  # CALL
  ['fn=function(arg){a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]};return fn(2)', UNE_RT_LIST, '[2.200, "str"]', ATTR_NO_IMPLICIT_RETURN],
  ['a()', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED],
  ['a=function(b)return b;c=function(d)return a();c(1)', UNE_RT_ERROR, UNE_ET_CALLABLE_ARG_COUNT],
  ['a=function(b, c){return b};a(1, [1]**2)', UNE_RT_ERROR, UNE_ET_TYPE],
  ['1()', UNE_RT_ERROR, UNE_ET_TYPE],
  
  # DATATYPES
  ['print(int)', UNE_RT_VOID, 'VOID'],
  ['if int{return 1}else{return 0}', UNE_RT_INT, '1', ATTR_NO_IMPLICIT_RETURN],
  ['print(function(){})', UNE_RT_VOID, 'VOID'],
  ['if function(){}{return 1}else{return 0}', UNE_RT_INT, '1', ATTR_NO_IMPLICIT_RETURN],
  ['if print(1){return 1}else{return 0}', UNE_RT_INT, '0', ATTR_NO_IMPLICIT_RETURN],
  
  # ERROR DISPLAY
  ['1\n2\n3/0\n4', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, ATTR_FILE_ONLY],
  
  # ORDER OF OPERATIONS
  ['-2**2', UNE_RT_INT, '-4'],
  ['a=[2];return -a[0]**2', UNE_RT_INT, '-4', ATTR_NO_IMPLICIT_RETURN],
  ['a=function()return 2;return -a()**2', UNE_RT_INT, '-4', ATTR_NO_IMPLICIT_RETURN],
  
  # COPYING FUNCTION NODE
  ['function(){function(){}}', UNE_RT_FUNCTION, 'FUNCTION'],
]
TESTS_LEN = len(tests)

##### PROGRAM

def dict_get(dictionary, value):
  if not value in dictionary:
    return value
  return dictionary.get(value)

def check_report(type, test, i):
  passed = True
  with open(FILE_STATUS, mode='r', encoding='utf-8-sig') as report_status:
    for line in report_status:
      if line.startswith('result_type:'):
        result_type = int(line.split(':')[1])
        result_type_want = dict_get(result_types, test[1])
        result_type_have = dict_get(result_types, result_type)
        if result_type != test[1]:
          print(f'\33[0m[{i}/{TESTS_LEN}] \33[31m\33[1m<{type}> RSTYPE (\33[7m{result_type_have}\33[27m != {result_type_want})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      elif line.startswith('alloc_count:'):
        alloc_count = int(line.split(':')[1])
        if alloc_count != 0:
          print(f'\33[0m[{i}/{TESTS_LEN}] \33[31m\33[1m<{type}> ALLOCS ({alloc_count})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      elif line.startswith('alert_count:'):
        alert_count = int(line.split(':')[1])
        if alert_count != 0:
          print(f'\33[0m[{i}/{TESTS_LEN}] \33[31m\33[1m<{type}> ALERTS ({alert_count})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      else:
        print('Internal error.')
        exit(1)
  with open(FILE_RETURN, mode='r', encoding='utf-8-sig') as report_return:
    return_value = report_return.read()
    if return_value != str(test[2]):
      if result_type == UNE_RT_ERROR:
        return_value_want = dict_get(error_types, test[2])
        return_value_have = dict_get(error_types, int(return_value))
      else:
        return_value_want = f'\'{str(test[2])}\''
        return_value_have = f'\'{return_value}\''
      print(f'\33[0m[{i}/{TESTS_LEN}] \33[31m\33[1m<{type}> RETURN (\33[7m{return_value_have}\33[27m != {return_value_want})\33[0m')
      passed = False
  return passed

if CLEAR:
  cmd('cls' if is_cmd_exe() else 'clear')
  
if ENUMERATE_TESTS:
  for i, j in enumerate(tests):
    print(i, j)

cd = os.getcwd()
os.chdir(DIR)
print("\33[33m\33[1mEnsure UNE_DEBUG_SIZES is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_REPORT is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_MEMDBG is enabled.\33[0m")
i = 0
for i, test in enumerate(tests):
  if i+1 < SKIP_UNTIL:
    continue
  if HIDE_OUTPUT and not (len(test) > 3 and test[3] == ATTR_NEVER_HIDE_OUTPUT):
    une = '1>' + ('nul' if is_cmd_exe() else '/dev/null') + f' 2>&1 {UNE}'
  else:
    une = UNE
  i += 1
  passed = True
  if len(test) > 3 and test[3] == ATTR_DIRECT_ARG:
    cmd(f'{une} {test[0]}')
    if not check_report('main', test, i):
      passed = False
  else:
    if len(test) <= 3 or test[3] != ATTR_FILE_ONLY:
      sanitized = test[0].replace('\\', '\\\\')
      sanitized = sanitized.replace('"', '\\"')
      sanitized = sanitized.replace('\n', '\\n')
      if test[1] != UNE_RT_ERROR and (len(test) <= 3 or test[3] != ATTR_NO_IMPLICIT_RETURN):
        sanitized = f'return {sanitized}'
      command = f'{une} -s "{sanitized}"'
      cmd(command)
      if not check_report('cmdl', test, i):
        passed = False
        print(f'\33[31m{command}\33[0m')
    if passed or not STOP_AT_FAIL:
      script = test[0]
      if test[1] != UNE_RT_ERROR and (len(test) <= 3 or test[3] != ATTR_NO_IMPLICIT_RETURN):
        script = f'return {script}'
      status = open(FILE_SCRIPT, 'w')
      status.write(script)
      status.close()
      cmd(f'{une} "{FILE_SCRIPT}"')
      if not check_report('file', test, i):
        passed = False
        print(f'\33[31m{script}\33[0m')
  if passed:
    print(f'\33[0m[{i}/{TESTS_LEN}] \33[32mPassed\33[0m')
  elif STOP_AT_FAIL:
    print('\33[33m(test.py) Failed, stopping.\33[0m')
    break
else:
  print('\33[32m\33[1m(test.py) ALL TESTS PASSED\33[0m')
  exit(0)
exit(1)
