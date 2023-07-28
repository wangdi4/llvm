; RUN: opt -intel-libirc-allowed -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>" -disable-hir-cross-loop-array-contraction-post-processing=true -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s


; Verify that contraction does not occur between loops when unsafe call is present (stackrestore).
; %"shell_$B22" would otherwise be legal to contract.

; *** IR Dump Before HIR Cross-Loop Array Contraction (hir-cross-loop-array-contraction) ***
; Function: shell_
;  BEGIN REGION { }
;        if (%"shell_$NZL_fetch.106" < 1)
;        {
;           @llvm.stackrestore(&((%savedstack)[0]));
;        }
;        else
;        {
;           %"s2_$T1_fetch.22.i" = (%"shell_$T1")[0];
;           + DO i1 = 0, zext.i32.i64(%"shell_$NZL_fetch.106") + -1, 1   <DO_LOOP>
;           |      %"(double)s2_$K_fetch.23$.i" = sitofp.i32.double(i1 + 1);
;           |      %sub.1.i = %"s2_$T1_fetch.22.i"  -  %"(double)s2_$K_fetch.23$.i";
;           |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch.105") + -1, 1   <DO_LOOP>
;           |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch.104") + -1, 1   <DO_LOOP>
;           |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   |   (%"s2_$TMP6.i")[i1][i2][i3][i4][i5] = %sub.1.i;
;           |   |   |   |   + END LOOP
;           |   |   |   + END LOOP
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;           + DO i1 = 0, zext.i32.i64(%"shell_$NZL_fetch.106") + -1, 1   <DO_LOOP>
;           |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch.105") + -1, 1   <DO_LOOP>
;           |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch.104") + -1, 1   <DO_LOOP>
;           |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   |   %"s2_$TMP6[][][][][]_fetch.65.i" = (%"s2_$TMP6.i")[i1][i2][i3][i5][i4];
;           |   |   |   |   |   %add.10.i = %"s2_$TMP6[][][][][]_fetch.65.i"  +  %"s2_$T1_fetch.22.i";
;           |   |   |   |   |   (%"shell_$B22")[i1][i2][i3][i5][i4] = %add.10.i;
;           |   |   |   |   + END LOOP
;           |   |   |   + END LOOP
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;
;           @llvm.stackrestore(&((%savedstack)[0]));
;
;           + DO i1 = 0, zext.i32.i64(%"shell_$NZL_fetch.106") + -1, 1   <DO_LOOP>
;           |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch.105") + -1, 1   <DO_LOOP>
;           |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch.104") + -1, 1   <DO_LOOP>
;           |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;           |   |   |   |   |   %"shell_$B22[][][][][]_fetch.126" = (%"shell_$B22")[i1][i2][i3][i4][i5];
;           |   |   |   |   |   (%"shell_$A")[i1][i2][i3][i4][i5] = %"shell_$B22[][][][][]_fetch.126";
;           |   |   |   |   + END LOOP
;           |   |   |   + END LOOP
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;        }
;        ret ;
;  END REGION

