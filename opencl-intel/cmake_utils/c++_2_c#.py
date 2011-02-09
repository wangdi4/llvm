#!python

import sys,os,re

def parse():
    if len(sys.argv) < 3 :
        print """
Usage: """, sys.argv[0],""" <.sln-file-name> <project-name-to-convert>

Convert project type from C++ to C# in the Dev Studio Solution 2008 file.
"""
        exit(1)

    file = sys.argv[1]
    proj = sys.argv[2:]
    return (file, proj)


def isPatternsExist(line, patterns):
    for pattern in patterns:
	if pattern.search(line):
	    return True
    return False
	
def main():
    (file, proj) = parse()
    tmpfile_name = file + '.tmp'
    oldfile_name = file + '.orig'
    CXX_UUID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
    C_SHARP_UUID = '{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}'

    try:
        infile = open(file, 'r')
    except IOError,(errno, strerror):
        print 'Cannot open input file',file,':',strerror
        sys.exit(1)
    
    try:
        tmpfile = open(tmpfile_name, 'w')
    except IOError,(errno, strerror):
        print 'Cannot open output file',tmpfile_name,':',strerror
        sys.exit(1)

    patterns = []
    for p in proj:
	patterns.append(re.compile('^Project.+"' + p + '".+'))
    for line in infile:
        if isPatternsExist(line, patterns): #pattern.search(line):
            line = re.sub(CXX_UUID,C_SHARP_UUID,line)
        tmpfile.write(line)

    infile.close()
    tmpfile.close()

    try:
        if os.path.isfile( oldfile_name ):
            os.remove(oldfile_name)
    except IOError,(errno, strerror):
        print 'Cannot remove file',oldfile_name,':',strerror
        sys.exit(1)

    try:
        os.rename( file, oldfile_name )
    except WindowsError,(errno, strerror):
        print 'Cannot rename file',file, 'to',oldfile_name,':',strerror
        sys.exit(1)

    try:
        os.rename( tmpfile_name, file )
    except WindowsError,(errno, strerror):
        print 'Cannot rename file',tmpfile_name, 'to',file,':',strerror
        print 'Reverting....'
        os.rename( oldfile_name, file )
        print 'Done'
        sys.exit(1)

# main
if __name__ == "__main__":
    main()
    sys.exit(0)

        
        
        
        
    
