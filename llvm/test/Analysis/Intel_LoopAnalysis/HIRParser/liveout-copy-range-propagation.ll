; RUN: opt %s -passes="print<scalar-evolution>" -disable-output 2>&1 | FileCheck %s

; CHECK: %k.0840 = phi i64 [ %inc, %for.inc190 ], [ 1, %entry ], !in.de.ssa !0
; CHECK-NEXT:  -->  {1,+,1}<%for.body184> U: [[URANGE:.*]] S: [[SRANGE:.*[)]]]

; CHECK: %k.0840.out = call i64 @llvm.ssa.copy.i64(i64 %k.0840), !out.de.ssa !0
; CHECK-NEXT:  -->  %k.0840.out U: [[URANGE]] S: [[SRANGE]]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64 %in, i64 %t) {
entry:
  %shr163 = ashr i64 %in, 2
  br label %for.body184

for.body184:                                      ; preds = %for.inc190, %entry
  %k.0840 = phi i64 [ %inc, %for.inc190 ], [ 1, %entry ], !in.de.ssa !0
  %k.0840.out = call i64 @llvm.ssa.copy.i64(i64 %k.0840), !out.de.ssa !0
  %cmp186 = icmp eq i64 %t, 5
  br i1 %cmp186, label %for.inc190, label %for.end191.loopexit

for.inc190:                                       ; preds = %for.body184
  %inc = add i64 %k.0840, 1
  %cmp182 = icmp sgt i64 %inc, %shr163
  %k.0840.in = call i64 @llvm.ssa.copy.i64(i64 %inc), !in.de.ssa !0
  br i1 %cmp182, label %for.end191.loopexit, label %for.body184

for.end191.loopexit:                              ; preds = %for.inc190, %for.body184
  %k.0.lcssa.ph = phi i64 [ %inc, %for.inc190 ], [ %k.0840.out, %for.body184 ]
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @llvm.ssa.copy.i64(i64 returned)

!0 = !{!"k.0840.de.ssa"}

