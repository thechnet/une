#!python3
import os
from os import system as cmd
from sys import platform, argv

def is_win():
	return platform=='win32' or platform=='cygwin' or platform=='msys'

def is_cmd_exe():
	return platform=='win32'

class Case:
	def __init__(self, input, result_kind, result_value, attributes):
		self.input = input
		self.result_kind = result_kind
		self.result_value = result_value
		self.attributes = attributes

# Options
SKIP_UNTIL = int(argv[1]) if len(argv) > 1 and argv[1] != "enumerate_cases" else 0 # 0, 2
HIDE_OUTPUT = True
SHOW_ERRORS = False
CLEAR = False
STOP_AT_FAIL = True
ENUMERATE_CASES = True if len(argv) > 1 and argv[1] == "enumerate_cases" else False
FILE_SCRIPT = 'test.py.une'
DIR = 'debug'
UNE = '.\\\\une.exe' if is_win() else './une'
FILE_RETURN = 'une_report_return.txt'
FILE_STATUS = 'une_report_status.txt'
UNE_R_END_TYPE_RESULT_KINDS = 9

##### CONSTANTS

ATTR_DIRECT_ARG = 1
ATTR_FILE_ONLY = 2
ATTR_NO_IMPLICIT_RETURN = 3
ATTR_NEVER_HIDE_OUTPUT = 4
ATTR_NO_SECOND_ESCAPE = 5
ATTR_STDIN = 6

# Result kinds
UNE_RK_ERROR = 1
UNE_RK_VOID = 2
UNE_RK_INT = 3
UNE_RK_FLT = 4
UNE_RK_STR = 5
UNE_RK_LIST = 6
UNE_RK_OBJECT = 7
UNE_RK_FUNCTION = 8
UNE_RK_NATIVE = 9
result_kinds = {
	UNE_RK_ERROR: 'UNE_RK_ERROR',
	UNE_RK_VOID: 'UNE_RK_VOID',
	UNE_RK_INT: 'UNE_RK_INT',
	UNE_RK_FLT: 'UNE_RK_FLT',
	UNE_RK_STR: 'UNE_RK_STR',
	UNE_RK_LIST: 'UNE_RK_LIST',
	UNE_RK_OBJECT: 'UNE_RK_OBJECT',
	UNE_RK_FUNCTION: 'UNE_RK_FUNCTION',
	UNE_RK_NATIVE: 'UNE_RK_NATIVE',
}

# Error kinds
UNE_ERROR_INPUT = 1
UNE_EK_SYNTAX = UNE_R_END_TYPE_RESULT_KINDS+1
UNE_EK_BREAK_OUTSIDE_LOOP = UNE_R_END_TYPE_RESULT_KINDS+2
UNE_EK_CONTINUE_OUTSIDE_LOOP = UNE_R_END_TYPE_RESULT_KINDS+3
UNE_EK_SYMBOL_NOT_DEFINED = UNE_R_END_TYPE_RESULT_KINDS+4
UNE_EK_INDEX = UNE_R_END_TYPE_RESULT_KINDS+5
UNE_EK_ZERO_DIVISION = UNE_R_END_TYPE_RESULT_KINDS+6
UNE_EK_UNREAL_NUMBER = UNE_R_END_TYPE_RESULT_KINDS+7
UNE_EK_CALLABLE_ARG_COUNT = UNE_R_END_TYPE_RESULT_KINDS+8
UNE_EK_FILE = UNE_R_END_TYPE_RESULT_KINDS+9
UNE_EK_ENCODING = UNE_R_END_TYPE_RESULT_KINDS+10
UNE_EK_TYPE = UNE_R_END_TYPE_RESULT_KINDS+11
UNE_EK_ASSERTION_NOT_MET = UNE_R_END_TYPE_RESULT_KINDS+12
UNE_EK_MISPLACED_ANY_OR_ALL = UNE_R_END_TYPE_RESULT_KINDS+13
UNE_EK_SYSTEM = UNE_R_END_TYPE_RESULT_KINDS+14
error_kinds = {
	UNE_ERROR_INPUT: 'UNE_ERROR_INPUT',
	UNE_EK_SYNTAX: 'UNE_EK_SYNTAX',
	UNE_EK_BREAK_OUTSIDE_LOOP: 'UNE_EK_BREAK_OUTSIDE_LOOP',
	UNE_EK_CONTINUE_OUTSIDE_LOOP: 'UNE_EK_CONTINUE_OUTSIDE_LOOP',
	UNE_EK_SYMBOL_NOT_DEFINED: 'UNE_EK_SYMBOL_NOT_DEFINED',
	UNE_EK_INDEX: 'UNE_EK_INDEX',
	UNE_EK_ZERO_DIVISION: 'UNE_EK_ZERO_DIVISION',
	UNE_EK_UNREAL_NUMBER: 'UNE_EK_UNREAL_NUMBER',
	UNE_EK_CALLABLE_ARG_COUNT: 'UNE_EK_CALLABLE_ARG_COUNT',
	UNE_EK_FILE: 'UNE_EK_FILE',
	UNE_EK_ENCODING: 'UNE_EK_ENCODING',
	UNE_EK_TYPE: 'UNE_EK_TYPE',
	UNE_EK_ASSERTION_NOT_MET: 'UNE_EK_ASSERTION_NOT_MET',
	UNE_EK_MISPLACED_ANY_OR_ALL: 'UNE_EK_MISPLACED_ANY_OR_ALL',
	UNE_EK_SYSTEM: 'UNE_EK_SYSTEM',
}

