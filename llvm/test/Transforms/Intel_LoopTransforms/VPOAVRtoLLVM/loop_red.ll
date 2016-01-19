;RUN: opt -VPODriver -S %s | FileCheck %s

; CHECK: simd.end.region
; CHECK:  extractelement <4 x float> {{.*}}, i32 0
; CHECK:  extractelement <4 x float> {{.*}}, i32 1
; CHECK:  %reduction{{.*}} = fadd float 
; CHECK:  extractelement <4 x float> {{.*}}, i32 2
; CHECK:  %reduction{{.*}} = fadd float %reduction
; CHECK:  extractelement <4 x float> {{.*}}, i32 3
; CHECK:  %reduction{{.*}} = fadd float %reduction
; CHECK:  ret float %reduction
  
; CHECK: vector.body:
; CHECK: %vec.rdx.phi = phi <4 x float> [ zeroinitializer, %entry ], [{{.*}}, %vector.body ]
; CHECK: fadd <4 x float> %vec.rdx.phi


target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

@sum = global float 0.000000e+00, align 4

; Function Attrs: nounwind uwtable
define void @foo(float %a) #0 {
entry:
  %a.addr = alloca float, align 4
  store float %a, float* %a.addr, align 4
  %0 = load float, float* %a.addr, align 4
  %1 = load float, float* @sum, align 4
  %add = fadd float %1, %0
  store float %add, float* @sum, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define x86_regcallcc float @_ZGVxN4v_foo(<4 x float> %a) #0 {
entry:
  %vec.a = alloca <4 x float>
  store <4 x float> %a, <4 x float>* %vec.a, align 4
  %vec.a.cast = bitcast <4 x float>* %vec.a to float*
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !9)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !10, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !11, <4 x float> %a)
  call void @llvm.intel.directive(metadata !12)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %sum1 = phi float [0.000000e+00, %simd.begin.region ], [ %sum, %simd.loop.exit ]

  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !16, float %sum1)
  call void @llvm.intel.directive(metadata !12)

  %vec.a.cast.gep = getelementptr float, float* %vec.a.cast, i32 %index
  %0 = load float, float* %vec.a.cast.gep, align 4
  %sum = fadd float %sum1, %0
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 1, %index
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !13

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !15)
  call void @llvm.intel.directive(metadata !12)
  br label %return

return:                                           ; preds = %simd.end.region
  ret float %sum
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

attributes #0 = { nounwind uwtable "_ZGVxN4v_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!cilk.functions = !{!0}
!llvm.module.flags = !{!7}
!llvm.ident = !{!8}

!0 = !{void (float)* @foo, !1, !2, !3, !4, !5, !6}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a"}
!3 = !{!"arg_step", i32 undef}
!4 = !{!"arg_alig", i32 undef}
!5 = !{!"vec_length", float undef, i32 4}
!6 = !{!"mask", i1 false}
!7 = !{i32 1, !"PIC Level", i32 2}
!8 = !{!"clang version 3.8.0 (branches/vpo 1722)"}
!9 = !{!"DIR.OMP.SIMD"}
!10 = !{!"QUAL.OMP.SIMDLEN"}
!11 = !{!"QUAL.OMP.PRIVATE"}
!12 = !{!"DIR.QUAL.LIST.END"}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.unroll.disable"}
!15 = !{!"DIR.OMP.END.SIMD"}
!16 = !{!"QUAL.OMP.REDUCTION.ADD"}
