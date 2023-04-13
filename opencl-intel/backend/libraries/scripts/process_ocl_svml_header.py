#!/usr/bin/env python3

"""A script to process raw ocl svml headers."""


import argparse
from collections import defaultdict
from operator import itemgetter
import re
import sys


DEBUG = False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('inputfile', type=str)
    parser.add_argument('--outputfile', required=True,
                        help='Output filename')
    parser.add_argument('--generate-shared-header',
                        default=False, action='store_true')
    parser.add_argument('--debug', default=False, action='store_true')
    args = parser.parse_args()

    global DEBUG
    DEBUG = args.debug

    process_file(args.inputfile, args.outputfile, args.generate_shared_header)


# map shortname to target
shortname_target = {
    'z1': 'amx',
    'z0': 'avx512',
    'l9': 'avx2',
    'e9': 'avx',
    'h8': 'sse',
    'x1': 'amx',
    'x0': 'avx512',
    's9': 'avx2',
    'g9': 'avx',
    'n8': 'sse'
}

# map shortname to calling convention
shortname_cc = {
    'z1': 'avx512',
    'z0': 'avx512',
    'l9': 'avx',
    'e9': 'avx',
    'h8': 'sse',
    'x1': 'avx512',
    'x0': 'avx512',
    's9': 'avx',
    'g9': 'avx',
    'n8': 'sse'
}


copyright_header = '''// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

'''

typedefs = '''
typedef short short1;
typedef ushort ushort1;
typedef int int1;
typedef uint uint1;
typedef long long1;
typedef ulong ulong1;
typedef half half1;
typedef float float1;
typedef double double1;

'''

const_attr = '__attribute__((const))'
# INTEL_CUSTOMIZATION
def get_cc_attr(cc):
    if not cc:
        return ''
    if cc == 'sse':
        cc = ''
    else:
        cc = '_' + cc
    return '__attribute__((intel_ocl_bicc{}))'.format(cc)
# end INTEL_CUSTOMIZATION

def process_file(inputfile, outputfile, generate_shared_header):
    with open(inputfile, 'r') as f:
        lines = f.readlines()
    # collect declaration entries
    # element := (return_type, shortname, funcname, params, cc)
    declarations = []
    i, n = 0, len(lines)
    while i < n:
        line = lines[i].strip()
        ok, return_type, shortname, funcname, params = parse_declaration(line)
        if ok:
            # remove type ISA suffixes
            return_type = materialize_type(return_type)
            params = materialize_type(params)
            # skip decls containing 'double1x2' like types
            if 'x' in return_type or 'x' in params:
                i += 1
                continue
            # fix wrong types
            return_type, params = fix_types(funcname, return_type, params)

            # check calling convention
            if i + 1 < n and lines[i+1].strip().startswith('#pragma linkage'):
                cc = shortname_cc.get(shortname, '')
            else:
                cc = ''
            if DEBUG:
                print('Decl: ', return_type, shortname, funcname, params, cc)
            declarations.append((return_type, shortname, funcname, params, cc))
        i += 1

    if generate_shared_header:
        declarations = filter_shared_decls(declarations)
    emit_declarations(declarations, outputfile)


# If there's a pointer parameter, then the function is non-const.
def is_const(params):
    return '*' not in params


def materialize_type(type_str):
    return type_str.replace('_sse', '').replace('_avx', '').replace('_uisa', '')


def fix_types(funcname, return_type, params):
    # Parameter types and return type of 'udiv' and 'urem' should be 'uint' instead of 'int'.
    if funcname.startswith('udiv') or funcname.startswith('urem'):
        return_type = return_type.replace('int', 'uint')
        params = params.replace('int', 'uint')
    return return_type, params


def filter_shared_decls(declarations):
    """
    Filter function declarations that exist for all targets. Alter the
    shortname to 'shared', and alter cc to '__attribute__((intel_ocl_bicc))'.
    This is used to generate __ocl_svml_shared_* declarations that are later
    replaced by BuiltinImport pass.
    """
    func_target_map = defaultdict(set)
    func_data = {}
    for return_type, shortname, funcname, params, _ in declarations:
        func_target_map[funcname].add(shortname_target[shortname])
        if funcname not in func_data:
            func_data[funcname] = (return_type, params)
        else:
            assert(func_data[funcname] == (return_type, params))

    return [(func_data[funcname][0], "shared", funcname, func_data[funcname][1], 'sse')
            for funcname, targets in func_target_map.items()
            if targets == {"sse", "avx", "avx2", "avx512", "amx"}]


def emit_declarations(declarations, outputfile):
    # sort declartions by funcname, cc
    declarations.sort(key=itemgetter(2, 4))
    with open(outputfile, 'w') as f:
        f.write('// This file is auto-generated by:\n')
        f.write('//  python ' + ' '.join(sys.argv) + '\n\n')
        f.write(copyright_header)
        f.write(typedefs)
        for return_type, shortname, funcname, params, cc in declarations:
            # build attributes
            attr = (const_attr + ' ') if is_const(params) else ''
            attr += get_cc_attr(cc)

            # emit declaration
            f.write('{return_type} {attr} __ocl_svml_{shortname}_{funcname}({params});\n'.format(
                return_type=return_type,
                attr=attr, shortname=shortname, funcname=funcname,
                params=params))


def parse_declaration(line):
    pattern = r'(?P<return_type>\w+)\s+__ocl_svml_(?P<shortname>[a-z][0-9])_(?P<funcname>\w+)\s+\((?P<params>[\w\s,\*]*)\);'
    m = re.fullmatch(pattern, line)
    if m:
        return True, m.group('return_type'), m.group('shortname'), m.group('funcname'), m.group('params')
    return False, None, None, None, None


if __name__ == '__main__':
    main()
