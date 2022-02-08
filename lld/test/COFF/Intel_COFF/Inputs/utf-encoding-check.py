# This python script checks if the input file has the correct UTF encoding.
# It opens the input file (arg 1) using the input encoding (arg 2) and checks
# if the string is in the first line of the file. If the encoding is correct
# then it will print "Pass", else it will call the exit error with the
# message "Encoding mismatch". This script is used by the following test cases:
#
#   lib_utf8_bom.ll
#   lib_utf16_bom_be.ll
#   lib_utf16_bom_le.ll
#   lib_utf32_bom_be.ll
#   lib_utf32_bom_le.ll
#
# Usage:
#
#   python utf-encoding-check.py FILENAME ENCODING_TYPE
#
# Supported input ENCODING_TYPE:
#
#   utf-8-sig -> UTF-8 with byte order mark (BOM)
#   utf-16le  -> UTF-16 BOM little endian (LE)
#   utf-16be  -> UTF-16 BOM big endian (BE)
#   utf-32le  -> UTF-32 BOM little endian (LE)
#   utf-32be  -> UTF-32-BOM big endian (BE)

import io
import os
import sys

def main():
    if len(sys.argv) != 3:
        sys.exit('Incorrect number of arguments')

    # Check that the file exists in the system
    input_filename = sys.argv[1]
    if not os.path.exists(input_filename):
        sys.exit('Incorrect input file')

    # Identify the input encoding type
    encoding_type = sys.argv[2].lower()
    search_encoding = ""

    if encoding_type == "utf-8-sig":
        search_encoding = "UTF-8 BOM"
    elif encoding_type == "utf-16le":
        search_encoding = "UTF-16 BOM LE"
    elif encoding_type == "utf-16be":
        search_encoding = "UTF-16 BOM BE"
    elif encoding_type == "utf-32le":
        search_encoding = "UTF-32 BOM LE"
    elif encoding_type == "utf-32be":
        search_encoding = "UTF-32 BOM BE"
    else:
        sys.exit('Incorrect input encoding')

    # String that will be searched in the input file
    full_string = "This file is encoded in " + search_encoding

    # Open the file with the encoding type. We use errors="ignore" to prevent
    # raising an exception error. This is to prevent maintaining the test case
    # if the error message changes in the future. If the file was opened
    # correctly then the string we are looking for will be there.
    encoding_correct = False
    with io.open(input_filename, 'r', encoding=encoding_type, errors="ignore") as fp:
        for line in fp:
            if full_string in line:
                encoding_correct = True
            break

    # Check if encoding matches
    if encoding_correct:
        print("Pass")
    else:
        sys.exit('Encoding mismatch')

if __name__ == "__main__":
    main()
