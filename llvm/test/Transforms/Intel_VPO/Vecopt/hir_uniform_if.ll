; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -S -disable-output -vplan-force-linearization-hir < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-LIN
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -S -disable-output -vplan-force-linearization-hir < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-LIN
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -S -disable-output -vplan-force-linearization-hir=false < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-UNI
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -S -disable-output -vplan-force-linearization-hir=false < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-UNI

; The loop in the test here does a store to %arr under a uniform condition.
; The test checks for linearization of control flow by checking that the store
; to %arr is done under a mask when linearization is forced. When linearization
; is not forced, the test checks for generation of if/else and that store to
; %arr is unmasked.
; The scalar loop is expected to look like the following before vectorization.
;  DO i1 = 0, 99, 1   <DO_LOOP>
;    if (%n1 != 0)
;    {
;       (%arr)[i1] = i1;
;    }
;  END LOOP

; CHECK: DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK: [[Mask:%.*]] = %n1 != 0
; CHECK-LIN: (<4 x i32>*)(%arr)[{{.*}}] = {{.*}}; Mask = @{[[Mask]]}
; CHECK-UNI: [[IfCond:%.*]] = extractelement [[Mask]],  0
; CHECK-UNI: if ([[IfCond]] == 1)
; CHECK-UNI:     goto [[ThenBlock:BB.*]];
; CHECK-UNI: else
; CHECK-UNI:     goto [[ElseBlock:BB.*]];
; CHECK-UNI: [[ThenBlock]]:
; CHECK-UNI:     (<4 x i32>*)(%arr)[{{.*}}] = {{.*}}
; CHECK-UNI:      goto [[ElseBlock]]
; CHECK-UNI: [[ElseBlock]]:
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
