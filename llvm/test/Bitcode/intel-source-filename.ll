; Check that the full source filname path is stripped by default but
; allowed with an internal option.
; RUN: llvm-as %s -o %t-default.bc
; RUN: llvm-as %s -strip-module-src-path=true -strip-strtab-src-path=true -o %t-strip-path.bc
; RUN: llvm-as %s -strip-module-src-path=false -strip-strtab-src-path=false -o %t-no-strip-path.bc
; RUN: llvm-dis %t-default.bc -o - | FileCheck %s --check-prefix=NOPATH
; RUN: llvm-dis %t-strip-path.bc -o - | FileCheck %s --check-prefix=NOPATH
; RUN: llvm-dis %t-no-strip-path.bc -o - | FileCheck %s --check-prefix=FULLPATH

; RUN: llvm-bcanalyzer -dump %t-default.bc | FileCheck %s --check-prefix=DUMP-NOPATH
; RUN: llvm-bcanalyzer -dump %t-strip-path.bc | FileCheck %s --check-prefix=DUMP-NOPATH
; RUN: llvm-bcanalyzer -dump %t-no-strip-path.bc | FileCheck %s --check-prefix=DUMP-FULLPATH

source_filename = "/path/to/source.c"

; NOPATH: source_filename = "source.c"
; FULLPATH: source_filename = "/path/to/source.c"

; This case checks a dump of the entire file to make sure the path isn't
; embedded anywhere in the file unless a special option is used.
; DUMP-NOPATH-NOT: /path/to
; DUMP-NOPATH-LABEL: SOURCE_FILENAME
; DUMP-NOPATH-NOT: /path/to
; DUMP-NOPATH: 'source.c'
; DUMP-NOPATH-LABEL: STRTAB_BLOCK
; DUMP-NOPATH-NOT: /path/to
; DUMP-NOPATH: source.c
; DUMP-NOPATH-NOT: /path/to

; DUMP-FULLPATH-LABEL: SOURCE_FILENAME
; DUMP-FULLPATH: /path/to/source.c
; DUMP-FULLPATH-LABEL: STRTAB_BLOCK
; DUMP-FULLPATH: /path/to/source.c