; *** IR Dump After HIR Cross-Loop Array Contraction (hir-cross-loop-array-contraction) ***
; CHECK:  Function: shell_
; CHECK:  BEGIN REGION { modified }
;         if (%"shell_$NZL_fetch.106" < 1)
;         {
;            @llvm.stackrestore(&((%savedstack)[0]));
;         }
;         else
;         {
;            %"s2_$T1_fetch.22.i" = (%"shell_$T1")[0];
;            + DO i1 = 0, zext.i32.i64(%"shell_$NZL_fetch.106") + -1, 1   <DO_LOOP>
;            |      %temp = sitofp.i32.double(i1 + 1);
;            |      %temp16 = %"s2_$T1_fetch.22.i"  -  %temp;
;            |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch.105") + -1, 1   <DO_LOOP>
;            |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch.104") + -1, 1   <DO_LOOP>
;            |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;            |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;            |   |   |   |   |   (%ContractedArray)[0][i4][i5] = %temp16;
;            |   |   |   |   + END LOOP
;            |   |   |   + END LOOP
;            |   |   |
;            |   |   |
;            |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;            |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;            |   |   |   |   |   %"s2_$TMP6[][][][][]_fetch.65.i" = (%ContractedArray)[0][i5][i4];
;            |   |   |   |   |   %add.10.i = %"s2_$TMP6[][][][][]_fetch.65.i"  +  %"s2_$T1_fetch.22.i";
;            |   |   |   |   |   (%"shell_$B22")[i1][i2][i3][i5][i4] = %add.10.i;
;            |   |   |   |   + END LOOP
;            |   |   |   + END LOOP
;            |   |   + END LOOP
;            |   + END LOOP
;            + END LOOP
;
;            @llvm.stackrestore(&((%savedstack)[0]));
;
; CHECK:     + DO i1 = 0, zext.i32.i64(%"shell_$NZL_fetch.106") + -1, 1   <DO_LOOP>
; CHECK:     |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch.105") + -1, 1   <DO_LOOP>
; CHECK:     |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch.104") + -1, 1   <DO_LOOP>
; CHECK:     |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
; CHECK:     |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
; CHECK:     |   |   |   |   |   %"shell_$B22[][][][][]_fetch.126" = (%"shell_$B22")[i1][i2][i3][i4][i5];
; CHECK:     |   |   |   |   |   (%"shell_$A")[i1][i2][i3][i4][i5] = %"shell_$B22[][][][][]_fetch.126";
; CHECK:     |   |   |   |   + END LOOP
; CHECK:     |   |   |   + END LOOP
; CHECK:     |   |   + END LOOP
; CHECK:     |   + END LOOP
; CHECK:     + END LOOP
;         }
;         ret ;
;   END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @s2_(ptr noalias nocapture readonly dereferenceable(4) %"s2_$NX", ptr noalias nocapture readonly dereferenceable(4) %"s2_$NY", ptr noalias nocapture readnone %"s2_$NZ", ptr noalias nocapture readonly dereferenceable(4) %"s2_$NZL", ptr noalias nocapture dereferenceable(8) %"s2_$B", ptr noalias nocapture readonly dereferenceable(8) %"s2_$T1") local_unnamed_addr #0 {
alloca_0:
  %"s2_$NX_fetch.13" = load i32, ptr %"s2_$NX", align 1
  %"s2_$NY_fetch.14" = load i32, ptr %"s2_$NY", align 1
  %"s2_$NZL_fetch.15" = load i32, ptr %"s2_$NZL", align 1
  %int_sext = sext i32 %"s2_$NX_fetch.13" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %mul.3 = mul nuw nsw i64 %slct.1, 200
  %int_sext2 = sext i32 %"s2_$NY_fetch.14" to i64
  %rel.2 = icmp sgt i64 %int_sext2, 0
  %slct.2 = select i1 %rel.2, i64 %int_sext2, i64 0
  %mul.4 = mul nsw i64 %mul.3, %slct.2
  %int_sext4 = sext i32 %"s2_$NZL_fetch.15" to i64
  %rel.3 = icmp sgt i64 %int_sext4, 0
  %slct.3 = select i1 %rel.3, i64 %int_sext4, i64 0
  %mul.5 = mul nsw i64 %mul.4, %slct.3
  %div.1 = lshr exact i64 %mul.5, 3
  %"s2_$TMP6" = alloca double, i64 %div.1, align 8
  %mul.25 = mul nsw i64 %int_sext, 200
  %mul.26 = mul nsw i64 %mul.25, %int_sext2
  %rel.13 = icmp slt i32 %"s2_$NZL_fetch.15", 1
  br i1 %rel.13, label %bb22, label %bb1.preheader

bb1.preheader:                                    ; preds = %alloca_0
  %rel.14 = icmp slt i32 %"s2_$NY_fetch.14", 1
  %rel.15 = icmp slt i32 %"s2_$NX_fetch.13", 1
  %"s2_$T1_fetch.22" = load double, ptr %"s2_$T1", align 1
  %0 = add nuw nsw i32 %"s2_$NX_fetch.13", 1
  %1 = add nuw nsw i32 %"s2_$NY_fetch.14", 1
  %2 = add nuw nsw i32 %"s2_$NZL_fetch.15", 1
  %wide.trip.count148150 = zext i32 %2 to i64
  %wide.trip.count144 = sext i32 %1 to i64
  %wide.trip.count140 = sext i32 %0 to i64
  br label %bb1

