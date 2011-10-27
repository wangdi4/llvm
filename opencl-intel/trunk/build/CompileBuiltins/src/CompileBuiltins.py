'''
Created on Nov 29, 2010

@author: myatsina
'''
from optparse import OptionParser
import os
import subprocess

'''
Compiling built-in funciton solution
'''
if __name__ == '__main__':
    
    # Adding parser options
    parser = OptionParser()
    parser.add_option('-c', '--config', dest = 'configuration', help = 'Configuration type (Debug, Release), default: Release', default = 'Release')
    parser.add_option('-p', '--platform', dest = 'platform', help = 'Platform type (Win32, x64), default: Win32', default = 'Win32')
    parser.add_option('-d', '--devenv', dest = 'devenvPath', help = 'Devenv path, default: C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\IDE', default = 'C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\IDE')
    parser.add_option('-s', '--solution', dest = 'solution', help = 'Solution')
    parser.add_option('-l', '--log', dest = 'logFile', help = 'Log file')
    
    options = parser.parse_args()[0]
    
    # Changing path to recognize devenv command
    os.environ['Path'] = os.environ.get('Path') + os.pathsep +  options.devenvPath
    
    # Creating build solution command
    command  = 'devenv.com /rebuild "' + options.configuration +  '|' + options.platform + '" ' + options.solution 
    print command

    # Open build log file
    if options.logFile is not None:
        normLog = os.path.normpath(options.logFile)
        if not os.path.exists(os.path.dirname(normLog)):
            os.makedirs(os.path.dirname(normLog))
        logFile = open(normLog, 'w')
        
        proc = subprocess.Popen(command, stdout=logFile, stderr=logFile)
    else:
        proc = subprocess.Popen(command, stderr=subprocess.STDOUT)

    proc.communicate()
        
    if proc.returncode != 0:
        print 'Build failed with exit status: ' + str(proc.returncode)
    
    if options.logFile is not None:    
        logFile.close()
    

