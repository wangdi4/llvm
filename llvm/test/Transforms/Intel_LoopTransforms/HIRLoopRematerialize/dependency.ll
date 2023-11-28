; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-rematerialize" -print-changed -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

;void mul_transposed_m3_v3(float mat[3][3], float vec[3])
;{
;  const float x = vec[0];
;  const float y = vec[1];
;
;  vec[0] = x * mat[0][0] + y * mat[0][1] + mat[0][2] * vec[2];
;  vec[1] = x * mat[1][0] + y * mat[1][1] + mat[1][2] * vec[2];
;  vec[2] = x * mat[2][0] + y * mat[2][1] + mat[2][2] * vec[2];
;}

; Currently, loop is not materialized because of ANTI dependencies
; With a simple rematerialization
; The loop would have been
;       DO i = 0 to 2
;               %0 = (%vec)[0];  // In the second iteration a new value of (%vec)[0] is used.
;               %1 = (%vec)[1];
;               %mul = %0  *  (%mat)[0][0];
;               %mul6 = %1  *  (%mat)[0][1];
;               %add = %mul  +  %mul6;
;               %5 = (%vec)[2];
;               %mul10 = (%mat)[0][2]  *  %5;
;               %add11 = %add  +  %mul10;
;               (%vec)[i] = %add11;
;        END DO
; If %0 = (%vec)[0] and %1 = (%vec)[1] and %5 = (%vec)[2] were all
; could have hoisted outside of the loop, the loop's behavior would have been the same
; as the original code. However, currently, hoisting logic is not implemented and
; this loop should not be materialized for a correct result.



; CHECK: Function: mul_transposed_m3_v3

; CHECK:          BEGIN REGION { }
; CHECK:                %0 = (%vec)[0];
; CHECK:                %1 = (%vec)[1];
; CHECK:                %mul = %0  *  (%mat)[0];
; CHECK:                %mul6 = %1  *  (%mat)[0][1];
; CHECK:                %add = %mul  +  %mul6;
; CHECK:                %5 = (%vec)[2];
; CHECK:                %mul10 = (%mat)[0][2]  *  %5;
; CHECK:                %add11 = %add  +  %mul10;
; CHECK:                (%vec)[0] = %add11;
; CHECK:                %mul15 = %0  *  (%mat)[1][0];
; CHECK:                %mul18 = %1  *  (%mat)[1][1];
; CHECK:                %add19 = %mul15  +  %mul18;
; CHECK:                %mul23 = %5  *  (%mat)[1][2];
; CHECK:                %add24 = %add19  +  %mul23;
; CHECK:                (%vec)[1] = %add24;
; CHECK:                %mul28 = %0  *  (%mat)[2][0];
; CHECK:                %mul31 = %1  *  (%mat)[2][1];
; CHECK:                %add32 = %mul28  +  %mul31;
; CHECK:                %mul36 = %5  *  (%mat)[2][2];
; CHECK:                %add37 = %add32  +  %mul36;
; CHECK:                (%vec)[2] = %add37;
; CHECK:                ret ;
; CHECK:          END REGION

; CHECK: Function: mul_transposed_m3_v3

; CHECK:         BEGIN REGION { }
; CHECK:               %0 = (%vec)[0];
; CHECK:               %1 = (%vec)[1];
; CHECK:               %mul = %0  *  (%mat)[0];
; CHECK:               %mul6 = %1  *  (%mat)[0][1];
; CHECK:               %add = %mul  +  %mul6;
; CHECK:               %5 = (%vec)[2];
; CHECK:               %mul10 = (%mat)[0][2]  *  %5;
; CHECK:               %add11 = %add  +  %mul10;
; CHECK:               (%vec)[0] = %add11;
; CHECK:               %mul15 = %0  *  (%mat)[1][0];
; CHECK:               %mul18 = %1  *  (%mat)[1][1];
; CHECK:               %add19 = %mul15  +  %mul18;
; CHECK:               %mul23 = %5  *  (%mat)[1][2];
; CHECK:               %add24 = %add19  +  %mul23;
; CHECK:               (%vec)[1] = %add24;
; CHECK:               %mul28 = %0  *  (%mat)[2][0];
; CHECK:               %mul31 = %1  *  (%mat)[2][1];
; CHECK:               %add32 = %mul28  +  %mul31;
; CHECK:               %mul36 = %5  *  (%mat)[2][2];
; CHECK:               %add37 = %add32  +  %mul36;
; CHECK:               (%vec)[2] = %add37;
; CHECK:               ret ;
; CHECK:         END REGION


