; Test for explicit vectorization going through HIR.
; RUN: opt -hir-ssa-deconstruction -hir-parser -VPODriverHIR -hir-cg -S  < %s | FileCheck %s
; Check for vectorized HIR loop

; CHECK: fadd <4 x float>
; CHECK-NEXT: store <4 x float>
; CHECK: nextiv{{.*}} = add {{.*}}, 4

; Generated from following testcase using:
; clang -fcilkplus -fdeclspec -O0 -S -mllvm -enable-vec-clone -emit-llvm add.c
; Generated ll file was then edited to only retain the vector clone function. 
;  __attribute__((vector(nomask)))
; float foo(float a, float b){
;   return a + b;
; }

; ModuleID = 'add.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define x86_regcallcc <4 x float> @_ZGVxN4vv_foo(<4 x float> %a, <4 x float> %b) #0 {
entry:
  %vec.a = alloca <4 x float>
  %vec.b = alloca <4 x float>
  %vec.retval = alloca <4 x float>
  store <4 x float> %a, <4 x float>* %vec.a, align 4
  store <4 x float> %b, <4 x float>* %vec.b, align 4
  %vec.a.cast = bitcast <4 x float>* %vec.a to float*
  %vec.b.cast = bitcast <4 x float>* %vec.b to float*
  %ret.cast = bitcast <4 x float>* %vec.retval to float*
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !8)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !9, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !10, <4 x float> %a, <4 x float> %b)
  call void @llvm.intel.directive(metadata !11)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %vec.a.cast.gep = getelementptr float, float* %vec.a.cast, i32 %index
  %0 = load float, float* %vec.a.cast.gep, align 4
  %vec.b.cast.gep = getelementptr float, float* %vec.b.cast, i32 %index
  %1 = load float, float* %vec.b.cast.gep, align 4
  %add = fadd float %0, %1
  %ret.cast.gep = getelementptr float, float* %ret.cast, i32 %index
  store float %add, float* %ret.cast.gep
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 1, %index
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !12

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !14)
  call void @llvm.intel.directive(metadata !11)
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret.cast = bitcast float* %ret.cast to <4 x float>*
  %vec.ret = load <4 x float>, <4 x float>* %vec.ret.cast
  ret <4 x float> %vec.ret
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

attributes #0 = { nounwind uwtable "_ZGVxN4vv_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!7}

!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b"}
!3 = !{!"arg_step", i32 undef, i32 undef}
!4 = !{!"arg_alig", i32 undef, i32 undef}
!5 = !{!"vec_length", float undef, i32 4}
!6 = !{!"mask", i1 false}
!7 = !{!"clang version 3.8.0 (branches/vpo 1987)"}
!8 = !{!"DIR.OMP.SIMD"}
!9 = !{!"QUAL.OMP.SIMDLEN"}
!10 = !{!"QUAL.OMP.PRIVATE"}
!11 = !{!"DIR.QUAL.LIST.END"}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.unroll.disable"}
!14 = !{!"DIR.OMP.END.SIMD"}
