#!/usr/bin/python3 -u
import sys
import os
import re
import argparse
import json
from subprocess import run, PIPE
import datetime

now = datetime.datetime.now()

DEFAULT_CONFIG = "config.json"


# Setup command line arguments
def parse_args():
    parser = argparse.ArgumentParser(description='Get optional reviewer ids')
    parser.add_argument('-f',
                        '--file',
                        required=True,
                        help='file contains file names to be queried')
    parser.add_argument('-v',
                        '--verbose',
                        required=False,
                        help='Verbose output',
                        action='store_true')
    parser.add_argument('-c',
                        '--config',
                        required=False,
                        help='config file to be used',
                        default=DEFAULT_CONFIG)
    parser.add_argument('-e',
                        '--exclude',
                        required=False,
                        help='id to be excluded, eg: author of the pr.')
    args = parser.parse_args()
    return (args)


def load_config(args):

    try:
        with open(args.config) as config:
            data = json.load(config)
    except:
        print("Error loading json config file")
        exit(-1)

    return data


def read_filenames(args):
    try:
        with open(args.file) as f:
            filenames = f.read().splitlines()
    except:
        print("Error reading input file: {}".format(args.file))
        exit(-2)

    return filenames


def get_review_id(files, config, args):

    pattern = re.compile(config['match'])
    for file in files:
        if (pattern.match(file)):
            if (args.verbose):
                print("{} requires review from {}".format(file, config['ids']))
            return config['ids']
    return None


def main():

    args = parse_args()
    data = load_config(args)
    files = read_filenames(args)

    ids = []
    for item in data:
        if (args.verbose):
            print(item)
        reviewids = get_review_id(files, item, args)
        if reviewids is not None:
            ids.extend(reviewids)

    if (args.verbose):
        print("reviewers are: {}".format(ids))

    uniqueids = list(set(ids))

    if (args.exclude and args.exclude in uniqueids):
        uniqueids.remove(args.exclude)

    if len(uniqueids) == 0:
        print("")
    else:
        print(', '.join(['"{}"'.format(uid) for uid in uniqueids]))


if __name__ == "__main__":
    main()
