; RUN: opt -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-complete-unroll-opt-level=3 -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that this highly profitable loopnest is unrolled in prevec pass.

; CHECK: Before HIR PreVec Complete Unroll

; CHECK: + DO i1 = 0, 35, 1   <DO_LOOP>
; CHECK: |   %3 = 2 * i1 + 1  +  18;
; CHECK: |   %4 = 0;
; CHECK: |
; CHECK: |   + DO i2 = 0, 17, 1   <DO_LOOP>
; CHECK: |   |   %7 = (@main_mp3.hybridIn)[0][%index1][i2];
; CHECK: |   |   %11 = 2 * i1 + 2 * %3 * i2 + 19  %  144;
; CHECK: |   |   %13 = (@COS_int)[0][%11];
; CHECK: |   |   %15 = (%13 * %7)  /  32768;
; CHECK: |   |   %4 = %15  +  %4;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %21 = (@imdct_win_int)[0][%index2][i1];
; CHECK: |   %23 = (%21 * %4)  /  32768;
; CHECK: |   (%ptr)[0][i1] = %23;
; CHECK: + END LOOP


; CHECK: After HIR PreVec Complete Unroll

; CHECK-NOT: DO i1


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

@imdct_win_int = internal global [4 x [36 x i32]] [[36 x i32] [i32 1429, i32 4277, i32 7092, i32 9853, i32 12539, i32 15130, i32 17606, i32 19947, i32 22137, i32 24159, i32 25996, i32 27636, i32 29065, i32 30273, i32 31251, i32 31991, i32 32487, i32 32736, i32 32736, i32 32487, i32 31991, i32 31251, i32 30273, i32 29065, i32 27636, i32 25996, i32 24159, i32 22137, i32 19947, i32 17606, i32 15130, i32 12539, i32 9853, i32 7092, i32 4277, i32 1429], [36 x i32] [i32 1429, i32 4277, i32 7092, i32 9853, i32 12539, i32 15130, i32 17606, i32 19947, i32 22137, i32 24159, i32 25996, i32 27636, i32 29065, i32 30273, i32 31251, i32 31991, i32 32487, i32 32736, i32 32768, i32 32768, i32 32768, i32 32768, i32 32768, i32 32768, i32 32487, i32 30273, i32 25996, i32 19947, i32 12539, i32 4277, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], [36 x i32] [i32 4277, i32 12539, i32 19947, i32 25996, i32 30273, i32 32487, i32 32487, i32 30273, i32 25996, i32 19947, i32 12539, i32 4277, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], [36 x i32] [i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 4277, i32 12539, i32 19947, i32 25996, i32 30273, i32 32487, i32 32768, i32 32768, i32 32768, i32 32768, i32 32768, i32 32768, i32 32736, i32 32487, i32 31991, i32 31251, i32 30273, i32 29065, i32 27636, i32 25996, i32 24159, i32 22137, i32 19947, i32 17606, i32 15130, i32 12539, i32 9853, i32 7092, i32 4277, i32 1429]], align 4

