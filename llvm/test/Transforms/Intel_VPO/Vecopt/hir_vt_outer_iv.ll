; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec' \
; RUN:     -debug-only=vplan-value-tracking \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external global [256 x i32], align 16

;CHECK:      BEGIN REGION { }
;CHECK-NEXT:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;CHECK-NEXT:       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;CHECK:            |   |   + DO i3 = 0, %n + -1, 1   <DO_LOOP>
;CHECK-NEXT:       |   |   |   (@A)[0][i2 + i3] = 0;
;CHECK-NEXT:       |   |   |   (@A)[0][16 * i2 + i3] = 0;
;CHECK-NEXT:       |   |   |   (@A)[0][i1 + i2 + i3] = 0;
;CHECK-NEXT:       |   |   + END LOOP
;CHECK:            |   + END LOOP
;CHECK-NEXT:       + END LOOP
;CHECK-NEXT: END REGION

define void @foo(i64 %n) local_unnamed_addr {

entry:
  br label %loop1.header

loop1.header:                                            ; preds = %outer.latch, %entry
  %loop1.iv = phi i64 [ 0, %entry ], [ %loop1.iv.next, %loop1.latch ]
  br label %loop2.header

loop2.header:                                            ; preds = %outer.latch, %entry
  %loop2.iv = phi i64 [ 0, %loop1.header ], [ %loop2.iv.next, %loop2.latch ]
  br label %DIR.OMP.SIMD

DIR.OMP.SIMD:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop3.body

loop3.body:                                            ; preds = %loop3.body, %loop2.header
  %loop3.iv = phi i64 [ 0, %DIR.OMP.SIMD ], [ %loop3.iv.next, %loop3.body ]

; CHECK: getKnownBits({(4 * i2 + @A),+,0})
; CHECK-NEXT: -> ??????????????????????????????????????????????????????????????00
  %idx = add nuw nsw i64 %loop2.iv, %loop3.iv
  %elem = getelementptr i32, ptr @A, i64 %idx
  store i32 0, ptr %elem, align 4

; CHECK: getKnownBits({(64 * i2 + @A),+,0})
; CHECK-NEXT: -> ????????????????????????????????????????????????????????????0000
  %loop2.iv.scaled = mul nuw nsw i64 %loop2.iv, 16
  %idx2 = add nuw nsw i64 %loop2.iv.scaled, %loop3.iv
  %elem2 = getelementptr i32, ptr @A, i64 %idx2
  store i32 0, ptr %elem2, align 4

; CHECK: getKnownBits({(4 * i1 + 4 * i2 + @A),+,0})
; CHECK-NEXT: -> ??????????????????????????????????????????????????????????????00
  %iv.sum = add nuw nsw i64 %loop1.iv, %loop2.iv
  %idx3 = add nuw nsw i64 %iv.sum, %loop3.iv
  %elem3 = getelementptr i32, ptr @A, i64 %idx3
  store i32 0, ptr %elem3, align 4

  %loop3.iv.next = add nuw nsw i64 %loop3.iv, 1
  %loop3.exitcond = icmp eq i64 %loop3.iv.next, %n
  br i1 %loop3.exitcond, label %DIR.OMP.END.SIMD, label %loop3.body

DIR.OMP.END.SIMD:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %loop2.latch

loop2.latch:
  %loop2.iv.next = add nuw nsw i64 %loop2.iv, 1
  %loop2.exitcond = icmp eq i64 %loop2.iv.next, %n
  br i1 %loop2.exitcond, label %loop1.latch, label %loop2.header

loop1.latch:
  %loop1.iv.next = add nuw nsw i64 %loop1.iv, 1
  %loop1.exitcond = icmp eq i64 %loop1.iv.next, %n
  br i1 %loop1.exitcond, label %exit, label %loop1.header

exit:                                            ; preds = %outer.latch
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
