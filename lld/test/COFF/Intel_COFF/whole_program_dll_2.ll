; REQUIRES: assert
; This test checks that whole program read was achieved when linking
; an executable and a library is present. Whole program seen won't be
; achieved since @exportfn doesn't have IR.

; RUN: llvm-as -o %T/wp_dll_2.bc %p/Inputs/whole_program_dll_2_exportfn.ll
; RUN: llvm-as -o %T/wp_dll_2_main.bc %s
; RUN: lld-link /entry:exportfn /out:%T/wp_dll_2.dll /dll %T/wp_dll_2.bc
; RUN: lld-link /out:%T/wp_dll_2.exe /entry:main %T/wp_dll_2_main.bc %T/wp_dll_2.lib \
; RUN:     /subsystem:console /mllvm:-debug-only=whole-program-analysis \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK:  Main definition seen
; CHECK:  LIBFUNCS NOT FOUND: 1
; CHECK:      exportfn
; CHECK:  VISIBLE OUTSIDE LTO: 1
; CHECK:      exportfn
; CHECK:  WHOLE PROGRAM NOT DETECTED
; CHECK:  WHOLE PROGRAM SAFE is *NOT* determined:
; CHECK:  whole program not seen;
; CHECK-NOT:       whole program not read;

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

declare dllimport i32 @exportfn(i32)

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @exportfn(i32 %argc)
  ret i32 %call1
}