; 2:7 %0 --> %0 FLOW (=) (0)
; 2:21 %0 --> %0 FLOW (=) (0)
; 2:33 %0 --> %0 FLOW (=) (0)
; 4:10 %1 --> %1 FLOW (=) (0)
; 4:24 %1 --> %1 FLOW (=) (0)
; 4:36 %1 --> %1 FLOW (=) (0)
; 7:11 %mul --> %mul FLOW (=) (0)
; 10:11 %mul6 --> %mul6 FLOW (=) (0)
; 11:17 %add --> %add FLOW (=) (0)
; 15:16 %5 --> %5 FLOW (=) (0)
; 15:28 %5 --> %5 FLOW (=) (0)
; 15:40 %5 --> %5 FLOW (=) (0)
; 16:17 %mul10 --> %mul10 FLOW (=) (0)
; 17:18 %add11 --> %add11 FLOW (=) (0)
; 21:25 %mul15 --> %mul15 FLOW (=) (0)
; 24:25 %mul18 --> %mul18 FLOW (=) (0)
; 25:29 %add19 --> %add19 FLOW (=) (0)
; 28:29 %mul23 --> %mul23 FLOW (=) (0)
; 29:30 %add24 --> %add24 FLOW (=) (0)
; 33:37 %mul28 --> %mul28 FLOW (=) (0)
; 36:37 %mul31 --> %mul31 FLOW (=) (0)
; 37:41 %add32 --> %add32 FLOW (=) (0)
; 40:41 %mul36 --> %mul36 FLOW (=) (0)
; 41:42 %add37 --> %add37 FLOW (=) (0)
; 2:18 (%vec)[0] --> (%vec)[0] ANTI (=) (0)
; 2:30 (%vec)[0] --> (%vec)[1] ANTI (=) (0)
; 2:42 (%vec)[0] --> (%vec)[2] ANTI (=) (0)
; 4:18 (%vec)[1] --> (%vec)[0] ANTI (=) (0)
; 4:30 (%vec)[1] --> (%vec)[1] ANTI (=) (0)
; 4:42 (%vec)[1] --> (%vec)[2] ANTI (=) (0)
; 15:18 (%vec)[2] --> (%vec)[0] ANTI (=) (0)
; 15:30 (%vec)[2] --> (%vec)[1] ANTI (=) (0)
; 15:42 (%vec)[2] --> (%vec)[2] ANTI (=) (0)
; 18:30 (%vec)[0] --> (%vec)[1] OUTPUT (=) (0)
; 18:42 (%vec)[0] --> (%vec)[2] OUTPUT (=) (0)
; 30:42 (%vec)[1] --> (%vec)[2] OUTPUT (=) (0)

; Rematerialization Inhibiting edge:
; 15:30 (%vec)[2] --> (%vec)[1] ANTI (=) (0)

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLoopRematerialize

;Module Before HIR
; ModuleID = 'dependency.c'
source_filename = "dependency.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @mul_transposed_m3_v3(ptr noalias nocapture readonly %mat, ptr noalias nocapture %vec) local_unnamed_addr #0 {
entry:
  %0 = load float, ptr %vec, align 4, !tbaa !2
  %arrayidx1 = getelementptr inbounds float, ptr %vec, i64 1
  %1 = load float, ptr %arrayidx1, align 4, !tbaa !2
  %2 = load float, ptr %mat, align 4, !tbaa !6
  %mul = fmul float %0, %2
  %arrayidx5 = getelementptr inbounds [3 x float], ptr %mat, i64 0, i64 1, !intel-tbaa !6
  %3 = load float, ptr %arrayidx5, align 4, !tbaa !6
  %mul6 = fmul float %1, %3
  %add = fadd float %mul, %mul6
  %arrayidx8 = getelementptr inbounds [3 x float], ptr %mat, i64 0, i64 2, !intel-tbaa !6
  %4 = load float, ptr %arrayidx8, align 4, !tbaa !6
  %arrayidx9 = getelementptr inbounds float, ptr %vec, i64 2
  %5 = load float, ptr %arrayidx9, align 4, !tbaa !2
  %mul10 = fmul float %4, %5
  %add11 = fadd float %add, %mul10
  store float %add11, ptr %vec, align 4, !tbaa !2
  %arrayidx14 = getelementptr inbounds [3 x float], ptr %mat, i64 1, i64 0
  %6 = load float, ptr %arrayidx14, align 4, !tbaa !6
  %mul15 = fmul float %0, %6
  %arrayidx17 = getelementptr inbounds [3 x float], ptr %mat, i64 1, i64 1
  %7 = load float, ptr %arrayidx17, align 4, !tbaa !6
  %mul18 = fmul float %1, %7
  %add19 = fadd float %mul15, %mul18
  %arrayidx21 = getelementptr inbounds [3 x float], ptr %mat, i64 1, i64 2
  %8 = load float, ptr %arrayidx21, align 4, !tbaa !6
  %mul23 = fmul float %5, %8
  %add24 = fadd float %add19, %mul23
  store float %add24, ptr %arrayidx1, align 4, !tbaa !2
  %arrayidx27 = getelementptr inbounds [3 x float], ptr %mat, i64 2, i64 0
  %9 = load float, ptr %arrayidx27, align 4, !tbaa !6
  %mul28 = fmul float %0, %9
  %arrayidx30 = getelementptr inbounds [3 x float], ptr %mat, i64 2, i64 1
  %10 = load float, ptr %arrayidx30, align 4, !tbaa !6
  %mul31 = fmul float %1, %10
  %add32 = fadd float %mul28, %mul31
  %arrayidx34 = getelementptr inbounds [3 x float], ptr %mat, i64 2, i64 2
  %11 = load float, ptr %arrayidx34, align 4, !tbaa !6
  %mul36 = fmul float %5, %11
  %add37 = fadd float %add32, %mul36
  store float %add37, ptr %arrayidx9, align 4, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA3_f", !3, i64 0}
