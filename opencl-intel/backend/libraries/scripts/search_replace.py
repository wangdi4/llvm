#!/usr/bin/env python3

"""A script to search and replace regex patterns for given files."""

import argparse
import glob
import os
import re


DEBUG = False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('inputfile', type=str, nargs='+',
                        help='Input file may be specified using Unix shell wildcards like "*"')
    parser.add_argument('--batch-file', default=None,
                        help='Specify search-replace patterns from file')
    parser.add_argument('--search', default='',
                        help='The regex pattern to match')
    parser.add_argument('--replace', default='',
                        help='The replacement pattern')
    parser.add_argument('--in-place', default=False, action='store_true',
                        help='Modify input files in-place')
    parser.add_argument('--output-dir', type=str,
                        default=None, help='The output directory')
    parser.add_argument('--debug', default=False, action='store_true')
    args = parser.parse_args()

    global DEBUG
    DEBUG = args.debug

    inputfiles = parse_inputfiles(args.inputfile)

    if args.batch_file:
        searches, replaces = parse_batch_file(args.batch_file)
    else:
        searches, replaces = [args.search], [args.replace]

    for file in inputfiles:
        process_file(file, searches, replaces,
                     args.in_place, args.output_dir)


def parse_inputfiles(inputfiles):
    files = set()
    for inputfile in inputfiles:
        files.update(glob.glob(inputfile))
    if DEBUG:
        print("Input files to process: ", files)
    return files


def parse_batch_file(batch_file):
    searches = []
    replaces = []
    with open(batch_file, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            # Skip comment
            if line.startswith('#'):
                continue
            if line.startswith('s:'):
                searches.append(line[2:])
            if line.startswith('r:'):
                replaces.append(line[2:])
            if line.startswith('c:'):
                searches.append('({})'.format(line[2:]))
                replaces.append(r'/* \1 */')
    assert(len(searches) == len(replaces)
           and "Batch file syntax error: the number of 's:' lines must match with that of 'r:' lines")
    return searches, replaces


def process_file(filename, searches, replaces, in_place, output=None):
    with open(filename, 'r', errors='ignore') as f:
        content = f.read()
    for search, replace in zip(searches, replaces):
        assert(len(search) > 0 and "Invalid empty search pattern!")
        if DEBUG:
            matches = re.findall(search, content)
            print("All matched patterns in {} for\n  {}\n  {}\n".format(
                filename, search, matches))
            print("  Replacing with {}\n".format(replace))
        content = re.sub(
            search, replace + '/*MODIFIED BY search_replace.py*/', content)

    if in_place:
        output = filename
    else:
        assert(output is not None)
        output = os.path.join(output, os.path.basename(filename))

    os.makedirs(os.path.dirname(output), exist_ok=True)
    with open(output, 'w') as f:
        if DEBUG:
            print("Writing to {}".format(output))
        f.write(content)


if __name__ == '__main__':
    main()
