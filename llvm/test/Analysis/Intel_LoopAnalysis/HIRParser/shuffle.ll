; RUN: opt -passes="hir-ssa-deconstruction,print<hir>"  < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define void @foo(ptr %a, ptr %b) {
; CHECK:       + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:  |   %ld = (%a)[i1];
; CHECK-NEXT:  |   %vec = bitcast.i64.<2 x i32>(%ld);
; CHECK-NEXT:  |   %shuffle = shufflevector %vec,  %vec,  <i32 3, i32 0>;
; CHECK-NEXT:  + END LOOP
entry:
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]

  ; Pointers to vectors aren't supported yet, create vectors by bitcasting for now.
  %ptr = getelementptr i64, ptr %a, i64 %iv
  %ld = load i64, ptr %ptr
  %vec = bitcast i64 %ld to <2 x i32>
  %shuffle = shufflevector <2 x i32> %vec, <2 x i32> %vec, <2 x i32><i32 3, i32 0>

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 128
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}
