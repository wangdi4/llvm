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
import optparse
import os
import random
import re
import sys
import argparse

def PrintAndExit(msg):
    """
    Prints an error message exit.

    """
    sys.stderr.write(msg)
    sys.stderr.write("\n")
    sys.exit(-1)

def PrintMsg(msg):
    """
    Prints an message 
    """
    sys.stdout.write(msg)
    sys.stdout.write("\n")
    sys.stdout.flush()

def PrintMsgNoCR(msg):
    """
    Prints an message 
    """
    sys.stdout.write(msg)
    sys.stdout.flush()

def OpenFile(fl, type_str):
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

def IsInt(s):
    """
    Is a string an integer number?

    @return boolean
    """
    try:
        int(s)
        return True
    except ValueError:
        return False

def IsFloat(s):
    """
    Is a string an floating point number?

    @return boolean
    """
    try:
        float(s)
        return True
    except ValueError:
        return False



def OpenFVFile(fv_file, type_str):
    """
    Open a frequency vector file and check to make sure it's valid.  A valid
    FV file must contain at least one line which starts with the string 'T:'.

    @return file pointer
    """

    # import pdb;  pdb.set_trace()
    fp = OpenFile(fv_file, type_str)
    line = fp.readline()
    while not line.startswith('T:') and line:
        line = fp.readline()
    if not line.startswith('T:'):
        err_msg(type_str + fv_file)
    fp.seek(0, 0)

    return fp


def OpenSimpointFile(sp_file, type_str):
    """
    Open a simpoint file and check to make sure it's valid.  A valid
    simpoint file must start with an integer.

    @return file pointer
    """

    fp = OpenFile(sp_file, type_str)
    line = fp.readline()
    field = line.split()
    if not IsInt(field[0]):
        err_msg(type_str + sp_file)
    fp.seek(0, 0)

    return fp


def OpenWeightsFile(wt_file, type_str):
    """
    Open a weight file and check to make sure it's valid.  A valid
    wieght file must start with an floating point number.

    @return file pointer
    """

    fp = OpenFile(wt_file, type_str)
    line = fp.readline()
    field = line.split()
    if not IsFloat(field[0]):
        err_msg(weight_str + wt_file)
    fp.seek(0, 0)

    return fp


def GetSlice(fp):
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
    while not line.startswith('T') and line:
        # print 'Skipping line: ' + line

        # Don't want to skip the part of BBV files at the end which give
        # information on the basic blocks in the file.  If 'Block id:' is
        # found, then back up the file pointer to before this string.
        #
        if line.startswith('Block id:'):
            fp.seek(0 - len(line), os.SEEK_CUR)
            return []
        line = fp.readline()
    if line == '': return []

    # If vector only contains the char 'T', then assume it's a slice which
    # contains no data.
    #
    if line == 'T\n':
        fv.append((0, 0))
    else:
        blocks = re.findall(':\s*(\d+)\s*:\s*(\d+)\s*', line)
        # print 'Slice:'
        for block in blocks:
            # print block
            bb = int(block[0])
            count = int(block[1])
            fv.append((bb, count))

    # import pdb;  pdb.set_trace()
    return fv


