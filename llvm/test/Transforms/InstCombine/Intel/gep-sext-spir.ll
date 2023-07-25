; RUN: opt -passes="instcombine" < %s -S | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

declare void @use(i32) readonly

;; Make sure not to perform SExt to ZExt xform for SPIR_KERNEL/SPIR_FUNC
;; See intel-gep-sext.ll on why we wouldn't want ZExt too early.

define spir_kernel void @test1(ptr %p, i32 %index) {
; CHECK-LABEL: @test1
; CHECK:   sext
; CHECK-NOT: zext
  %addr_begin = getelementptr i32, ptr %p, i64 40
  %addr_fixed = getelementptr i32, ptr %addr_begin, i64 48
  %val_fixed = load i32, ptr %addr_fixed, !range !0
  %addr = getelementptr i32, ptr %addr_begin, i32 %val_fixed
  %val = load i32, ptr %addr
  call void @use(i32 %val)
  ret void
}

define spir_func void @test2(ptr %p, i32 %index) {
; CHECK-LABEL: @test2
; CHECK:   sext
; CHECK-NOT: zext
  %addr_begin = getelementptr i32, ptr %p, i64 40
  %addr_fixed = getelementptr i32, ptr %addr_begin, i64 48
  %val_fixed = load i32, ptr %addr_fixed, !range !0
  %addr = getelementptr i32, ptr %addr_begin, i32 %val_fixed
  %val = load i32, ptr %addr
  call void @use(i32 %val)
  ret void
}


define spir_kernel void @test3(ptr %p) {
; CHECK-LABEL: @test3
; CHECK-NOT: zext
%addr_begin = getelementptr i32, ptr %p, i64 40
%addr_fixed = getelementptr i32, ptr %addr_begin, i64 48
%val_fixed = load i64, ptr %addr_fixed
%trunc = trunc i64 %val_fixed to i32
%add = add nuw nsw i32 %trunc, 1
%addr = getelementptr i32, ptr %addr_begin, i32 %add
%val = load i32, ptr %addr
call void @use(i32 %val)
ret void
}

define spir_func void @test4(ptr %p) {
; CHECK-LABEL: @test4
; CHECK-NOT: zext
%addr_begin = getelementptr i32, ptr %p, i64 40
%addr_fixed = getelementptr i32, ptr %addr_begin, i64 48
%val_fixed = load i64, ptr %addr_fixed
%trunc = trunc i64 %val_fixed to i32
%add = add nuw nsw i32 %trunc, 1
%addr = getelementptr i32, ptr %addr_begin, i32 %add
%val = load i32, ptr %addr
call void @use(i32 %val)
ret void
}

;;  !range !0
!0 = !{i32 0, i32 2147483647}
