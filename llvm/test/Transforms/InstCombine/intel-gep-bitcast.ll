; RUN: opt < %s -instcombine -S | FileCheck %s

; This series of tests ensures that InstCombine can transform things like:
;   %t = getelementptr i8* bitcast (i32* %arr to i8*), %scale
; into:
;   %t = getelementptr i32* %arr, i32 (%scale/4)
;
; The idea is to eliminate the unnecessary bitcasts by "descaling" %scale and
; changing the bitcast to a larger type. This should be possible in some cases
; even when scale is not a simple constant. For example, %scale itself may be a
; product or sum.

declare void @use(i32) readonly
declare void @use64(i64) readonly

; Convert gep/bitcast when there are multiple uses of the scale.
define void @test_multi_uses(i32* %in1, i32* %in2, i64 %in) {
; CHECK-LABEL: @test_multi_uses
; CHECK-NEXT: getelementptr inbounds i32, i32* %in1, i64 %in
; CHECK-NEXT: getelementptr inbounds i32, i32* %in2, i64 %in
; CHECK-NOT: bitcast
  %index = shl nuw nsw i64 %in, 2
  %bc1 = bitcast i32* %in1 to i8*
  %bc2 = bitcast i32* %in2 to i8*
  %gep1 = getelementptr inbounds i8, i8* %bc1, i64 %index
  %gep2 = getelementptr inbounds i8, i8* %bc2, i64 %index
  %gep1.bc = bitcast i8* %gep1 to i32*
  %gep2.bc = bitcast i8* %gep2 to i32*
  %val1 = load i32, i32* %gep1.bc
  %val2 = load i32, i32* %gep2.bc
  call void @use(i32 %val1)
  call void @use(i32 %val2)
  ret void
}

; Support walking through a neg for scale
define void @test_scale_neg(i32* %src, i64 %in) {
; CHECK-LABEL: @test_scale_neg
; INTEL_CUSTOMIZATION
; CHECK-NEXT: %index = sub i64 0, %in
; CHECK-NEXT: getelementptr i32, i32* %src, i64 %index
; end INTEL_CUSTOMIZATION
; CHECK-NOT: bitcast
  %shift = shl nuw nsw i64 %in, 2
  %index = sub i64 0, %shift
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  ret void
}

; Support descaling sums
define void @test_descale_add(i32* %src, i64 %in) {
; CHECK-LABEL: @test_descale_add
; CHECK-NEXT: %index1 = add nsw i64 %in, 2
; CHECK-NEXT: getelementptr i32, i32* %src, i64 %index1
; CHECK-NOT: bitcast
  %shift = shl nuw nsw i64 %in, 2
  %index = add nsw i64 %shift, 8
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  ret void
}

; Support descaling differences
define void @test_descale_sub(i32* %src, i64 %in) {
; CHECK-LABEL: @test_descale_sub
; CHECK-NEXT: %index1 = add nsw i64 %in, -4
; CHECK-NEXT: getelementptr i32, i32* %src, i64 %index1
; CHECK-NOT: bitcast
  %shift = shl nuw nsw i64 %in, 2
  %index = sub nsw i64 %shift, 16
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  ret void
}

; Support descaling sums implemented as "or"
define void @test_descale_or(i32* %src, i64 %in) {
; CHECK-LABEL: @test_descale_or
; CHECK-NEXT: %chop = lshr i64 %in, 63
; CHECK-NEXT: %index1 = or i64 %chop, 2
; CHECK-NEXT: getelementptr i32, i32* %src, i64 %index1
; CHECK-NOT: bitcast
  %chop  = lshr i64 %in, 63
  %shift = shl nuw nsw i64 %chop, 2
  %index = or i64 %shift, 8
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  call void @use64(i64 %chop)
  ret void
}

; Support descaling sums with multiple users. Here, descaling ; can only happen
; by looking through a multiplication and then realizing that both addends can
; be descaled. In CMPLRLLVM-11526 this was happening, but the sum had a second
; use which was incorrectly also being descaled. This tests that the gep use is
; descaled while the second use (by ret) is preserved.
define i64 @test_descale_add2(i32* %src, i64 %in, i64 %cantdescale) {
; CHECK-LABEL: @test_descale_add2
; Ensure that we descaled:
; CHECK-NOT: bitcast

; Verify the descaled sum:
; CHECK-DAG: getelementptr i32, i32* %src, i64 [[DESCALED:.*]]
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
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  ret i64 %index
}

; Verify that we don't descale sums without "nsw"
define void @test_dont_descale_wrapping_add(i32* %src, i64 %in) {
; CHECK-LABEL: @test_dont_descale_wrapping_add
; CHECK-NEXT: %shift = shl nuw nsw i64 %in, 2
; CHECK-NEXT: %index = add i64 %shift, 8
; CHECK-NEXT: %bc = bitcast i32* %src to i8*
; CHECK-NEXT: getelementptr inbounds i8, i8* %bc, i64 %index
; CHECK-NEXT: bitcast i8*
  %shift = shl nuw nsw i64 %in, 2
  %index = add i64 %shift, 8
  %bc = bitcast i32* %src to i8*
  %gep = getelementptr inbounds i8, i8* %bc, i64 %index
  %gep.bc = bitcast i8* %gep to i32*
  %val = load i32, i32* %gep.bc
  call void @use(i32 %val)
  ret void
}

; Don't run into an infinite loop if InstCombine::Descale fails.
define void @test_descale_failed(i32* %in1, i32* %in2, i64 %in, i64 %add) {
; CHECK-LABEL: @test_descale_failed
; CHECK: bitcast
; CHECK: getelementptr
  %index = mul nuw nsw i64 %in, 3
  %neg = mul nuw nsw i64 %index, %add
  %bc1 = bitcast i32* %in1 to i8*
  %bc2 = bitcast i32* %in2 to i8*
  %gep1 = getelementptr inbounds i8, i8* %bc1, i64 %neg
  %gep2 = getelementptr inbounds i8, i8* %bc2, i64 %neg
  %gep1.bc = bitcast i8* %gep1 to i32*
  %gep2.bc = bitcast i8* %gep2 to i32*
  %val1 = load i32, i32* %gep1.bc
  %val2 = load i32, i32* %gep2.bc
  call void @use(i32 %val1)
  call void @use(i32 %val2)
  ret void
}

