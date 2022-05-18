# Usage of search_replace.py

```
usage: search_replace.py [-h] [--batch-file BATCH_FILE] [--search SEARCH]
                         [--replace REPLACE] [--in-place]
                         [--output-dir OUTPUT_DIR] [--debug]
                         inputfile [inputfile ...]

A script to search and replace regex patterns for given files.

positional arguments:
  inputfile

optional arguments:
  -h, --help            show this help message and exit
  --batch-file BATCH_FILE
                        Specify search-replace patterns from file
  --search SEARCH       The regex pattern to match
  --replace REPLACE     The replacement pattern
  --in-place            Modify input files in-place
  --output-dir OUTPUT_DIR
                        The output directory
  --debug
```

Example usage:

```bash
$ python opencl-intel/backend/libraries/scripts/search_replace.py clang/lib/Headers/ia32intrin.h --output-dir=./ --search='(LLVM)' --replace='Hello \1' --debug

All matched patterns in clang/lib/Headers/ia32intrin.h for
  (LLVM)
  ['LLVM', 'LLVM', 'LLVM']

  Replacing with Hello \1

Writing to ./ia32intrin.h

$ head ia32intrin.h

/* ===-------- ia32intrin.h ---------------------------------------------------===
 *
 * Part of the Hello LLVM/*MODIFIED BY search_replace.py*/ Project, under the Apache License v2.0 with Hello LLVM/*MODIFIED BY search_replace.py*/ Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH Hello LLVM/*MODIFIED BY search_replace.py*/-exception
 *
 *===-----------------------------------------------------------------------===
 */
```

# Batch file syntax

1. A comment line starts with a `#` character.
2. Empty lines are ignored.
3. A **s**earch line starting with `s:` (no spaces around) defines a search pattern.
4. A **r**eplace line starting with `r:` (no spaces around) must follow the previous defined search line, which indicates the replacement of the corresponding search pattern.
5. Regex patterns are parsed using python `re` module. See https://docs.python.org/3/library/re.html
6. Each replaced pattern will be attached with a suffix `/*MODIFIED BY search_replace.py*/` as a proof of the modification.
7. A **c**omment-out line starting with `c:` (no spaces around) is a shortcut of search-capture-and-comment-out:

```
c:search pattern
# is equivalent to
s:(search pattern)
r:/* \1 */
```
