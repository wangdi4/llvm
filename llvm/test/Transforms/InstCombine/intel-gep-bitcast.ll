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
; CHECK-NEXT: %index = sub i64 0, %in
; CHECK-NEXT: getelementptr i32, i32* %src, i64 %index
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
  ret void
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

