; RUN: opt < %s -tbaa -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="require<tbaa>,hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
;
; RUN: opt < %s -opaque-pointers -tbaa -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="require<tbaa>,hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
;
; // C source:
; #define SWAP(x,y) do{ typeof(x) t = x; x = y; y = t; }while(0)
; typedef struct { float x[1024]; float y[1024]; } q;
;
; void foo(q *qA, q *qB) {
;   for (int i = 0; i < 1024; i++) {
;     qA->x[i] = qB->y[i] + 1;
;     SWAP(qA, qB);
;   }
; }
;
; In this example, base pointers are varying, making it unsafe to query alias()
; to break the dependence.  However, there's no memory dependence due to the
; fact that the references access different parts of the same struct, and TBAA
; should be able to determine this no matter how the pointers evolve.  This
; test verifies that TBAA is still able to conclude NoAlias with
; loopCarriedAlias(), which is always safe to use.
;
; CHECK: DD graph for function foo
; CHECK-NOT: FLOW (*) (?)
; CHECK-NOT: ANIT (*) (?)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.q = type { [1024 x float], [1024 x float] }

; Function Attrs: nofree norecurse nounwind optsize uwtable
define dso_local void @foo(%struct.q* nocapture %qA, %struct.q* nocapture %qB) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %qA.addr.012 = phi %struct.q* [ %qA, %entry ], [ %qB.addr.010, %for.body ]
  %qB.addr.010 = phi %struct.q* [ %qB, %entry ], [ %qA.addr.012, %for.body ]
  %arrayidx = getelementptr inbounds %struct.q, %struct.q* %qB.addr.010, i64 0, i32 1, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %add = fadd float %0, 1.000000e+00
  %arrayidx2 = getelementptr inbounds %struct.q, %struct.q* %qA.addr.012, i64 0, i32 0, i64 %indvars.iv, !intel-tbaa !8
  store float %add, float* %arrayidx2, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind optsize uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 4096}
!3 = !{!"struct@", !4, i64 0, !4, i64 4096}
!4 = !{!"array@_ZTSA1024_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!3, !5, i64 0}
