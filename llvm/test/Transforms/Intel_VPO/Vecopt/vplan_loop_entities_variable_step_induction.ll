; Test to check that VPInduction descriptor is constructed for an auto-recognized induction with variable step.

; RUN: opt -loopopt=0 -vplan-vec -vpo-vplan-build-stress-test -vplan-build-stress-only-innermost -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -loopopt=0 -passes="vplan-vec" -vpo-vplan-build-stress-test -vplan-build-stress-only-innermost -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK: i64 {{%vp.*}} = induction-init{add} i64 -1024 i64 [[VAR_STEP:%[0-9]+]]
; CHECK: i64 {{%vp.*}} = induction-init-step{add} i64 [[VAR_STEP]]

@Out = dso_local local_unnamed_addr global [1024 x [1024 x i32]] zeroinitializer, align 16

define dso_local void @_Z11supersampleiiPi(i32 %x, i32 %y, i32* nocapture readonly %a) local_unnamed_addr mustprogress {
entry:
  %add.neg = sub i32 2, %x
  %sub = sub i32 %add.neg, %y
  %mul = mul nsw i32 %y, %x
  %0 = sext i32 %sub to i64
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %entry, %for.inc14
  %indvars.iv36 = phi i64 [ -1024, %entry ], [ %indvars.iv.next37, %for.inc14 ]
  br label %for.body5

for.body5:                                        ; preds = %for.cond3.preheader, %for.inc
  %indvars.iv = phi i64 [ -1024, %for.cond3.preheader ], [ %indvars.iv.next, %for.inc ]
  %1 = or i64 %indvars.iv, %indvars.iv36
  %2 = trunc i64 %1 to i32
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %for.inc, label %if.end

if.end:                                           ; preds = %for.body5
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx, align 4
  %add8 = add nsw i32 %4, %mul
  %arrayidx12 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @Out, i64 0, i64 %indvars.iv36, i64 %indvars.iv
  store i32 %add8, i32* %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body5, %if.end
  %indvars.iv.next = add nsw i64 %indvars.iv, %0
  %cmp4 = icmp slt i64 %indvars.iv.next, 1025
  br i1 %cmp4, label %for.body5, label %for.inc14

for.inc14:                                        ; preds = %for.inc
  %indvars.iv.next37 = add nsw i64 %indvars.iv36, %0
  %cmp = icmp slt i64 %indvars.iv.next37, 1025
  br i1 %cmp, label %for.cond3.preheader, label %for.end16

for.end16:                                        ; preds = %for.inc14
  ret void
}