def GetMarker(fp):
    """
    Get the marker ("S:") for one slice 

    Marker data format:
        "S:" marker count info
     e.g.
        "S: 289 320 main:for.body3.i"

    @return (marker, count, info)
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
    if line == '': return {'bbid':0,'bbcount':0, 'bbname':"noinfo"}

    # If vector only contains the char 'T', then assume it's a slice which
    # contains no data.
    #
    if line == 'S\n': return {'bbid':0,'bbcount':0, 'bbname':"noinfo"}
    mr = line.split()
    return {'bbid':mr[1],'bbcount':mr[2], 'bbname':mr[3]}

def GetFirstBbinfo(fp):
    """
    Get information about the first block executed

    Extract from the record "B: fbbid fbbname" near the beginning of the file

    @return {'bbid':fbbid,'bbcount':1, 'bbname':fbbname}
    """

    marker = {'bbid':0,'bbcount':0, 'bbname':"noinfo:0:noinfo"}
    line = fp.readline()
    while not line.startswith('B:') and line:
        line = fp.readline()
    if line:
            fbbid = int(line.split('B:')[1].split()[0])
            fbbname = line.split()[2]
            marker = {'bbid':fbbid,'bbcount':1, 'bbname':fbbname}

    # import pdb;  pdb.set_trace()
    return marker

############################################################################
#
#  Functions for generating regions CSV files
#
############################################################################


def GetWeights(fp):
    """
    Get the regions and weights from a weights file.

    @return lists of regions and weights
    """

    # This defines the pattern used to parse one line of the weights file.
    # There are three components to each line:
    #
    #   1) a floating point number  (group number 1 in the match object)
    #   2) white space
    #   3) a decimal number         (group number 2 in the match object)
    #
    # 1) This matches floating point numbers in either fixed point or scientific notation:
    #   -?        optionally matches a negative sign (zero or one negative signs)
    #   \ *       matches any number of spaces (to allow for formatting variations like - 2.3 or -2.3)
    #   [0-9]+    matches one or more digits
    #   \.?       optionally matches a period (zero or one periods)
    #   [0-9]*    matches any number of digits, including zero
    #   (?: ... ) groups an expression, but without forming a "capturing group" (look it up)
    #   [Ee]      matches either "e" or "E"
    #   \ *       matches any number of spaces (to allow for formats like 2.3E5 or 2.3E 5)
    #   -?        optionally matches a negative sign
    #   \ *       matches any number of spaces
    #   [0-9]+    matches one or more digits
    #   ?         makes the entire non-capturing group optional (to allow for
    #             the presence or absence of the exponent - 3000 or 3E3
    #
    # 2) This matches white space:
    #   \s
    #
    # 3) This matches a decimal number with one, or more digits:
    #   \d+
    #
    pattern = '(-?\ *[0-9]+\.?[0-9]*(?:[Ee]\ *-?\ *[0-9]+)?)\s(\d+)'

    weight_dict = {}
    for line in fp.readlines():
        field = re.match(pattern, line)

        # Look for the special case where the first field is a single digit
        # without the decimal char '.'.  This should be the weight of '1'.
        #
        if not field:
            field = re.match('(\d)\s(\d)', line)
        if field:
            weight = float(field.group(1))
            region = int(field.group(2))
            weight_dict[region] = weight

    return weight_dict


def GetSimpoints(fp):
    """
    Get the regions and slices from the Simpoint file.

    @return list of regions and slices from a Simpoint file
    """

    RegionToSlice = {}
    max_region_number = 0
    for line in fp.readlines():
        field = re.match('(\d+)\s(\d+)', line)
        if field:
            slice_num = int(field.group(1))
            region = int(field.group(2))
            if region > max_region_number:
                max_region_number = region
            RegionToSlice[region] = slice_num

    return RegionToSlice, max_region_number


def GetRegionBBV(fp, RegionToSlice, max_region_number):
    """
    Read all the frequency vector slices and the basic block id info from a
    basic block vector file.  Put the data into a set of lists which are used
    in generating CSV regions.

    @return cumulative_icount, region_bbv, region_start_markers, region_end_markers, first_bb_marker
    """
    slice_set = set(RegionToSlice.values())

    num_regions = max_region_number + 1
    # region number and cluster id are the same
    # cluster id start at 0 and need not be contiguous.
    # so we need to size our arrays to max_cluster_id + 1

    # List of lists of basic block vectors, each inner list contains the blocks for one of the
    # representative regions. 
    #
    region_bbv = []

    # A tuple {'bbid':x, 'bbcount':y, 'bbinfo':"<fnname:bbname>"}
    current_marker = GetFirstBbinfo(fp)
    first_bb_marker = current_marker
    previous_marker = {}

    # List of start markers for representative regions. 
    region_start_markers = [None] * num_regions

    # List of start markers for representative regions. 
    region_end_markers = [None] * num_regions

    # List of the cumulative sum of instructions in the slices.  There is one
    # entry for each slice in the BBV file which contains the total icount up
    # to the end of the slice.
    #
    cumulative_icount = []

    # Cumulative sum of instructions so far
    #
    run_sum = 0

    # Get each slice & generate some data on it.
    #
    slice_num = 0
    while True:
        fv = GetSlice(fp)
        if fv == []:
            break
        # print fv
        previous_marker = current_marker
        current_marker = GetMarker(fp)

        # Get total icount for the basic blocks in this slice
        #
        sum = 0
        for bb in fv:
            count = bb[1]
            sum += count

        # Record the cumulative icount for this slice.
        #
        if sum != 0:
            run_sum += sum
            cumulative_icount += [run_sum]

        # If slice is a representative region, record the basic blocks of the slice.
        #
        if slice_num in slice_set:
            marker_index = RegionToSlice.keys()[RegionToSlice.values().index(slice_num)]
            region_start_markers[marker_index] = previous_marker 
            region_end_markers[marker_index] = current_marker 
        slice_num += 1

    # import pdb;  pdb.set_trace()
    return cumulative_icount, region_bbv, region_start_markers, region_end_markers, first_bb_marker

def CheckRegions(RegionToSlice, weight_dict):
    """
    Check to make sure the simpoint and weight files contain the same regions.

    @return no return value
    """

    if len(RegionToSlice) != len(weight_dict) or \
       RegionToSlice.keys() != weight_dict.keys():
        PrintMsg('ERROR: Regions in these two files are not identical')
        PrintMsg('   Simpoint regions: ' + str(RegionToSlice.keys()))
        PrintMsg('   Weight regions:   ' + str(weight_dict.keys()))
        cleanup()
        sys.exit(-1)


def GenRegionCSV(fp_bbv, fp_simp, fp_weight):
    """
    Read in three files (BBV, weights, simpoints) and print to stdout
    a regions CSV file which defines the representative regions.

    @return no return value
    """

    # Read data from weights, simpoints and BBV files.
    # Error check the regions.
    #
    weight_dict = GetWeights(fp_weight)
    RegionToSlice, max_region_number = GetSimpoints(fp_simp)
    cumulative_icount, region_bbv, region_start_markers, region_end_markers , first_bb_marker = GetRegionBBV(fp_bbv, RegionToSlice, max_region_number)
    CheckRegions(RegionToSlice, weight_dict)

    total_num_slices = len(cumulative_icount)

    # Print header information
    #
    PrintMsgNoCR('# Regions based on: ')
    for string in sys.argv:
        PrintMsgNoCR(string + ' '),
    PrintMsg('')
    PrintMsg(
        '# comment,thread-id,region-id,start-bbl,start-bbl-count,end-bbl,end-bbl-count,region-weight')

    tid = 0
    total_icount = 0
    for region in sorted(RegionToSlice.keys()):
        # Calculate the info for the regions and print it.
        #
        slice_num = RegionToSlice[region]
        weight = weight_dict[region]
        start_marker = region_start_markers[region]
        end_marker = region_end_markers[region]
        # import pdb;  pdb.set_trace()
        if slice_num > 0:
            start_icount = cumulative_icount[slice_num - 1] + 1
        else:
            # If this is the first slice, set the initial icount to 0
            #
            start_icount = 0
        end_icount = cumulative_icount[slice_num]
        length = end_icount - start_icount + 1
        total_icount += length
        PrintMsg('# Region = %d Slice = %d Icount = %d Length = %d Weight = %.5f' % \
            (region + 1, slice_num, start_icount, length, weight))
        PrintMsg('#Start: bb : %s bbcount: %s bbname: %s' % (start_marker['bbid'], start_marker['bbcount'], start_marker['bbname'])); 
        PrintMsg('#End: bb : %s bbcount: %s bbname: %s' % (end_marker['bbid'], end_marker['bbcount'], end_marker['bbname'])); 
        PrintMsg('cluster %d from slice %d,%d,%d,%s,%s,%s,%s,%.5f\n' % \
            (region, slice_num, tid, region + 1, start_marker['bbid'], start_marker['bbcount'], end_marker['bbid'], end_marker['bbcount'], weight))


        # Print summary statistics
        #
        # import pdb;  pdb.set_trace()
    PrintMsg('# First Block, %d, %s' %
                 (first_bb_marker['bbid'], first_bb_marker['bbname']))
    PrintMsg('# Total llvm instructions in %d regions = %d' %
                 (len(RegionToSlice), total_icount))
    PrintMsg('# Total llvm instructions in workload = %d' %
                 cumulative_icount[total_num_slices - 1])
    PrintMsg('# Total slices in workload = %d' % total_num_slices)

############################################################################
#
#  Functions for normalization and projection
#
############################################################################


def cleanup():
    """
    Close all open files and any other cleanup required.

    @return no return value
    """

    if fp_bbv:
        fp_bbv.close()
    if fp_simp:
        fp_simp.close()
    if fp_weight:
        fp_weight.close()

############################################################################

# llvmregions.py --bbv_file t.bb --region_file t.simpoints --weight_file t.weights > $prefix.llvmpoints.csv
parser = argparse.ArgumentParser()
parser.add_argument("--bbv_file", help="basic block vector file")
parser.add_argument("--region_file", help="files showing simpoint regions")
parser.add_argument("--weight_file", help="files showing simpoint weights")
args = parser.parse_args()
fp_bbv = OpenFVFile(args.bbv_file, 'Basic Block Vector (bbv) file: ')
fp_simp = OpenSimpointFile(args.region_file, 'simpoints file: ')
fp_weight = OpenWeightsFile(args.weight_file, 'weights file: ')


GenRegionCSV(fp_bbv, fp_simp, fp_weight)

cleanup()
sys.exit(0)
