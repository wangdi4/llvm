; RUN: opt %s -vector-library=SVML -passes=vplan-vec -disable-vplan-predicator -vplan-force-vf=4 -S | FileCheck %s

; Verify vectorization of Opencl select build-in. Generic case.
; This test is from Volcano compiler but it's been highly modified by hand.
; The IR is only intended to trigger the code generation of the vector select
; build-in.


; CHECK: @test_select_cmp
; CHECK: icmp ne
; CHECK: sext
; CHECK: _Z6selectDv4_fS_Dv4_i
; CHECK: ret void
define void @test_select_cmp(<8 x float> %accumulated_occupancy, ptr addrspace(2) %kernel_parameters) #5 {
simd.begin.region:
  %vec.accumulated_occupancy = alloca <8 x float>, align 32
  store <8 x float> %accumulated_occupancy, ptr %vec.accumulated_occupancy, align 32
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %simd.begin.region
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %DIR.OMP.SIMD.3
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop, %DIR.QUAL.LIST.END.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %simd.loop ], [ 0, %DIR.QUAL.LIST.END.1 ]
  %0 = load float, ptr addrspace(2) %kernel_parameters, align 4
  %1 = load float, ptr addrspace(2) %kernel_parameters, align 4
  %conv = bitcast float %0 to i32
  %call4 = tail call afn float @_Z6selectffi(float %0, float %1, i32 %conv) #9
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
