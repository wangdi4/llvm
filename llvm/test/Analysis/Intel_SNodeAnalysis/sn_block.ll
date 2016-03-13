; RUN:  opt  -sna -analyze < %s | FileCheck %s
;
; The compiler expects the following output.
;
; CHECK: SN[0]  pred:  succ: 
; CHECK: SN_BLOCK SN[0] B[%entry]
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define i32 @foo() {
entry:
  ret i32 1
}


