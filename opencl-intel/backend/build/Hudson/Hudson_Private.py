from optparse import OptionParser
import os, sys, platform, re
import framework.cmdtool
import framework.resultPrinter
import framework.svn_utils
from Volcano_Common import REPOSITORY_ROOT
from Hudson_Common import VOLCANO_JENKINS_URL
from framework.hudson.utils import startJob
from getpass import getuser

SANITY_BRANCHES_ROOT = 'OpenCL/branches/sanity'
SANITY_SVN_ROOT      = '/'.join([REPOSITORY_ROOT, SANITY_BRANCHES_ROOT])
SANITY_HUDSON_JOB    = 'PrivatePreCommit'

class ConfigError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)   

class LogicError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)   

class PersonalBuildConfig:
    """
    Configuration used for personal build
    """
    def __init__(self, root_dir, branch_suffix, username):
        self.username = username
        self.hosttype = self.detectOS()
        # Current root path
        self.root_dir       = root_dir
        if self.root_dir == '' or self.root_dir == None:
            self.root_dir = os.path.join(os.path.curdir, os.path.pardir, os.path.pardir, os.path.pardir, os.path.pardir)
        self.root_dir = os.path.abspath(self.root_dir)
        # Current user
        if '' == self.username or None == self.username:
            self.username = getuser()
        # Target branch
        self.branch_name = '/'.join([SANITY_SVN_ROOT, self.username, ] )
        self.branch_name_full = self.branch_name if( "" == branch_suffix ) else '/'.join([self.branch_name, branch_suffix])

    def detectOS(self):
        hosttype = "unknown"
        operating_system_name = platform.system()
        if operating_system_name.find('CYGWIN') != -1:
           hosttype = "cygwin"
        elif operating_system_name == 'Windows':
           hosttype = "windows"
        elif operating_system_name == 'Linux':
           hosttype = "linux"
        elif operating_system_name == 'Darwin':
           hosttype = "mac"
        elif operating_system_name == 'FreeBSD':
           hosttype = "bsd"
        else:
           raise ConfigError("Configuration Error: Could not detect operating system type:" + operating_system_name)
        return hosttype

def create_branch(svntool, branch_name, force_remove, reuse_branch):
    "Make the required branch directories and commit current tree to the branch"
    if svntool.test_branch_exists(branch_name):
        if force_remove :
            print 'Removing existing branch:' + branch_name
            svntool.remove_branch(branch_name)
        elif reuse_branch :
            print 'Reusing existing branch. Local changes in your working copy will NOT be submitted'
            return
        else:
            raise LogicError("You already have a branch called:\n\t"+ branch_name +"\n\tTo remove it rerun sanity, using the --force option. To reuse it use the --reuse option.")
    else:
        #Make sure that parent branch exists
        parent_branch_name = branch_name.rpartition('/')[0]
        print 'Making sure that the parent branch exist: ' + parent_branch_name
        svntool.test_and_make_branch(parent_branch_name)

    print 'Copying local working copy to the target branch: ' + branch_name
    svntool.copy_to_branch(branch_name)

def main():
    parser = OptionParser()
    parser.add_option("-r", "--root",         action="store",        dest="root_dir",      default=None,     help="Project root directory. Default: ../../../../")
    parser.add_option("-b", "--branch",       action="store",        dest="branch_name",   default='sanity', help="One word description of your branch. Default is \'sanity\' ")
    parser.add_option("-f", "--force",        action="store_true",   dest="force_remove",  default=False,    help="Force to remove the already existing branch if one exist. Default: False")
    parser.add_option("-u", "--rebuild",      action="store_true",   dest="reuse_branch",  default=False,    help="Just run the personal build on already existing branch. No local changes will be copied. If the given branch doesn't exist this option is ignored. This option is usefull if your personal build has failed because of some environment error and you just want to re-run it. Default: False")
    parser.add_option("-e", "--email",        action="store",        dest="email",                           help="EMail address of the job notification receipients.Multiple e-mail addresses could be specified, comma separated")
    parser.add_option("-v", "--verbose",      action="store",        dest="verbose_level", default= 0,       help="Verbosity level: 1 - print actual commands, 2 - print also commands output. Default: 0")
    parser.add_option("--username",           action="store",        dest="username",      default="",       help="User name used for SVN authentication. Default: current username")
    parser.add_option("--password",           action="store",        dest="password",      default="",       help="Password for SVN authentication. Default: used cached")
    parser.add_option("--non-interactive",    action="store_false",  dest="interactive",   default=True,     help="Do no interactive prompting. Default: interactive")

    (options, args) = parser.parse_args()

    # Process the command line options
    if re.search(r"[.]",options.branch_name):
        parser.error("Due to extremely silly IT email filters, we cannot safely have a dot in the review branch file name;\n\tUse a dash.")

    if options.reuse_branch and options.force_remove: 
        parser.error("Command line options conflict. You can't reuse the branch if you selected to forcefully remove it")

    if '' == options.email or None == options.email:
        parser.error("Please supply the e-mail address (-e option) for job run result notification. It can't be detected from your environment")

    # Configure the environment
    framework.cmdtool.print_cmd    = int(options.verbose_level) > 0
    framework.cmdtool.print_output = int(options.verbose_level) > 1

    try:
        config = PersonalBuildConfig(options.root_dir, 
                                     options.branch_name,
                                     options.username)
        os.chdir(config.root_dir)

        # figure out what the SVN urls look like
        svntool= framework.svn_utils.SvnTool( config.hosttype, 
                                              config.username, 
                                              options.password,
                                              options.interactive)
        (local_root,local_url,is_dir) = svntool.svn_info('.')
        if local_root == '':
           raise Exception("No local root specified by svn info." + local_url )

        print 'Personal build'
        print '--------------'
        print '[WORKCOPY ROOT] :' + config.root_dir
        print '[SVN ROOT]      :' + local_root
        print '[SVN URL]       :' + local_url
        print '[TARGET BRANCH] :' + config.branch_name_full

        # then create a branch in there
        print '[PREPARING BRANCH]'
        create_branch(svntool, config.branch_name_full, options.force_remove, options.reuse_branch)

        # prepare the job parameters
        print '[START JENKINS JOB]'
        jobparams = { "SVN_Requested_Url": config.branch_name_full,
                      "EmailReceipients":options.email}
        startJob(VOLCANO_JENKINS_URL, SANITY_HUDSON_JOB, jobparams)
        print '[FINISHED]      :The job notification will be sent to: ' + options.email
    except LogicError as e:
        print "Error." + e.value
        print "[ABORTED]"
        return 1
    except ConfigError as e:
        print "Configuration Error."  + e.value
        print "[ABORTED]"
        return 1
    except framework.svn_utils.SvnError as e:
        print "SVN Error: " + e.value
        if options.verbose_level < 2 :
            print "Consider increasing the verbosity level (-v) option to get more information"
        print "[ABORTED]"
        return 1
    except:
        raise
        
    return 0

if __name__ == "__main__":
    if platform.platform().startswith("CYGWIN"):
        print "Cygwin Python is not supported. Please use ActiveState Python."
        sys.exit(1);
    if sys.version_info < (2, 6):
        print "Python version 2.6 or later required"
        sys,exit(1)
    main_result = main()
    sys.exit(main_result)