bb1:                                              ; preds = %bb1.preheader, %bb6
  %indvars.iv146 = phi i64 [ 1, %bb1.preheader ], [ %indvars.iv.next147, %bb6 ]
  br i1 %rel.14, label %bb6, label %bb5.preheader

bb5.preheader:                                    ; preds = %bb1
  %3 = trunc i64 %indvars.iv146 to i32
  %"(double)s2_$K_fetch.23$" = sitofp i32 %3 to double
  %sub.1 = fsub fast double %"s2_$T1_fetch.22", %"(double)s2_$K_fetch.23$"
  %"s2_$TMP6[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.26, ptr elementtype(double) nonnull %"s2_$TMP6", i64 %indvars.iv146)
  br label %bb5

bb5:                                              ; preds = %bb5.preheader, %bb10
  %indvars.iv142 = phi i64 [ 1, %bb5.preheader ], [ %indvars.iv.next143, %bb10 ]
  br i1 %rel.15, label %bb10, label %bb9.preheader

bb9.preheader:                                    ; preds = %bb5
  %"s2_$TMP6[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.25, ptr elementtype(double) nonnull %"s2_$TMP6[]", i64 %indvars.iv142)
  br label %bb9

bb9:                                              ; preds = %bb9.preheader, %bb16
  %indvars.iv138 = phi i64 [ 1, %bb9.preheader ], [ %indvars.iv.next139, %bb16 ]
  %"s2_$TMP6[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$TMP6[][]", i64 %indvars.iv138)
  br label %bb13

bb13:                                             ; preds = %bb20, %bb9
  %indvars.iv135 = phi i64 [ %indvars.iv.next136, %bb20 ], [ 1, %bb9 ]
  %"s2_$TMP6[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$TMP6[][][]", i64 %indvars.iv135)
  br label %bb17

bb17:                                             ; preds = %bb17, %bb13
  %indvars.iv132 = phi i64 [ %indvars.iv.next133, %bb17 ], [ 1, %bb13 ]
  %"s2_$TMP6[][][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$TMP6[][][][]", i64 %indvars.iv132)
  store double %sub.1, ptr %"s2_$TMP6[][][][][]", align 1
  %indvars.iv.next133 = add nuw nsw i64 %indvars.iv132, 1
  %exitcond134.not = icmp eq i64 %indvars.iv.next133, 6
  br i1 %exitcond134.not, label %bb20, label %bb17

bb20:                                             ; preds = %bb17
  %indvars.iv.next136 = add nuw nsw i64 %indvars.iv135, 1
  %exitcond137.not = icmp eq i64 %indvars.iv.next136, 6
  br i1 %exitcond137.not, label %bb16, label %bb13

bb16:                                             ; preds = %bb20
  %indvars.iv.next139 = add nuw nsw i64 %indvars.iv138, 1
  %exitcond141 = icmp eq i64 %indvars.iv.next139, %wide.trip.count140
  br i1 %exitcond141, label %bb10.loopexit, label %bb9

bb10.loopexit:                                    ; preds = %bb16
  br label %bb10

bb10:                                             ; preds = %bb10.loopexit, %bb5
  %indvars.iv.next143 = add nuw nsw i64 %indvars.iv142, 1
  %exitcond145 = icmp eq i64 %indvars.iv.next143, %wide.trip.count144
  br i1 %exitcond145, label %bb6.loopexit, label %bb5

bb6.loopexit:                                     ; preds = %bb10
  br label %bb6

bb6:                                              ; preds = %bb6.loopexit, %bb1
  %indvars.iv.next147 = add nuw nsw i64 %indvars.iv146, 1
  %exitcond149 = icmp eq i64 %indvars.iv.next147, %wide.trip.count148150
  br i1 %exitcond149, label %bb21.preheader, label %bb1

bb21.preheader:                                   ; preds = %bb6
  br label %bb21

bb21:                                             ; preds = %bb21.preheader, %bb26
  %indvars.iv128 = phi i64 [ 1, %bb21.preheader ], [ %indvars.iv.next129, %bb26 ]
  br i1 %rel.14, label %bb26, label %bb25.preheader

