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

import cmd_options
import msg
import util

err_msg = lambda string: msg.PrintAndExit('This is not a valid ' + string + \
            '\nUse -h for help.')


def OpenFile(fl, type_str):
    """
    Check to make sure a file exists and open it.

    @return file pointer
    """

    # import pdb;  pdb.set_trace()
    if not os.path.isfile(fl):
        msg.PrintAndExit('File does not exist: %s' % fl)
    fp = util.OpenCompressFile(fl)
    if fp == None:
        err_msg(type_str + fl)

    return fp


def OpenNormalFVFile(fv_file, type_str):
    """
    Open a normalized frequency vector file and check to make sure it's valid.

    The first line of a valid normalized FV file contains the number of vectors
    in the file followed by the char ':' and an optional char 'w'.

    Subsequent lines contain an optional weight (if 'w' is present on first line) followed
    by the number of fields in the vector, the char ':' and the values for the vector:  For example:

    80:w
    0.01250000000000000069 3:  0.00528070484084160793 0.00575272877173275011 0.00262986399034366479
    0.01250000000000000069 2:  0.00528070484084160793 0.00575272877173275011

    @return file pointer
    """

    # Read in the 1st line of the file and check for errors.
    #
    # import pdb;  pdb.set_trace()
    type_str = 'normalized frequency vector file: '
    fp = OpenFile(fv_file, type_str)
    line = fp.readline()
    field = line.split(':')
    num_vect = field[0]
    if not util.IsInt(num_vect):
        err_msg(type_str + fv_file)
    if len(field) == 2:
        if not 'w' in field[1]:
            err_msg(type_str + fv_file)
        else:
            weights = True

    # Read the 2nd line: an optional weight, the number of values in the vector and the vector itself.
    #
    line = fp.readline()
    if line == '':
        err_msg(type_str + fv_file)
    field = line.split()
    if weights:
        if not util.IsFloat(field[0]):
            err_msg(type_str + fv_file)
        field = field[1:]
    if len(field) < 2:
        err_msg(type_str + fv_file)
    num_field = int(field[0].split(':')[0])
    if not util.IsInt(num_field):
        err_msg(type_str + fv_file)
    field = field[1:]
    if len(field) != num_field:
        err_msg(type_str + fv_file)
    for f in field:
        if not util.IsFloat(f):
            err_msg(type_str + fv_file)
    fp.seek(0, 0)

    return fp


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
    if not util.IsInt(field[0]):
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
    if not util.IsFloat(field[0]):
        err_msg(weight_str + wt_file)
    fp.seek(0, 0)

    return fp


