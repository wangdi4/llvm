; RUN: opt -disable-output -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; Currently reroller bails out with CmpInst

;int A[50], B[50];
;
;void foo() {
;  int i;
;
;  for(i=0; i<10; i++) {
;    B[2*i] = A[2*i] > 5;
;    B[2*i+1] = A[2*i+1] < 5;
;  }
;}

; CHECK:Function: foo

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:               |   %cmp1 = (@A)[0][2 * i1] > 5;
; CHECK:               |   (@B)[0][2 * i1] = %cmp1;
; CHECK:               |   %cmp8 = (@A)[0][2 * i1 + 1] < 5;
; CHECK:               |   (@B)[0][2 * i1 + 1] = %cmp8;
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:              |   %cmp1 = (@A)[0][2 * i1] > 5;
; CHECK:              |   (@B)[0][2 * i1] = %cmp1;
; CHECK:              |   %cmp8 = (@A)[0][2 * i1 + 1] < 5;
; CHECK:              |   (@B)[0][2 * i1 + 1] = %cmp8;
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'cmp.c'
source_filename = "cmp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %0, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx, align 8, !tbaa !2
  %cmp1 = icmp sgt i32 %1, 5
  %conv = zext i1 %cmp1 to i32
  %arrayidx4 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %0, !intel-tbaa !2
  store i32 %conv, ptr %arrayidx4, align 8, !tbaa !2
  %2 = or i64 %0, 1
  %arrayidx7 = getelementptr inbounds [50 x i32], ptr @A, i64 0, i64 %2, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx7, align 4, !tbaa !2
  %cmp8 = icmp slt i32 %3, 5
  %conv9 = zext i1 %cmp8 to i32
  %arrayidx13 = getelementptr inbounds [50 x i32], ptr @B, i64 0, i64 %2, !intel-tbaa !2
  store i32 %conv9, ptr %arrayidx13, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang b97cd1e0ccdf66edc0b2a4aadd0de0874ecd119f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f111594d37223481ddeefd35ed5a93867c3660cb)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA50_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
