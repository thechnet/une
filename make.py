#!python3
# Make Une

import os
import sys
import argparse
import shutil
cmd = os.system

class Builder:
  def __init__(self, args):
    self.args = args
    self.O = 0
    self.g = 0
    self.D = []
    self.W = ["pedantic", "all", "no-switch", "no-unused-function"]
    self.other = []
    self._feedback = ""
  
  def get_source_files(self, where):
    source_files = []
    for root, dirs, files in os.walk(where):
      for file in files:
        if not file.endswith(".c"):
          continue
        source_files.append(os.path.join(root, file))
    return source_files
  
  def assemble_source_files(self, source_files):
    source_files_str = "\"" + "\" \"".join(source_files) + "\""
    return source_files_str
  
  def assemble_flags(self):
    O_str = "-O" + str(self.O)
    g_str = "-g" + str(self.g)
    D_str = " ".join(["-D" + define for define in self.D])
    W_str = " ".join(["-W" + warn for warn in self.W])
    other_str = " ".join(self.other)
    flags = " ".join(filter(None, [O_str, g_str, D_str, W_str, other_str, args.flags]))
    return flags
  
  def assemble_build_command(self, flags_str, source_files_str):
    out_str = "-o " + self.args.out
    build_command = " ".join(filter(None, [self.args.toolchain, flags_str, out_str, source_files_str]))
    return build_command
  
  def compile(self):
    source_files = self.get_source_files(self.args.dir)
    source_files_str = self.assemble_source_files(source_files)
    flags_str = self.assemble_flags()
    build_command = self.assemble_build_command(flags_str, source_files_str)
    self._feedback = " ".join(filter(None, [self.args.toolchain, flags_str]))
    if not cmd(build_command) == 0:
      exit(1)
  
  def post_run(self):
    pass
  
  def build(self):
    if args.clear:
      cmd("cls" if sys.platform == "win32" else "clear")
    self.compile()
    if args.run:
      cmd("./" + self.args.out + " " + args.run)
    self.post_run()
    print("\33[35m" + self._feedback + "\33[0m")

class ReleaseBuilder(Builder):
  def __init__(self, args):
    super().__init__(args)
    self.O = 3
    self.W.append("no-unused-function")

class DebugBuilder(Builder):
  def __init__(self, args):
    super().__init__(args)
    self.g = 3
    self.D.append("UNE_DEBUG")

class GcovBuilder(Builder):
  def __init__(self, args):
    super().__init__(args)
    self.out = "./private/gcov/une" + (".exe" if sys.platform == "win32" else "")
    self.args.dir = os.path.join("../../", self.args.dir)
    self.other.append("--coverage")
  
  def clear_gcov(self):
    try:
      shutil.rmtree('./src')
    except:
      pass
    for root, dirs, files in os.walk("."):
      for file in files:
        try:
          os.remove(file)
        except:
          pass
  
  def run_tests(self):
    _dir = os.getcwd()
    os.chdir("../../testing")
    cmd("../private/gcov/une stable.une")
    cmd("python3 errors.py")
    os.chdir(_dir)
  
  def build(self):
    if args.clear:
      cmd("cls" if sys.platform == "win32" else "clear")
    _dir = os.getcwd()
    os.chdir("private/gcov")
    self.clear_gcov()
    self.compile()
    self.run_tests()
    self.post_run()
    os.chdir(_dir)
    print("\33[35m" + self._feedback + "\33[0m")
  
  def post_run(self):
    cmd("lcov --capture --directory . --output-file=coverage.info")
    cmd("genhtml coverage.info --output-directory=.")
    cmd(("start" if sys.platform == "win32" else "open")+" index.html")

class ReleasegcovBuilder(GcovBuilder):
  def __init__(self, args):
    super().__init__(args)
    self.O = 3
    self.W.append("no-unused-function")

class DebuggcovBuilder(GcovBuilder):
  def __init__(self, args):
    super().__init__(args)
    self.g = 3
    self.D.append("UNE_DEBUG")

TOOLCHAINS = ["gcc", "clang"]
BUILD_MODES = ["release", "debug", "releasegcov", "debuggcov"]

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = "Make Une.")
  parser.add_argument("-m", dest="mode", choices=BUILD_MODES, default=BUILD_MODES[0], help="build mode")
  parser.add_argument("-cc", dest="toolchain", choices=TOOLCHAINS, default=TOOLCHAINS[0], help="toolchain")
  parser.add_argument("-d", dest="dir", type=str, default="src", help="source directory")
  parser.add_argument("-o", dest="out", type=str, default="une" + (".exe" if sys.platform == "win32" else ""), help="output file")
  parser.add_argument("-r", dest="run", type=str, help="script to run after successful build")
  parser.add_argument("-nc", dest="clear", action="store_false", help="do not clear console")
  parser.add_argument("-f", dest="flags", type=str, help="additional toolchain flags")
  args = parser.parse_args()
  
  builder = eval(args.mode.capitalize() + "Builder(args)")
  builder.build()
