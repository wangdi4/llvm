; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>" -hir-details -disable-output  2>&1 | FileCheck %s

; Verify that %K is parsed as a self-blob in the copy %K = %indvars.iv.next3265,
; The underlying llvm inst is this livein copy-
; %K.in2 = call i32 @llvm.ssa.copy.i32(i32 %0), !in.de.ssa

; Previously, it was being defined in terms of %0 (which is now optimized away)
; resulting in a blob mismatch between lval and rval. This mismatch was causing
; missing loop liveout assertion in a more complicated case as %0 was not marked
; as liveout. 

; Previous parsing for the copy inst looked like this-
;   %"module_ra_cam_mp_radclwmx_$K2.1" = %indvars.iv.next3265;
;   <LVAL-REG> NON-LINEAR i32 %0
;   <BLOB> NON-LINEAR i32 %0
;   <RVAL-REG> NON-LINEAR trunc.i64.i32(%indvars.iv.next3265)

; The mismatch happens because the rval of the copy %0 is a cast inst. There is
; some special logic in parser to try to parse the cast inst's operand by moving
; the cast directly into the src/dest type of the CE. This logic doesn't kick in
; for the lval (%K).

; Technically, lval of a copy inst can always use the same CE as the rval but this
; is left as a TODO for now. 

; CHECK: + DO i64 i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   + DO i64 i2 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   if (undef true undef)
; CHECK: |   |   {
; CHECK: |   |      goto bb_new2911_then;
; CHECK: |   |   }
; CHECK: |   |   %indvars.iv.next3265 = i2  +  1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %K = %indvars.iv.next3265;

; CHECK-NEXT: |   <LVAL-REG> NON-LINEAR i32 %K
; CHECK-NEXT: |   <RVAL-REG> NON-LINEAR trunc.i64.i32(%indvars.iv.next3265)

; CHECK: |   goto bb857;
; CHECK: |   bb_new2911_then:
; CHECK: |   %K = 0;
; CHECK: |   bb857:
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @module_ra_cam_mp_radclwmx_() {
alloca_13:
  br label %bb987

bb987:                                            ; preds = %bb1004_endif.thread, %alloca_13
  br label %bb991

bb991:                                            ; preds = %bb995_else, %bb987
  %indvars.iv3264 = phi i64 [ 0, %bb987 ], [ %indvars.iv.next3265, %bb995_else ]
  br i1 false, label %bb995_else, label %bb_new2911_then

bb_new2911_then:                                  ; preds = %bb991
  br label %bb857

bb995_else:                                       ; preds = %bb991
  %indvars.iv.next3265 = add i64 %indvars.iv3264, 1
  %rel.1103.not.not = icmp sgt i64 0, 0
  br i1 %rel.1103.not.not, label %bb991, label %bb857.loopexit

bb857.loopexit:                                   ; preds = %bb995_else
  %indvars.iv.next3265.lcssa = phi i64 [ %indvars.iv.next3265, %bb995_else ]
  %0 = trunc i64 %indvars.iv.next3265.lcssa to i32
  br label %bb857

bb857:                                            ; preds = %bb857.loopexit, %bb_new2911_then
  %K = phi i32 [ 0, %bb_new2911_then ], [ %0, %bb857.loopexit ]
  br label %bb1004_endif.thread

bb997.preheader:                                  ; No predecessors!
  %1 = sext i32 %K to i64
  br label %bb1004_endif.thread

bb1004_endif.thread:                              ; preds = %bb997.preheader, %bb857
  %rel.1117.not = icmp ugt i64 1, 0
  br i1 %rel.1117.not, label %bb1024.preheader, label %bb987

bb1024.preheader:                                 ; preds = %bb1004_endif.thread
  br label %bb1085

bb1085:                                           ; preds = %bb1085, %bb1024.preheader
  br label %bb1085
}
