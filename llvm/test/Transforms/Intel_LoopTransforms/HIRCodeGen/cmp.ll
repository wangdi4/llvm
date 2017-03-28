; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s
;Verify cmp instructions are correctly generated
;          BEGIN REGION { }
;<29>         + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
;<2>          |   %small.030.out = %small.030;
;<4>          |   %0 = (%A)[i1];
;<6>          |   %1 = (%B)[i1];
;<7>          |   %cmp3 = %0 < %1;
;<8>          |   %tobool = %small.030.out != 0;
;<11>         |   %small.030 = %cmp3  ||  %tobool;
;<13>         |   if (%small.030 == 0)
;<13>         |   {
;<17>         |      (%B)[i1] = %0;
;<13>         |   }
;<13>         |   else
;<13>         |   {
;<27>         |      (%A)[i1] = %1;
;<13>         |   }
;<29>         + END LOOP
;          END REGION
;
;CHECK: region.0:
;CHECK: {{loop.[0-9]+:}}
;CHECK: [[A_ADDR:%.*]] = getelementptr inbounds i32, i32* %A
;CHECK: [[A_LOAD:%.*]] = load i32, i32* [[A_ADDR]]
;CHECK: store i32 [[A_LOAD]], i32* [[A_SLOT:%t[0-9]+]]

;CHECK: [[B_ADDR:%.*]] = getelementptr inbounds i32, i32* %B
;CHECK: [[B_LOAD:%.*]] = load i32, i32* [[B_ADDR]]
;CHECK: store i32 [[B_LOAD]], i32* [[B_SLOT:%t[0-9]+]]

; A[i] < B[i]
;CHECK: [[CMP1:%hir.cmp.[0-9]+]] = icmp slt i32 [[A_SLOT]].{{[0-9]*}}, [[B_SLOT]].{{[0-9]*}}
;CHECK: store i1 [[CMP1]], i1* [[CMP1_SLOT:%t[0-9]+]]

;tobool = small.030 != 0
;CHECK: [[CMP2:%hir.cmp.[0-9]+]] = icmp ne i8 {{.*}}, 0
;CHECK: store i1 [[CMP2]], i1* [[CMP2_SLOT:%t[0-9]+]]

;extend cmp3 to i8 for or op. Done in original IR
;CHECK: [[CMP1_LOAD:%.*]] = load i1, i1* [[CMP1_SLOT]]
;CHECK: [[CMP1_EXT:%.*]] = zext i1 [[CMP1_LOAD]] to i8

;zext the ne 0 cmp
;CHECK: [[CMP2_LOAD:%.*]] = load i1, i1* [[CMP2_SLOT]]
;CHECK: [[CMP2_EXT:%.*]] = zext i1 [[CMP2_LOAD]] to i8

;CHECK: [[OR_CMP:%.*]] = or i8 [[CMP1_EXT]], [[CMP2_EXT]]
;CHECK: store i8 [[OR_CMP]], i8* [[OR_SLOT:%t[0-9]+]]


;CHECK: [[OR_LOAD:%.*]] = load i8, i8* [[OR_SLOT]]
;CHECK: [[CMP_ZERO:%hir.cmp.[0-9]+]] = icmp eq i8 [[OR_LOAD]], 0

;ModuleID = 'cmp1.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPiS_i(i32* nocapture %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.28 = icmp sgt i32 %n, 0
  br i1 %cmp.28, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %small.030 = phi i8 [ %frombool, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp slt i32 %0, %1
  %tobool = icmp ne i8 %small.030, 0
  %cmp326 = zext i1 %cmp3 to i8
  %tobool27 = zext i1 %tobool to i8
  %frombool = or i8 %cmp326, %tobool27
  %tobool4 = icmp eq i8 %frombool, 0
  br i1 %tobool4, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  store i32 %1, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  store i32 %0, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
