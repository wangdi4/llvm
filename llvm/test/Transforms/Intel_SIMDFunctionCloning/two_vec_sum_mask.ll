; Do a sanity check on the structure of the LLVM that SIMDFunctionCloning produces for the masked variant.

; RUN: opt -simd-function-cloning -S < %s | FileCheck %s

; Begin non-masked variant checking

; CHECK-LABEL: define __regcall <4 x i32> @_ZGVxM4vv_vec_sum(<4 x i32> %i, <4 x i32> %j, <4 x i32> %mask)
; CHECK-NEXT: entry:
; CHECK-NEXT: %vec_i.addr = alloca <4 x i32>
; CHECK-NEXT: %vec_j.addr = alloca <4 x i32>
; CHECK-NEXT: %vec_mask = alloca <4 x i32>
; CHECK-NEXT: %vec_retval = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> %i, <4 x i32>* %vec_i.addr, align 4
; CHECK-NEXT: store <4 x i32> %j, <4 x i32>* %vec_j.addr, align 4
; CHECK-NEXT: store <4 x i32> %mask, <4 x i32>* %vec_mask
; CHECK-NEXT: %veccast = bitcast <4 x i32>* %vec_retval to i32*
; CHECK-NEXT: %veccast.1 = bitcast <4 x i32>* %vec_i.addr to i32*
; CHECK-NEXT: %veccast.2 = bitcast <4 x i32>* %vec_j.addr to i32*
; CHECK-NEXT: %veccast.3 = bitcast <4 x i32>* %vec_mask to i32*
; CHECK-NEXT: br label %simd.begin.region

; CHECK: simd.begin.region:
; CHECK-NEXT: call void @llvm.intel.directive
; CHECK-NEXT: call void @llvm.intel.directive.qual.opnd
; CHECK-SAME: i32 4
; CHECK-NEXT: call void @llvm.intel.directive.qual
; CHECK-NEXT: br label %simd.loop

; CHECK: simd.loop:
; CHECK-NEXT: %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
; CHECK-NEXT: %maskgep = getelementptr i32, i32* %veccast.3, i32 %index
; CHECK-NEXT: %mask5 = load i32, i32* %maskgep
; CHECK-NEXT: %maskcond = icmp eq i32 %mask5, 1
; CHECK-NEXT: br i1 %maskcond, label %simd.loop.then, label %simd.loop.else

; CHECK: simd.loop.then:
; CHECK-NEXT: %vecgep = getelementptr i32, i32* %veccast.1, i32 %index
; CHECK-NEXT: %0 = load i32, i32* %vecgep, align 4
; CHECK-NEXT: %vecgep4 = getelementptr i32, i32* %veccast.2, i32 %index
; CHECK-NEXT: %1 = load i32, i32* %vecgep4, align 4
; CHECK-NEXT: %add = add nsw i32 %0, %1
; CHECK-NEXT: %vec_gep = getelementptr i32, i32* %veccast, i32 %index
; CHECK-NEXT: store i32 %add, i32* %vec_gep
; CHECK-NEXT: br label %simd.loop.exit

; CHECK: simd.loop.else:
; CHECK-NEXT: br label %simd.loop.exit

; CHECK: simd.loop.exit:
; CHECK-NEXT: %indvar = add nuw i32 1, %index
; CHECK-NEXT: %vlcond = icmp ult i32 %indvar, 4
; CHECK-NEXT: br i1 %vlcond, label %simd.loop, label %simd.end.region

; CHECK: simd.end.region:
; CHECK-NEXT: call void @llvm.intel.directive
; CHECK-NEXT: br label %return

; CHECK: return:
; CHECK-NEXT: %cast = bitcast i32* %veccast to <4 x i32>*
; CHECK-NEXT: %vec_ret = load <4 x i32>, <4 x i32>* %cast
; CHECK-NEXT: ret <4 x i32> %vec_ret

; ModuleID = 'two_vec_sum.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @vec_sum(i32 %i, i32 %j) #0 {
entry:
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %j, i32* %j.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %1 = load i32, i32* %j.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

attributes #0 = { nounwind uwtable "_ZGVxM4vv_" "_ZGVxN4vv_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
