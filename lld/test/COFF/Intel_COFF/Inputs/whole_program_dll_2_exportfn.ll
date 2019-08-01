; This test case will be used for creating a DLL and then checking
; if the executable achieves whole program read while the
; library is included in the linking process. See the test case
; COFF/Intel_COFF/whole_program_dll_2.ll for more details.

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