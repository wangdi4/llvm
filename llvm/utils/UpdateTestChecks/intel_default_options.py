import os
import re
import subprocess

from .common import debug

def get_default_options_for_tool(ti, toolpath):
  default_options = ""
  litcmd = "{}/llvm-lit".format(os.path.dirname(toolpath))
  if not os.path.exists(litcmd):
      return default_options
  toolbasename = os.path.basename(toolpath)
  if "clang" in toolbasename:
      toolbasename = "clang_cc1"
  litcmds=[]
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

