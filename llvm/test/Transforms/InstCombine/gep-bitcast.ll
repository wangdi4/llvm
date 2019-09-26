; RUN: opt < %s -instcombine -S | FileCheck %s
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

