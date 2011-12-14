"""
SVN specific utilities
"""
import os,re
import cmdtool
      
def cond_die(v, cmd, msg, lines=''):
   if v != 0:
      if lines != '':
         print(lines)
      s = msg + "\n  [CMD] " + cmd + ' returns:' + str(v)
      raise Exception( s )

class SvnTool:
    """
    Simple wrapper around SVN command line
    """
    def __init__(self, hosttype):
        self.hosttype = hosttype
        self.svn_cmd = self.get_svn_cmd()
        self.svn_cmd_local = self.get_svn_cmd_local()
        self.check_svn_version()
        self.verbosity = 0

    def get_svn_cmd(self):
        linux_svn     = '/usr/local/bin/svn'
        mac_svn       = '/usr/local/bin/svn'
        bsd_svn       = '/usr/local/bin/svn'
        cygwin_svn    = '/bin/svn'
        windows_svn   = 'svn'
        
        if self.hosttype == "linux":
            return usr_intel_svn
        elif self.hosttype == "mac":
            return mac_svn
        elif self.hosttype == "cygwin":
            return cygwin_svn
        elif self.hosttype == "windows":
            return windows_svn
        elif self.hosttype == "bsd":
            return bsd_svn
        else:
            return 'svn'
            
    def get_svn_cmd_local(self):
        if 'SVNCMD' in os.environ:
          svn = os.environ['SVNCMD']
          if os.path.exists(svn):
             if ' ' in svn:
                svn = '"' + svn + '"'
             return svn
          raise Exception("Could not find svn command locally using env var SVNCMD: " + svn )
          
        svn = self.get_svn_cmd()
        if svn != "svn":
          if os.path.exists(svn):
             return  svn
          raise Exception("Could not find svn command locally: " + svn )
        return svn

    def check_svn_version(self):
        svn_cmd = self.svn_cmd_local
        svn_cmd += ' --version'

        cmd_tool = cmdtool.CommandLineTool()
        (retval, lines) = cmd_tool.runCommand(svn_cmd)
        cond_die(retval,svn_cmd, "Could not check svn version.",lines)

        if '' == lines:
          raise Exception("svn version check had no output.")

        p = re.search(r'.*version ([0-9.]+)',lines)
        if not p:
          raise Exception("Could not find svn version in line: " + lines)
        version = p.group(1)
        
        digits = version.split('.')
        if len(digits) < 2:
          raise Exception("svn version check could not find proper version number in: " + lines)
        if digits[0] >= '1':
          if digits[1] >= 6:
              return
        raise Exception("Svn version was not sufficient: " + version + " -- Must be >= 1.6.0")

    def svn_info(self, path = '.'):
        cmd = self.svn_cmd_local + " info "
        
        if self.hosttype == "windows":
            cmd = cmd + path
        else:
            cmd = cmd + '\''+ path + '\''
        
        cmd_tool = cmdtool.CommandLineTool()
        (retval, stdout) = cmd_tool.runCommand(cmd)
        
        cond_die(retval,cmd, "Could not get info about the path:",path)
        
        lines = stdout.split('\n')
        url_pattern  = re.compile(r"^URL:")
        root_pattern = re.compile(r"^Repository Root: ")
        dir_pattern  = re.compile(r"^Node Kind: directory")

        url = ''
        root = ''
        is_dir = False
        
        for line in lines:
            if url_pattern.search(line):
                url = re.sub('^URL: ','',line.strip())
            if root_pattern.search(line):
                root = re.sub('^Repository Root: ','',line.strip())
            if dir_pattern.search(line):
                is_dir = True
        return (root,url,is_dir)

    def test_branch_exists(self, branch_name):
        cmd = self.svn_cmd_local + " info "
        
        if self.hosttype == "windows":
            cmd = cmd + branch_name
        else:
            cmd = cmd + '\''+ branch_name + '\''
        
        cmd_tool = cmdtool.CommandLineTool()
        (retval, stdout) = cmd_tool.runCommand(cmd)
        
        lines = stdout.split('\n')
        dir_pattern  = re.compile(r"^Node Kind: directory")
        url_err_pattern = re.compile(r"Not a valid URL")
        
        branch_exist = False

        if 0 == retval:
            for line in lines:
                if dir_pattern.search(line):
                    return True
        else:
            for line in lines:
                if url_err_pattern.search(line):
                    return False
            
        cond_die(retval, cmd, "Could not get info about the path:",branch_name)

    def mkdir_branch(self, branch_name):
        "make the svn branch directory"
        cmd = self.svn_cmd_local + ' mkdir ' + branch_name + ' -m sanity-testing'

        cmd_tool = cmdtool.CommandLineTool()
        (retval, lines) = cmd_tool.runCommand(cmd)
        cond_die(retval,cmd, "Could not create branch.",lines)
        
    def test_and_make_branch(self, branch_name):
        if not self.test_branch_exists(branch_name):
            parent_branch_name = branch_name.rpartition('/')[0]
            self.test_and_make_branch(parent_branch_name)
            self.mkdir_branch(branch_name)
        
    def remove_branch(self, branch_name):
        "Run the command to remove the branch"
        cmd = self.svn_cmd_local + ' rm ' + branch_name + ' -m sanity-testing-remove-branch'

        cmd_tool = cmdtool.CommandLineTool()
        (retval, lines) = cmd_tool.runCommand(cmd)
        cond_die(retval,cmd, "Could not remove branch.",lines)

    
            
    def copy_to_branch(self, branch_name):
        "Copy the current tree to the  branch"
        cmd = self.svn_cmd_local + ' copy . ' + branch_name  + ' -m sanity-testing-create-branch'

        cmd_tool = cmdtool.CommandLineTool()
        (retval, stdout) = cmd_tool.runCommand(cmd)
        cond_die(retval,cmd, "Could not copy branch.",stdout)

        return branch_name
