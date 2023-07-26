import os
import re
import sys
import subprocess

from .common import debug

def get_default_options_for_tool(ti, toolpath):
  default_options = ""
  if sys.platform == "win32":
     llvmlit="llvm-lit.py"
  else:
     llvmlit="llvm-lit"
  litcmd = os.path.join(os.path.dirname(toolpath), llvmlit)
  debug("litcmd: {}".format(litcmd))
  toolbasename = os.path.basename(toolpath)
  if "clang" in toolbasename:
      toolbasename = "clang_cc1"
  litcmds=["python"]
  litcmds.append(litcmd)
  litcmds.append(ti.path)
  try:
    finalsubsts = subprocess.check_output(
      litcmds + ['--show-final-subst']).decode().strip()
  except subprocess.CalledProcessError:
     debug("Error running {}".format(litcmds))
     return default_options
  toolpattern = re.compile(r'.* => {}\s*(.*)'.format(toolbasename))
  for subst in finalsubsts.splitlines():
     p = toolpattern.match(subst)
     if p:
         default_options = p.group(1)
         debug('Default options extracted from lit cfg are: {}'.format(default_options))
         return default_options
  return default_options

