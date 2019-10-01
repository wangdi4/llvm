; Test case to make sure that whole program won't be achieved when creating
; a DLL because lld isn't linking an executable.

; RUN: llvm-as -o %T/wp_dll_1.bc %s
; RUN: lld-link /out:%T/wp_dll_1.dll /dll %T/wp_dll_1.bc /mllvm:-whole-program-trace \
; RUN:     2>&1 | FileCheck %s

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define void @_DllMainCRTStartup() {
  ret void
}

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define internal i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define dllexport i32 @exportfn(i32 %argc) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}

; CHECK: WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK: UNRESOLVED CALLSITES: 0
; CHECK: VISIBLE OUTSIDE LTO: 0
; CHECK: WHOLE PROGRAM NOT DETECTED
; CHECK: not linking an executable;
