;RUN: opt -VPlanDriver -enable-vp-value-codegen=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
;RUN: opt -VPlanDriver -enable-vp-value-codegen=true -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPVAL

; CHECK-LABEL: expl_reduction_add
; CHECK: vector.body
; CHECK: %[[VRES:.*]] = fadd <8 x float>

; CHECK-VPVAL: VPlannedBB
; CHECK-VPVAL:  %[[RES:.*]] = call float @llvm.experimental.vector.reduce.v2.fadd.f32.v8f32(float %x.promoted, <8 x float> %[[VRES]])

; CHECK-LLVM: middle.block
; CHECK-LLVM: shufflevector <8 x float> %[[VRES]]
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: shufflevector <8 x float>
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: shufflevector <8 x float>
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: %[[RES:.*]] = extractelement <8 x float>

; CHECK: phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %x.promoted, %min.iters.checked ], [ %[[RES]], %middle.block ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"x = %f\0A\00", align 1

define float @expl_reduction_add(float* nocapture %a) {
entry:
  %x = alloca float, align 4
  store float 0.000000e+00, float* %x, align 4
  br label %entry.split

entry.split:                                      ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.REDUCTION.ADD"(float* %x) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %entry.split
  %x.promoted = load float, float* %x, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %DIR.QUAL.LIST.END.2
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.body ]
  %add7 = phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %add, %for.body ]
  %add = fadd float %add7, 5.000000e-01
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  br label %for.end1

for.end1:                                         ; preds = %for.end
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %for.end
  store float %add.lcssa, float* %x, align 4
  %conv6 = fpext float %add.lcssa to double
  %call = call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %conv6)
  %x1 = load float, float* %x, align 4
  ret float %x1
}

; CHECK-LABEL: expl_reduction_sub
; CHECK: vector.body
; CHECK: %[[VRES:.*]] = fsub <8 x float>

; CHECK-VPVAL: VPlannedBB
; CHECK-VPVAL: %[[RES:.*]] = call float @llvm.experimental.vector.reduce.v2.fadd.f32.v8f32(float %x.promoted, <8 x float> %[[VRES]])

; CHECK-LLVM: middle.block
; CHECK-LLVM: shufflevector <8 x float> %[[VRES]]
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: shufflevector <8 x float>
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: shufflevector <8 x float>
; CHECK-LLVM: fadd <8 x float>
; CHECK-LLVM: %[[RES:.*]] = extractelement <8 x float>

; CHECK: phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %x.promoted, %min.iters.checked ], [ %[[RES]], %middle.block ]

define float @expl_reduction_sub(float* nocapture %a) {
entry:
  %x = alloca float, align 4
  store float 0.000000e+00, float* %x, align 4
  br label %entry.split

entry.split:                                      ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.REDUCTION.ADD"(float* %x) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %entry.split
  %x.promoted = load float, float* %x, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %DIR.QUAL.LIST.END.2
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.body ]
  %add7 = phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %add, %for.body ]
  %add = fsub float %add7, 5.000000e-01
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  br label %for.end1

for.end1:                                         ; preds = %for.end
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %for.end
  store float %add.lcssa, float* %x, align 4
  %conv6 = fpext float %add.lcssa to double
  %call = call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %conv6)
  %x1 = load float, float* %x, align 4
  ret float %x1
}

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @printf(i8* nocapture readonly, ...)

attributes #0 = { argmemonly nounwind }
