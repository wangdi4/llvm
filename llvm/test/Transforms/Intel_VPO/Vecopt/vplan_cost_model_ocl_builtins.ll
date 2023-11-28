; Test to check VPlan's cost-modelling of OCL builtins that are vectorized
; via LibraryFunc vectorization scenarios. These builtins are not lowered
; via SVML library and the vector function mapping is explicitly added
; to the lookup table.

; RUN: opt -passes=vplan-vec -vector-library=SVML -vplan-cost-model-print-analysis-for-vf=4 -disable-output < %s | FileCheck %s

; CHECK-LABEL:   Cost Model for VPlan foo:for.body.#{{[0-9]+}} with VF = 4:
; CHECK:         Cost 104 for float {{%vp.*}} = call float {{%vp.*}} float {{%vp.*}} i32 {{%vp.*}} _Z6selectDv4_fS_Dv4_i [x 1]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %arg1, ptr %arg2, ptr %arg3, ptr %dest, i64 %wide.trip.count) {
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.preheader ], [ %indvars.iv.next, %for.body ]
  %arg1.gep = getelementptr inbounds float, ptr %arg1, i64 %indvars.iv
  %arg1.load = load float, ptr %arg1.gep, align 4
  %arg2.gep = getelementptr inbounds float, ptr %arg2, i64 %indvars.iv
  %arg2.load = load float, ptr %arg2.gep, align 4
  %arg3.gep = getelementptr inbounds i32, ptr %arg3, i64 %indvars.iv
  %arg3.load = load i32, ptr %arg3.gep, align 4
  %call = tail call afn float @_Z6selectffi(float %arg1.load, float %arg2.load, i32 %arg3.load) #9
  %res.ptr = getelementptr inbounds float, ptr %dest, i64 %indvars.iv
  store float %call, ptr %res.ptr, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare float @_Z6selectffi(float %x, float %y, i32 %m) nounwind readnone
