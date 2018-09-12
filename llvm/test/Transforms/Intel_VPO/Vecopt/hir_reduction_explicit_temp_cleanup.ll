; Generated from following source using 
; icx -O1 -S -emit-llvm exp1.c -mllvm -disable-vpo-directive-cleanup -fopenmp -Qoption,c,-fintel-openmp
; float arr[1024];
; 
; float foo()
; {
;   float sum = 0.0;
;   int index;
; 
;   sum = 0.0;
; #pragma omp simd reduction(+:sum)
;   for (index = 0; index < 1024; index++)
;     sum += arr[index];
; 
;   return sum;
; }
; 
; ModuleID = 'exp1.c'
;RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPODriverHIR -hir-cg -mem2reg -S %s | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK: loop
; CHECK: phi <4 x float> [ zeroinitializer
; CHECK: fadd <4 x float>
; CHECK: afterloop
; CHECK: shufflevector <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: fadd <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: fadd float
; CHECK: ret float
source_filename = "exp1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: nounwind readonly uwtable
define float @foo() local_unnamed_addr #0 {
entry:
  %sum = alloca float, align 4
  %0 = bitcast float* %sum to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  store float 0.000000e+00, float* %sum, align 4, !tbaa !1
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.ADD", float* nonnull %sum)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %sum.promoted = load float, float* %sum, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add5 = phi float [ %sum.promoted, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !5
  %add = fadd float %1, %add5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.lifetime.end(i64 4, i8* %0) #2
  ret float %add
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20316)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA1024_f", !2, i64 0}
