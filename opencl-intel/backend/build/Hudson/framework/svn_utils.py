"""
SVN specific utilities
"""
import os,re
import cmdtool
from getpass import getpass

class SvnError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)   

      
def cond_die(v, cmd, msg):
   if v != 0:
      s = "Command '" + cmd + "' failed. Returns:" + str(v) + ' : ' + msg
      raise SvnError( s )

class SvnTool:
    """
    Simple wrapper around SVN command line
    """
    def __init__(self, hosttype, username, password, interactive):
        self.hosttype = hosttype
        self.svn_cmd = self.get_svn_cmd()
        self.svn_cmd_local = self.get_svn_cmd_local()
        self.check_svn_version()
        self.verbosity = 0
        self.username = username
        self.password = password
        self.interactive = interactive
        self.retrycount = 1 if self.interactive == False else 3
        
    def normalize_path(self, path):
        if self.hosttype == "windows":
            return path
        return '\''+ path + '\''

    def get_svn_cmd(self):
        linux_svn     = 'svn'
        mac_svn       = 'svn'
        bsd_svn       = 'svn'
        cygwin_svn    = 'svn'
        windows_svn   = 'svn'
        
        if self.hosttype == "linux":
            return linux_svn
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
          raise SvnError("Could not find svn command locally using env var SVNCMD: " + svn )
          
        svn = self.get_svn_cmd()
        if svn != "svn":
          if os.path.exists(svn):
             return  svn
          raise SvnError("Could not find svn command locally: " + svn )
        return svn

    def check_svn_version(self):
        svn_cmd = self.svn_cmd_local
        svn_cmd += ' --version'

        cmd_tool = cmdtool.CommandLineTool()
        (retval, lines) = cmd_tool.runCommand(svn_cmd)
        cond_die(retval, svn_cmd, "Could not check svn version.\n" + lines)

        if '' == lines:
          raise SvnError("svn version check had no output.")

        p = re.search(r'.*version ([0-9.]+)',lines)
        if not p:
          raise SvnError("Could not find svn version in line: " + lines)
        version = p.group(1)
        
        digits = version.split('.')
        if len(digits) < 2:
          raise SvnError("svn version check could not find proper version number in: " + lines)
        if digits[0] >= '1':
          if digits[1] >= 6:
              return
        raise SvnError("Svn version was not sufficient: " + version + " -- Must be >= 1.6.0")

    def svn_run_cmd_internal(self, cmd, *args):
        "Executes the given SVN command"
        svn_cmd = self.svn_cmd_local + ' ' + cmd
        
        if( None != self.username and '' != self.username):
            svn_cmd += ' --username ' + self.username
            
        if( None != self.password and '' != self.password):
            svn_cmd += ' --password ' + self.password
        
        # Allways use the non-interactive mode. Handle the password prompt internally
        svn_cmd += ' --non-interactive '
        svn_cmd += ' '.join(args)
        
        cmd_tool = cmdtool.CommandLineTool()
        return cmd_tool.runCommand(svn_cmd)
        

    def svn_run_cmd(self, cmd, *args):
        "Executes the given SVN command, optionally taking care of username password interactive prompt"
        retval = -1
        stdout = ''
        authz_failed = False

        for c in range(self.retrycount):
            (retval, stdout) = self.svn_run_cmd_internal(cmd, *args)

            if 0 == retval:
                return (retval, stdout)
            else:
                lines = stdout.split('\n')
                pattern  = re.compile(r"authorization failed")
                for line in lines:
                    if pattern.search(line):
                        authz_failed = True

                if( authz_failed ):
                    if( self.interactive):
                        self.password = getpass("Password for '%(username)s':" %{"username":self.username})
                    else:
                        cond_die(retval, cmd, 'Authorization failed. Please supply the valid credentials or use the interactive mode')
                else:
                    break;

        return (retval, stdout)

    def svn_info(self, path = '.'):
        "Returns the information about given path. Path could be a local working copy or SVN URL"
        (retval, stdout) = self.svn_run_cmd('info', self.normalize_path(path))
        
        cond_die(retval, 'svn info', "Could not get info about the path: " + path)
        
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
        "Tests if the given URL exists"
        (retval, stdout) = self.svn_run_cmd('info', self.normalize_path(branch_name))

        lines = stdout.split('\n')
        dir_pattern  = re.compile(r"^Node Kind: directory")
        url_err_pattern = re.compile(r"Not a valid URL")
        
        if 0 == retval:
            for line in lines:
                if dir_pattern.search(line):
                    return True
        else:
            for line in lines:
                if url_err_pattern.search(line):
                    return False

        cond_die(retval, 'svn info', "Could not get info about the path: " + branch_name)
        return False

    def mkdir_branch(self, branch_name):
        "Make the svn branch directory"
        (retval, stdout) = self.svn_run_cmd('mkdir', self.normalize_path(branch_name), '-m sanity-testing')
        cond_die(retval, 'svn mkdir', "Could not create branch." + stdout)
        
    def test_and_make_branch(self, branch_name):
        "Ensure that the given branch exists, creating it if needed"
        if not self.test_branch_exists(branch_name):
            parent_branch_name = branch_name.rpartition('/')[0]
            self.test_and_make_branch(parent_branch_name)
            self.mkdir_branch(branch_name)
        
    def remove_branch(self, branch_name):
        "Remove the given branch"
        (retval, stdout) = self.svn_run_cmd('rm', self.normalize_path(branch_name), '-m sanity-testing-remove-branch')
        cond_die(retval, 'svn rm', "Could not remove branch.\n" + stdout)

    def copy_to_branch(self, branch_name):
        "Copy the current working copy to the  branch"
        (retval, stdout) = self.svn_run_cmd('copy', '.', self.normalize_path(branch_name), '-m sanity-testing-create-branch')
        cond_die(retval, 'svn copy', "Could not copy branch.\n" + stdout)
        return branch_name
