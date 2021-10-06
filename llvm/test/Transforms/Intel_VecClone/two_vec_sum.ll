; Do a sanity check on the structure of the LLVM that VecClone produces for the non-masked variant.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; Begin non-masked variant checking

; CHECK-LABEL: <4 x i32> @_ZGVbN4vv_vec_sum(<4 x i32> %i, <4 x i32> %j)
; CHECK-NEXT: entry:
; CHECK-NEXT: %vec.i = alloca <4 x i32>
; CHECK-NEXT: %vec.j = alloca <4 x i32>
; CHECK-NEXT: %vec.retval = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> %i, <4 x i32>* %vec.i
; CHECK-NEXT: store <4 x i32> %j, <4 x i32>* %vec.j
; CHECK-NEXT: %vec.i.cast = bitcast <4 x i32>* %vec.i to i32*
; CHECK-NEXT: %vec.j.cast = bitcast <4 x i32>* %vec.j to i32*
; CHECK-NEXT: %ret.cast = bitcast <4 x i32>* %vec.retval to i32*
; CHECK-NEXT: br label %simd.begin.region

; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; CHECK-NEXT: br label %simd.loop

; CHECK: simd.loop:
; CHECK-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; CHECK-NEXT: %vec.i.cast.gep = getelementptr i32, i32* %vec.i.cast, i32 %index
; CHECK-NEXT: %0 = load i32, i32* %vec.i.cast.gep, align 4
; CHECK-NEXT: %vec.j.cast.gep = getelementptr i32, i32* %vec.j.cast, i32 %index
; CHECK-NEXT: %1 = load i32, i32* %vec.j.cast.gep, align 4
; CHECK-NEXT: %add = add nsw i32 %0, %1
; CHECK-NEXT: %ret.cast.gep = getelementptr i32, i32* %ret.cast, i32 %index
; CHECK-NEXT: store i32 %add, i32* %ret.cast.gep
; CHECK-NEXT: br label %simd.loop.exit

; CHECK: simd.loop.exit:
; CHECK-NEXT: %indvar = add nuw i32 %index, 1
; CHECK-NEXT: %vl.cond = icmp ult i32 %indvar, 4
; CHECK-NEXT: br i1 %vl.cond, label %simd.loop, label %simd.end.region

; CHECK: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)
; CHECK-SAME: DIR.OMP.END.SIMD
; CHECK-NEXT: br label %return

; CHECK: return:
; CHECK-NEXT: %vec.ret.cast = bitcast i32* %ret.cast to <4 x i32>*
; CHECK-NEXT: %vec.ret = load <4 x i32>, <4 x i32>* %vec.ret.cast
; CHECK-NEXT: ret <4 x i32> %vec.ret

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

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4vv_,_ZGVbN4vv_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