bb25.preheader:                                   ; preds = %bb21
  %"s2_$TMP6[]47" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.26, ptr elementtype(double) nonnull %"s2_$TMP6", i64 %indvars.iv128)
  %"s2_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.26, ptr elementtype(double) nonnull %"s2_$B", i64 %indvars.iv128)
  br label %bb25

bb25:                                             ; preds = %bb25.preheader, %bb30
  %indvars.iv124 = phi i64 [ 1, %bb25.preheader ], [ %indvars.iv.next125, %bb30 ]
  br i1 %rel.15, label %bb30, label %bb29.preheader

bb29.preheader:                                   ; preds = %bb25
  %"s2_$TMP6[][]48" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.25, ptr elementtype(double) nonnull %"s2_$TMP6[]47", i64 %indvars.iv124)
  %"s2_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.25, ptr elementtype(double) nonnull %"s2_$B[]", i64 %indvars.iv124)
  br label %bb29

bb29:                                             ; preds = %bb29.preheader, %bb36
  %indvars.iv121 = phi i64 [ 1, %bb29.preheader ], [ %indvars.iv.next122, %bb36 ]
  %"s2_$TMP6[][][]49" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$TMP6[][]48", i64 %indvars.iv121)
  %"s2_$B[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$B[][]", i64 %indvars.iv121)
  br label %bb33

bb33:                                             ; preds = %bb40, %bb29
  %indvars.iv118 = phi i64 [ %indvars.iv.next119, %bb40 ], [ 1, %bb29 ]
  br label %bb37

bb37:                                             ; preds = %bb37, %bb33
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb37 ], [ 1, %bb33 ]
  %"s2_$TMP6[][][][]50" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$TMP6[][][]49", i64 %indvars.iv)
  %"s2_$TMP6[][][][][]51" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$TMP6[][][][]50", i64 %indvars.iv118)
  %"s2_$TMP6[][][][][]_fetch.65" = load double, ptr %"s2_$TMP6[][][][][]51", align 1
  %add.10 = fadd fast double %"s2_$TMP6[][][][][]_fetch.65", %"s2_$T1_fetch.22"
  %"s2_$B[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$B[][][]", i64 %indvars.iv)
  %"s2_$B[][][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$B[][][][]", i64 %indvars.iv118)
  store double %add.10, ptr %"s2_$B[][][][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %bb40, label %bb37

bb40:                                             ; preds = %bb37
  %indvars.iv.next119 = add nuw nsw i64 %indvars.iv118, 1
  %exitcond120.not = icmp eq i64 %indvars.iv.next119, 6
  br i1 %exitcond120.not, label %bb36, label %bb33

bb36:                                             ; preds = %bb40
  %indvars.iv.next122 = add nuw nsw i64 %indvars.iv121, 1
  %exitcond123 = icmp eq i64 %indvars.iv.next122, %wide.trip.count140
  br i1 %exitcond123, label %bb30.loopexit, label %bb29

bb30.loopexit:                                    ; preds = %bb36
  br label %bb30

bb30:                                             ; preds = %bb30.loopexit, %bb25
  %indvars.iv.next125 = add nuw nsw i64 %indvars.iv124, 1
  %exitcond127 = icmp eq i64 %indvars.iv.next125, %wide.trip.count144
  br i1 %exitcond127, label %bb26.loopexit, label %bb25

bb26.loopexit:                                    ; preds = %bb30
  br label %bb26

bb26:                                             ; preds = %bb26.loopexit, %bb21
  %indvars.iv.next129 = add nuw nsw i64 %indvars.iv128, 1
  %exitcond131 = icmp eq i64 %indvars.iv.next129, %wide.trip.count148150
  br i1 %exitcond131, label %bb22.loopexit, label %bb21

bb22.loopexit:                                    ; preds = %bb26
  br label %bb22

bb22:                                             ; preds = %bb22.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn mustprogress
declare ptr @llvm.stacksave() #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nofree nosync nounwind willreturn mustprogress
declare void @llvm.stackrestore(ptr) #1

