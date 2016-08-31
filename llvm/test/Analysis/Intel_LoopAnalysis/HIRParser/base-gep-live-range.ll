; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that the loop is parsed correctly. We form the following SCC: 
; (%1 -> %add.ptr.i -> %3)
; %add.ptr.i is marked with live range metadata. That should prevent the uses
; of %add.ptr.i in the loop (for ex- %add.ptr4.i) from tracing back incorrectly.


; CHECK: + DO i1 = 0, 11, 1   <DO_LOOP>
; CHECK: |   %2 = {al:4}(@size)[0][i1];
; CHECK: |   %1 = &((%1)[%2]);
; CHECK: |   %retval.0.i = 0;
; CHECK: |   if (&((%1)[0]) <= &((@allocbuf)[1][0]))
; CHECK: |   {
; CHECK: |      {al:8}(@allocp)[0] = &((%1)[0]);
; CHECK: |      %retval.0.i = &((%1)[-1 * sext.i32.i64(%2)]);
; CHECK: |   }
; CHECK: |   {al:8}(%p)[0][i1] = &((%retval.0.i)[0]);
; CHECK: + END LOOP

source_filename = "kr_097.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@allocp = internal unnamed_addr global i8* getelementptr inbounds ([1024 x i8], [1024 x i8]* @allocbuf, i64 0, i64 0), align 8
@allocbuf = internal global [1024 x i8] zeroinitializer, align 16
@size = internal unnamed_addr constant [12 x i32] [i32 2, i32 24, i32 16, i32 40, i32 9, i32 22, i32 1, i32 13, i32 17, i32 19, i32 102, i32 7], align 16

; Function Attrs: nounwind uwtable
define i32 @main() #1 {
entry:
  %p = alloca [12 x i8*], align 16
  %0 = bitcast [12 x i8*]* %p to i8*
  %.pre = load i8*, i8** @allocp, align 8
  br label %for.body

for.body:                                         ; preds = %my_alloc.exit, %entry
  %1 = phi i8* [ %.pre, %entry ], [ %3, %my_alloc.exit ]
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %my_alloc.exit ]
  %arrayidx = getelementptr inbounds [12 x i32], [12 x i32]* @size, i64 0, i64 %indvars.iv27
  %2 = load i32, i32* %arrayidx, align 4
  %idx.ext.i = sext i32 %2 to i64
  %add.ptr.i = getelementptr inbounds i8, i8* %1, i64 %idx.ext.i
  %cmp.i = icmp ugt i8* %add.ptr.i, getelementptr inbounds ([1024 x i8], [1024 x i8]* @allocbuf, i64 1, i64 0)
  br i1 %cmp.i, label %my_alloc.exit, label %if.then.i

if.then.i:                                        ; preds = %for.body
  store i8* %add.ptr.i, i8** @allocp, align 8
  %idx.neg.i = sub nsw i64 0, %idx.ext.i
  %add.ptr4.i = getelementptr inbounds i8, i8* %add.ptr.i, i64 %idx.neg.i
  br label %my_alloc.exit

my_alloc.exit:                                    ; preds = %for.body, %if.then.i
  %3 = phi i8* [ %add.ptr.i, %if.then.i ], [ %1, %for.body ]
  %retval.0.i = phi i8* [ %add.ptr4.i, %if.then.i ], [ null, %for.body ]
  %arrayidx3 = getelementptr inbounds [12 x i8*], [12 x i8*]* %p, i64 0, i64 %indvars.iv27
  store i8* %retval.0.i, i8** %arrayidx3, align 8
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond = icmp eq i64 %indvars.iv.next28, 12
  br i1 %exitcond, label %for.body7.preheader, label %for.body

for.body7.preheader:                              ; preds = %my_alloc.exit
  ret i32 0
}


