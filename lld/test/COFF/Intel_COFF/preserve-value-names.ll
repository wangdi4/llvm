; REQUIRES: asserts
; Test that checks if value names are preserved in linker when
; /opt:fintel-preserve-value-names is provided and discarded 
; otherwise
;
; eg. '%call1' will not be replaced by a numbered identifier such as 
;     '%3' when value names are preserved

; RUN: llvm-as -o %t.bc %s
; RUN: lld-link /out:%t.exe /entry:wmain %t.bc /subsystem:console \
; RUN:        /opt:ltonewpassmanager \
; RUN:        /opt:fintel-preserve-value-names \
; RUN:        /mllvm:-print-after-all \
; RUN:        2>&1 | FileCheck %s -check-prefix=CHECK-PRSV

; Check that value names are preserved
; CHECK-PRSV: %call1 = call i32 @add(i32 %argc)
; CHECK-PRSV: %call2 = call i32 @sub(i32 %call1)
; CHECK-PRSV: ret i32 %call2

; RUN: llvm-as -o %t.bc %s
; RUN: lld-link /out:%t.exe /entry:wmain %t.bc /subsystem:console \
; RUN:        /opt:ltonewpassmanager \
; RUN:        /mllvm:-print-after-all \
; RUN:        2>&1 | FileCheck %s

; Check that value names are discarded
; CHECK-NOT: %call1 = call i32 @add(i32 %argc)
; CHECK-NOT: %call2 = call i32 @sub(i32 %call1)
; CHECK-NOT: ret i32 %call2
; CHECK: %3 = call i32 @add(i32 %0)
; CHECK: %4 = call i32 @sub(i32 %3)
; CHECK: ret i32 %4

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

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

define i32 @wmain(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
