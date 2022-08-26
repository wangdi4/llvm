; Test for explicit vectorization going through HIR.

; Generated from following testcase using:
; icx -fiopenmp -O2 -mllvm -print-before=vplan-vec -S -c add.c
; Generated ll file was then edited to only retain the vector clone function.
; #pragma omp declare simd nomask
; float foo(float a, float b){
;   return a + b;
; }

; RUN: opt -print-after=hir-vplan-vec -hir-ssa-deconstruction -hir-framework -hir-vplan-vec < %s 2>&1 -disable-output | FileCheck %s
; Check for vectorized HIR loop

; CHECK:      BEGIN REGION {
; CHECK-NEXT: %.vec = (<4 x float>*)(%vec.a.cast)[0];
; CHECK-NEXT: %.vec2 = (<4 x float>*)(%vec.b.cast)[0];
; CHECK-NEXT: %.vec3 = %.vec2  +  %.vec;
; CHECK-NEXT: (<4 x float>*)(%ret.cast)[0] = %.vec3;
; CHECK:      END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable willreturn
define dso_local float @foo(float %a, float %b) local_unnamed_addr #0 {
entry:
  %add = fadd fast float %b, %a
  ret float %add
}

; Function Attrs: norecurse nounwind readnone uwtable willreturn
define dso_local <4 x float> @_ZGVbN4vv_foo(<4 x float> %a, <4 x float> %b) local_unnamed_addr #0 {
entry:
  %vec.a = alloca <4 x float>, align 16
  %vec.b = alloca <4 x float>, align 16
  %vec.retval = alloca <4 x float>, align 16
  %vec.a.cast = bitcast <4 x float>* %vec.a to float*
  store <4 x float> %a, <4 x float>* %vec.a, align 16
  %vec.b.cast = bitcast <4 x float>* %vec.b to float*
  store <4 x float> %b, <4 x float>* %vec.b, align 16
  %ret.cast = bitcast <4 x float>* %vec.retval to float*
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %vec.a.cast.gep = getelementptr float, float* %vec.a.cast, i32 %index
  %vec.a.elem = load float, float* %vec.a.cast.gep, align 4
  %vec.b.cast.gep = getelementptr float, float* %vec.b.cast, i32 %index
  %vec.b.elem = load float, float* %vec.b.cast.gep, align 4
  %add = fadd fast float %vec.b.elem, %vec.a.elem
  %ret.cast.gep = getelementptr float, float* %ret.cast, i32 %index
  store float %add, float* %ret.cast.gep, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !2

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <4 x float>, <4 x float>* %vec.retval, align 16
  ret <4 x float> %vec.ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1


attributes #0 = { norecurse nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" "vector-variants"="_ZGVbN4vv_foo,_ZGVcN8vv_foo,_ZGVdN8vv_foo,_ZGVeN16vv_foo,_ZGVbM4vv_foo,_ZGVcM8vv_foo,_ZGVdM8vv_foo,_ZGVeM16vv_foo" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
!4 = distinct !{!4, !3}
!5 = distinct !{!5, !3}
!6 = distinct !{!6, !3}
!7 = distinct !{!7, !3}
!8 = distinct !{!8, !3}
!9 = distinct !{!9, !3}
!10 = distinct !{!10, !3}
