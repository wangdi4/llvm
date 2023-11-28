; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa"  -disable-output < %s 2>&1 | FileCheck %s

; Currently, the delinearized group is not recognized as contiguous accesses.
; First time it sees a non-load size byte-distance between [4*i2+2] and [4*i2+4], contiguous access logic
; returns false.
; Then, runtimeDD for the innermost loop is disabled by the cost model.

; Delinearized refs:
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2] -> (%arg)[2 * i1][4 * i2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 1] -> (%arg)[2 * i1][4 * i2 + 1]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 2] -> (%arg)[2 * i1][4 * i2 + 2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 4] -> (%arg)[2 * i1][4 * i2 + 4]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2)] -> (%arg)[2 * i1 + 1][4 * i2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 1] -> (%arg)[2 * i1 + 1][4 * i2 + 1]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 2] -> (%arg)[2 * i1 + 1][4 * i2 + 2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 3] -> (%arg)[2 * i1 + 1][4 * i2 + 3]
;
; From
;    BEGIN REGION { }
;          + DO i1 = 0, 99, 1   <DO_LOOP>
;          |   %base = (%arg1)[0];
;          |
;          |   + DO i2 = 0, (sext.i32.i64(%arg3) + -4)/u4, 1   <DO_LOOP>
;          |   |   %load = (%base)[i2];
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2] = %load;;
;          |   |   %load10 = (%base)[i2];
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 1] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 2] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 4] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2)] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 1] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 2] = %load10;
;          |   |   (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 3] = %load10;
;          |   + END LOOP
;          + END LOOP
;    END REGION


; Delinearized refs:
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2] -> (%arg)[2 * i1][4 * i2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 1] -> (%arg)[2 * i1][4 * i2 + 1]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 2] -> (%arg)[2 * i1][4 * i2 + 2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + 4] -> (%arg)[2 * i1][4 * i2 + 4]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2)] -> (%arg)[2 * i1 + 1][4 * i2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 1] -> (%arg)[2 * i1 + 1][4 * i2 + 1]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 2] -> (%arg)[2 * i1 + 1][4 * i2 + 2]
; (%arg)[2 * sext.i32.i64(%arg2) * i1 + 4 * i2 + sext.i32.i64(%arg2) + 3] -> (%arg)[2 * i1 + 1][4 * i2 + 3]
;
; [RTDD] Delinearization done. Required pre-conditions: 2
; Group 0 contains (1) refs:
; (%arg1)[0]
; Group 1 contains (2) refs:
; (%base)[i2]
; (%base)[i2]
; Group 2 contains (8) refs:
; (%arg)[2 * i1][4 * i2]
; (%arg)[2 * i1][4 * i2 + 1]
; (%arg)[2 * i1][4 * i2 + 2]
; (%arg)[2 * i1][4 * i2 + 4]
; (%arg)[2 * i1 + 1][4 * i2]
; (%arg)[2 * i1 + 1][4 * i2 + 1]
; (%arg)[2 * i1 + 1][4 * i2 + 2]
; (%arg)[2 * i1 + 1][4 * i2 + 3]

; CHECK: DO i1
; CHECK-NOT: if
; CHECK-NOT: Delinearized
; CHECK: DO i2

; ModuleID = 'delinearize-contiguous-access.ll'
source_filename = "delinearize-contiguous-access.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @spam(ptr nocapture %arg, ptr nocapture readonly %arg1, i32 %arg2, i32 %arg3 ) {
bb:
  %icmp = icmp sgt i32 %arg3, 0
  br i1 %icmp, label %bb4, label %bb19

bb4:                                              ; preds = %bb
  %sext_pre = sext i32 %arg2 to i64
  %sext = mul nsw i64 2, %sext_pre
  %sext5 = sext i32 %arg3 to i64
  br label %bb6

bb6:                                              ; preds = %bb15, %bb4
  %phi = phi i64 [ 0, %bb4 ], [ %add16, %bb15 ]
  %mul = mul nsw i64 %phi, %sext
  %base = load ptr, ptr %arg1, align 4
  br label %bb7

bb7:                                              ; preds = %bb7, %bb6
  %phi8 = phi i64 [ 0, %bb6 ], [ %add13, %bb7 ]
  %phi0 = phi i64 [ 0, %bb6 ], [ %add0, %bb7 ]
  %getelementptr = getelementptr inbounds i32, ptr %base, i64 %phi0
  %load = load i32, ptr %getelementptr, align 4
  %add = add nsw i64 %phi8, %mul
  %getelementptr9 = getelementptr inbounds i32, ptr %arg, i64 %add
  store i32 %load, ptr %getelementptr9, align 4
  %load10 = load i32, ptr %getelementptr, align 4
  %add11 = add nsw i64 %add, 1
  %getelementptr12 = getelementptr inbounds i32, ptr %arg, i64 %add11
  store i32 %load10, ptr %getelementptr12, align 4

  %add12 = add nsw i64 %add, 2
  %getelementptr13 = getelementptr inbounds i32, ptr %arg, i64 %add12
  store i32 %load10, ptr %getelementptr13, align 4

  %add14 = add nsw i64 %add, 4
  %getelementptr14 = getelementptr inbounds i32, ptr %arg, i64 %add14
  store i32 %load10, ptr %getelementptr14, align 4

  %add111 = add nsw i64 %add, %sext_pre
  %getelementptr121 = getelementptr inbounds i32, ptr %arg, i64 %add111
  store i32 %load10, ptr %getelementptr121, align 4

  %add121 = add nsw i64 %add111, 1
  %getelementptr131 = getelementptr inbounds i32, ptr %arg, i64 %add121
  store i32 %load10, ptr %getelementptr131, align 4

  %add141 = add nsw i64 %add111, 2
  %getelementptr141 = getelementptr inbounds i32, ptr %arg, i64 %add141
  store i32 %load10, ptr %getelementptr141, align 4

  %add151 = add nsw i64 %add111, 3
  %getelementptr151 = getelementptr inbounds i32, ptr %arg, i64 %add151
  store i32 %load10, ptr %getelementptr151, align 4

  %add0 = add nuw nsw i64 %phi0, 1
  %add13 = add nuw nsw i64 %phi8, 4
  %icmp14 = icmp eq i64 %add13, %sext5
  br i1 %icmp14, label %bb15, label %bb7

bb15:                                             ; preds = %bb7
  %add16 = add nuw nsw i64 %phi, 1
  %icmp17 = icmp eq i64 %add16, 100
  br i1 %icmp17, label %bb18, label %bb6

bb18:                                             ; preds = %bb15
  br label %bb19

bb19:                                             ; preds = %bb18, %bb
  ret void
}
