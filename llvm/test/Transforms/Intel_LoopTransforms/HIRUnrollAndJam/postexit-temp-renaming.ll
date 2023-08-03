; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that the liveout temp %div is correctly renamed for different unrolled
; iterations of i3 loop postexit.

; Incoming HIR-
; + DO i1 = 0, 84, 1   <DO_LOOP>
; |   + DO i2 = 0, 41, 1   <DO_LOOP>
; |   |   + DO i3 = 0, -1 * i1 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; |   |   |   %t3 = (%a)[0][i2 + 1][i1 + i3 + 1];
; |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; |   |   |   %div = %t4  /  %t3;
; |   |   + END LOOP
; |   |      (%a)[0][i2 + 1][0] = %div;
; |   + END LOOP
; + END LOOP

; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   |   + DO i3 = 0, -1 * i1 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 1][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 2][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp3 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 3][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp4 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 4][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp5 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 5][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp6 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 6][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp7 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 7][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %temp8 = %t4  /  %t3;
; CHECK: |   |   |   %t3 = (%a)[0][8 * i2 + 8][i1 + i3 + 1];
; CHECK: |   |   |   %t4 = (%vq8)[0][i1 + i3 + 1];
; CHECK: |   |   |   %div = %t4  /  %t3;
; CHECK: |   |   + END LOOP
; CHECK: |   |      (%a)[0][8 * i2 + 1][0] = %temp;
; CHECK: |   |      (%a)[0][8 * i2 + 2][0] = %temp3;
; CHECK: |   |      (%a)[0][8 * i2 + 3][0] = %temp4;
; CHECK: |   |      (%a)[0][8 * i2 + 4][0] = %temp5;
; CHECK: |   |      (%a)[0][8 * i2 + 5][0] = %temp6;
; CHECK: |   |      (%a)[0][8 * i2 + 6][0] = %temp7;
; CHECK: |   |      (%a)[0][8 * i2 + 7][0] = %temp8;
; CHECK: |   |      (%a)[0][8 * i2 + 8][0] = %div;
; CHECK: |   + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %vq8 = alloca [100 x i32], align 16
  %a = alloca [100 x [100 x i32]], align 16
  %t0 = bitcast ptr %vq8 to ptr
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %t0, i8 0, i64 400, i1 false)
  %t1 = bitcast ptr %a to ptr
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(40000) %t1, i8 0, i64 40000, i1 false)
  br label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc17, %for.cond1.preheader.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc17 ], [ 1, %for.cond1.preheader.preheader ]
  %j6.043 = phi i32 [ %inc18, %for.inc17 ], [ 1, %for.cond1.preheader.preheader ]
  %cmp539 = icmp ult i32 %j6.043, 17
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc14, %for.cond1.preheader
  %indvars.iv46 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next47, %for.inc14 ]
  br i1 %cmp539, label %for.body6.lr.ph, label %for.inc14

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], ptr %a, i64 0, i64 %indvars.iv46, i64 0
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %indvars.iv44 = phi i64 [ %indvars.iv, %for.body6.lr.ph ], [ %indvars.iv.next45, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], ptr %a, i64 0, i64 %indvars.iv46, i64 %indvars.iv44
  %t3 = load i32, ptr %arrayidx8, align 4
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr %vq8, i64 0, i64 %indvars.iv44
  %t4 = load i32, ptr %arrayidx10, align 4
  %div = sdiv i32 %t4, %t3
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond = icmp eq i64 %indvars.iv.next45, 17
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body6

for.inc14.loopexit:                               ; preds = %for.body6
  %div.lcssa = phi i32 [ %div, %for.body6 ]
  store i32 %div.lcssa, ptr %arrayidx13, align 16
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond4.preheader
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 43
  br i1 %exitcond48, label %for.inc17, label %for.cond4.preheader

for.inc17:                                        ; preds = %for.inc14
  %inc18 = add nuw nsw i32 %j6.043, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next, 86
  br i1 %exitcond49, label %for.exit, label %for.cond1.preheader

for.exit:                           ; preds = %for.inc17
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2


