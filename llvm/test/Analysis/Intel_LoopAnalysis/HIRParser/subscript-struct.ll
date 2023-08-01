; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; RUN: opt < %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; Check that loop would be parsed same in GEP and in Subscript mode.

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:       |   (@a)[0][i1].0[i1].0[10 * i1] = i1;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { [10 x %struct.B] }
%struct.B = type { [100 x i32] }

@a = common dso_local local_unnamed_addr global [10 x %struct.A] zeroinitializer, align 16

define dso_local void @foo(i32 %n) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 10
  %arrayidx4 = getelementptr inbounds [10 x %struct.A], ptr @a, i64 0, i64 %indvars.iv, i32 0, i64 %indvars.iv, i32 0, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

