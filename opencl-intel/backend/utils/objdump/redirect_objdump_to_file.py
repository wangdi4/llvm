#!/usr/bin/env python
#
# This script modifies copied llvm-objdump files so that dump is redirected to a
# raw_fd_ostream object. Modified files are written to current build dir.
# 1. Define function dumpObjectToFile which is externally visible.
# 2. Rename main function and make it static.
# 3. Rename options to avoid conflicting with LLVM options.
# 4. Replace outs() with *Out which is a raw_fd_ostream object.

from __future__ import print_function
import re
import sys

if __name__ == "__main__":
  if len(sys.argv) < 3:
    print('usage is: ./redirect_objdump_to_file.py input_filenames output_filenames')
    sys.exit(1)

  num_files = (len(sys.argv) - 1) // 2
  input_filenames = sys.argv[1:num_files+1]
  output_filenames = sys.argv[num_files+1:]

  re_include = re.compile('^\s*(//|/\*|\*/|\n|\#include)')
  re_double_quote = re.compile('"')
  re_opt = re.compile('^(static)?\s*cl::((opt|list)<|alias)')

  for input_filename, output_filename in zip(input_filenames, output_filenames):
    try:
      with open(input_filename, 'r') as f:
        lines = f.readlines()
    except EnvironmentError as e:
      print(e)
      sys.exit(1)

    if input_filename.endswith('.cpp'):
      is_llvm_objdump = input_filename.endswith('llvm-objdump.cpp')

      include_insert_point = 0 # insert point of #include ObjectDump.h
      for i, line in enumerate(lines):
        if not include_insert_point and not re_include.search(line):
          include_insert_point = i

        # prefix all options with __ to avoid conflicting with LLVM options.
        if re_opt.search(line):
          line, number_of_subs_quote = re_double_quote.subn('"__', line, 1)
          if not number_of_subs_quote:
            lines[i+1] = re_double_quote.sub('"__', lines[i+1], 1)
        # remove cl::Grouping which won't work after prefixing __
        line = line.replace('cl::Grouping,', '')

        if is_llvm_objdump:
          # prefix main with __ and make it static
          line = line.replace('int main(', 'static int __main(')
        # dump to *Out instead
        lines[i] = line.replace('outs()', '(*Out)')

      lines.insert(include_insert_point,
          '#include "ObjectDump.h"\n' +
          'using namespace Intel::OpenCL::DeviceBackend::Utils;\n\n');

      if is_llvm_objdump:
        lines.append('\n' +
            'namespace Intel {\n' +
            'namespace OpenCL {\n' +
            'namespace DeviceBackend {\n' +
            'namespace Utils {\n\n' +
            'void dumpObjectToFile(ObjectFile *O) {\n' +
            '  Disassemble = true;\n' +
            '  DisassembleAll = true;\n' +
            '  ShowRawInsn = false;\n' +
            '  Relocations = true;\n' +
            '  SectionHeaders = true;\n' +
            '  SymbolTable = true;\n\n' +
            '  dumpObject(O);\n' +
            '}\n\n' +
            '} // namespace Utils\n' +
            '} // namespace DeviceBackend\n' +
            '} // namespace OpenCL\n' +
            '} // namespace Intel\n');

    # write to output file
    try:
      with open(output_filename, 'w') as f:
        f.writelines(lines)
    except EnvironmentError as e:
      print(e)
      sys.exit(1)