@COS_int = internal global [144 x i32] [i32 32768, i32 32736, i32 32643, i32 32487, i32 32270, i32 31991, i32 31651, i32 31251, i32 30791, i32 30273, i32 29697, i32 29065, i32 28377, i32 27636, i32 26841, i32 25996, i32 25101, i32 24159, i32 23170, i32 22137, i32 21062, i32 19947, i32 18794, i32 17606, i32 16384, i32 15130, i32 13848, i32 12539, i32 11207, i32 9853, i32 8480, i32 7092, i32 5690, i32 4277, i32 2855, i32 1429, i32 0, i32 -1429, i32 -2855, i32 -4277, i32 -5690, i32 -7092, i32 -8480, i32 -9853, i32 -11207, i32 -12539, i32 -13848, i32 -15130, i32 -16384, i32 -17606, i32 -18794, i32 -19947, i32 -21062, i32 -22137, i32 -23170, i32 -24159, i32 -25101, i32 -25996, i32 -26841, i32 -27636, i32 -28377, i32 -29065, i32 -29697, i32 -30273, i32 -30791, i32 -31251, i32 -31651, i32 -31991, i32 -32270, i32 -32487, i32 -32643, i32 -32736, i32 -32768, i32 -32736, i32 -32643, i32 -32487, i32 -32270, i32 -31991, i32 -31651, i32 -31251, i32 -30791, i32 -30273, i32 -29697, i32 -29065, i32 -28377, i32 -27636, i32 -26841, i32 -25996, i32 -25101, i32 -24159, i32 -23170, i32 -22137, i32 -21062, i32 -19947, i32 -18794, i32 -17606, i32 -16384, i32 -15130, i32 -13848, i32 -12539, i32 -11207, i32 -9853, i32 -8480, i32 -7092, i32 -5690, i32 -4277, i32 -2855, i32 -1429, i32 0, i32 1429, i32 2855, i32 4277, i32 5690, i32 7092, i32 8480, i32 9853, i32 11207, i32 12539, i32 13848, i32 15130, i32 16384, i32 17606, i32 18794, i32 19947, i32 21062, i32 22137, i32 23170, i32 24159, i32 25101, i32 25996, i32 26841, i32 27636, i32 28377, i32 29065, i32 29697, i32 30273, i32 30791, i32 31251, i32 31651, i32 31991, i32 32270, i32 32487, i32 32643, i32 32736], align 4

@main_mp3.hybridIn = internal global [32 x [18 x i32]] zeroinitializer, align 4

define void @foo(i32 %index1, i32 %index2, [36 x i32]* %ptr) {
entry:
 br label %for.loop

for.loop:                                   ; preds = %entry, %backedge
  %0 = phi i32 [ %25, %backedge ], [ 0, %entry ]
  %1 = shl nsw i32 %0, 1
  %2 = or i32 %1, 1
  %3 = add nuw nsw i32 %2, 18
  br label %for.inner.loop

for.inner.loop:                                   ; preds = %for.inner.loop, %for.loop
  %4 = phi i32 [ 0, %for.loop ], [ %16, %for.inner.loop ]
  %5 = phi i32 [ 0, %for.loop ], [ %17, %for.inner.loop ]
  %6 = getelementptr inbounds [32 x [18 x i32]], [32 x [18 x i32]]* @main_mp3.hybridIn, i32 0, i32 %index1, i32 %5
  %7 = load i32, i32* %6, align 4
  %8 = shl nsw i32 %5, 1
  %9 = or i32 %8, 1
  %10 = mul nuw nsw i32 %9, %3
  %11 = srem i32 %10, 144
  %12 = getelementptr inbounds [144 x i32], [144 x i32]* @COS_int, i32 0, i32 %11
  %13 = load i32, i32* %12, align 4
  %14 = mul nsw i32 %13, %7
  %15 = sdiv i32 %14, 32768
  %16 = add nsw i32 %15, %4
  %17 = add nuw nsw i32 %5, 1
  %18 = icmp eq i32 %17, 18
  br i1 %18, label %backedge, label %for.inner.loop

backedge:                                   ; preds = %for.inner.loop
  %19 = phi i32 [ %16, %for.inner.loop ]
  %20 = getelementptr inbounds [4 x [36 x i32]], [4 x [36 x i32]]* @imdct_win_int, i32 0, i32 %index2, i32 %0
  %21 = load i32, i32* %20, align 4
  %22 = mul nsw i32 %21, %19
  %23 = sdiv i32 %22, 32768
  %24 = getelementptr inbounds [36 x i32], [36 x i32]* %ptr, i32 0, i32 %0
  store i32 %23, i32* %24, align 4
  %25 = add nuw nsw i32 %0, 1
  %26 = icmp eq i32 %25, 36
  br i1 %26, label %for.exit, label %for.loop

for.exit:
  ret void
}
