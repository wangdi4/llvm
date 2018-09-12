;RUN: opt -hir-ssa-deconstruction -VPODriverHIR -hir-cg -mem2reg -S %s | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK: loop
; CHECK: phi <8 x float> [ zeroinitializer
; CHECK: fadd <8 x float>
; CHECK: afterloop
; CHECK: shufflevector <8 x float>
; CHECK: shufflevector <8 x float>
; CHECK: fadd <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: fadd <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: fadd float
; CHECK: ret float

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"x = %f\0A\00", align 1

; Function Attrs: nounwind uwtable
define float @foo(float* nocapture %a) #0 {
entry:
  %x = alloca float, align 4
  store float 0.000000e+00, float* %x, align 4
  br label %entry.split

entry.split:
  tail call void @llvm.intel.directive(metadata !40)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !42, i32 8)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !41, float* nonnull %x)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %x.promoted = load float, float* %x, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry.split
  %indvars.iv = phi i64 [ 0, %entry.split ], [ %indvars.iv.next, %for.body ]
  %add7 = phi float [ %x.promoted, %entry.split ], [ %add, %for.body ]
  %add = fadd float %add7, 0.5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  store float %add, float* %x, align 4
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %conv6 = fpext float %add to double
  %call = call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %conv6) #4
  %x1 = load float, float* %x, align 4
  ret float %x1
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.ident = !{!12}

!2 = !{}
!12 = !{!"clang version 3.8.0 (branches/vpo 1936)"}
!40 = !{!"DIR.OMP.SIMD"}
!41 = !{!"QUAL.OMP.REDUCTION.ADD"}
!42 = !{!"QUAL.OMP.SIMDLEN"}