; Function Attrs: nofree nosync nounwind uwtable
define void @shell_(ptr noalias nocapture readonly dereferenceable(4) %"shell_$NX", ptr noalias nocapture readonly dereferenceable(4) %"shell_$NY", ptr noalias nocapture readnone dereferenceable(4) %"shell_$NZ", ptr noalias nocapture readonly dereferenceable(4) %"shell_$NZL", ptr noalias nocapture dereferenceable(8) %"shell_$A", ptr noalias nocapture readonly dereferenceable(8) %"shell_$T1") local_unnamed_addr #0 {
alloca_1:
  %"shell_$NX_fetch.104" = load i32, ptr %"shell_$NX", align 1
  %"shell_$NY_fetch.105" = load i32, ptr %"shell_$NY", align 1
  %"shell_$NZL_fetch.106" = load i32, ptr %"shell_$NZL", align 1
  %int_sext = sext i32 %"shell_$NX_fetch.104" to i64
  %rel.29 = icmp sgt i64 %int_sext, 0
  %slct.13 = select i1 %rel.29, i64 %int_sext, i64 0
  %mul.50 = mul nuw nsw i64 %slct.13, 200
  %int_sext2 = sext i32 %"shell_$NY_fetch.105" to i64
  %rel.30 = icmp sgt i64 %int_sext2, 0
  %slct.14 = select i1 %rel.30, i64 %int_sext2, i64 0
  %mul.51 = mul nsw i64 %mul.50, %slct.14
  %int_sext4 = sext i32 %"shell_$NZL_fetch.106" to i64
  %rel.31 = icmp sgt i64 %int_sext4, 0
  %slct.15 = select i1 %rel.31, i64 %int_sext4, i64 0
  %mul.52 = mul nsw i64 %mul.51, %slct.15
  %div.7 = lshr exact i64 %mul.52, 3
  %"shell_$B22" = alloca double, i64 %div.7, align 8
  %mul.72 = mul nsw i64 %int_sext, 200
  %mul.73 = mul nsw i64 %mul.72, %int_sext2
  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)
  %savedstack = tail call ptr @llvm.stacksave()
  %"s2_$TMP6.i" = alloca double, i64 %div.7, align 8
  %rel.13.i = icmp slt i32 %"shell_$NZL_fetch.106", 1
  br i1 %rel.13.i, label %s2_.exit.thread, label %bb1.i.preheader

bb1.i.preheader:                                  ; preds = %alloca_1
  %rel.14.i = icmp slt i32 %"shell_$NY_fetch.105", 1
  %rel.15.i = icmp slt i32 %"shell_$NX_fetch.104", 1
  %"s2_$T1_fetch.22.i" = load double, ptr %"shell_$T1", align 1
  %0 = add nuw nsw i32 %"shell_$NX_fetch.104", 1
  %1 = add nuw nsw i32 %"shell_$NY_fetch.105", 1
  %2 = add nuw nsw i32 %"shell_$NZL_fetch.106", 1
  %wide.trip.count132134 = zext i32 %2 to i64
  %wide.trip.count128 = sext i32 %1 to i64
  %wide.trip.count124 = sext i32 %0 to i64
  br label %bb1.i

s2_.exit.thread:                                  ; preds = %alloca_1
  tail call void @llvm.stackrestore(ptr %savedstack)
  br label %bb46

bb1.i:                                            ; preds = %bb1.i.preheader, %bb6.i
  %indvars.iv130 = phi i64 [ 1, %bb1.i.preheader ], [ %indvars.iv.next131, %bb6.i ]
  br i1 %rel.14.i, label %bb6.i, label %bb5.i.preheader

bb5.i.preheader:                                  ; preds = %bb1.i
  %3 = trunc i64 %indvars.iv130 to i32
  %"(double)s2_$K_fetch.23$.i" = sitofp i32 %3 to double
  %sub.1.i = fsub fast double %"s2_$T1_fetch.22.i", %"(double)s2_$K_fetch.23$.i"
  %"s2_$TMP6[].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.73, ptr elementtype(double) nonnull %"s2_$TMP6.i", i64 %indvars.iv130) #4
  br label %bb5.i

bb5.i:                                            ; preds = %bb5.i.preheader, %bb10.i
  %indvars.iv126 = phi i64 [ 1, %bb5.i.preheader ], [ %indvars.iv.next127, %bb10.i ]
  br i1 %rel.15.i, label %bb10.i, label %bb9.i.preheader

bb9.i.preheader:                                  ; preds = %bb5.i
  %"s2_$TMP6[][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.72, ptr elementtype(double) nonnull %"s2_$TMP6[].i", i64 %indvars.iv126) #4
  br label %bb9.i