##### CASES

cases = [
	
	# NEW NUMBER LEXER
	Case('0', UNE_RK_INT, '0', []),
	Case('00.00', UNE_RK_FLT, '0.0', []),
	Case('0.', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('.0', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('0b10', UNE_RK_INT, '2', []),
	Case('0o10', UNE_RK_INT, '8', []),
	Case('0xf', UNE_RK_INT, '15', []),
	Case('0x.8', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('2E2', UNE_RK_FLT, '200.0', []),
	Case('2E-2', UNE_RK_FLT, '0.02', []),
	Case('0b1E1', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	
	# OCTAL AND HEXADECIMAL CHARACTER CONSTANTS IN STRINGS
	Case('"\\o43"', UNE_RK_STR, '#', [ATTR_NO_SECOND_ESCAPE]),
	Case('"\\o431"', UNE_RK_STR, '#1', [ATTR_NO_SECOND_ESCAPE]),
	Case('"\\x23"', UNE_RK_STR, '#', [ATTR_NO_SECOND_ESCAPE]),
	Case('"\\x231"', UNE_RK_STR, '#1', [ATTR_NO_SECOND_ESCAPE]),
	
	# Natives
	
	Case('input("Write \'test\' and press enter: ")', UNE_RK_STR, 'test', [ATTR_STDIN]),
	Case('input(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('input()', UNE_RK_ERROR, UNE_EK_CALLABLE_ARG_COUNT, []),
	
	Case('print(100.9)', UNE_RK_VOID, 'Void', []),
	Case('print("str")', UNE_RK_VOID, 'Void', []),
	Case('print([1])', UNE_RK_VOID, 'Void', []),
	Case('print([1, "str"])', UNE_RK_VOID, 'Void', []),
	
	Case('put(1)', UNE_RK_VOID, 'Void', []),
	
	Case('int(100)', UNE_RK_INT, '100', []),
	Case('int(100.9)', UNE_RK_INT, '100', []),
	Case('int("100")', UNE_RK_INT, '100', []),
	Case('int([1])', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('int("100x")', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	Case('int("")', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	
	Case('flt(100)', UNE_RK_FLT, '100.0', []),
	Case('flt(100.9)', UNE_RK_FLT, '100.9', []),
	Case('flt("100.9")', UNE_RK_FLT, '100.9', []),
	Case('flt([1])', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('flt("100.9x")', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	Case('flt("")', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	
	Case('str(1)', UNE_RK_STR, '1', []),
	Case('str(1.1)', UNE_RK_STR, '1.1', []),
	Case('str("str")', UNE_RK_STR, 'str', []),
	Case('str([1])', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('len("str")', UNE_RK_INT, '3', []),
	Case('len([1, 2, 3])', UNE_RK_INT, '3', []),
	Case('len(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('sleep(1)', UNE_RK_VOID, 'Void', []),
	Case('sleep(1.1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('chr(65)', UNE_RK_STR, 'A', []),
	Case('chr(1.1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('chr(999999999999)', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	
	Case('ord("A")', UNE_RK_INT, '65', []),
	Case('ord(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('write("script.une", "return 46")', UNE_RK_VOID, 'Void', []),
	Case('write(1, "test")', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('write("test", 1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('write("/", "test")', UNE_RK_ERROR, UNE_EK_FILE, []),
	
	Case('read("script.une")', UNE_RK_STR, 'return 46', []),
	Case('read(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('read("1lkj23")', UNE_RK_ERROR, UNE_EK_FILE, []),
	
	Case('script("script.une")', UNE_RK_INT, '46', []),
	Case('script(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('script("/")', UNE_RK_ERROR, UNE_EK_FILE, []),
	Case('write("script.une", "msg=128");script("script.une");return msg', UNE_RK_INT, '128', [ATTR_NO_IMPLICIT_RETURN]),
	
	Case('write("script.une", "4");append("script.une", "\n6");return read("script.une")', UNE_RK_STR, '4\n6', [ATTR_NO_IMPLICIT_RETURN]),
	
	Case('exist("script.une")', UNE_RK_INT, '1', []),
	Case('exist(1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('split("1.2.3", ["."])', UNE_RK_LIST, '["1", "2", "3"]', []),
	Case('split(1, ["."])', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('split("1.2.3", 1)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('split("1.2.3", [1])', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	Case('join(["1","2","3"], ".")', UNE_RK_STR, "1.2.3", []),
	Case('join(["1","2",3], ".")', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('join(["1","2","3"], 0)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('join(["1"], ".")', UNE_RK_STR, "1", []),
	Case('join([], ".")', UNE_RK_STR, "", []),
	
	Case('eval(0)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('eval("23*2")', UNE_RK_INT, '46', []),
	
	Case('replace("a","b","xax")', UNE_RK_STR, 'xbx', []),
	Case('replace("a","","xax");return replace("a","","xax")', UNE_RK_STR, 'xx', [ATTR_NO_IMPLICIT_RETURN]),
	Case('replace("","","")', UNE_RK_ERROR, UNE_EK_ENCODING, []),
	
	# main
	Case('unknown', UNE_RK_ERROR, UNE_EK_FILE, [ATTR_DIRECT_ARG]),
	Case('', UNE_RK_ERROR, UNE_ERROR_INPUT, [ATTR_DIRECT_ARG]),
	Case('-s', UNE_RK_ERROR, UNE_ERROR_INPUT, [ATTR_DIRECT_ARG]),
	
	# Syntax
	Case('\r# comment', UNE_RK_VOID, 'Void', []),
	Case('"\\\\\\"\\n\\\n"', UNE_RK_STR, '\\"\n', [ATTR_FILE_ONLY]),
	Case('\r1 $', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('1.', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('"ab', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('"\\q"', UNE_RK_ERROR, UNE_EK_SYNTAX, [ATTR_NO_SECOND_ESCAPE]),
	Case('1 ? +', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('1 ? 1,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('1 ? 1 : +', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('[1', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('a(', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('(1', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case(',', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i 1', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i from ,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i from 0 ,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i from 0 till', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i from 0 till 1 ,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i in ,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i in "a" ,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('while 1 {+}', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('if 1 )', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('if 1 1 else', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('continue', UNE_RK_ERROR, UNE_EK_CONTINUE_OUTSIDE_LOOP, []),
	Case('break', UNE_RK_ERROR, UNE_EK_BREAK_OUTSIDE_LOOP, []),
	Case('a[', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('a=a[1,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('a[1,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('a=*', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('1+)', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('[1,', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('[1, +]', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('[1 1]', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('for i from 1.1', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('a=(a, 1)->{return a}', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	Case('()->', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	
	# Data
	Case('Void', UNE_RK_VOID, 'Void', []),
	Case('100', UNE_RK_INT, '100', []),
	Case('100.9', UNE_RK_FLT, '100.9', []),
	Case('[]', UNE_RK_LIST, '[]', []),
	Case('[1]', UNE_RK_LIST, '[1]', []),
	Case('[1, 2]', UNE_RK_LIST, '[1, 2]', []),
	Case('[int([])]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('"str"', UNE_RK_STR, 'str', []),
	Case('"s{1+2}t"', UNE_RK_STR, 's3t', []),
	Case('({a:1,b:{c:2}})', UNE_RK_OBJECT, '{a: 1, b: {c: 2}}', []),
	Case('({a:b})', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('({a:46,b:()->return this.a,c:n->{this.a=n;return this}}).c(128).b()', UNE_RK_INT, '128', []),
	Case('a={b:0,c:()->return this.b,d:n->this.b=n};a.d(46);return a.c()', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('main={attr:0,with_set_attr:value->{this.attr=value;return this}};return main.with_set_attr(({value:23,double:()->return this.value*2}).double()).attr;', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('c=[{o:{a:0,s:n->this.a=n}}];c[0].o.s(46);return c[0].o.a', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('o={a:0,m:v->{this.a=v;return this}};o.m(1).m(2);return o.a', UNE_RK_INT, '2', [ATTR_NO_IMPLICIT_RETURN]), # Method chaining
	Case('a=()->return{b:()->return a()};a()', UNE_RK_OBJECT, '{b: <function>}', [ATTR_NO_IMPLICIT_RETURN]), # Ensure member_seek receives object references
	Case('this', UNE_RK_VOID, 'Void', []),
	
	# COP
	Case('1 ? 2 : 3', UNE_RK_INT, '2', []),
	Case('0.0 ? 1 : 2', UNE_RK_INT, '2', []),
	Case('"str" ? 1 : 2', UNE_RK_INT, '1', []),
	Case('[] ? 1 : 2', UNE_RK_INT, '2', []),
	Case('unknown ? 1 : 0', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# NOT
	Case('!0', UNE_RK_INT, '1', []),
	Case('1==!0', UNE_RK_INT, '1', []),
	Case('!unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# AND
	Case('1&&1', UNE_RK_INT, '1', []),
	Case('1&&0', UNE_RK_INT, '0', []),
	Case('0&&1', UNE_RK_INT, '0', []),
	Case('unknown&&1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# OR
	Case('0||0', UNE_RK_INT, '0', []),
	Case('1||0', UNE_RK_INT, '1', []),
	Case('0||1', UNE_RK_INT, '1', []),
	Case('unknown||1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# EQU
	Case('1==1', UNE_RK_INT, '1', []),
	Case('1==1.0', UNE_RK_INT, '1', []),
	Case('1.0==1', UNE_RK_INT, '1', []),
	Case('1.0==1.0', UNE_RK_INT, '1', []),
	Case('"str"=="str"', UNE_RK_INT, '1', []),
	Case('[1]==[1]', UNE_RK_INT, '1', []),
	Case('unknown==1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1==unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1 == any [1, 2, 3]', UNE_RK_INT, '1', []),
	Case('1 == all [1, 2, 3]', UNE_RK_INT, '0', []),
	Case('"a" == any "abc"', UNE_RK_INT, '1', []),
	Case('any 1 == 1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('any 1', UNE_RK_ERROR, UNE_EK_MISPLACED_ANY_OR_ALL, []),
	
	# NEQ
	Case('1!=1', UNE_RK_INT, '0', []),
	Case('unknown!=1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1 != any [1, 2, 3]', UNE_RK_INT, '1', []),
	Case('1 != all [1, 2, 3]', UNE_RK_INT, '0', []),
	
	# GTR
	Case('2>1', UNE_RK_INT, '1', []),
	Case('2>1.0', UNE_RK_INT, '1', []),
	Case('2.0>1', UNE_RK_INT, '1', []),
	Case('2.0>1.0', UNE_RK_INT, '1', []),
	Case('"string">"str"', UNE_RK_INT, '1', []),
	Case('[1,2,3]>[1,2]', UNE_RK_INT, '1', []),
	Case('unknown>1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1>unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('[1]>1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('Void>0', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1 > any [0, 1, 2]', UNE_RK_INT, '1', []),
	Case('1 > all [0, 1, 2]', UNE_RK_INT, '0', []),
	
	# GEQ
	Case('1>=1', UNE_RK_INT, '1', []),
	Case('1>=1.0', UNE_RK_INT, '1', []),
	Case('1.0>=1', UNE_RK_INT, '1', []),
	Case('1.0>=1.0', UNE_RK_INT, '1', []),
	Case('"str">="str"', UNE_RK_INT, '1', []),
	Case('[1,2]>=[1,2]', UNE_RK_INT, '1', []),
	Case('unknown>=1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1>=unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('[1]>=1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('Void>=0', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1 >= any [0, 1, 2]', UNE_RK_INT, '1', []),
	Case('1 >= all [0, 1, 2]', UNE_RK_INT, '0', []),
	
	# LSS
	Case('2<1', UNE_RK_INT, '0', []),
	Case('2<1.0', UNE_RK_INT, '0', []),
	Case('2.0<1', UNE_RK_INT, '0', []),
	Case('2.0<1.0', UNE_RK_INT, '0', []),
	Case('"string"<"str"', UNE_RK_INT, '0', []),
	Case('[1,2,3]<[1,2]', UNE_RK_INT, '0', []),
	Case('unknown<1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1<unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('[1]<1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('Void<0', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1 < any [0, 1, 2]', UNE_RK_INT, '1', []),
	Case('1 < all [0, 1, 2]', UNE_RK_INT, '0', []),
	
	# LEQ
	Case('1<=1', UNE_RK_INT, '1', []),
	Case('1<=1.0', UNE_RK_INT, '1', []),
	Case('1.0<=1', UNE_RK_INT, '1', []),
	Case('1.0<=1.0', UNE_RK_INT, '1', []),
	Case('"str"<="str"', UNE_RK_INT, '1', []),
	Case('[1,2]<=[1,2]', UNE_RK_INT, '1', []),
	Case('unknown<=1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1<=unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('[1]<=1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('Void<=0', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1 <= any [0, 1, 2]', UNE_RK_INT, '1', []),
	Case('1 <= all [0, 1, 2]', UNE_RK_INT, '0', []),
	
	# COVER
	Case('1/0 cover 46', UNE_RK_INT, '46', []),
	Case('1/0 cover 2/0 cover 96', UNE_RK_INT, '96', []),
	
	# ADD
	Case('1+1', UNE_RK_INT, '2', []),
	Case('1+1.0', UNE_RK_FLT, '2.0', []),
	Case('1.0+1', UNE_RK_FLT, '2.0', []),
	Case('1.0+1.0', UNE_RK_FLT, '2.0', []),
	Case('"str"+"str"', UNE_RK_STR, 'strstr', []),
	Case('[1]+[2]', UNE_RK_LIST, '[1, 2]', []),
	Case('unknown+1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1+unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1+[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1.0+"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('[1]+1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('"str"+1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('"str"*"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# SUB
	Case('1-1', UNE_RK_INT, '0', []),
	Case('1-1.0', UNE_RK_FLT, '0.0', []),
	Case('1.0-1', UNE_RK_FLT, '0.0', []),
	Case('1.0-1.0', UNE_RK_FLT, '0.0', []),
	Case('unknown-1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1-unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1-[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1.0-"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# MUL
	Case('2*2', UNE_RK_INT, '4', []),
	Case('2*2.5', UNE_RK_FLT, '5.0', []),
	Case('2.5*2', UNE_RK_FLT, '5.0', []),
	Case('2.5*2.0', UNE_RK_FLT, '5.0', []),
	Case('3*"str"', UNE_RK_STR, 'strstrstr', []),
	Case('"string"*3', UNE_RK_STR, 'stringstringstring', []),
	Case('"string"*-1', UNE_RK_STR, '', []),
	Case('3*[1]', UNE_RK_LIST, '[1, 1, 1]', []),
	Case('[2]*3', UNE_RK_LIST, '[2, 2, 2]', []),
	Case('[2]*-1', UNE_RK_LIST, '[]', []),
	Case('unknown*1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1*unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('[1]*[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1.0*"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1*int', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# DIV
	Case('6/2', UNE_RK_INT, '3', []),
	Case('1/2', UNE_RK_FLT, '0.5', []),
	Case('1/4.0', UNE_RK_FLT, '0.25', []),
	Case('4.0/1', UNE_RK_FLT, '4.0', []),
	Case('1.0/3.0', UNE_RK_FLT, '0.3333333333333', []),
	Case('unknown/1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1/unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1/[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1/0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('1/0.0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('1.0/"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# FDIV
	Case('1//3', UNE_RK_INT, '0', []),
	Case('5//2.0', UNE_RK_INT, '2', []),
	Case('0.1//1', UNE_RK_INT, '0', []),
	Case('6.0//4.0', UNE_RK_INT, '1', []),
	Case('unknown//1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1//unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1//[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1//0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('1//0.0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('1.0//"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1.0/0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('1.0/0.0', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	
	# MOD
	Case('3%2', UNE_RK_INT, '1', []),
	Case('5.5%2', UNE_RK_FLT, '1.5', []),
	Case('5%3.5', UNE_RK_FLT, '1.5', []),
	Case('5.5%3.5', UNE_RK_FLT, '2.0', []),
	Case('unknown%1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1%unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1%[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1.0%"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# POW
	Case('3**3', UNE_RK_INT, '27', []),
	Case('4**0.5', UNE_RK_FLT, '2.0', []),
	Case('0.8**3', UNE_RK_FLT, '0.512', []),
	Case('4.0**2.0', UNE_RK_FLT, '16.0', []),
	Case('2**-2', UNE_RK_FLT, '0.25', []),
	Case('unknown**1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1**unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('1**[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('(-1)**1.1', UNE_RK_ERROR, UNE_EK_UNREAL_NUMBER, []),
	Case('1.0**"str"', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# NEG
	Case('--100', UNE_RK_INT, '100', []),
	Case('--100.9', UNE_RK_FLT, '100.9', []),
	Case('-unknown', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('-[1]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# SET
	Case('a=46;return a', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('global a=46;return a', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=()->{global b=46};a();return b', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[1]**2', UNE_RK_ERROR, UNE_EK_TYPE, [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=a', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, [ATTR_NO_IMPLICIT_RETURN]),
	
	# SET_IDX
	Case('a=[0];a[0]=1;return a', UNE_RK_LIST, '[1]', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[0];global a[0]=1;return a', UNE_RK_LIST, '[1]', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[[0]];a[0][0]=46;a', UNE_RK_LIST, "[[46]]", [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[0];a[0]=[1]**2', UNE_RK_ERROR, UNE_EK_TYPE, [ATTR_NO_IMPLICIT_RETURN]),
	Case('a[0]=1', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=1;a[0]=1', UNE_RK_ERROR, UNE_EK_TYPE, [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[0];a[[1]]=1', UNE_RK_ERROR, UNE_EK_INDEX, [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[0];a[1]=1', UNE_RK_ERROR, UNE_EK_INDEX, [ATTR_NO_IMPLICIT_RETURN]),
	
	# Assignment operations.
	Case('a=23;a+=23;return a', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=23;a-=23;return a', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=4;a**=2;return a', UNE_RK_INT, '16', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=4;a*=2;return a', UNE_RK_INT, '8', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=5;a//=2;return a', UNE_RK_INT, '2', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=5;a/=2;return a', UNE_RK_FLT, '2.5', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=5;a%=2;return a', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	
	# GET
	Case('a=46;return a', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# GET_IDX
	Case('[46][0]', UNE_RK_INT, '46', []),
	Case('"str"[0]', UNE_RK_STR, 's', []),
	Case('a[0]', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('a="str";a[1/0]', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('([1]**2)[0]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('[0][[0]]', UNE_RK_ERROR, UNE_EK_INDEX, []),
	Case('1[0]', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('[0][1]', UNE_RK_ERROR, UNE_EK_INDEX, []),
	Case('"a"[1]', UNE_RK_ERROR, UNE_EK_INDEX, []),
	
	# Get member.
	Case('({a:{b:46}}).a.b', UNE_RK_INT, '46', []),
	Case('({a:46}).b', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	
	# Set member.
	Case('a={b:0};a.b=46;return a.b', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	
	# FOR
	Case('a=0;for i from 0 till 3 a=a+i;return a', UNE_RK_INT, '3', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=0;for i from 3 till 0{if i==2 continue;a=a+i};return a', UNE_RK_INT, '4', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=0;for i from 0 till 0 a=1;return a', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=0;for i from 0 till 1{break;a=1};return a', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	Case('for i from [1]**2 till 3 print(i)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('for i from [1] till 3 print(i)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('for i from 0 till [1]**2 print(i)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('for i from 0 till 3 print([i]**2)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('for i in 0 print(i)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('a=0;for i in [1,2,3] a=a+i;return a', UNE_RK_INT, '6', [ATTR_NO_IMPLICIT_RETURN]),
	
	# WHILE
	Case('i=3;while i>0 i=i-1;return i', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	Case('i=0;while i>0{i=-1;break};return i', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	Case('i=3;while [i]**2 i=i-1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('i=3;while i>0{[i]**2;i=i-1}', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# IF
	Case('if 1 return 1', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if 0 return 0;return -1', UNE_RK_INT, '-1', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if 1 return 1 elif 0 return 0 else return 0', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if 0 return 0 elif 2 return 2 else return 0', UNE_RK_INT, '2', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if 0 return 0 elif 0 return 0 else return 3', UNE_RK_INT, '3', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if 0 return 0 elif 0 return 0;return', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if [1]**2 1', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# CALLABLES
	Case('fn=arg->{a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]}', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=()->{return};a=()->{return}', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('int=()->{return}', UNE_RK_ERROR, UNE_EK_SYNTAX, []),
	
	# CALL
	Case('fn=arg->{a=[0];a[0]=1;for i from 0 till 2{if i==0 continue;break};return [arg*1*1.1, "str"]};return fn(2)', UNE_RK_LIST, '[2.2, "str"]', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a()', UNE_RK_ERROR, UNE_EK_SYMBOL_NOT_DEFINED, []),
	Case('a=b->return b;c=d->return a();c(1)', UNE_RK_ERROR, UNE_EK_CALLABLE_ARG_COUNT, []),
	Case('a=(b,c)->{return b};a(1, [1]**2)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	Case('1()', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# TYPES
	Case('print(int)', UNE_RK_VOID, 'Void', []),
	Case('if int{return 1}else{return 0}', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	Case('print(()->{})', UNE_RK_VOID, 'Void', []),
	Case('if ()->{}{return 1}else{return 0}', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	Case('if print(1){return 1}else{return 0}', UNE_RK_INT, '0', [ATTR_NO_IMPLICIT_RETURN]),
	
	# ERROR DISPLAY
	Case('1\n2\n3/0\n4', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, [ATTR_FILE_ONLY]),
	
	# ORDER OF OPERATIONS
	Case('-2**2', UNE_RK_INT, '-4', []),
	Case('a=[2];return -a[0]**2', UNE_RK_INT, '-4', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=()->return 2;return -a()**2', UNE_RK_INT, '-4', [ATTR_NO_IMPLICIT_RETURN]),
	
	# COPYING FUNCTION NODE
	Case('()->{()->{}}', UNE_RK_FUNCTION, '<function>', []),
	
	# REPEATED GET_IDX OR CALL OPERATIONS
	Case('()->{return ()->{return [[()->{return 46}]]}}()()[0][0]()', UNE_RK_INT, '46', []),
	
	# STALE POINTER IN FOR LOOP IMPLEMENTATION
	Case('a=0;for i from 1 till 3 for j from 4 till 6 a=a+i*j;return a', UNE_RK_INT, '27', [ATTR_NO_IMPLICIT_RETURN]),
	
	# LAST RESULT ALWAYS RETURNED
	Case('46', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	
	# RETURN WITHOUT RETURN VALUE
	Case('return;return 46', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	
	# EXIT
	Case('exit;return 0', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('exit 46;return 0', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	Case('exit "string"', UNE_RK_ERROR, UNE_EK_TYPE, [ATTR_NO_IMPLICIT_RETURN]),
	Case('()->{exit(46)}();return 0', UNE_RK_INT, '46', [ATTR_NO_IMPLICIT_RETURN]),
	
	# Slices
	Case('a="";a="b"+"c"', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('[1, 2, 3, 4, 5][1..-1][1..Void]', UNE_RK_LIST, '[3, 4]', []),
	Case('a=[1, 2, 3, 4, 5];return a[1..-1][1..Void]', UNE_RK_LIST, '[3, 4]', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a=[1, 2, 3, 4, 5];a[1..-1][1..Void]=[23, 46];return a', UNE_RK_LIST, '[1, 2, 23, 46, 5]', [ATTR_NO_IMPLICIT_RETURN]),
	Case('"string"[1..-1][1..Void]', UNE_RK_STR, 'rin', []),
	Case('a="string";return a[1..-1][1..Void]', UNE_RK_STR, 'rin', [ATTR_NO_IMPLICIT_RETURN]),
	Case('a="string";a[1..-1][1..Void]="4t6";return a', UNE_RK_STR, 'st4t6g', [ATTR_NO_IMPLICIT_RETURN]),
	Case('"string"[100..]', UNE_RK_STR, '', []),
	Case('[1, 2, 3][100..]', UNE_RK_LIST, '[]', []),
	
	# Dereference 'this' before registering new one for method call
	Case('({m:o->{return o}}).m(this)', UNE_RK_VOID, 'Void', []),
	
	# Assertions
	Case('assert True', UNE_RK_VOID, 'Void', [ATTR_NO_IMPLICIT_RETURN]),
	Case('assert False', UNE_RK_ERROR, UNE_EK_ASSERTION_NOT_MET, [ATTR_NO_IMPLICIT_RETURN]),
	
	# Don't propagate 'return' out of eval() and script().
	Case('eval("return");return 1', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
	
	# sort
	Case('sort([2,1,3],(a,b)->return a-b)', UNE_RK_LIST, '[1, 2, 3]', []),
	Case('sort([2,1,3],(a,b)->return b-a)', UNE_RK_LIST, '[3, 2, 1]', []),
	Case('sort([2,1,3],(a,b)->return 1/0)', UNE_RK_ERROR, UNE_EK_ZERO_DIVISION, []),
	Case('sort([2,1,3],(a,b)->return 1.0)', UNE_RK_ERROR, UNE_EK_TYPE, []),
	
	# getwd
	Case('split(getwd(),["/","\\\\"])[-1..]', UNE_RK_LIST, '["debug"]', []),
	
	# setwd
	Case('a=getwd();setwd("..");b=getwd();setwd(a);return a!=b', UNE_RK_INT, '1', [ATTR_NO_IMPLICIT_RETURN]),
]
CASES_LEN = len(cases)

##### PROGRAM

def dict_get(dictionary, value):
	if not value in dictionary:
		return value
	return dictionary.get(value)

def check_report(interface, case: Case, i):
	passed = True
	result_kind_checked = False
	alloc_count_checked = False
	alert_count_checked = False
	with open(FILE_STATUS, mode='r', encoding='utf-8-sig') as report_status:
		for line in report_status:
			if line.startswith('result_kind:'):
				result_kind = int(line.split(':')[1])
				result_kind_want = dict_get(result_kinds, case.result_kind)
				result_kind_have = dict_get(result_kinds, result_kind)
				if result_kind != case.result_kind:
					print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{interface}> RSKIND (\33[7m{result_kind_have}\33[27m != {result_kind_want})\33[0m')
					passed = False
					if STOP_AT_FAIL:
						return passed
				result_kind_checked = True
			elif line.startswith('alloc_count:'):
				alloc_count = int(line.split(':')[1])
				if alloc_count != 0:
					print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{interface}> ALLOCS ({alloc_count})\33[0m')
					passed = False
					if STOP_AT_FAIL:
						return passed
				alloc_count_checked = True
			elif line.startswith('alert_count:'):
				alert_count = int(line.split(':')[1])
				if alert_count != 0:
					print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{interface}> ALERTS ({alert_count})\33[0m')
					passed = False
					if STOP_AT_FAIL:
						return passed
				alert_count_checked = True
			else:
				print('Internal error.')
				exit(1)
	if not result_kind_checked or not alloc_count_checked or not alert_count_checked:
		print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1mCRASH\33[0m')
		passed = False
		if STOP_AT_FAIL:
			return passed
	with open(FILE_RETURN, mode='r', encoding='utf-8-sig') as report_return:
		return_value = report_return.read()
		if return_value != str(case.result_value):
			if result_kind == UNE_RK_ERROR:
				return_value_want = dict_get(error_kinds, case.result_value)
				return_value_have = dict_get(error_kinds, int(return_value))
			else:
				return_value_want = f'\'{str(case.result_value)}\''
				return_value_have = f'\'{return_value}\''
			print(f'\33[0m[{i}/{CASES_LEN}] \33[31m\33[1m<{interface}> RETURN (\33[7m{return_value_have}\33[27m != {return_value_want})\33[0m')
			passed = False
	# Clear files after reading them.
	with open(FILE_STATUS, mode='w', encoding='utf-8-sig') as report_status:
		report_status.write("")
	with open(FILE_RETURN, mode='w', encoding='utf-8-sig') as report_return:
		report_return.write("")
	return passed

if CLEAR:
	cmd('cls' if is_cmd_exe() else 'clear')
	
if ENUMERATE_CASES:
	for i, j in enumerate(cases):
		print(i+1, j.input)
	exit()

cd = os.getcwd()
os.chdir(DIR)
print("\33[33m\33[1mEnsure UNE_DEBUG_MEMDBG is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_SIZES is enabled.\33[0m")
print("\33[33m\33[1mEnsure UNE_DEBUG_REPORT is enabled.\33[0m")
i = 0
for i, case in enumerate(cases):
	if i+1 < SKIP_UNTIL:
		continue
	une_no_output = '1>' + ('nul' if is_cmd_exe() else '/dev/null') + f' 2>&1 {UNE}'
	une_with_output = UNE
	if HIDE_OUTPUT and not ATTR_NEVER_HIDE_OUTPUT in case.attributes:
		une = une_no_output
	else:
		une = une_with_output
	i += 1
	passed = True
	if ATTR_DIRECT_ARG in case.attributes:
		command = f'{une} {case.input}'
		if ATTR_STDIN in case.attributes:
			command = 'echo test|' + command
		cmd(command)
		if not check_report('main', case, i):
			passed = False
	else:
		if not ATTR_FILE_ONLY in case.attributes:
			sanitized = case.input
			if not ATTR_NO_SECOND_ESCAPE in case.attributes:
				sanitized = sanitized.replace('\\', '\\\\')
			sanitized = sanitized.replace('"', '\\"')
			sanitized = sanitized.replace('\n', '\\n')
			if case.result_kind != UNE_RK_ERROR and not ATTR_NO_IMPLICIT_RETURN in case.attributes:
				sanitized = f'return {sanitized}'
			command = f'{une} -s "{sanitized}"'
			if ATTR_STDIN in case.attributes:
				command = 'echo test|' + command
			cmd(command)
			if not check_report('cmdl', case, i):
				passed = False
				print(f'\33[31m{command}\33[0m')
		if passed or not STOP_AT_FAIL:
			if SHOW_ERRORS and case.result_kind == UNE_RK_ERROR:
				une = une_with_output
			script = case.input
			if case.result_kind != UNE_RK_ERROR and not ATTR_NO_IMPLICIT_RETURN in case.attributes:
				script = f'return {script}'
			status = open(FILE_SCRIPT, 'w')
			status.write(script)
			status.close()
			command = f'{une} "{FILE_SCRIPT}"'
			if ATTR_STDIN in case.attributes:
				command = 'echo test|' + command
			cmd(command)
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
