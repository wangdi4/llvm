; RUN: opt < %s -vec-clone -vpo-cfg-restructuring -avr-generate -analyze | FileCheck %s

;
; Check the correctness of generated Abstract Layer for masked function
;

;CHECK: Printing analysis 'AVR Generate' for function '_ZGVbM4vv_vec_sum':
;CHECK-NEXT: WRN

;CHECK: simd.begin.region:
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: call void @llvm.intel.directive.qual.opnd.i32
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: br label %simd.loop

;CHECK-NEXT: LOOP
;CHECK: simd.loop:
;CHECK-NEXT: %index = phi [0, simd.begin.region], [%indvar, simd.loop.exit]

;TEMP-DO-NOT-CHECK: br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else
;CHECK: simd.loop.then:
;CHECK-NEXT: %vec.a.cast.gep = %vec.a.cast getelementptr %index
;CHECK: br label %simd.loop.exit

;CHECK: simd.loop.else:
;CHECK-NEXT: br label %simd.loop.exit

;CHECK: simd.loop.exit:
;CHECK-NEXT: %indvar =  %index add 1
;CHECK: br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop

;CHECK: simd.end.region:
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: br label %return

;CHECK-NEXT: return:
;CHECK-NEXT: %vec.ret.cast = bitcast %ret.cast
;CHECK-NEXT: %vec.ret = load %vec.ret.cast
;CHECK-NEXT: ret <4 x i32> %vec.ret

; ModuleID = 'krtest.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @vec_sum(i32 %a, i32 %b) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32, i32* %a.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4vv_,_ZGVbN4vv_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32, i32)* @vec_sum, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b"}
!3 = !{!"arg_step", i32 undef, i32 undef}
!4 = !{!"arg_alig", i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.7.0 (branches/vpo 1169)"}