bb9.i:                                            ; preds = %bb9.i.preheader, %bb16.i
  %indvars.iv122 = phi i64 [ 1, %bb9.i.preheader ], [ %indvars.iv.next123, %bb16.i ]
  %"s2_$TMP6[][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$TMP6[][].i", i64 %indvars.iv122) #4
  br label %bb13.i

bb13.i:                                           ; preds = %bb20.i, %bb9.i
  %indvars.iv119 = phi i64 [ %indvars.iv.next120, %bb20.i ], [ 1, %bb9.i ]
  %"s2_$TMP6[][][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$TMP6[][][].i", i64 %indvars.iv119) #4
  br label %bb17.i

bb17.i:                                           ; preds = %bb17.i, %bb13.i
  %indvars.iv116 = phi i64 [ %indvars.iv.next117, %bb17.i ], [ 1, %bb13.i ]
  %"s2_$TMP6[][][][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$TMP6[][][][].i", i64 %indvars.iv116) #4
  store double %sub.1.i, ptr %"s2_$TMP6[][][][][].i", align 1, !noalias !3
  %indvars.iv.next117 = add nuw nsw i64 %indvars.iv116, 1
  %exitcond118.not = icmp eq i64 %indvars.iv.next117, 6
  br i1 %exitcond118.not, label %bb20.i, label %bb17.i

bb20.i:                                           ; preds = %bb17.i
  %indvars.iv.next120 = add nuw nsw i64 %indvars.iv119, 1
  %exitcond121.not = icmp eq i64 %indvars.iv.next120, 6
  br i1 %exitcond121.not, label %bb16.i, label %bb13.i

bb16.i:                                           ; preds = %bb20.i
  %indvars.iv.next123 = add nuw nsw i64 %indvars.iv122, 1
  %exitcond125 = icmp eq i64 %indvars.iv.next123, %wide.trip.count124
  br i1 %exitcond125, label %bb10.i.loopexit, label %bb9.i

bb10.i.loopexit:                                  ; preds = %bb16.i
  br label %bb10.i

bb10.i:                                           ; preds = %bb10.i.loopexit, %bb5.i
  %indvars.iv.next127 = add nuw nsw i64 %indvars.iv126, 1
  %exitcond129 = icmp eq i64 %indvars.iv.next127, %wide.trip.count128
  br i1 %exitcond129, label %bb6.i.loopexit, label %bb5.i

bb6.i.loopexit:                                   ; preds = %bb10.i
  br label %bb6.i

bb6.i:                                            ; preds = %bb6.i.loopexit, %bb1.i
  %indvars.iv.next131 = add nuw nsw i64 %indvars.iv130, 1
  %exitcond133 = icmp eq i64 %indvars.iv.next131, %wide.trip.count132134
  br i1 %exitcond133, label %bb21.i.preheader, label %bb1.i

bb21.i.preheader:                                 ; preds = %bb6.i
  br label %bb21.i

bb21.i:                                           ; preds = %bb21.i.preheader, %bb26.i
  %indvars.iv112 = phi i64 [ 1, %bb21.i.preheader ], [ %indvars.iv.next113, %bb26.i ]
  br i1 %rel.14.i, label %bb26.i, label %bb25.i.preheader

bb25.i.preheader:                                 ; preds = %bb21.i
  %"s2_$TMP6[]47.i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.73, ptr elementtype(double) nonnull %"s2_$TMP6.i", i64 %indvars.iv112) #4
  %"s2_$B[].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.73, ptr elementtype(double) nonnull %"shell_$B22", i64 %indvars.iv112) #4
  br label %bb25.i

bb25.i:                                           ; preds = %bb25.i.preheader, %bb30.i
  %indvars.iv108 = phi i64 [ 1, %bb25.i.preheader ], [ %indvars.iv.next109, %bb30.i ]
  br i1 %rel.15.i, label %bb30.i, label %bb29.i.preheader

bb29.i.preheader:                                 ; preds = %bb25.i
  %"s2_$TMP6[][]48.i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.72, ptr elementtype(double) nonnull %"s2_$TMP6[]47.i", i64 %indvars.iv108) #4
  %"s2_$B[][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.72, ptr elementtype(double) nonnull %"s2_$B[].i", i64 %indvars.iv108) #4
  br label %bb29.i

