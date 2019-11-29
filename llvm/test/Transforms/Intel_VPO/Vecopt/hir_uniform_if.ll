; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -S -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -S -disable-output < %s 2>&1 | FileCheck %s

; Currently HIR vector code generation does not preserve uniform control flow.
; The loop in the test here does a store to %arr under a uniform condition.
; Until we extend code generation, the test checks for linearization of control
; flow by checking that the store to %arr is done under a mask.

; CHECK: DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK: [[Mask:%.*]] = %n1 != 0
; CHECK: (<4 x i32>*)(%arr)[{{.*}}] = {{.*}}; Mask = @{[[Mask]]}
; CHECK: END LOOP

define void @foo(i32* %arr, i32 %n1) {
entry:
  %tobool = icmp eq i32 %n1, 0
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}
