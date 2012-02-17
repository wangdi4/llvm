"""
Simple test framework
"""
import killableprocess, platform, threading, traceback, sys, subprocess
from StringIO import StringIO
from logger import gLog, STYLE

if platform.system() == 'Windows':
    TIMEOUT_RETCODE = 127
else:
    TIMEOUT_RETCODE = -9

demo_mode = False
print_output = True
print_cmd = False

class CommandLineTool:
    def __init__(self):
        self.out = StringIO()
        self.finished = False
        self.stdout = None
        self.print_output = print_output # by default initialize with global value
        self.print_cmd = print_cmd

    def _readerthread(self):
        while True:
            line = self.stdout.readline()
            if line is None or line == '': 
                if self.finished:
                    break;
            else:
                self.out.write(line)
                if( self.print_output ):
                    gLog.write(sys.stdout, '\t' + line.rstrip() + '\n')

    def runCommand(self, command, timeout=-1 ):
        """Returns errorcode, stdout"""
        retcode = 0
        errstr  = ''

        useShell=True
        if platform.system() == 'Windows':
            useShell=False
        
        stdout_thread = None
        
        try:
            if( self.print_cmd):
                gLog.write(sys.stdout, 'Running:' + command + '\n')
        
            proc = killableprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, shell=useShell)
            self.stdout = proc.stdout
            stdout_thread = threading.Thread(target=self._readerthread)
            stdout_thread.setDaemon(True)
            stdout_thread.start()        
            proc.wait(timeout)
            
            retcode = proc.returncode
            if TIMEOUT_RETCODE == retcode:
                errstr = "!!! Task '" + command + "' has timed out (" + str(timeout) + " sec) and was terminated"
                
        except OSError,e:
            errstr = '!!! Exception raised during command execution:' + str(e) + '\n' +  traceback.format_exc()
            retcode = -1
            
        finally:
            self.finished = True  #Lame way to signal the output reader thread that we are finished
            if stdout_thread != None:
                stdout_thread.join()
            if errstr != '':
                if( self.print_output):
                    gLog.write(sys.stdout, errstr + '\n')
                self.out.write(errstr)
            if( self.print_output):    
                sys.stdout.flush()
        
        self.out.seek(0)
        return (retcode, self.out.read())


