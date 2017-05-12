; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s

; Verify that return instruction is correctly handled by CG.

; CHECK: region.0:
; CHECK: loop.[[LOOPNUM:[0-9]+]]:
; CHECK: afterloop.[[LOOPNUM]]:
; CHECK: ret void

; ModuleID = 't2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr, i32* nocapture %barr) #0 {
entry:
  call void @llvm.intel.directive(metadata !5)
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %index.022 = phi i64 [ 0, %entry ], [ %inc9, %for.inc ]
  %rem = srem i64 %index.022, 3
  %tobool = icmp eq i64 %rem, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %add = add nuw nsw i64 %index.022, 2
  %conv = trunc i64 %add to i32
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %index.022
  store i32 %conv, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds i32, i32* %barr, i64 %index.022
  %0 = load i32, i32* %arrayidx1, align 4, !tbaa !1
  %tobool2 = icmp eq i32 %0, 0
  br i1 %tobool2, label %for.inc, label %if.then.3

if.then.3:                                        ; preds = %if.then
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx1, align 4, !tbaa !1
  br label %for.inc

if.else:                                          ; preds = %for.body
  %conv6 = trunc i64 %index.022 to i32
  %arrayidx7 = getelementptr inbounds i32, i32* %arr, i64 %index.022
  store i32 %conv6, i32* %arrayidx7, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else, %if.then.3
  %inc9 = add nuw nsw i64 %index.022, 1
  %exitcond = icmp eq i64 %inc9, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.intel.directive(metadata !6)
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (branches/vpo 1440)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!"DIR.OMP.SIMD"}
!6 = !{!"DIR.OMP.END.SIMD"}