bb29.i:                                           ; preds = %bb29.i.preheader, %bb36.i
  %indvars.iv104 = phi i64 [ 1, %bb29.i.preheader ], [ %indvars.iv.next105, %bb36.i ]
  %"s2_$TMP6[][][]49.i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$TMP6[][]48.i", i64 %indvars.iv104) #4
  %"s2_$B[][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"s2_$B[][].i", i64 %indvars.iv104) #4
  br label %bb33.i

bb33.i:                                           ; preds = %bb40.i, %bb29.i
  %indvars.iv101 = phi i64 [ %indvars.iv.next102, %bb40.i ], [ 1, %bb29.i ]
  br label %bb37.i

bb37.i:                                           ; preds = %bb37.i, %bb33.i
  %indvars.iv98 = phi i64 [ %indvars.iv.next99, %bb37.i ], [ 1, %bb33.i ]
  %"s2_$TMP6[][][][]50.i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$TMP6[][][]49.i", i64 %indvars.iv98) #4
  %"s2_$TMP6[][][][][]51.i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$TMP6[][][][]50.i", i64 %indvars.iv101) #4
  %"s2_$TMP6[][][][][]_fetch.65.i" = load double, ptr %"s2_$TMP6[][][][][]51.i", align 1, !noalias !3
  %add.10.i = fadd fast double %"s2_$TMP6[][][][][]_fetch.65.i", %"s2_$T1_fetch.22.i"
  %"s2_$B[][][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"s2_$B[][][].i", i64 %indvars.iv98) #4
  %"s2_$B[][][][][].i" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"s2_$B[][][][].i", i64 %indvars.iv101) #4
  store double %add.10.i, ptr %"s2_$B[][][][][].i", align 1, !alias.scope !0, !noalias !8
  %indvars.iv.next99 = add nuw nsw i64 %indvars.iv98, 1
  %exitcond100.not = icmp eq i64 %indvars.iv.next99, 6
  br i1 %exitcond100.not, label %bb40.i, label %bb37.i

bb40.i:                                           ; preds = %bb37.i
  %indvars.iv.next102 = add nuw nsw i64 %indvars.iv101, 1
  %exitcond103.not = icmp eq i64 %indvars.iv.next102, 6
  br i1 %exitcond103.not, label %bb36.i, label %bb33.i

bb36.i:                                           ; preds = %bb40.i
  %indvars.iv.next105 = add nuw nsw i64 %indvars.iv104, 1
  %exitcond107 = icmp eq i64 %indvars.iv.next105, %wide.trip.count124
  br i1 %exitcond107, label %bb30.i.loopexit, label %bb29.i

bb30.i.loopexit:                                  ; preds = %bb36.i
  br label %bb30.i

bb30.i:                                           ; preds = %bb30.i.loopexit, %bb25.i
  %indvars.iv.next109 = add nuw nsw i64 %indvars.iv108, 1
  %exitcond111 = icmp eq i64 %indvars.iv.next109, %wide.trip.count128
  br i1 %exitcond111, label %bb26.i.loopexit, label %bb25.i

bb26.i.loopexit:                                  ; preds = %bb30.i
  br label %bb26.i

bb26.i:                                           ; preds = %bb26.i.loopexit, %bb21.i
  %indvars.iv.next113 = add nuw nsw i64 %indvars.iv112, 1
  %exitcond115 = icmp eq i64 %indvars.iv.next113, %wide.trip.count132134
  br i1 %exitcond115, label %bb45.preheader, label %bb21.i

bb45.preheader:                                   ; preds = %bb26.i
  tail call void @llvm.stackrestore(ptr %savedstack)
  br label %bb45

bb45:                                             ; preds = %bb45.preheader, %bb50
  %indvars.iv94 = phi i64 [ 1, %bb45.preheader ], [ %indvars.iv.next95, %bb50 ]
  br i1 %rel.14.i, label %bb50, label %bb49.preheader

bb49.preheader:                                   ; preds = %bb45
  %"shell_$B22[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.73, ptr elementtype(double) nonnull %"shell_$B22", i64 %indvars.iv94)
  %"shell_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.73, ptr elementtype(double) nonnull %"shell_$A", i64 %indvars.iv94)
  br label %bb49

