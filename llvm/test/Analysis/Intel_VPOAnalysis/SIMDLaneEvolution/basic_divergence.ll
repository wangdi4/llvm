; Test basic data divergence - all inputs are uniform,
; expect no RANDOM classification.
; 
; RUN: opt < %s -analyze -slev | FileCheck -check-prefix=LLVM %s
; RUN: opt < %s -analyze -slev-hir | FileCheck -check-prefix=HIR %s
;
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR
 
; LLVM: data_divergence
; LLVM: EXPR{({{[0-9]+}}) add ({{[0-9]+}})} ===> {{{[0-9]+}}|RANDOM|AVR-{{[0-9]+}}}
; 
; HIR: data_divergence
; HIR: RANDOM
; 
; LLVM: control_divergence
; LLVM: IF ({{[0-9]+}}) ===> {{{[0-9]+}}|RANDOM|AVR-{{[0-9]+}}|BC}
; 
; HIR: control_divergence
; HIR: RANDOM
; Produced from the program:
; 
; __declspec(vector(nomask, uniform(b)))
; int data_divergence(int a, int b) {
;   return a + b;
; }
; 
; __declspec(vector(nomask, uniform(b), uniform(c)))
; int control_divergence(int a, int b, int c) {
;   if (a > 50)
;     return b + c;
;   else
;     return b * c;
; }

; ModuleID = 'basic_divergence-opt.ll'
source_filename = "basic_divergence.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define x86_regcallcc <4 x i32> @_ZGVxN4vu_data_divergence(<4 x i32> %a, i32 %b) #0 {
entry:
  %vec.a = alloca <4 x i32>
  %vec.retval = alloca <4 x i32>
  %vec.a.cast = bitcast <4 x i32>* %vec.a to i32*
  %ret.cast = bitcast <4 x i32>* %vec.retval to i32*
  store <4 x i32> %a, <4 x i32>* %vec.a
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !12)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !13, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !14, i32 %b)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !15, <4 x i32> %a)
  call void @llvm.intel.directive(metadata !16)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %vec.a.cast.gep = getelementptr i32, i32* %vec.a.cast, i32 %index
  %vec.a.elem = load i32, i32* %vec.a.cast.gep
  %add = add nsw i32 %vec.a.elem, %b
  %ret.cast.gep = getelementptr i32, i32* %ret.cast, i32 %index
  store i32 %add, i32* %ret.cast.gep
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !17

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !19)
  call void @llvm.intel.directive(metadata !16)
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret.cast = bitcast i32* %ret.cast to <4 x i32>*
  %vec.ret = load <4 x i32>, <4 x i32>* %vec.ret.cast
  ret <4 x i32> %vec.ret
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #2

; Function Attrs: nounwind uwtable
define x86_regcallcc <4 x i32> @_ZGVxN4vuu_control_divergence(<4 x i32> %a, i32 %b, i32 %c) #1 {
entry:
  %vec.a = alloca <4 x i32>
  %vec.retval = alloca <4 x i32>
  %vec.a.cast = bitcast <4 x i32>* %vec.a to i32*
  %ret.cast = bitcast <4 x i32>* %vec.retval to i32*
  store <4 x i32> %a, <4 x i32>* %vec.a
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !12)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !13, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !14, i32 %b, i32 %c)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !15, <4 x i32> %a)
  call void @llvm.intel.directive(metadata !16)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %vec.a.cast.gep = getelementptr i32, i32* %vec.a.cast, i32 %index
  %vec.a.elem = load i32, i32* %vec.a.cast.gep
  %cmp = icmp sgt i32 %vec.a.elem, 50
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %simd.loop
  %add = add nsw i32 %b, %c
  br label %simd.loop.exit

if.else:                                          ; preds = %simd.loop
  %mul = mul nsw i32 %b, %c
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %if.else, %if.then
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !19)
  call void @llvm.intel.directive(metadata !16)
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret.cast = bitcast i32* %ret.cast to <4 x i32>*
  %vec.ret = load <4 x i32>, <4 x i32>* %vec.ret.cast
  ret <4 x i32> %vec.ret
}

attributes #0 = { nounwind uwtable "_ZGVxN4vu_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "_ZGVxN4vuu_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }

!llvm.ident = !{!11}

!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b"}
!3 = !{!"arg_step", i32 undef, i32 0}
!4 = !{!"arg_alig", i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"mask", i1 false}
!8 = !{!"arg_name", !"a", !"b", !"c"}
!9 = !{!"arg_step", i32 undef, i32 0, i32 0}
!10 = !{!"arg_alig", i32 undef, i32 undef, i32 undef}
!11 = !{!"clang version 3.9.0 (branches/vpo 12250)"}
!12 = !{!"DIR.OMP.SIMD"}
!13 = !{!"QUAL.OMP.SIMDLEN"}
!14 = !{!"QUAL.OMP.UNIFORM"}
!15 = !{!"QUAL.OMP.PRIVATE"}
!16 = !{!"DIR.QUAL.LIST.END"}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.unroll.disable"}
!19 = !{!"DIR.OMP.END.SIMD"}
!20 = distinct !{!20, !18}
