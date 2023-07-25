; RUN: opt -passes="instcombine" < %s -S | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; Tests on INTEL_CUSTOMIZATION for reducing SExt to Zext transforms
; when the value is known to be non-negative.
;
; Rationale
; 32bit NSW/NUW on "loop-index" is important for vectorizer to determine
; whether the memory reference in the form of
;       A[sext(32bit_loop-index + ...)]
; can be unit-stride load/store or has to become gather/scatter.
;
; Once this becomes
;       A[zext(32bit_loop-index + ...)]
; later optimizer(s) try to widen the IV using trunc.
;       A[zext(trunc(64bit_loop-index + ...))]
; At this point, 32bit "no wrap" information is lost forever.
;
; Even if we manage to keep
;       A[zext(32bit_loop-index + ...)]
; we need to teach vectorizer that the value range is non-negative
; and thus zext is the same as sext (i.e., no wrap)
;
; Until such improvements are in place, we scale back SExt to ZExt xform.
;
; For now, "loop index" check is very loose. Just checking for phi or trunc.
; Phi check can be replaced by a real check for primary induction w.g., w/
; unit-increment used in the bottom test.
; Trunc check can be replaced by trunc of OpenCL get_global_id/get_local_id.
;
; On the consumption side, SExt=>GEP=>Load_or_Store in single use manner,
; w/ or w/o addrspacecast between GEP and load/store.

declare void @use(i32) readonly
declare i64 @get_global_id(i32) readonly

; Do not convert sext to zext. DPC++ like example.
define void @test1(ptr %p, i32 %x) {
; CHECK-LABEL: @test1
; CHECK:   sext
; CHECK-NOT: zext
  %addr_begin = getelementptr i32, ptr %p, i64 40
  %val_fixed = call i64 @get_global_id(i32 0), !range !0
  %trunc = trunc i64 %val_fixed to i32
  %add = add nsw nuw i32 %trunc, %x
  %sext = sext i32 %add to i64
  %addr = getelementptr i32, ptr %addr_begin, i64 %sext
  %val = load i32, ptr %addr
  call void @use(i32 %val)
  ret void
}

; Do not convert sext to zext. Loop index example.
define void @test2(ptr %p, i32 %x) {
; CHECK-LABEL: @test2
; CHECK:   sext
; CHECK-NOT: zext
bb0:
  %addr_begin = getelementptr i32, ptr %p, i64 40
  br label %bb1
bb1:
  %index = phi i32 [ zeroinitializer, %bb0 ], [ %inc, %bb1 ]
  %add = add nsw nuw i32 %index, %x
  %i = sext i32 %add to i64
  %addr = getelementptr i32, ptr %addr_begin, i64 %i
  %val = load i32, ptr %addr
  call void @use(i32 %val)
  %inc = add nsw nuw i32 %index, 1
  %cond = icmp ult i32 %inc, 100000
  br i1 %cond, label %bb1, label %bb2
bb2:
  ret void
}

; Test that following transform is inhibited:
;   from:     sext_i32_to_i64(trunc_i64_to_i32(X) + C)
;   to:       ((X << 32) + (C << 32)) >> 32
;
define void @test3(ptr %p, i32 %x) {
; CHECK-LABEL: @test3
; CHECK:        trunc
; CHECK-NEXT:   add
; CHECK-NEXT:   sext
; CHECK-NOT:    shl
; CHECK-NOT:    ashr
  %addr_begin = getelementptr i32, ptr %p, i64 40
  %val_fixed = call i64 @get_global_id(i32 0)
  %trunc = trunc i64 %val_fixed to i32
  %add = add i32 %trunc, 2
  %sext = sext i32 %add to i64
  %addr = getelementptr i32, ptr %addr_begin, i64 %sext
  %val = load i32, ptr %addr
  call void @use(i32 %val)
  ret void
}

;;  !range !0
!0 = !{i64 0, i64 2147483647}