def GetOptions():
    """
    Get users command line options/args and check to make sure they are correct.

    @return List of options and 3 file pointers for bbv, simpoint and weights files
    """

    version = '$Revision: 1.30 $'
    version = version.replace(' ', '')
    ver = version.replace(' $', '')
    us = '%prog [options] action file_name [file_name]'
    desc = 'Implements several actions used to process FV (Frequency Vector) files.  ' \
           'One action, and only one, must be defined in order for the script to run.  '\
           'All actions require at least one file name be given using an option. \n\n'\
           '' \
           'There are two types of frequency vector files:  '\
           '                                                            '\
           'BBV = Basic Block Vector, '\
           '                                                            '\
           'LDV = LRU stack Distance Vector'

    def combine(parser, group):
        """
        IMPORTANT NOTE:
        This is a local definition for the option which has more help
        information than the default defined in cmd_options.py.  This info is
        specific to this script and is not applicable to the other locations
        where the option is used.

        Default value for combine to 'none' instead of setting it to a value
        (as it is in cmd_options.py).  This allows the option to be used to
        determine what to do.

        @return  No return value
        """

        method = cmd_options.GetMethod(parser, group)
        method(
            "--combine",
            dest="combine",
            default=None,
            help="Combine the vectors for BBV and LDV files into a single FV file, use scaling "
            "factor COMBINE (0.0 >= COMBINE <= 1.0).  The BB vectors "
            "are scaled by COMBINE, while the LD vectors are scaled by 1-COMBINE.  Default: 0.5  "
            "Assumes both files have already been transformed by the appropriate process "
            "(project/normal for BBV, weight/normal for LDV). "
            "Must use --normal_bbv and --normal_ldv to define files to process.")

    util.CheckNonPrintChar(sys.argv)
    parser = optparse.OptionParser(
        usage=us,
        version=ver,
        description=desc,
        formatter=cmd_options.BlankLinesIndentedHelpFormatter())

    cmd_options.dimensions(parser, '')
    cmd_options.focus_thread(parser, '')

    # Options which define the actions the script to execute
    #
    action_group = cmd_options.ActionGroup(parser)

    combine(parser, action_group)
    cmd_options.csv_region(parser, action_group)
    cmd_options.project_bbv(parser, action_group)
    cmd_options.weight_ldv(parser, action_group)

    parser.add_option_group(action_group)

    # Options which list the files the script can process
    #
    # import pdb;  pdb.set_trace()
    file_group = cmd_options.FileGroup(parser)

    cmd_options.bbv_file(parser, file_group)
    cmd_options.ldv_file(parser, file_group)
    cmd_options.normal_bbv(parser, file_group)
    cmd_options.normal_ldv(parser, file_group)
    cmd_options.region_file(parser, file_group)
    cmd_options.vector_file(parser, file_group)
    cmd_options.weight_file(parser, file_group)

    parser.add_option_group(file_group)

    # Parse command line options and get any arguments.
    #
    (options, args) = parser.parse_args()

    # Added method cbsp() to 'options' to check if running CBSP.
    #
    util.AddMethodcbsp(options)

    def TrueXor(*args):
        """Return xor of some booleans."""
        return sum(args) == 1

    # Must have one, and only one, action on command line.
    #
    # import pdb;  pdb.set_trace()
    if not TrueXor(options.csv_region, options.project_bbv, options.weight_ldv, \
       options.combine != None, options.vector_file != None):
        msg.PrintAndExit(
            'Must give one, and only one, action for script to execute.\n'
            'Use -h to get help.')

    # Check to see if options required for the various actions are given.
    #
    file_error = lambda file, action: msg.PrintAndExit("Must use option '" + file + \
        "' to define the file to use with '" + action + "'.   \nUse -h for help.")

    # import pdb;  pdb.set_trace()
    fp_bbv = fp_ldv = fp_simp = fp_weight = None

    if options.combine:
        # Check to make sure the option 'combine' is an acceptable value.  If so, then turn it into a float.
        #
        util.CheckCombine(options)
        options.combine = float(options.combine)

        # Then check to make sure required files are given.
        #
        if not options.normal_bbv:
            file_error('--normal_bbv', '--combine')
        if not options.normal_ldv:
            file_error('--normal_ldv', '--combine')
        fp_bbv = OpenNormalFVFile(options.normal_bbv,
                                  'projected, normalized BBV file: ')
        fp_ldv = OpenNormalFVFile(options.normal_ldv,
                                  'projected, normalized BBV file: ')

    if options.csv_region:
        if not options.bbv_file:
            file_error('--bbv_file', '--csv_region')
        if not options.region_file:
            file_error('--region_file', '--csv_region')
        if not options.weight_file:
            file_error('--weight_file', '--csv_region')
        fp_bbv = OpenFVFile(options.bbv_file, 'Basic Block Vector (bbv) file: ')
        fp_simp = OpenSimpointFile(options.region_file, 'simpoints file: ')
        fp_weight = OpenWeightsFile(options.weight_file, 'weights file: ')

    if options.project_bbv:
        if not options.bbv_file:
            file_error('--bbv_file', '--project_bbv')
        fp_bbv = OpenFVFile(options.bbv_file, 'Basic Block Vector (bbv) file: ')

    if options.weight_ldv:
        if not options.ldv_file:
            file_error('--ldv_file', '--weight_ldv')
        fp_ldv = util.OpenCompressFile(options.ldv_file)

    return (options, fp_bbv, fp_ldv, fp_simp, fp_weight)


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

    simp_dict = {}
    for line in fp.readlines():
        field = re.match('(\d+)\s(\d+)', line)
        if field:
            slice_num = int(field.group(1))
            region = int(field.group(2))
            simp_dict[region] = slice_num

    return simp_dict


