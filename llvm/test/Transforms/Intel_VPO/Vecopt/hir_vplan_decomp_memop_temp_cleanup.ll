; Test HIR decomposition of memory operands when loads are cleaned up by HIR Temp Cleanup pass.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; float ip[1024];
; float arr1[1024];
; float arr2[1024];
;
; void foo()
; {
;     int index;
;
;     for (index = 0; index < 1024; index++) {
;         ip[index] = arr1[index] + arr2[index];
;     }
; }

; Input HIR
; <15>      + DO i1 = 0, 1023, 1   <DO_LOOP>
; <6>       |   %add = (@arr1)[0][i1]  +  (@arr2)[0][i1];
; <8>       |   (@ip)[0][i1] = %add;
; <15>      + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s


; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK-NEXT: float* [[ADDR1:%vp.*]] = subscript inbounds [1024 x float]* @arr1 {i64 0 : i64 0 : i64 4096 : [1024 x float]*([1024 x float])} {i64 0 : i64 [[I1]] : i64 4 : [1024 x float](float)}
; CHECK-NEXT: float [[LOAD1:%vp.*]] = load float* [[ADDR1]]
; CHECK-NEXT: float* [[ADDR2:%vp.*]] = subscript inbounds [1024 x float]* @arr2 {i64 0 : i64 0 : i64 4096 : [1024 x float]*([1024 x float])} {i64 0 : i64 [[I1]] : i64 4 : [1024 x float](float)}
; CHECK-NEXT: float [[LOAD2:%vp.*]] = load float* [[ADDR2]]
; CHECK-NEXT: float [[ADD:%vp.*]] = fadd float [[LOAD1]] float [[LOAD2]]
; CHECK-NEXT: float* [[ADDR3:%vp.*]] = subscript inbounds [1024 x float]* @ip {i64 0 : i64 0 : i64 4096 : [1024 x float]*([1024 x float])} {i64 0 : i64 [[I1]] : i64 4 : [1024 x float](float)}
; CHECK-NEXT: store float [[ADD]] float* [[ADDR3]]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@ip = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr1, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [1024 x float], [1024 x float]* @arr2, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx2, align 4, !tbaa !2
  %add = fadd float %0, %1
  %arrayidx4 = getelementptr inbounds [1024 x float], [1024 x float]* @ip, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %add, float* %arrayidx4, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
