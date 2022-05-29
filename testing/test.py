#!python3
import os
from os import system as cmd
from sys import platform

def is_win():
  return platform=='win32' or platform=='cygwin' or platform=='msys'

def is_cmd_exe():
  return platform=='win32'

class Case:
  def __init__(self, input, result_type, result_value, attributes):
    self.input = input
    self.result_type = result_type
    self.result_value = result_value
    self.attributes = attributes

# Options
SKIP_UNTIL = 0 # 0, 2
HIDE_OUTPUT = True
CLEAR = True
STOP_AT_FAIL = True
ENUMERATE_CASES = False
FILE_SCRIPT = 'test.py.une'
DIR = '.'
UNE = '..\\\\une.exe' if is_win() else '../une'
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
ATTR_NO_SECOND_ESCAPE = 5

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

##### CASES

cases = [
  
  # Built-in Commands
  
  Case('input("Write \'test\' and press enter: ")', UNE_RT_STR, 'test', [ATTR_NEVER_HIDE_OUTPUT]),
  Case('input(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('input()', UNE_RT_ERROR, UNE_ET_CALLABLE_ARG_COUNT, []),
  
  Case('print(100.9)', UNE_RT_VOID, 'VOID', []),
  Case('print("str")', UNE_RT_VOID, 'VOID', []),
  Case('print([1])', UNE_RT_VOID, 'VOID', []),
  Case('print([1, "str"])', UNE_RT_VOID, 'VOID', []),
  
  Case('put(1)', UNE_RT_VOID, 'VOID', []),
  
  Case('int(100)', UNE_RT_INT, '100', []),
  Case('int(100.9)', UNE_RT_INT, '100', []),
  Case('int("100")', UNE_RT_INT, '100', []),
  Case('int([1])', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('int("100x")', UNE_RT_ERROR, UNE_ET_ENCODING, []),
  
  Case('flt(100)', UNE_RT_FLT, '100.000', []),
  Case('flt(100.9)', UNE_RT_FLT, '100.900', []),
  Case('flt("100.9")', UNE_RT_FLT, '100.900', []),
  Case('flt([1])', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('flt("100.9x")', UNE_RT_ERROR, UNE_ET_ENCODING, []),
  
  Case('str(1)', UNE_RT_STR, '1', []),
  Case('str(1.1)', UNE_RT_STR, '1.100', []),
  Case('str("str")', UNE_RT_STR, 'str', []),
  Case('str([1])', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  Case('len("str")', UNE_RT_INT, '3', []),
  Case('len([1, 2, 3])', UNE_RT_INT, '3', []),
  Case('len(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  Case('sleep(1)', UNE_RT_VOID, 'VOID', []),
  Case('sleep(1.1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  Case('chr(65)', UNE_RT_STR, 'A', []),
  Case('chr(1.1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('chr(999999999999)', UNE_RT_ERROR, UNE_ET_ENCODING, []),
  
  Case('ord("A")', UNE_RT_INT, '65', []),
  Case('ord(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  Case('write("script.une", "return 46")', UNE_RT_VOID, 'VOID', []),
  Case('write(1, "test")', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('write("test", 1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('write("/", "test")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND, []),
  
  Case('read("script.une")', UNE_RT_STR, 'return 46', []),
  Case('read(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('read("1lkj23")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND, []),
  
  Case('script("script.une")', UNE_RT_INT, '46', []),
  Case('script(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('script("/")', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND, []),
  Case('write("script.une", "msg=128");script("script.une");return msg', UNE_RT_INT, '128', [ATTR_NO_IMPLICIT_RETURN]),
  
  Case('write("script.une", "4");append("script.une", "\n6");return read("script.une")', UNE_RT_STR, '4\n6', [ATTR_NO_IMPLICIT_RETURN]),
  
  Case('exist("script.une")', UNE_RT_INT, '1', []),
  Case('exist(1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  Case('split("1.2.3", ["."])', UNE_RT_LIST, '["1", "2", "3"]', []),
  Case('split(1, ["."])', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('split("1.2.3", 1)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('split("1.2.3", [1])', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # main
  Case('unknown', UNE_RT_ERROR, UNE_ET_FILE_NOT_FOUND, [ATTR_DIRECT_ARG]),
  Case('', UNE_RT_ERROR, UNE_ERROR_INPUT, [ATTR_DIRECT_ARG]),
  Case('-s', UNE_RT_ERROR, UNE_ERROR_INPUT, [ATTR_DIRECT_ARG]),
  
  # Syntax
  Case('\r# comment', UNE_RT_VOID, 'VOID', []),
  Case('"\\\\\\"\\n\\\n"', UNE_RT_STR, '\\"\n', [ATTR_FILE_ONLY]),
  Case('\r1 $', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('1.', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('"ab', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('"\\a"', UNE_RT_ERROR, UNE_ET_SYNTAX, [ATTR_NO_SECOND_ESCAPE]),
  Case('def 1', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('1 ? +', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('1 ? 1,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('1 ? 1 : +', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('[1', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('a(', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('(1', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case(',', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('def a(', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('def a()', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i 1', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i from ,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i from 0 ,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i from 0 till', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i from 0 till 1 ,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('while 1 {+}', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('if 1 )', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('if 1 1 else', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('continue', UNE_RT_ERROR, UNE_ET_CONTINUE_OUTSIDE_LOOP, []),
  Case('break', UNE_RT_ERROR, UNE_ET_BREAK_OUTSIDE_LOOP, []),
  Case('a[', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('a=a[1,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('a[1,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('a=*', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('1+)', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('def a', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('[1,', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('[1, +]', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('[1 1]', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('for i from 1.1', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('a=function(a, 1){return a}', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  Case('function()', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  
  # Data
  Case('100', UNE_RT_INT, '100', []),
  Case('100.9', UNE_RT_FLT, '100.900', []),
  Case('[]', UNE_RT_LIST, '[]', []),
  Case('[1]', UNE_RT_LIST, '[1]', []),
  Case('[1, 2]', UNE_RT_LIST, '[1, 2]', []),
  Case('[int([])]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('"str"', UNE_RT_STR, 'str', []),
  
  # COP
  Case('1 ? 2 : 3', UNE_RT_INT, '2', []),
  Case('0.0 ? 1 : 2', UNE_RT_INT, '2', []),
  Case('"str" ? 1 : 2', UNE_RT_INT, '1', []),
  Case('[] ? 1 : 2', UNE_RT_INT, '2', []),
  Case('unknown ? 1 : 0', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # NOT
  Case('!0', UNE_RT_INT, '1', []),
  Case('!unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # AND
  Case('1&&1', UNE_RT_INT, '1', []),
  Case('1&&0', UNE_RT_INT, '0', []),
  Case('0&&1', UNE_RT_INT, '0', []),
  Case('unknown&&1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # OR
  Case('0||0', UNE_RT_INT, '0', []),
  Case('1||0', UNE_RT_INT, '1', []),
  Case('0||1', UNE_RT_INT, '1', []),
  Case('unknown||1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # EQU
  Case('1==1', UNE_RT_INT, '1', []),
  Case('1==1.0', UNE_RT_INT, '1', []),
  Case('1.0==1', UNE_RT_INT, '1', []),
  Case('1.0==1.0', UNE_RT_INT, '1', []),
  Case('"str"=="str"', UNE_RT_INT, '1', []),
  Case('[1]==[1]', UNE_RT_INT, '1', []),
  Case('unknown==1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1==unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # NEQ
  Case('1!=1', UNE_RT_INT, '0', []),
  Case('unknown!=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # GTR
  Case('2>1', UNE_RT_INT, '1', []),
  Case('2>1.0', UNE_RT_INT, '1', []),
  Case('2.0>1', UNE_RT_INT, '1', []),
  Case('2.0>1.0', UNE_RT_INT, '1', []),
  Case('"string">"str"', UNE_RT_INT, '1', []),
  Case('[1,2,3]>[1,2]', UNE_RT_INT, '1', []),
  Case('unknown>1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1>unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('[1]>1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # GEQ
  Case('1>=1', UNE_RT_INT, '1', []),
  Case('1>=1.0', UNE_RT_INT, '1', []),
  Case('1.0>=1', UNE_RT_INT, '1', []),
  Case('1.0>=1.0', UNE_RT_INT, '1', []),
  Case('"str">="str"', UNE_RT_INT, '1', []),
  Case('[1,2]>=[1,2]', UNE_RT_INT, '1', []),
  Case('unknown>=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1>=unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('[1]>=1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # LSS
  Case('2<1', UNE_RT_INT, '0', []),
  Case('2<1.0', UNE_RT_INT, '0', []),
  Case('2.0<1', UNE_RT_INT, '0', []),
  Case('2.0<1.0', UNE_RT_INT, '0', []),
  Case('"string"<"str"', UNE_RT_INT, '0', []),
  Case('[1,2,3]<[1,2]', UNE_RT_INT, '0', []),
  Case('unknown<1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1<unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('[1]<1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # LEQ
  Case('1<=1', UNE_RT_INT, '1', []),
  Case('1<=1.0', UNE_RT_INT, '1', []),
  Case('1.0<=1', UNE_RT_INT, '1', []),
  Case('1.0<=1.0', UNE_RT_INT, '1', []),
  Case('"str"<="str"', UNE_RT_INT, '1', []),
  Case('[1,2]<=[1,2]', UNE_RT_INT, '1', []),
  Case('unknown<=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1<=unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('[1]<=1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # ADD
  Case('1+1', UNE_RT_INT, '2', []),
  Case('1+1.0', UNE_RT_FLT, '2.000', []),
  Case('1.0+1', UNE_RT_FLT, '2.000', []),
  Case('1.0+1.0', UNE_RT_FLT, '2.000', []),
  Case('"str"+"str"', UNE_RT_STR, 'strstr', []),
  Case('[1]+[2]', UNE_RT_LIST, '[1, 2]', []),
  Case('unknown+1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1+unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1+[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1.0+"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('[1]+1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('"str"+1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('"str"*"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # SUB
  Case('1-1', UNE_RT_INT, '0', []),
  Case('1-1.0', UNE_RT_FLT, '0.000', []),
  Case('1.0-1', UNE_RT_FLT, '0.000', []),
  Case('1.0-1.0', UNE_RT_FLT, '0.000', []),
  Case('unknown-1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1-unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1-[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1.0-"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # MUL
  Case('2*2', UNE_RT_INT, '4', []),
  Case('2*2.5', UNE_RT_FLT, '5.000', []),
  Case('2.5*2', UNE_RT_FLT, '5.000', []),
  Case('2.5*2.0', UNE_RT_FLT, '5.000', []),
  Case('3*"str"', UNE_RT_STR, 'strstrstr', []),
  Case('"string"*3', UNE_RT_STR, 'stringstringstring', []),
  Case('3*[1]', UNE_RT_LIST, '[1, 1, 1]', []),
  Case('[2]*3', UNE_RT_LIST, '[2, 2, 2]', []),
  Case('unknown*1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1*unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('[1]*[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1.0*"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1*int', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # DIV
  Case('6/2', UNE_RT_INT, '3', []),
  Case('1/2', UNE_RT_FLT, '0.500', []),
  Case('1/4.0', UNE_RT_FLT, '0.250', []),
  Case('4.0/1', UNE_RT_FLT, '4.000', []),
  Case('1.0/3.0', UNE_RT_FLT, '0.333', []),
  Case('unknown/1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1/unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1/[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1/0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('1/0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('1.0/"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # FDIV
  Case('1//3', UNE_RT_INT, '0', []),
  Case('5//2.0', UNE_RT_INT, '2', []),
  Case('0.1//1', UNE_RT_INT, '0', []),
  Case('6.0//4.0', UNE_RT_INT, '1', []),
  Case('unknown//1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1//unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1//[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1//0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('1//0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('1.0//"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1.0/0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('1.0/0.0', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  
  # MOD
  Case('3%2', UNE_RT_INT, '1', []),
  Case('5.5%2', UNE_RT_FLT, '1.500', []),
  Case('5%3.5', UNE_RT_FLT, '1.500', []),
  Case('5.5%3.5', UNE_RT_FLT, '2.000', []),
  Case('unknown%1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1%unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1%[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1.0%"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # POW
  Case('3**3', UNE_RT_INT, '27', []),
  Case('4**0.5', UNE_RT_FLT, '2.000', []),
  Case('0.8**3', UNE_RT_FLT, '0.512', []),
  Case('4.0**2.0', UNE_RT_FLT, '16.000', []),
  Case('unknown**1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1**unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('1**[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('(-1)**1.1', UNE_RT_ERROR, UNE_ET_UNREAL_NUMBER, []),
  Case('1.0**"str"', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # NEG
  Case('--100', UNE_RT_INT, '100', []),
  Case('--100.9', UNE_RT_FLT, '100.900', []),
  Case('-unknown', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('-[1]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # SET
  Case('a=46;return a', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
  Case('global a=46;return a', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=[1]**2', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # SET_IDX
  Case('a=[0];a[0]=1;return a', UNE_RT_LIST, '[1]', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=[0];global a[0]=1;return a', UNE_RT_LIST, '[1]', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=[0];a[0]=[1]**2', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('a[0]=1', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('a=1;a[0]=1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('a=[0];a[[1]]=1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('a=[0];a[-1]=1', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  Case('a=[0];a[1]=1', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  
  # GET
  Case('a=46;return a', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  
  # GET_IDX
  Case('[46][0]', UNE_RT_INT, '46', []),
  Case('"str"[0]', UNE_RT_STR, 's', []),
  Case('a[0]', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('a="str";a[1/0]', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, []),
  Case('([1]**2)[0]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('[0][[0]]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1[0]', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('[0][-1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  Case('[0][1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  Case('"a"[-1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  Case('"a"[1]', UNE_RT_ERROR, UNE_ET_INDEX_OUT_OF_RANGE, []),
  
  # FOR
  Case('a=0;for i from 0 till 3 a=a+i;return a', UNE_RT_INT, '3', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=0;for i from 3 till 0{if i==2 continue;a=a+i};return a', UNE_RT_INT, '4', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=0;for i from 0 till 0 a=1;return a', UNE_RT_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=0;for i from 0 till 1{break;a=1};return a', UNE_RT_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
  Case('for i from [1]**2 till 3 print(i)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('for i from [1] till 3 print(i)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('for i from 0 till [1]**2 print(i)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('for i from 0 till 3 print([i]**2)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # WHILE
  Case('i=3;while i>0 i=i-1;return i', UNE_RT_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
  Case('i=0;while i>0{i=-1;break};return i', UNE_RT_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
  Case('i=3;while [i]**2 i=i-1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('i=3;while i>0{[i]**2;i=i-1}', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # IF
  Case('if 1 return 1', UNE_RT_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if 0 return 0;return -1', UNE_RT_INT, '-1', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if 1 return 1 elif 0 return 0 else return 0', UNE_RT_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if 0 return 0 elif 2 return 2 else return 0', UNE_RT_INT, '2', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if 0 return 0 elif 0 return 0 else return 3', UNE_RT_INT, '3', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if 0 return 0 elif 0 return 0;return', UNE_RT_VOID, 'VOID', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if [1]**2 1', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # CALLABLES
  Case('fn=function(arg){a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]}', UNE_RT_VOID, 'VOID', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=function(){return};a=function(){return}', UNE_RT_VOID, 'VOID', [ATTR_NO_IMPLICIT_RETURN]),
  Case('int=function(){return}', UNE_RT_ERROR, UNE_ET_SYNTAX, []),
  
  # CALL
  Case('fn=function(arg){a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]};return fn(2)', UNE_RT_LIST, '[2.200, "str"]', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a()', UNE_RT_ERROR, UNE_ET_SYMBOL_NOT_DEFINED, []),
  Case('a=function(b)return b;c=function(d)return a();c(1)', UNE_RT_ERROR, UNE_ET_CALLABLE_ARG_COUNT, []),
  Case('a=function(b, c){return b};a(1, [1]**2)', UNE_RT_ERROR, UNE_ET_TYPE, []),
  Case('1()', UNE_RT_ERROR, UNE_ET_TYPE, []),
  
  # DATATYPES
  Case('print(int)', UNE_RT_VOID, 'VOID', []),
  Case('if int{return 1}else{return 0}', UNE_RT_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
  Case('print(function(){})', UNE_RT_VOID, 'VOID', []),
  Case('if function(){}{return 1}else{return 0}', UNE_RT_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
  Case('if print(1){return 1}else{return 0}', UNE_RT_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
  
  # ERROR DISPLAY
  Case('1\n2\n3/0\n4', UNE_RT_ERROR, UNE_ET_ZERO_DIVISION, [ATTR_FILE_ONLY]),
  
  # ORDER OF OPERATIONS
  Case('-2**2', UNE_RT_INT, '-4', []),
  Case('a=[2];return -a[0]**2', UNE_RT_INT, '-4', [ATTR_NO_IMPLICIT_RETURN]),
  Case('a=function()return 2;return -a()**2', UNE_RT_INT, '-4', [ATTR_NO_IMPLICIT_RETURN]),
  
  # COPYING FUNCTION NODE
  Case('function(){function(){}}', UNE_RT_FUNCTION, 'FUNCTION', []),
  
  # REPEATED GET_IDX OR CALL OPERATIONS
  Case('function(){return function(){return [[function(){return 46}]]}}()()[0][0]()', UNE_RT_INT, '46', []),
  
  # STALE POINTER IN FOR LOOP IMPLEMENTATION
  Case('a=0;for i from 1 till 3 for j from 4 till 6 a=a+i*j;return a', UNE_RT_INT, '27', [ATTR_NO_IMPLICIT_RETURN]),
  
  # LAST RESULT ALWAYS RETURNED
  Case('46', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
  
  # RETURN WITHOUT RETURN VALUE
  Case('return;return 46', UNE_RT_VOID, 'VOID', [ATTR_NO_IMPLICIT_RETURN]),
  
  # EXIT
  Case('exit;return 0', UNE_RT_VOID, 'VOID', [ATTR_NO_IMPLICIT_RETURN]),
  Case('exit 46;return 0', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
  Case('exit "string"', UNE_RT_ERROR, UNE_ET_TYPE, [ATTR_NO_IMPLICIT_RETURN]),
  Case('function(){exit(46)}();return 0', UNE_RT_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
]
CASES_LEN = len(cases)

##### PROGRAM

def dict_get(dictionary, value):
  if not value in dictionary:
    return value
  return dictionary.get(value)

def check_report(type, case: Case, i):
  passed = True
  with open(FILE_STATUS, mode='r', encoding='utf-8-sig') as report_status:
    for line in report_status:
      if line.startswith('result_type:'):
        result_type = int(line.split(':')[1])
        result_type_want = dict_get(result_types, case.result_type)
        result_type_have = dict_get(result_types, result_type)
        if result_type != case.result_type:
          print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{type}> RSTYPE (\33[7m{result_type_have}\33[27m != {result_type_want})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      elif line.startswith('alloc_count:'):
        alloc_count = int(line.split(':')[1])
        if alloc_count != 0:
          print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{type}> ALLOCS ({alloc_count})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      elif line.startswith('alert_count:'):
        alert_count = int(line.split(':')[1])
        if alert_count != 0:
          print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{type}> ALERTS ({alert_count})\33[0m')
          passed = False
          if STOP_AT_FAIL:
            return passed
      else:
        print('Internal error.')
        exit(1)
  with open(FILE_RETURN, mode='r', encoding='utf-8-sig') as report_return:
    return_value = report_return.read()
    if return_value != str(case.result_value):
      if result_type == UNE_RT_ERROR:
        return_value_want = dict_get(error_types, case.result_value)
        return_value_have = dict_get(error_types, int(return_value))
      else:
        return_value_want = f'\'{str(case.result_value)}\''
        return_value_have = f'\'{return_value}\''
      print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{type}> RETURN (\33[7m{return_value_have}\33[27m != {return_value_want})\33[0m')
      passed = False
  return passed

if CLEAR:
  cmd('cls' if is_cmd_exe() else 'clear')
  
if ENUMERATE_CASES:
  for i, j in enumerate(cases):
    print(i, j)

cd = os.getcwd()
os.chdir(DIR)
print("\33[33m\33[1mEnsure UNE_DEBUG_MEMDBG is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_SIZES is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_REPORT is enabled.\33[0m")
i = 0
for i, case in enumerate(cases):
  if i+1 < SKIP_UNTIL:
    continue
  if HIDE_OUTPUT and not ATTR_NEVER_HIDE_OUTPUT in case.attributes:
    une = '1>' + ('nul' if is_cmd_exe() else '/dev/null') + f' 2>&1 {UNE}'
  else:
    une = UNE
  i += 1
  passed = True
  if ATTR_DIRECT_ARG in case.attributes:
    cmd(f'{une} {case.input}')
    if not check_report('main', case, i):
      passed = False
  else:
    if not ATTR_FILE_ONLY in case.attributes:
      sanitized = case.input
      if not ATTR_NO_SECOND_ESCAPE in case.attributes:
        sanitized = sanitized.replace('\\', '\\\\')
      sanitized = sanitized.replace('"', '\\"')
      sanitized = sanitized.replace('\n', '\\n')
      if case.result_type != UNE_RT_ERROR and not ATTR_NO_IMPLICIT_RETURN in case.attributes:
        sanitized = f'return {sanitized}'
      command = f'{une} -s "{sanitized}"'
      cmd(command)
      if not check_report('cmdl', case, i):
        passed = False
        print(f'\33[31m{command}\33[0m')
    if passed or not STOP_AT_FAIL:
      script = case.input
      if case.result_type != UNE_RT_ERROR and not ATTR_NO_IMPLICIT_RETURN in case.attributes:
        script = f'return {script}'
      status = open(FILE_SCRIPT, 'w')
      status.write(script)
      status.close()
      cmd(f'{une} "{FILE_SCRIPT}"')
      if not check_report('file', case, i):
        passed = False
        print(f'\33[31m{script}\33[0m')
  if passed:
    print(f'\33[0m[{i}/{CASES_LEN}] \33[32mPassed\33[0m')
  elif STOP_AT_FAIL:
    print('\33[33m(test.py) Failed, stopping.\33[0m')
    break
else:
  print('\33[32m\33[1m(test.py) ALL TESTS PASSED\33[0m')
  exit(0)
exit(1)
