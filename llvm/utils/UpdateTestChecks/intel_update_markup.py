#!/usr/bin/env python3
"""A intel markup update script.

This script is a utility to update intel markup for test file.
Supported file extension: .ll, .mir.
"""

import argparse
import os
import tempfile
import subprocess
from subprocess import PIPE


def get_single_line_markup(file_ext):
    return ';INTEL'


def get_multi_line_markup(file_ext):
    if file_ext == '.ll':
        return '; INTEL_CUSTOMIZATION', '; end INTEL_CUSTOMIZATION'
    elif file_ext == '.mir':
        return '# INTEL_CUSTOMIZATION', '# end INTEL_CUSTOMIZATION'

    raise NotImplementedError(f'Unsupported file extension {file_ext}')


def get_ext(exp):
    _, file_ext = os.path.splitext(exp)
    if file_ext not in {'.ll', '.mir'}:
        raise NotImplementedError(f'Unsupported file extension {file_ext}')
    return file_ext


def get_markup(exp):
    file_ext = get_ext(exp)
    s_markup = get_single_line_markup(file_ext)
    m_markup_start, m_markup_end = get_multi_line_markup(file_ext)
    return s_markup, m_markup_start, m_markup_end


def get_dir():
    return os.path.dirname(os.path.realpath(__file__))


def has_git():
    version = subprocess.run(['git', '--version'], stdout=PIPE, stderr=PIPE)
    return version.returncode == 0


def has_sed():
    version = subprocess.run(['sed', '--version'], stdout=PIPE, stderr=PIPE)
    return version.returncode == 0


def get_llorg_ref(exp):
    """Get the latest llorg version of the file"""
    dir_path = get_dir()
    ref_commit = subprocess.run(['git', 'merge-base', 'origin/main', 'HEAD'],
                                stdout=PIPE,
                                stderr=PIPE,
                                cwd=dir_path,
                                check=True)
    ref_commit = ref_commit.stdout.decode('utf-8').strip()

    # Get the path of the file relative to git repository root, so that
    # we can run the script in any subdirectory.
    exp_path = subprocess.run(['git', 'ls-files', '--full-name', exp],
                             stdout=PIPE,
                             stderr=PIPE)
    exp = exp_path.stdout.decode().strip()
    ref_str = subprocess.run(['git', 'show', f'{ref_commit}:{exp}'],
                             stdout=PIPE,
                             stderr=PIPE,
                             cwd=dir_path)
    if ref_str.returncode:
        return None
    # Clone the older version to a temp file
    ref_file = tempfile.NamedTemporaryFile(delete=False)
    try:
        ref_file.write(ref_str.stdout)
    finally:
        ref_file.close()
        ref = ref_file.name

    return ref


def drop(exp):
    """Remove existing intel markup"""
    # No error and no ouput by intention b/c the API may be called by a test
    if not has_sed():
        return
    s_markup, m_markup_start, m_markup_end = get_markup(exp)

    script = R'/{m_markup_start}\|{m_markup_end}/Id;s/\s\+{s_markup}$//'
    script = script.replace(R'{m_markup_start}', str(m_markup_start))
    script = script.replace(R'{m_markup_end}', str(m_markup_end))
    script = script.replace(R'{s_markup}', str(s_markup))
    subprocess.run(['sed', '-i', '-b', script, exp], check=True)


def add(exp, max_line, ref):
    """Add intel markup for a clean file"""
    # No error and no ouput by intention b/c the API may be called by a test
    if not has_git() or not has_sed():
        return

    s_markup, m_markup_start, m_markup_end = get_markup(exp)

    # Use the latest file from llorg by default
    if not ref:
        ref = get_llorg_ref(exp)
    # ref does not exist (a intel-only file)
    if not ref:
        return

    # Format for output of git diff:
    #
    # @@ -[ref_start][,ref_lines] +[exp_start],[,exp_lines] @@
    #
    # [,ref_lines] or [,exp_lines] can be omitted if it's a single-line change
    diff = subprocess.run([
        'git', 'diff', '--ignore-cr-at-eol', '--unified=0', '--no-index',
        '--no-color', ref, exp
    ],
                          stdout=PIPE)
    diff = subprocess.run(['sed', '-nE', R"s/^@@[0-9 ,-]+\+([0-9,]+).*/\1/p"],
                          input=diff.stdout,
                          stdout=PIPE,
                          check=True)
    diff = diff.stdout.decode('utf-8').strip().split()
    for line_range in diff:
        exp_start = int(line_range.split(',')[0])
        exp_lines = int(line_range.split(',')[1]) if ',' in line_range else 1
        if not exp_lines:
            continue
        exp_end = exp_start + exp_lines - 1
        if exp_lines <= max_line:
            s_script = f'{exp_start},{exp_end}s/$/ {s_markup}/'
            subprocess.run(['sed', '-i', '-b', s_script, exp])
        else:
            # No new line here, so the line info from git diff is still valid
            m_script = f'{exp_start}s/^/{m_markup_start} /;{exp_end}s/$/{m_markup_end} /'
            subprocess.run(['sed', '-i', '-b', m_script, exp])

    # Make multi-line markup in a stand-alone line
    m_script = R's/^\({m_markup_start}\) /\1\n/;s/\({m_markup_end}\) $/\n\1/'
    m_script = m_script.replace(R'{m_markup_start}', str(m_markup_start))
    m_script = m_script.replace(R'{m_markup_end}', str(m_markup_end))
    subprocess.run(['sed', '-i', '-b', m_script, exp])

    # Drop one-line markup for blank line change
    script = R's/^\s\+{s_markup}$//'
    script = script.replace(R'{s_markup}', str(s_markup))
    subprocess.run(['sed', '-i', '-b', script, exp], check=True)


def update(exp, max_line=3, ref=None):
    """Update intel markup for a file (Do nothing if there is an explicit INTEL_MARKUP=0) """
    markup = os.environ.get('INTEL_MARKUP', '1')
    if markup == '0':
        return

    drop(exp)
    add(exp, max_line, ref)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exp', metavar='EXP', help='The file to be updated')
    parser.add_argument(
        '--max',
        metavar='MAX',
        help=
        'single-line markup for change within MAX, otherwise multi-line markup',
        type=int,
        default=3)
    parser.add_argument(
        '--ref',
        metavar='REF',
        help=
        'The file used a reference (latest llorg version is used if not provided'
    )
    parser.add_argument('--drop', help='Drop the markup', action='store_true')
    args = parser.parse_args()

    # Argument checking
    max_line = args.max
    if max_line < 0:
        raise ValueError(f'--max value {max_line} is less than 0')
    exp = args.exp
    if not os.path.isfile(exp):
        raise OSError(f'Non-existing file {exp}')
    ref = args.ref
    if ref and not os.path.isfile(ref):
        raise OSError(f'Non-existing file {ref}')

    drop(exp)
    if args.drop:
        return

    add(exp, max_line, ref)


if __name__ == '__main__':
    main()
