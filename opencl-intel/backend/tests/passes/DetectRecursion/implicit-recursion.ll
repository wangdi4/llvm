; RUN: %oclopt -analyze -detect-recursion -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
; RUN: %oclopt -detect-recursion -enable-debugify 2>&1 --disable-output -S %s | FileCheck %s -check-prefix=DEBUGIFY
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

define void @rec1() nounwind{
entry:
  call void @rec2()
  ret void
}

define void @rec2() nounwind{
entry:
  call void @rec3()
  ret void
}

define void @rec3() nounwind{
entry:
  call void @rec1()
  ret void
}

;;printed analysis
; CHECK: Printing analysis 'detects whether there are recursions'
; CHECK-NEXT: DetectRecursion: Found recursive calls.
; CHECK-NEXT: DetectRecursion: Functions with recursive calls:
; CHECK-NEXT: rec1.
; CHECK-NEXT: rec2.
; CHECK-NEXT: rec3.
; DEBUGIFY-NOT: WARNING
