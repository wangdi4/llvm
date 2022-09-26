; Test to check correctness of verification done for VPBlobs that are
; created upon decomposition of external defs from incoming HIR.

; Input HIR
;   + DO i64 i1 = 0, 3, 1   <DO_LOOP> <vectorize> <ivdep>
;   |   %0 = (%pix1)[sext.i32.i64(%i_pix1) * i1];
;   |   <LVAL-REG> NON-LINEAR i8 %0 {sb:8}
;   |   <RVAL-REG> {al:1}(LINEAR i8* %pix1)[LINEAR i64 sext.i32.i64(%i_pix1) * i1] inbounds  {sb:30}
;   |      <BLOB> LINEAR i32 %i_pix1 {sb:6}
;   |      <BLOB> LINEAR i8* %pix1 {sb:7}
;   |
;   |   %1 = (%pix2)[sext.i32.i64(%i_pix2) * i1];
;   |   <LVAL-REG> NON-LINEAR i8 %1 {sb:12}
;   |   <RVAL-REG> {al:1}(LINEAR i8* %pix2)[LINEAR i64 sext.i32.i64(%i_pix2) * i1] inbounds  {sb:30}
;   |      <BLOB> LINEAR i32 %i_pix2 {sb:10}
;   |      <BLOB> LINEAR i8* %pix2 {sb:11}
;   |
;   |   %2 = (%pix1)[sext.i32.i64(%i_pix1) * i1 + 4];
;   |   <LVAL-REG> NON-LINEAR i8 %2 {sb:16}
;   |   <RVAL-REG> {al:1}(LINEAR i8* %pix1)[LINEAR i64 sext.i32.i64(%i_pix1) * i1 + 4] inbounds  {sb:30}
;   |      <BLOB> LINEAR i32 %i_pix1 {sb:6}
;   |      <BLOB> LINEAR i8* %pix1 {sb:7}
;   |
;   |   %3 = (%pix2)[sext.i32.i64(%i_pix2) * i1 + 4];
;   |   <LVAL-REG> NON-LINEAR i8 %3 {sb:19}
;   |   <RVAL-REG> {al:1}(LINEAR i8* %pix2)[LINEAR i64 sext.i32.i64(%i_pix2) * i1 + 4] inbounds  {sb:30}
;   |      <BLOB> LINEAR i32 %i_pix2 {sb:10}
;   |      <BLOB> LINEAR i8* %pix2 {sb:11}
;   |
;   |   (%result)[i1] = zext.i8.i32(%0) + 65536 * zext.i8.i32(%2) + -65536 * zext.i8.i32(%3) + -1 * zext.i8.i32(%1);
;   |   <LVAL-REG> {al:4}(LINEAR i32* %result)[LINEAR i64 i1] inbounds  {sb:30}
;   |      <BLOB> LINEAR i32* %result {sb:25}
;   |   <RVAL-REG> NON-LINEAR i32 zext.i8.i32(%0) + 65536 * zext.i8.i32(%2) + -65536 * zext.i8.i32(%3) + -1 * zext.i8.i32(%1) {sb:2}
;   |      <BLOB> NON-LINEAR i8 %1 {sb:12}
;   |      <BLOB> NON-LINEAR i8 %3 {sb:19}
;   |      <BLOB> NON-LINEAR i8 %0 {sb:8}
;   |      <BLOB> NON-LINEAR i8 %2 {sb:16}
;   |
;   + END LOOP

; For the above input, VPlan HIR decomposer creates following 2 VPExternalDefs
; (as VPBlobs) of interest -
; %vp.ext.1 = {(sext i32 %i_pix2 to i64)}
; %vp.ext.2 = {(sext i32 %i_pix1 to i64)}

; These 2 VPBlobs are attached to the following HIR DDRefs -
; {al:1}(LINEAR i8* %pix1)[LINEAR i64 sext.i32.i64(%i_pix1) * i1] inbounds  {sb:30}
; {al:1}(LINEAR i8* %pix2)[LINEAR i64 sext.i32.i64(%i_pix2) * i1] inbounds  {sb:30}

; These DDRefs have the same Symbase value even though they are structurally not equal
; (since the arrays are not proven as non-overlapping). VPlan verifier should account
; for this while checking for unique entries in VPExternalDefsHIR.


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -S -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg" -S -vplan-force-vf=4  < %s 2>&1 | FileCheck %s


; Check for v4i8 gather to present as an sufficient check for successful vectorization.
; CHECK:      llvm.masked.gather.v4i8.v4p0i8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @verify_vpblobs(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2, i32* nocapture %result) local_unnamed_addr {
entry:
  %idx.ext = sext i32 %i_pix1 to i64
  %idx.ext9 = sext i32 %i_pix2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %pix1.addr.023 = phi i8* [ %pix1, %entry ], [ %add.ptr, %for.body ]
  %pix2.addr.022 = phi i8* [ %pix2, %entry ], [ %add.ptr10, %for.body ]
  %0 = load i8, i8* %pix1.addr.023, align 1
  %conv = zext i8 %0 to i32
  %1 = load i8, i8* %pix2.addr.022, align 1
  %conv2 = zext i8 %1 to i32
  %sub = sub nsw i32 %conv, %conv2
  %ptridx3 = getelementptr inbounds i8, i8* %pix1.addr.023, i64 4
  %2 = load i8, i8* %ptridx3, align 1
  %conv4 = zext i8 %2 to i32
  %ptridx5 = getelementptr inbounds i8, i8* %pix2.addr.022, i64 4
  %3 = load i8, i8* %ptridx5, align 1
  %conv6 = zext i8 %3 to i32
  %sub7 = sub nsw i32 %conv4, %conv6
  %shl = shl nsw i32 %sub7, 16
  %add = add nsw i32 %sub, %shl
  %ptridx8 = getelementptr inbounds i32, i32* %result, i64 %indvars.iv
  store i32 %add, i32* %ptridx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add.ptr = getelementptr inbounds i8, i8* %pix1.addr.023, i64 %idx.ext
  %add.ptr10 = getelementptr inbounds i8, i8* %pix2.addr.022, i64 %idx.ext9
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

!7 = distinct !{!7, !8, !9, !10}
!8 = !{!"llvm.loop.vectorize.ivdep_back"}
!9 = !{!"llvm.loop.vectorize.ignore_profitability"}
!10 = !{!"llvm.loop.vectorize.enable", i1 true}
