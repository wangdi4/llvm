; RUN: opt -disable-output -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=2 -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-dump-induction-init-details < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test makes sure iv range analyis during induction importing can handle int inductions
; for hir.

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; CHECK: IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023
; CHECK: induction-init{add, StartVal: i64 0, EndVal: i64 1023}

define dso_local void @foo(i32* noalias nocapture %a, i32* noalias nocapture readonly %b, i32 %n) local_unnamed_addr #0
{
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %add = add nsw i32 %0, %n
  %ptridx2 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %add, i32* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