def GetRegionBBV(fp, simp_dict):
    """
    Read all the frequency vector slices and the basic block id info from a
    basic block vector file.  Put the data into a set of lists which are used
    in generating CSV regions.

    @return cumulative_icount, region_bbv, region_start_markers, region_end_markers, first_bb_marker
    """
    slice_set = set(simp_dict.values())

    num_regions = len(slice_set);

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
            marker_index = simp_dict.keys()[simp_dict.values().index(slice_num)]
            region_start_markers[marker_index] = previous_marker 
            region_end_markers[marker_index] = current_marker 
        slice_num += 1

    # import pdb;  pdb.set_trace()
    return cumulative_icount, region_bbv, region_start_markers, region_end_markers, first_bb_marker

def CheckRegions(simp_dict, weight_dict):
    """
    Check to make sure the simpoint and weight files contain the same regions.

    @return no return value
    """

    if len(simp_dict) != len(weight_dict) or \
       simp_dict.keys() != weight_dict.keys():
        msg.PrintMsg('ERROR: Regions in these two files are not identical')
        msg.PrintMsg('   Simpoint regions: ' + str(simp_dict.keys()))
        msg.PrintMsg('   Weight regions:   ' + str(weight_dict.keys()))
        cleanup()
        sys.exit(-1)


def GenRegionCSV(options, fp_bbv, fp_simp, fp_weight):
    """
    Read in three files (BBV, weights, simpoints) and print to stdout
    a regions CSV file which defines the representative regions.

    @return no return value
    """

    # Read data from weights, simpoints and BBV files.
    # Error check the regions.
    #
    weight_dict = GetWeights(fp_weight)
    simp_dict = GetSimpoints(fp_simp)
    cumulative_icount, region_bbv, region_start_markers, region_end_markers , first_bb_marker = GetRegionBBV(fp_bbv, simp_dict)
    CheckRegions(simp_dict, weight_dict)

    total_num_slices = len(cumulative_icount)

    # Print header information
    #
    msg.PrintMsgNoCR('# Regions based on: ')
    for string in sys.argv:
        msg.PrintMsgNoCR(string + ' '),
    msg.PrintMsg('')
    msg.PrintMsg(
        '# comment,thread-id,region-id,start-bbl,start-bbl-count,end-bbl,end-bbl-count,region-weight')
    # msg.PrintMsg('')

    # Print region information
    #
    # import pdb;  pdb.set_trace()
    if options.focus_thread != -1:
        tid = int(options.focus_thread)
    else:
        tid = 0
    total_icount = 0
    for region in sorted(simp_dict.keys()):
        # Calculate the info for the regions and print it.
        #
        slice_num = simp_dict[region]
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
        msg.PrintMsg('# Region = %d Slice = %d Icount = %d Length = %d Weight = %.5f' % \
            (region + 1, slice_num, start_icount, length, weight))
        msg.PrintMsg('#Start: bb : %s bbcount: %s bbname: %s' % (start_marker['bbid'], start_marker['bbcount'], start_marker['bbname'])); 
        msg.PrintMsg('#End: bb : %s bbcount: %s bbname: %s' % (end_marker['bbid'], end_marker['bbcount'], end_marker['bbname'])); 
        msg.PrintMsg('cluster %d from slice %d,%d,%d,%s,%s,%s,%s,%.5f\n' % \
            (region, slice_num, tid, region + 1, start_marker['bbid'], start_marker['bbcount'], end_marker['bbid'], end_marker['bbcount'], weight))


        # Print summary statistics
        #
        # import pdb;  pdb.set_trace()
    msg.PrintMsg('# First Block, %d, %s' %
                 (first_bb_marker['bbid'], first_bb_marker['bbname']))
    msg.PrintMsg('# Total llvm instructions in %d regions = %d' %
                 (len(simp_dict), total_icount))
    msg.PrintMsg('# Total llvm instructions in workload = %d' %
                 cumulative_icount[total_num_slices - 1])
    msg.PrintMsg('# Total slices in workload = %d' % total_num_slices)

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

options, fp_bbv, fp_ldv, fp_simp, fp_weight = GetOptions()

if options.csv_region:
    GenRegionCSV(options, fp_bbv, fp_simp, fp_weight)

cleanup()
sys.exit(0)
