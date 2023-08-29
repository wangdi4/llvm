; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; This series of tests ensures that InstCombine can transform things like:
;   %t = getelementptr ptr bitcast (ptr %arr to ptr), %scale
; into:
;   %t = getelementptr ptr %arr, i32 (%scale/4)
;
; The idea is to eliminate the unnecessary bitcasts by "descaling" %scale and
; changing the bitcast to a larger type. This should be possible in some cases
; even when scale is not a simple constant. For example, %scale itself may be a
; product or sum.

; This file contains tests similar to those in intel-gep-bitcast.ll, but with a
; different datalayout, ensuring that GEP indices are additionally truncated to
; 32-bit.
target datalayout = "p:32:32"

declare void @use(i32) readonly
declare void @use64(i64) readonly

; Verify that we can descale sums through casts. (This is more likely to happen
; with 32-bit pointers.)
; CHECK-LABEL: @test_descale_uniques_casts
; CHECK-DAG: getelementptr i32, ptr %src, i32
; CHECK-DAG: = trunc i64 %in to i32
; CHECK-NOT: bitcast
define void @test_descale_uniques_casts(i64 %in, ptr %src) {
  %mul = mul i64 128, %in
  %add = add i64 %mul, 64
  %trunc = trunc i64 %add to i32
  %gep = getelementptr inbounds i8, ptr %src, i32 %trunc
  %val = load i32, ptr %gep
  call void @use(i32 %val)
  call void @use64(i64 %mul)
  ret void
}

; Convert gep/bitcast when there are multiple uses of the truncated scale.
; (This module's datalayout will result in the GEP indices being truncated to
; i32.)
define void @test_multi_uses(ptr %in1, ptr %in2, i64 %in) {
; CHECK-LABEL: @test_multi_uses
; CHECK-DAG: getelementptr i32, ptr %in1, i32 [[TRUNC1:.*]]
; CHECK-DAG: getelementptr i32, ptr %in2, i32 [[TRUNC2:.*]]
; CHECK-DAG: [[TRUNC1]] = trunc i64 %in to i32
; CHECK-DAG: [[TRUNC2]] = trunc i64 %in to i32
; CHECK-NOT: bitcast
  %index = shl nuw nsw i64 %in, 2
  %gep1 = getelementptr inbounds i8, ptr %in1, i64 %index
  %gep2 = getelementptr inbounds i8, ptr %in2, i64 %index
  %val1 = load i32, ptr %gep1
  %val2 = load i32, ptr %gep2
  call void @use(i32 %val1)
  call void @use(i32 %val2)
  ret void
}

; Support descaling sums with multiple users. Here, descaling can only happen
; by looking through a multiplication and then realizing that both addends can
; be descaled. In CMPLRLLVM-11526 this was happening, but the sum had a second
; use which was incorrectly also being descaled. This tests that the gep use is
; descaled while the second use (by ret) is preserved.
; (Note that this module's datalayout will result in the GEP indices being
; truncated to i32.)
define i64 @test_descale_add2(ptr %src, i64 %in, i64 %cantdescale) {
; CHECK-LABEL: @test_descale_add2
; Ensure that we descaled:
; CHECK-NOT: bitcast

; Verify the descaled sum:
; CHECK-DAG: getelementptr i32, ptr %src, i32 [[TRUNC:.*]]
; CHECK-DAG: [[TRUNC]] = trunc i64 [[DESCALED:.*]] to i32
; CHECK-DAG: [[DESCALED]] = mul {{.*}} i64 [[NEWSUM:.*]], %cantdescale
; CHECK-DAG: [[NEWSUM]] = add {{.*}} i64 [[NEWSHIFT:.*]], 2
; CHECK-DAG: [[NEWSHIFT]] = shl {{.*}} i64 %in, 1

; Verify that a use of the original sum was not modified:
; CHECK-DAG: ret i64 [[USE2:.*]]
; CHECK-DAG: [[USE2]] = mul {{.*}} i64 [[SUM:.*]], %cantdescale
; CHECK-DAG: [[SUM]] = add {{.*}} i64 [[SHIFT:.*]], 8
; CHECK-DAG: [[SHIFT]] = shl {{.*}} i64 %in, 3
  %shift = shl nuw nsw i64 %in, 3
  %add = add nsw i64 %shift, 8
  %index = mul nuw nsw i64 %add, %cantdescale
  %gep = getelementptr inbounds i8, ptr %src, i64 %index
  %val = load i32, ptr %gep
  call void @use(i32 %val)
  ret i64 %index
}