bb49:                                             ; preds = %bb49.preheader, %bb54
  %indvars.iv90 = phi i64 [ 1, %bb49.preheader ], [ %indvars.iv.next91, %bb54 ]
  br i1 %rel.15.i, label %bb54, label %bb53.preheader

bb53.preheader:                                   ; preds = %bb49
  %"shell_$B22[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.72, ptr elementtype(double) nonnull %"shell_$B22[]", i64 %indvars.iv90)
  %"shell_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.72, ptr elementtype(double) nonnull %"shell_$A[]", i64 %indvars.iv90)
  br label %bb53

bb53:                                             ; preds = %bb53.preheader, %bb60
  %indvars.iv87 = phi i64 [ 1, %bb53.preheader ], [ %indvars.iv.next88, %bb60 ]
  %"shell_$B22[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$B22[][]", i64 %indvars.iv87)
  %"shell_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 200, ptr elementtype(double) nonnull %"shell_$A[][]", i64 %indvars.iv87)
  br label %bb57

bb57:                                             ; preds = %bb64, %bb53
  %indvars.iv84 = phi i64 [ %indvars.iv.next85, %bb64 ], [ 1, %bb53 ]
  %"shell_$B22[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$B22[][][]", i64 %indvars.iv84)
  %"shell_$A[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %"shell_$A[][][]", i64 %indvars.iv84)
  br label %bb61

bb61:                                             ; preds = %bb61, %bb57
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb61 ], [ 1, %bb57 ]
  %"shell_$B22[][][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$B22[][][][]", i64 %indvars.iv)
  %"shell_$B22[][][][][]_fetch.126" = load double, ptr %"shell_$B22[][][][][]", align 1
  %"shell_$A[][][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"shell_$A[][][][]", i64 %indvars.iv)
  store double %"shell_$B22[][][][][]_fetch.126", ptr %"shell_$A[][][][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %bb64, label %bb61

bb64:                                             ; preds = %bb61
  %indvars.iv.next85 = add nuw nsw i64 %indvars.iv84, 1
  %exitcond86.not = icmp eq i64 %indvars.iv.next85, 6
  br i1 %exitcond86.not, label %bb60, label %bb57

bb60:                                             ; preds = %bb64
  %indvars.iv.next88 = add nuw nsw i64 %indvars.iv87, 1
  %exitcond89 = icmp eq i64 %indvars.iv.next88, %wide.trip.count124
  br i1 %exitcond89, label %bb54.loopexit, label %bb53

bb54.loopexit:                                    ; preds = %bb60
  br label %bb54

bb54:                                             ; preds = %bb54.loopexit, %bb49
  %indvars.iv.next91 = add nuw nsw i64 %indvars.iv90, 1
  %exitcond93 = icmp eq i64 %indvars.iv.next91, %wide.trip.count128
  br i1 %exitcond93, label %bb50.loopexit, label %bb49

bb50.loopexit:                                    ; preds = %bb54
  br label %bb50

bb50:                                             ; preds = %bb50.loopexit, %bb45
  %indvars.iv.next95 = add nuw nsw i64 %indvars.iv94, 1
  %exitcond97 = icmp eq i64 %indvars.iv.next95, %wide.trip.count132134
  br i1 %exitcond97, label %bb46.loopexit, label %bb45

bb46.loopexit:                                    ; preds = %bb50
  br label %bb46

bb46:                                             ; preds = %bb46.loopexit, %s2_.exit.thread
  ret void
}

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.experimental.noalias.scope.decl(metadata) #3

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { inaccessiblememonly nofree nosync nounwind willreturn mustprogress }
attributes #4 = { nounwind }

!omp_offload.info = !{}

!0 = !{!1}
!1 = distinct !{!1, !2, !"s2_: %s2_$B"}
!2 = distinct !{!2, !"s2_"}
!3 = !{!4, !5, !6, !1, !7}
!4 = distinct !{!4, !2, !"s2_: %s2_$NX"}
!5 = distinct !{!5, !2, !"s2_: %s2_$NY"}
!6 = distinct !{!6, !2, !"s2_: %s2_$NZL"}
!7 = distinct !{!7, !2, !"s2_: %s2_$T1"}
!8 = !{!4, !5, !6, !7}
