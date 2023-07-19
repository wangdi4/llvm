#!/usr/bin/env python

"""
The script converts a binary file provided to a C-array mimicking behavior
of xxd tool.
Example usages:

convert-to-c-array.py --input bin.file --output header.h --name resource
"""

import sys
import os

def main(argv):
    import argparse

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument(
        "--input",
        metavar="<binary file path>",
        type=str,
        help="Path to a binary for conversion",
        required = True,
    )

    arg_parser.add_argument(
        "--output",
        metavar="<header file path>",
        type=str,
        help="Path to a output file for writing C array",
        required = True,
    )

    arg_parser.add_argument(
        "--name",
        metavar="<C array name>",
        type=str,
        default="BinaryBlob",
        help="Name of the C array and prefix for _len variable",
    )

    args = arg_parser.parse_args()

    fileSize = 0
    with open(args.output, "w") as outFile:
        with open(args.input, mode='rb') as inFile:
            outFile.write("static unsigned char {0}[] = {{\n".format(args.name))
            bytes = inFile.read(12)
            while bytes != b"":
                BytesRead = len(bytes)
                outFile.write("  ")
                for i in range(BytesRead - 1):
                    outFile.write("0x{:02x}, ".format(bytes[i]))

                outFile.write("0x{:02x},\n".format(bytes[BytesRead - 1]))

                fileSize += BytesRead
                bytes = inFile.read(12)

            outFile.write("};\n")
            outFile.write("unsigned int {0}_len = {1};".format(args.name, fileSize))

if __name__ == "__main__":
    main(sys.argv)
