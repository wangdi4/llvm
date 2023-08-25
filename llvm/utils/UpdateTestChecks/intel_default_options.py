import os
import re
import sys
import subprocess
import shutil
from os import path

from .common import debug

def get_default_options_for_tool(ti, toolpath):
  lit = 'llvm-lit.py' if sys.platform == 'win32' else 'llvm-lit'
  lit_in_tooldir = path.join(path.dirname(toolpath), lit)
  if not path.isfile(lit_in_tooldir):
      lit_in_tooldir = None
  lit_in_path = shutil.which(lit)
  lit = lit_in_tooldir if lit_in_tooldir else lit_in_path
  if not lit:
      debug(f'Not found {lit}')
      return ''
  litcmds = [sys.executable, lit, ti.path, '--show-final-subst']
  debug(f'litcmds: {litcmds}')
  finalsubsts = subprocess.run(litcmds, stdout=subprocess.PIPE)
  if finalsubsts.returncode:
      return ''
  finalsubsts = finalsubsts.stdout.decode().strip()
  toolbasename = path.basename(toolpath)
  if 'clang' in toolbasename:
      toolbasename = 'clang_cc1'
  toolpattern = re.compile(r'.* => {}\s*(.*)'.format(toolbasename))
  for subst in finalsubsts.splitlines():
     p = toolpattern.match(subst)
     if p:
         default_options = p.group(1)
         debug(f'Default options extracted from lit cfg are: {default_options}')
         return default_options
  return ''
