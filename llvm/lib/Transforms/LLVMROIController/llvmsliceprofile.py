#!/usr/bin/env python
# BEGIN_LEGAL
# BSD License
#
# Copyright (c)2016 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.  Redistributions
# in binary form must reproduce the above copyright notice, this list of
# conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.  Neither the name of
# the Intel Corporation nor the names of its contributors may be used to
# endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
# ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# END_LEGAL
#
#
# @ORIGINAL_AUTHORS: T. Mack Stallcup, Cristiano Pereira, Harish Patil,
#  Chuck Yount
#

#
# Read in a file of frequency vectors (BBV or LDV) and execute one of several
# actions on it.  Default is to generate a regions CSV file from a BBV file.
# Other actions include:
#   normalizing and projecting FV file to a lower dimension
#
# November 2016: Modified by Harish Patil to create LLVMPoints
# Requires Python scripts from SDE (PinPlay) kit (sde-pinplay-*-lin)
# Used by  the script 'runllvmsimpoint.sh'


import datetime
import glob
import math
import argparse
import os
import random
import re
import sys

err_msg = lambda string: 'This is not a valid ' + string +  '\nUse -h for help.'


def PrintAndExit(msg):
    """
    Prints an error message exit.

    """
    print msg
    sys.exit(-1)


def OpenFile(fl):
    """
    Check to make sure a file exists and open it.

    @return file pointer
    """

    # import pdb;  pdb.set_trace()
    if not os.path.isfile(fl):
        PrintAndExit('File does not exist: %s' % fl)
    try:
        fp = open(fl, 'rb')
    except IOError:
        PrintAndExit('Could not open file: %s' % fl)

    return fp


def OpenFVFile(fv_file):
    """
    Open a frequency vector file and check to make sure it's valid.  A valid
    FV file must contain at least one line which starts with the string 'T:'.

    @return file pointer
    """

    # import pdb;  pdb.set_trace()
    fp = OpenFile(fv_file)
    line = fp.readline()
    while not line.startswith('T:') and line:
        line = fp.readline()
    if not line.startswith('T:'):
        err_msg(fv_file)
    fp.seek(0, 0)

    return fp

def ProcessSlice(fp, target_slice_num):
    """
    Get the frequency vector for one slice (i.e. line in the FV file).

    All the frequency vector data for a slice is contained in one line.  It
    starts with the char 'T'.  After the 'T', there should be a sequence
    containing one, or more, of the following sets of tokens:
       ':'  integer  ':' integer
    where the first integer is the dimension index and the second integer is
    the count for that dimension. Ignore any whitespace.

    @return list of the frequency vectors for the slice; element = (dimension, count)
    """

    fv = []
    line = fp.readline()
    slice_num = 0;
    while line:
        # print 'Skipping line: ' + line

        # Don't want to skip the part of BBV files at the end which give
        # information on the basic blocks in the file.  If 'Block id:' is
        # found, then back up the file pointer to before this string.
        #
        if line.startswith('T'):
            if slice_num == target_slice_num:
                break
            slice_num = slice_num+1
        line = fp.readline()
    if line == '':
        PrintAndExit('slice_num %d not found ' % target_slice_num)

    # If vector only contains the char 'T', then assume it's a slice which
    # contains no data.
    #
    if line == 'T\n':
        print 'slice_num %d is empty ' % target_slice_num
        return
    else:
        blocks = re.findall(':\s*(\d+)\s*:\s*(\d+)\s*', line)
        outfileName='Slice%d.profile.txt'%target_slice_num
        try:
            outf = open(outfileName, 'w')
        except IOError:
            PrintAndExit('Could not open file: %s' % outfileName)
        outf.write('blockname, filename:linenumber, execution-count\n');
        for block in blocks:
            bb = int(block[0])
            count = int(block[1])
            outstr=all_bbs[bb]['name']+", "+all_bbs[bb]['sourceinfo']+", "+str(count/all_bbs[bb]['inscount'])+"\n"
            outf.write(outstr);
        outf.close();
    # import pdb;  pdb.set_trace()
    fp.seek(0, 0) # re-position to the beginning
    return


def GetMarker(fp):
    """
    Get the marker ("S:") for one slice 

    Marker data format:
        "S:" marker count info sourceinfo
     e.g.
        "S: 289 320 main:for.body3.i foo.c:6"

    @return (marker, count, info, sourceinfo)
    """

    mr = []
    line = fp.readline()
    while not line.startswith('S') and line:
        # print 'Skipping line: ' + line

        # Don't want to skip the part of BBV files at the end which give
        # information on the basic blocks in the file.  If 'Block id:' is
        # found, then back up the file pointer to before this string.
        #
        if line.startswith('Block id:'):
            fp.seek(0 - len(line), os.SEEK_CUR)
            return []
        line = fp.readline()
    if line == '': return {'bbid':0,'bbcount':0, 'bbname':"noinfo", 'sourceinfo':"nofile:0"}

    # If vector only contains the char 'T', then assume it's a slice which
    # contains no data.
    #
    if line == 'S\n': return {'bbid':0,'bbcount':0, 'bbname':"noinfo", 'sourceinfo':"nofile:0"}
    mr = line.split()
    return {'bbid':mr[1],'bbcount':mr[2], 'bbname':mr[3], 'sourceinfo':mr[4]}

def GetBlockIDs(fp):
    """
    Get the information about each basic block which is stored at the end
    of BBV frequency files.

    Extract the values for fields 'block id' and 'instructions' from
    each block.  Here's an example block id entry:

      Block id: 2233 0x69297ff1:0x69297ff5 filename:linenumber static-instructions: 2 block-count: 1 block size: 5

    static instructions is number of instructions in the basic block
    block count is the number of times the block is executed

    @return list of the basic block info, elements are (block_id, bbname, icount of block, sourceinfo)
    """

    block_dict = {}
    line = fp.readline()
    while not line.startswith('Block id:') and line:
        line = fp.readline()
    if line:
        while line.startswith('Block id:'):
            bb = int(line.split('Block id:')[1].split()[0])
            bbname = line.split()[3]
            sourceinfo = line.split()[4]
            inscount = int(line.split('static-instructions:')[1].split()[0])
            block_dict[bb] = {'name':bbname, 'inscount':inscount, 
                'sourceinfo':sourceinfo}
            line = fp.readline()

    fp.seek(0, 0) # re-position to the beginning

    # import pdb;  pdb.set_trace()
    return block_dict

def cleanup():
    """
    Close all open files and any other cleanup required.

    @return no return value
    """

    if bbfp:
        bbfp.close()

############################################################################

parser = argparse.ArgumentParser()
parser.add_argument("--bbfile", help="basic block vector file")
parser.add_argument("slices", metavar="N", type=int, nargs='+',
     help="slice numbers to profile")
args = parser.parse_args()
print args.bbfile
print args.slices

bbfp = OpenFVFile(args.bbfile)
all_bbs = GetBlockIDs(bbfp)

for slice in args.slices:
    ProcessSlice(bbfp, slice)

cleanup()
sys.exit(0)
