; Source code:
;
;__attribute__((noinline))
;void foo(int *p) {
;  for (int i=0;i<10;++i) {
;    if (p[0] > 3) {
;      puts(".");
;    } else {
;      p[0] = i;
;    }
;  }
;}
;
;int A[100];
;int main() {
;  A[0] = 0;
; foo(A);
; return 0;
;}
;
; The DV will not be overridden to (=) ? When Src and Dst memrefs are the same and no IV and Src does not dominate or post dominate sink 
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=1 -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=1 -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK-DAG:  (%p)[0] --> (%p)[0] FLOW (*) (?)
; CHECK-DAG:  (%p)[0] --> (%p)[0] OUTPUT (*) (?)
; CHECK-DAG:  %0 --> %0 FLOW (=) (0)
; CHECK-DAG:  (%p)[0] --> (%p)[0] ANTI (*) (?)
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [2 x i8] c".\00", align 1
@A = common dso_local global [100 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32* nocapture %p) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.07 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %0 = load i32, i32* %p, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 3
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %call = tail call i32 (i8*, ...) bitcast (i32 (...)* @puts to i32 (i8*, ...)*)(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0)) #3
  br label %for.inc

if.else:                                          ; preds = %for.body
  store i32 %i.07, i32* %p, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond = icmp eq i32 %inc, 10
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare dso_local i32 @puts(...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  store i32 0, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16, !tbaa !6
  tail call void @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0))
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_i", !3, i64 0}
