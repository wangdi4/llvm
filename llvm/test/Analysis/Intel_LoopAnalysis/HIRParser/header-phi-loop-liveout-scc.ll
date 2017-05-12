; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we construct the HIR properly by suppressing creation of SCC when the header phi is live outside the loop in another SCC instruction. 

; CHECK: SCC1
; CHECK-SAME: %x0.055

; CHECK-NOT: SCC2

; CHECK: + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK: |   %x0.055.out1 = %x0.055;
; CHECK: |
; CHECK: |      %x0.152 = %x0.055.out1;
; CHECK: |   + DO i2 = 0, -1 * i1 + 22, 1   <DO_LOOP>
; CHECK: |   |   %x0.2 = %x0.152;
; CHECK: |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   |   %x0.2.out = %x0.2;
; CHECK: |   |   |   %x0.2 = 0;
; CHECK: |   |   + END LOOP
; CHECK: |   |   %x0.152 = %x0.2.out;
; CHECK: |   + END LOOP
; CHECK: |      %x0.055 = %x0.2.out;
; CHECK: |
; CHECK: |   %x0.055.out = %x0.055;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'cq328329.c'
source_filename = "cq328329.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"passed\00", align 1
@.str.2 = private unnamed_addr constant [7 x i8] c"failed\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %x0.055 = phi i32 [ 55, %entry ], [ %x0.1.lcssa, %for.inc11 ]
  %i.054 = phi i32 [ 1, %entry ], [ %inc12, %for.inc11 ]
  %cmp251 = icmp ult i32 %i.054, 24
  br i1 %cmp251, label %for.cond6.preheader.preheader, label %for.inc11

for.cond6.preheader.preheader:                    ; preds = %for.cond1.preheader
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.cond6.preheader.preheader, %for.inc9
  %k.053 = phi i32 [ %inc, %for.inc9 ], [ %i.054, %for.cond6.preheader.preheader ]
  %x0.152 = phi i32 [ %x0.2.lcssa, %for.inc9 ], [ %x0.055, %for.cond6.preheader.preheader ]
  br label %for.cond6

for.cond6:                                        ; preds = %for.cond6, %for.cond6.preheader
  %x0.2 = phi i32 [ 0, %for.cond6 ], [ %x0.152, %for.cond6.preheader ]
  %kr.0 = phi i32 [ %dec, %for.cond6 ], [ %k.053, %for.cond6.preheader ]
  %cmp7 = icmp ugt i32 %kr.0, %k.053
  %dec = add nsw i32 %kr.0, -1
  br i1 %cmp7, label %for.cond6, label %for.inc9

for.inc9:                                         ; preds = %for.cond6
  %x0.2.lcssa = phi i32 [ %x0.2, %for.cond6 ]
  %inc = add nuw nsw i32 %k.053, 1
  %exitcond = icmp eq i32 %inc, 24
  br i1 %exitcond, label %for.inc11.loopexit, label %for.cond6.preheader

for.inc11.loopexit:                               ; preds = %for.inc9
  %x0.2.lcssa.lcssa = phi i32 [ %x0.2.lcssa, %for.inc9 ]
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond1.preheader
  %x0.1.lcssa = phi i32 [ %x0.055, %for.cond1.preheader ], [ %x0.2.lcssa.lcssa, %for.inc11.loopexit ]
  %inc12 = add nuw nsw i32 %i.054, 1
  %exitcond56 = icmp eq i32 %inc12, 57
  br i1 %exitcond56, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
  %x0.1.lcssa.lcssa = phi i32 [ %x0.1.lcssa, %for.inc11 ]
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %x0.1.lcssa.lcssa)
  %cmp14 = icmp eq i32 %x0.1.lcssa.lcssa, 55
  br i1 %cmp14, label %if.then, label %if.else

if.then:                                          ; preds = %for.end13
  %call15 = tail call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.1, i64 0, i64 0))
  br label %cleanup

if.else:                                          ; preds = %for.end13
  %call16 = tail call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.2, i64 0, i64 0))
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then
  %retval.0 = phi i32 [ 0, %if.then ], [ -1, %if.else ]
  ret i32 %retval.0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) 

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) 

