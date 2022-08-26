; LIT test to check that we do not fold add operation when canon expression src and
; dest types are different.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -hir-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -aa-pipeline="basic-aa" -vplan-force-vf=4 -hir-details -disable-output < %s 2>&1 | FileCheck %s
; Incoming Scalar HIR:
;     DO i1 = 0, 79, 1   <DO_LOOP>
;       %0 = (@c1)[0][i1];
;       %1 = (@c2)[0][i1];
;       (@i1)[0][i1] = zext.i8.i32(%0) + zext.i8.i32(%1);
;     END LOOP

@c1 = dso_local local_unnamed_addr global [100 x i8] zeroinitializer, align 16
@c2 = dso_local local_unnamed_addr global [100 x i8] zeroinitializer, align 16
@i1 = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo() {
; CHECK:    DO i64 i1 = 0, 79, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:      [[DOTVEC0:%.*]] = (<4 x i8>*)(@c1)[0][i1]
; CHECK:      [[DOTVEC10:%.*]] = (<4 x i8>*)(@c2)[0][i1]
; CHECK:      [[DOTVEC20:%.*]] = [[DOTVEC0]]  +  [[DOTVEC10]]
; CHECK:      <LVAL-REG> NON-LINEAR <4 x i32> [[DOTVEC20]]
; CHECK:      <RVAL-REG> NON-LINEAR zext.<4 x i8>.<4 x i32>([[DOTVEC0]])
; CHECK:         <BLOB> NON-LINEAR <4 x i8> [[DOTVEC0]]
; CHECK:      <RVAL-REG> NON-LINEAR zext.<4 x i8>.<4 x i32>([[DOTVEC10]])
; CHECK:         <BLOB> NON-LINEAR <4 x i8> [[DOTVEC10]]
; CHECK:      (<4 x i32>*)(@i1)[0][i1] = [[DOTVEC20]]
; CHECK:    END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.011 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i8], [100 x i8]* @c1, i64 0, i64 %l1.011
  %0 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx1 = getelementptr inbounds [100 x i8], [100 x i8]* @c2, i64 0, i64 %l1.011
  %1 = load i8, i8* %arrayidx1, align 1
  %conv2 = zext i8 %1 to i32
  %add = add nuw nsw i32 %conv2, %conv
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @i1, i64 0, i64 %l1.011
  store i32 %add, i32* %arrayidx3, align 4
  %inc = add nuw nsw i64 %l1.011, 1
  %exitcond = icmp eq i64 %inc, 80
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
