; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-before=hir-opt-predicate -print-after=hir-opt-predicate %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; Verify that predicate in switch's default case is optimized successfully.

; CHECK-LABEL: Function

; CHECK: DO i1
; CHECK: default:
; CHECK: |      if (%p.0702 == 6)
; CHECK: |      {
; CHECK: |         (@ua)[0][i1] = -32768;
; CHECK: |         (@sa)[0][i1] = -32768;
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         (@ua)[0][i1] = 0;
; CHECK: |         (@sa)[0][i1] = 0;
; CHECK: |      }

; CHECK-LABEL: Function

; CHECK: default:
; CHECK: DO i1
; CHECK: |      (@ua)[0][i1] = -32768;
; CHECK: |      (@sa)[0][i1] = -32768;

; CHECK: DO i1
; CHECK: |      (@ua)[0][i1] = 0;
; CHECK: |      (@sa)[0][i1] = 0;


; ModuleID = 'martyn2b.c'
source_filename = "martyn2b.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sres = internal unnamed_addr global [65536 x i32] zeroinitializer, align 16
@ures = internal unnamed_addr global [65536 x i32] zeroinitializer, align 16
@ua = internal unnamed_addr global [65536 x i16] zeroinitializer, align 16
@sa = internal unnamed_addr global [65536 x i16] zeroinitializer, align 16
@up = internal unnamed_addr global i32 0, align 4
@uaccres = internal unnamed_addr constant [28 x i32] [i32 1431666688, i32 357924864, i32 -715816960, i32 1431666688, i32 1789591552, i32 -715816960, i32 1431666688, i32 1789591552, i32 -715816960, i32 1431666688, i32 357924864, i32 -715816960, i32 1431666688, i32 -2147450880, i32 65536, i32 1431666688, i32 -2147450880, i32 65536, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 -65535, i32 -131071, i32 -333], align 16
@sp = internal unnamed_addr global i32 0, align 4
@saccres = internal unnamed_addr constant [46 x i32] [i32 1431666688, i32 1431666688, i32 1431666688, i32 357924864, i32 -715816960, i32 1431666688, i32 1431666688, i32 -1431633920, i32 1789591552, i32 -715816960, i32 1431666688, i32 1431666688, i32 -1431633920, i32 1789591552, i32 -715816960, i32 1431666688, i32 1431666688, i32 1431666688, i32 357924864, i32 -715816960, i32 1431666688, i32 1431666688, i32 32768, i32 -2147450880, i32 65536, i32 1431666688, i32 1431666688, i32 32768, i32 -2147450880, i32 65536, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 -65535, i32 -131071, i32 -333], align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %p.0702) {
entry:
  %cmp67 = icmp eq i32 %p.0702, 6
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %i.0695 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  switch i32 %p.0702, label %if.else66 [
    i32 0, label %if.then
    i32 1, label %if.then14
    i32 2, label %if.then25
    i32 3, label %if.then37
    i32 4, label %if.then49
    i32 5, label %if.then59
  ]

if.then:                                          ; preds = %for.body3
  %conv = trunc i64 %indvars.iv to i16
  %arrayidx8 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 %conv, i16* %arrayidx8, align 2
  %sub = add nuw nsw i32 %i.0695, 32768
  %conv9 = trunc i32 %sub to i16
  %arrayidx11 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv9, i16* %arrayidx11, align 2
  br label %for.inc

if.then14:                                        ; preds = %for.body3
  %conv15 = trunc i64 %indvars.iv to i16
  %arrayidx17 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 %conv15, i16* %arrayidx17, align 2
  %sub18 = sub nsw i32 32767, %i.0695
  %conv19 = trunc i32 %sub18 to i16
  %arrayidx21 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv19, i16* %arrayidx21, align 2
  br label %for.inc

if.then25:                                        ; preds = %for.body3
  %tmp = sub nuw nsw i64 65535, %indvars.iv
  %conv27 = trunc i64 %tmp to i16
  %arrayidx29 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 %conv27, i16* %arrayidx29, align 2
  %sub30 = add nuw nsw i32 %i.0695, 32768
  %conv31 = trunc i32 %sub30 to i16
  %arrayidx33 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv31, i16* %arrayidx33, align 2
  br label %for.inc

if.then37:                                        ; preds = %for.body3
  %tmp1 = sub nuw nsw i64 65535, %indvars.iv
  %conv39 = trunc i64 %tmp1 to i16
  %arrayidx41 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 %conv39, i16* %arrayidx41, align 2
  %sub42 = sub nsw i32 32767, %i.0695
  %conv43 = trunc i32 %sub42 to i16
  %arrayidx45 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv43, i16* %arrayidx45, align 2
  br label %for.inc

if.then49:                                        ; preds = %for.body3
  %arrayidx51 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 -1, i16* %arrayidx51, align 2
  %sub52 = add nuw nsw i32 %i.0695, 32768
  %conv53 = trunc i32 %sub52 to i16
  %arrayidx55 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv53, i16* %arrayidx55, align 2
  br label %for.inc

if.then59:                                        ; preds = %for.body3
  %arrayidx61 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  store i16 -1, i16* %arrayidx61, align 2
  %sub62 = sub nsw i32 32767, %i.0695
  %conv63 = trunc i32 %sub62 to i16
  %arrayidx65 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 %conv63, i16* %arrayidx65, align 2
  br label %for.inc

if.else66:                                        ; preds = %for.body3
  %arrayidx71 = getelementptr inbounds [65536 x i16], [65536 x i16]* @ua, i64 0, i64 %indvars.iv
  br i1 %cmp67, label %if.then69, label %if.else74

if.then69:                                        ; preds = %if.else66
  store i16 -32768, i16* %arrayidx71, align 2
  %arrayidx73 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 -32768, i16* %arrayidx73, align 2
  br label %for.inc

if.else74:                                        ; preds = %if.else66
  store i16 0, i16* %arrayidx71, align 2
  %arrayidx81 = getelementptr inbounds [65536 x i16], [65536 x i16]* @sa, i64 0, i64 %indvars.iv
  store i16 0, i16* %arrayidx81, align 2
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.then25, %if.then49, %if.then69, %if.else74, %if.then59, %if.then37, %if.then14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.0695, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 65536
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.inc
  ret i32 0
}

