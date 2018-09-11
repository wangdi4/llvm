; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, 66, 1   <DO_LOOP>
; |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 67>
; |   |   %2 = (%oi)[0][i1 + -1 * i2 + 2];
; |   |   if (%2 * i1 + %2 != 0)
; |   |   {
; |   |      %3 = (%a7)[0][i1 + -1 * i2 + 2];
; |   |      (%a7)[0][i1 + -1 * i2 + 2] = %3 + 1;
; |   |   }
; |   + END LOOP
; + END LOOP

; Verify that the rval ref is marked as non-linear because it contains non-linear blob %2.

; CHECK: |   |   if (%2 * i1 + %2 != 0)
; CHECK: |   |   <RVAL-REG> NON-LINEAR i32 %2 * i1 + %2
; CHECK: |   |      <BLOB> NON-LINEAR i32 %2


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %a7 = alloca [100 x i32], align 16
  %oi = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %a7 to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %0, i8 0, i64 400, i1 false)
  %1 = bitcast [100 x i32]* %oi to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %1, i8 0, i64 400, i1 false)
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %oi, i64 0, i64 0
  store i32 1, i32* %arraydecay, align 16
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc7, %entry
  %indvars.iv = phi i64 [ 2, %entry ], [ %indvars.iv.next, %for.inc7 ]
  %n.022 = phi i32 [ 0, %entry ], [ %inc, %for.inc7 ]
  %inc = add nuw nsw i32 %n.022, 1
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.preheader
  %indvars.iv23 = phi i64 [ %indvars.iv, %for.body3.preheader ], [ %indvars.iv.next24, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %oi, i64 0, i64 %indvars.iv23
  %2 = load i32, i32* %arrayidx, align 4
  %mul = mul i32 %2, %inc
  %tobool = icmp eq i32 %mul, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %a7, i64 0, i64 %indvars.iv23
  %3 = load i32, i32* %arrayidx6, align 4
  %add = add i32 %3, 1
  store i32 %add, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body3
  %4 = trunc i64 %indvars.iv23 to i32
  %dec = add nsw i32 %4, -1
  %cmp2 = icmp ugt i32 %dec, 1
  %indvars.iv.next24 = add nsw i64 %indvars.iv23, -1
  br i1 %cmp2, label %for.body3, label %for.inc7

for.inc7:                                         ; preds = %for.inc
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i32 %inc, 67
  br i1 %exitcond, label %for.end9, label %for.body3.preheader

for.end9:                                         ; preds = %for.inc7
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #0

