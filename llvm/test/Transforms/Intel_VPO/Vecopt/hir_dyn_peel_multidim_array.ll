; LIT test to check that peeling kicks in for a multi dimension array access
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -hir-details-no-verbose-indent -hir-details -vplan-force-vf=4 -vplan-enable-peeling  -mattr=+avx2 -disable-output < %s 2>&1 | FileCheck %s
;
; Incoming HIR:
;      + DO i1 = 0, 1023, 1   <DO_LOOP>
;      |   (@arr)[0][2][i1 + %n] = i1;
;      + END LOOP
;
; We expect peeling to kick in and the store to arr to be aligned on 32-byte boundary
; in the generated vector loop.
;
; CHECK:    + DO i64 i1 = 0, %peel.ub, 1   <DO_LOOP>
; CHECK:    + DO i64 i1 = %phi.temp, %loop.ub, 4   <DO_LOOP>
; CHECK:    |   (<4 x i64>*)(@arr)[0][2][i1 + %n] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK:    |   <LVAL-REG> {al:32}(<4 x i64>*)(LINEAR [10 x [1024 x i64]]* @arr)[i64 0][i64 2][LINEAR i64 i1 + %n]
; CHECK:    + END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
@arr = dso_local local_unnamed_addr global [10 x [1024 x i64]] zeroinitializer, align 16

define void @foo(i64 noundef %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %l1.05, %n
  %arrayidx = getelementptr inbounds [10 x [1024 x i64]], [10 x [1024 x i64]]* @arr, i64 0, i64 2, i64 %add
  store i64 %l1.05, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
