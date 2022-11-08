; REQUIRES: asserts
; This test checks that whole program read was achieved when linking
; an executable and a library is present. Whole program seen won't be
; achieved since @exportfn doesn't have IR.

; RUN: llvm-as -o %t_wp_dll_2.bc %p/Inputs/whole_program_dll_2_exportfn.ll
; RUN: llvm-as -o %t_wp_dll_2_main.bc %s
; RUN: lld-link /entry:exportfn /out:%t_wp_dll_2.dll /dll %t_wp_dll_2.bc
; RUN: lld-link /out:%t_wp_dll_2.exe /entry:main %t_wp_dll_2_main.bc %t_wp_dll_2.lib \
; RUN:     /subsystem:console /mllvm:-debug-only=whole-program-analysis \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK:  LIBFUNCS NOT FOUND: 1
; CHECK:      exportfn
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:  MAIN DEFINITION:  DETECTED
; CHECK:  LINKING AN EXECUTABLE:  DETECTED
; CHECK:  WHOLE PROGRAM READ:  DETECTED
; CHECK:  WHOLE PROGRAM SEEN: NOT DETECTED
; CHECK:  WHOLE PROGRAM SAFE: NOT DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

declare dllimport i32 @exportfn(i32)

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @exportfn(i32 %argc)
  ret i32 %call1
}

