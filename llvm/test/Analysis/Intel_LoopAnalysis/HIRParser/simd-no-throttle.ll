; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that simd loop is not throttled due to presence of goto.

; CHECK: @llvm.intel.directive(!"DIR.OMP.SIMD");
; CHECK: @llvm.intel.directive(!"DIR.QUAL.LIST.END");
; CHECK: + DO i1 = 0, 2047, 1   <DO_LOOP>
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   if (%0 != 0.000000e+00)
; CHECK: |   {
; CHECK: |      %1 = (%b)[i1];
; CHECK: |      if (%1 != 0.000000e+00)
; CHECK: |      {
; CHECK: |         %2 = (%c)[i1];
; CHECK: |         %mul = %1  *  %2;
; CHECK: |         %add = %0  +  %mul;
; CHECK: |         (%d)[i1] = %add;
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         goto if.else;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      if.else:
; CHECK: |      %4 = (i32*)(%c)[i1];
; CHECK: |      (i32*)(%d)[i1] = %4;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: @llvm.intel.directive(!"DIR.OMP.END.SIMD");
; CHECK: @llvm.intel.directive(!"DIR.QUAL.LIST.END");



; ModuleID = 'short-cir.ll'
source_filename = "short-cir.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* nocapture readonly %a, float* nocapture readonly %b, float* nocapture readonly %c, float* nocapture %d) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %tobool = fcmp une float %0, 0.000000e+00
  br i1 %tobool, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %arrayidx2 = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4, !tbaa !1
  %tobool3 = fcmp une float %1, 0.000000e+00
  br i1 %tobool3, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %arrayidx9 = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %2 = load float, float* %arrayidx9, align 4, !tbaa !1
  %mul = fmul float %1, %2
  %add = fadd float %0, %mul
  %arrayidx11 = getelementptr inbounds float, float* %d, i64 %indvars.iv
  store float %add, float* %arrayidx11, align 4, !tbaa !1
  br label %for.inc

if.else:                                          ; preds = %land.lhs.true, %for.body
  %arrayidx13 = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %3 = bitcast float* %arrayidx13 to i32*
  %4 = load i32, i32* %3, align 4, !tbaa !1
  %arrayidx15 = getelementptr inbounds float, float* %d, i64 %indvars.iv
  %5 = bitcast float* %arrayidx15 to i32*
  store i32 %4, i32* %5, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2048
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.isVoid.p0i8.p0i8(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17926)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
