; This test checks to make sure that the load instruction correctly appears within the
; loop body (simd.loop) and is followed by a store to the widened alloca used as the
; return from the function.

; RUN: opt -vec-clone -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local global [4096 x i32] zeroinitializer, align 16

; CHECK:      define dso_local <4 x i32> @_ZGVbN4l_dowork(i32 [[K0:%.*]]) #0 {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_K0:%.*]] = alloca i32
; CHECK-NEXT:    store i32 [[K0]], i32* [[ALLOCA_K0]]
; CHECK-NEXT:    [[VEC_RETVAL0:%.*]] = alloca <4 x i32>
; CHECK-NEXT:    [[RET_CAST0:%.*]] = bitcast <4 x i32>* [[VEC_RETVAL0]] to i32*
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0:%.*]]

; CHECK:       simd.loop:
; CHECK-NEXT:    [[INDEX0:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER0:%.*]] ], [ [[INDVAR0:%.*]], [[SIMD_LOOP_EXIT0:%.*]] ]
; CHECK-NEXT:    [[IDXPROM0:%.*]] = sext i32 [[LOAD_K0:%.*]] to i64
; CHECK-NEXT:    [[STRIDE_MUL0:%.*]] = mul i32 1, [[INDEX0]]
; CHECK-NEXT:    [[STRIDE_CAST0:%.*]] = sext i32 [[STRIDE_MUL0]] to i64
; CHECK-NEXT:    [[STRIDE_ADD0:%.*]] = add i64 [[IDXPROM0]], [[STRIDE_CAST0]]
; CHECK-NEXT:    [[ARRAYIDX0:%.*]] = getelementptr inbounds [4096 x i32], [4096 x i32]* @a, i64 0, i64 [[STRIDE_ADD0]]
; CHECK-NEXT:    [[TMP0:%.*]] = load i32, i32* [[ARRAYIDX0]], align 4
; CHECK-NEXT:    [[RET_CAST_GEP0:%.*]] = getelementptr i32, i32* [[RET_CAST0]], i32 [[INDEX0]]
; CHECK-NEXT:    store i32 [[TMP0]], i32* [[RET_CAST_GEP0]]
; CHECK-NEXT:    br label [[SIMD_LOOP_EXIT0]]

; CHECK:       return:
; CHECK-NEXT:    [[VEC_RET_CAST0:%.*]] = bitcast i32* [[RET_CAST0]] to <4 x i32>*
; CHECK-NEXT:    [[VEC_RET0:%.*]] = load <4 x i32>, <4 x i32>* [[VEC_RET_CAST0]]
; CHECK-NEXT:    ret <4 x i32> [[VEC_RET0]]
; CHECK-NEXT: }

define dso_local i32 @dowork(i32 %k) #0 {
entry:
  %idxprom = sext i32 %k to i64
  %arrayidx = getelementptr inbounds [4096 x i32], [4096 x i32]* @a, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  ret i32 %0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4l_dowork" }
