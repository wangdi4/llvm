; Test to check that VPValue based code generation assigns the same symbases to
; vector memory references as the corresponding scalar memory reference.
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>" -aa-pipeline="basic-aa" -vplan-force-vf=4 -hir-details -S -disable-output < %s 2>&1 | FileCheck %s
; CHECK: DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:  <RVAL-REG> {{.*}}(LINEAR ptr %arr1){{.*}} {sb:[[ARR1SB:[0-9]+]]}
; CHECK:  <LVAL-REG> {{.*}}(LINEAR ptr %arr2){{.*}} {sb:[[ARR2SB:[0-9]+]]}
; CHECK:  <LVAL-REG> {{.*}}(LINEAR ptr %arr1){{.*}} {sb:[[ARR1SB]]}
; CHECK: END LOOP
; CHECK: DO i64 i1 = 0, 99, 4   <DO_LOOP>
; CHECK:  <RVAL-REG> {{.*}}(<4 x i32>*)(LINEAR ptr %arr1){{.*}} {sb:[[ARR1SB]]}
; CHECK:  <LVAL-REG> {{.*}}(<4 x i32>*)(LINEAR ptr %arr2){{.*}} {sb:[[ARR2SB]]}
; CHECK:  <LVAL-REG> {{.*}}(<4 x i32>*)(LINEAR ptr %arr1){{.*}} {sb:[[ARR1SB]]}
; CHECK: END LOOP
define void @foo(ptr noalias nocapture %arr1, ptr noalias nocapture %arr2) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr1, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 10
  %arrayidx2 = getelementptr inbounds i32, ptr %arr2, i64 %indvars.iv
  store i32 %add, ptr %arrayidx2, align 4
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
