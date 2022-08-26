; Test to check handling of @llvm.assume call in VPlan HIR vectorizer.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vec-dir-insert -print-after=hir-vplan-vec -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; Ensure that call to @llvm.assume does not prevent the insertion of vec directives.
; CHECK-LABEL: BEGIN REGION { }
; CHECK-NEXT:  @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK:       END REGION

; Ensure that call to @llvm.assume is serialized by vectorizer.
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:         + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:         |   @llvm.assume(%arg);
; CHECK-NEXT:         |   %.vec = (<4 x i32>*)(@arr.i32.1)[0][i1];
; CHECK-NEXT:         |   %.vec1 = (<4 x i32>*)(@arr.i32.3)[0][i1];
; CHECK-NEXT:         |   (<4 x i32>*)(@arr.i32.2)[0][i1] = %.vec + %.vec1;
; CHECK-NEXT:         + END LOOP
; CHECK:        END REGION

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.3 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

declare void @llvm.assume(i1)

define void @doit(i1 %arg) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ld.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %indvars.iv

  tail call void @llvm.assume(i1 %arg)

  %ld = load i32, i32* %ld.idx

  %ld2.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.3, i64 0, i64 %indvars.iv
  %ld2 = load i32, i32* %ld2.idx

  %add = add nsw i32 %ld, %ld2

  %st.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %indvars.iv
  store i32 %add, i32* %st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
