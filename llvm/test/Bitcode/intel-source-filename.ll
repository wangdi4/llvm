; Check that the full source filname path is stripped by default but
; allowed with an internal option.
; RUN: llvm-as %s -o %t-default.bc
; RUN: llvm-as %s -strip-module-src-path=false -o %t-strip-path.bc
; RUN: llvm-as %s -strip-module-src-path=true -o %t-no-strip-path.bc
; RUN: llvm-dis %t-default.bc -o - | FileCheck %s --check-prefix=NOPATH
; RUN: llvm-dis %t-no-strip-path.bc -o - | FileCheck %s --check-prefix=NOPATH
; RUN: llvm-dis %t-strip-path.bc -o - | FileCheck %s --check-prefix=FULLPATH

source_filename = "/path/to/source.c"

; NOPATH: source_filename = "source.c"
; FULLPATH: source_filename = "/path/to/source.c"
