from sets import Set
import xml.etree.ElementTree as ET
import re
import os
import sys
import argparse

# This maps feature names as found in the XML file to _FEATURE_ names expected
# by the compiler. Names that do not appear do not require translation
# TODO: Missing translations: PREFETCHWT1, TSC, FSGSBASE, MONITOR, RDTSCP
XMLToFeature = {
  "BMI1" : "BMI",
  "BMI2" : "BMI",
  "FP16C" : "F16C",
  "RDRAND" : "RDRND",
  "SSE4.1" : "SSE4_1",
  "SSE4.2" : "SSE4_2", 
  "PREFETCHWT1" : "GENERIC_IA32",
  "TSC" : "GENERIC_IA32",
  "FSGSBASE" : "GENERIC_IA32",
  "MONITOR" : "GENERIC_IA32",
  "RDTSCP" : "GENERIC_IA32",
  "AVX512F/KNCNI" : "AVX512F",
  "AVX512PF/KNCNI" : "AVX512PF",
  "FXSR" : "FXSAVE",
  "XSAVE": "GENERIC_IA32",
  "XSAVEOPT": "GENERIC_IA32",
  "XSAVEC": "GENERIC_IA32",
  "XSS": "GENERIC_IA32"
}

allIntrinsics = Set([])
intrinsicTech = {}

# Matches a function prototype. This relies on the fact that the coding style used in
# the intrinsic files is:
# static __inline__ <return-type> __attribute__((__always_inline__, __nodebug__))
# _<intrinsic-name>(<param-list>)
funcRegEx = re.compile(r'^(_.*?)\s*\(.*$')

# Matches a macro definition. The first group matches #define, the second
# the intrinsic name, and the third the param list. The fourth and fifth are
# used to distiniguish a multi-line macro from a single-line macro.
macroRegEx = re.compile(r'^(#define\s+)(_.*?)(\(.*?\))([^\\]*)(\\?)')

# Matches a "simple" macro definition, that simply re-defines an intrinsic
# in terms of an equivalent intrinsic, e.g.
# #define _mm_foo _mm_bar
simpleMacroRegEx = re.compile(r'^#define\s+(_.*?)\s')

# Matches "static __inline" with any number of spaces
staticInline = re.compile(r'^static\s+__inline') 

# Match do-while statemant.
# In macros we need to use semicolon instead of comma in do-while cases.
doWhile = re.compile(r'^do\s*\{(\n|.)*\}\s*while', re.M)

def handleFunction(line, inFile, outFile):
  # TODO: The original line may have the funcDecl, not the next line...
  # (This is not likely though, given how the intrinsic headers are written)
  outFile.write(line)
  # We expect the next line to start the prototype
  # TODO: There may be more than one line here.
  # (This actually happens for mm_malloc!)
  funcDecl = inFile.readline()
  outFile.write(funcDecl)
  stripped = funcDecl.strip()
  # Check that the line actually contains a function we're interested in
  match = funcRegEx.match(stripped)
  if match is None:
    return
  name = match.group(1)
  tech = intrinsicTech.get(name)
  if tech is None:
    return
  allIntrinsics.remove(name)
  # TODO: This assumes that after an opening brace, there is nothing on the same line
  while not stripped.endswith('{'):
    line = inFile.readline()
    outFile.write(line)
    stripped = line.strip()
  outFile.write('  __builtin_assume(__builtin_has_cpu_feature(%s));\n' % tech)

def handleMacro(line, inFile, outFile):
  # The "prototype" should be on the same line
  stripped = line.strip()
  match = macroRegEx.match(stripped)
  if match is None:
    outFile.write(line)
    # If this is a "define foo", remove it anyway
    match = simpleMacroRegEx.match(stripped)
    if match is not None:
      name = match.group(1)
      allIntrinsics.discard(name)
    return
  name = match.group(2)
  tech = intrinsicTech.get(name)
  if tech is None:
    outFile.write(line)
    # TODO: Should probably read to the end of the macro instead of returning early!
    return
  allIntrinsics.remove(name)
  # Output the prototype
  for i in [1, 2, 3]:
    outFile.write(match.group(i))
  # create string with the body of the define
  body = []
  body.append(match.group(4))
  if match.group(5) == '':
    outFile.write(' \\\n  (__builtin_assume(__builtin_has_cpu_feature(%s)), \\\n' % tech)
    outFile.write(''.join(body) + ')\n')
    return
  # The first line ended with a backslash. Write the backslash back
  body.append('\\\n')
  # Now, find the first line that doesn't end with a backslash, 
  # and add the closing paren to it.
  while True:
    line = inFile.readline()
    if not line.strip().endswith('\\'): break
    body.append(line)
  body.append(line.rstrip().rstrip(';'))
  # check if we have do-while statemant, in that case, use semicolon operator
  bodyStr = ''.join(body)
  doMatch = doWhile.search(bodyStr)
  if doMatch:
    # add the assume followed by the semicolon operator
    outFile.write(' \\\n  do {__builtin_assume(__builtin_has_cpu_feature(%s)); \\\n' % tech)
    outFile.write(bodyStr + '; \\\n')
    outFile.write('} while (0) \n')
  else:
    # add the assume followed by the comma operator
    outFile.write(' \\\n  (__builtin_assume(__builtin_has_cpu_feature(%s)), \\\n' % tech)
    outFile.write(bodyStr + ')\n')

def parseDataXML(xmlfile):
  root = ET.parse(xmlfile)
  for elem in root.findall("intrinsic"):
    # TODO: Handle elements with more than one CPUID
    IDs = elem.findall("CPUID")
    tech = elem.get("tech")
    # TODO: Figure out what to do with AVX512
    if (IDs and not "SVML" in tech):
      name = elem.get("name")
      IDs = ["_FEATURE_" + XMLToFeature.get(n.text, n.text) for n in IDs]
      intrinsicTech[name] =  '|'.join(IDs)
      allIntrinsics.add(name)

def main():
  argParser = argparse.ArgumentParser(description='Add __builtin_assume() calls to intrinsic header files')
  argParser.add_argument('descfile', type=argparse.FileType(),
    help='XML file describing the intrinsics and their feature associations')
  argParser.add_argument('dir', help='Directory where intrinsic header files are found')
  argParser.add_argument('filelist', nargs=argparse.REMAINDER, help='List of header files to process')
  args = argParser.parse_args()
  
  parseDataXML(args.descfile)
  includeString = '#include <__x86intrin_features.h>\n'
  for file in args.filelist:
    fullName = os.path.join(args.dir, file)
    tempName = fullName + ".tmp"
    os.rename(fullName, tempName)
    with open(tempName, "r") as inFile:
      line = inFile.readline()
      if line == includeString:
        # The file has already been processed by a previous invocation
        inFile.close()
        os.rename(tempName, fullName)
        continue
      print "Adding assumes to " + file
      with open(fullName, "w") as outFile:
        outFile.write(includeString);
        while True:
          if not line: break
          # TODO: Do better checking here
          if staticInline.match(line):
            handleFunction(line, inFile, outFile)
          elif line.startswith("#define"):
            handleMacro(line, inFile, outFile)
          else: 
            outFile.write(line)
          line = inFile.readline()
    os.remove(tempName)

if __name__ == "__main__":
  sys.exit(main())

