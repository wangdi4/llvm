; RUN: opt -aa-pipeline=basic-aa -passes="require<aa>,aa-eval" -print-all-alias-modref-info  -disable-output < %s 2>&1 | FileCheck  %s

; This test case checks that the alias analysis marks as NoAlias between the
; pointers that loads the fields for %Local and the pointer that access the
; entries in the array stored at field 2 in %input_ptr.

; This test case was created from the following C++ test:

; struct TestStruct {
;   int rows;
;   int cols;
;   double *data;
; };
;
; TestStruct *bar();
; void dealloc(TestStruct *ptr);
;
; void foo(TestStruct *input_ptr, int n) {
;   TestStruct Local;
;   Local.rows = 3;
;   Local.cols = 3;
;   Local.data = bar();
;
;   for(int i = 0; i < n; i+= 2) {
;     double val = 0;
;     for (int j = 0; j < Local.rows; j++) {
;       val += Local.data[j];
;     }
;
;     input_ptr->data[i] = val;
;   }
;   dealloc(&Local);
; }

; We know that function dealloc won't produce any alias for Local. We need to
; check that the pointers accessing Local.rows, Local.cols and Local.data won't
; alias with input_ptr->data[i].

; The HIR looks as follows (opt -disable-output intel-alloc-site-load-arg.ll
; -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>"):

; Function: _Z3fooP10TestStructi
;
; <0>          BEGIN REGION { }
; <27>               + DO i1 = 0, (zext.i32.i64(%n) + -1)/u2, 1   <DO_LOOP>
; <2>                |   %rows2 = (%Local)[0].0;
; <4>                |   %datai = (%Local)[0].2;
; <6>                |   %val.024 = 0.000000e+00;
; <28>               |
; <28>               |   + DO i2 = 0, zext.i32.i64(%rows2) + -1, 1   <DO_LOOP>
; <11>               |   |   %val.024 = (%datai)[i2]  +  %val.024;
; <28>               |   + END LOOP
; <28>               |
; <20>               |   (%0)[2 * i1] = %val.024;
; <27>               + END LOOP
; <0>          END REGION

; Check that Local.rows, Local.cols and Local.data won't alias with
; input_ptr->data[i].

; CHECK:  NoAlias:	double* %arrayidx9, i32* %rows
; CHECK:  NoAlias:	double* %arrayidx9, i32* %cols
; CHECK:  NoAlias:	double* %arrayidx9, ptr* %data

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TestStruct = type { i32, i32, ptr }

declare void @dealloc(ptr %arg)
declare dso_local noundef ptr @_Z3barv()

; Function Attrs: mustprogress uwtable
define dso_local void @_Z3fooP10TestStructi(ptr nocapture noundef %input_ptr, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %Local = alloca %struct.TestStruct 
  %call = tail call noundef ptr @_Z3barv()
  %rows = getelementptr inbounds %struct.TestStruct, ptr %Local, i32 0, i32 0 
  store i32 3, ptr %rows 
  %cols = getelementptr inbounds %struct.TestStruct, ptr %Local, i32 0, i32 1 
  store i32 3, ptr %cols 
  %data = getelementptr inbounds %struct.TestStruct, ptr %Local, i32 0, i32 2 
  store ptr %call, ptr %data 
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %data7 = getelementptr inbounds %struct.TestStruct, ptr %input_ptr, i64 0, i32 2 
  %0 = load ptr, ptr %data7 
  %1 = zext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup4
  %indvars.iv28 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next29, %for.cond.cleanup4 ]
  %rows2 = load i32, ptr %rows
  %rowsz = zext i32 %rows2 to i64
  %datai = load ptr, ptr %data
  br label %for.body5

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup4
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  call void @dealloc(ptr %Local)
  ret void

for.cond.cleanup4:                                ; preds = %for.body5
  %add.lcssa = phi double [ %add, %for.body5 ]
  %arrayidx9 = getelementptr inbounds double, ptr %0, i64 %indvars.iv28
  store double %add.lcssa, ptr %arrayidx9 
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 2
  %cmp = icmp ult i64 %indvars.iv.next29, %1
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.cleanup.loopexit 

for.body5:                                        ; preds = %for.cond1.preheader, %for.body5
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body5 ]
  %val.024 = phi double [ 0.000000e+00, %for.cond1.preheader ], [ %add, %for.body5 ]
  %arrayidx = getelementptr inbounds double, ptr %datai, i64 %indvars.iv
  %2 = load double, ptr %arrayidx 
  %add = fadd fast double %2, %val.024
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %rowsz
  br i1 %exitcond.not, label %for.cond.cleanup4, label %for.body5 
}
