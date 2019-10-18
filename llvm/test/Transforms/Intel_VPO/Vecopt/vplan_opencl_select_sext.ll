; RUN: opt %s -vector-library=SVML -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -vplan-force-vf=4 -S | FileCheck %s
; RUN: opt %s -vector-library=SVML -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -vplan-force-vf=4 -enable-vp-value-codegen -S | FileCheck %s

; Verify vectorization of Opencl select build-in. SExt case.
; This test is from Volcano compiler but it's been highly modified by hand.
; The IR is only intended to trigger the code generation of the vector select
; build-in.

; CHECK: @test_select_sext
; CHECK: sext
; CHECK: _Z6selectDv4_fS_Dv4_i
; CHECK: ret void

define void @test_select_sext(<8 x float> %accumulated_occupancy, float addrspace(2)* %kernel_parameters) #5 {
simd.begin.region:
  %vec.accumulated_occupancy = alloca <8 x float>, align 32
  store <8 x float> %accumulated_occupancy, <8 x float>* %vec.accumulated_occupancy, align 32
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %simd.begin.region
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %DIR.OMP.SIMD.3
  %weight_occupied = getelementptr inbounds float, float addrspace(2)* %kernel_parameters, i64 0
  %weight_free2 = getelementptr inbounds float, float addrspace(2)* %kernel_parameters, i64 0
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop, %DIR.QUAL.LIST.END.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %simd.loop ], [ 0, %DIR.QUAL.LIST.END.1 ]
  %0 = load float, float addrspace(2)* %weight_occupied, align 4
  %1 = load float, float addrspace(2)* %weight_free2, align 4
  %cmp = fcmp ogt float %0, 5.000000e-01
  %conv = sext i1 %cmp to i32
  %call4 = tail call float @_Z6selectffi(float %0, float %1, i32 %conv) #9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 8
  br i1 %exitcond, label %simd.loop, label %simd.end.region 

simd.end.region:                                  ; preds = %simd.loop
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %simd.end.region
  ret void
}

declare float @_Z6selectffi(float %x, float %y, i32 %m) nounwind readnone 

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
