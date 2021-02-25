; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -print-after=hir-vec-dir-insert \
; RUN:     -disable-output -vplan-force-vf=4 2>&1 | FileCheck %s --check-prefix=CHECK-DIRECTIVE

; RUN: opt -S < %s -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -instcombine -vplan-force-vf=4 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir,hir-cg,instcombine" -S < %s -vplan-force-vf=4 | FileCheck %s



@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.3 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

declare void @llvm.assume(i1)

define void @doit(i1 %arg) local_unnamed_addr #0 {
; Ensure that call to @llvm.assume does not prevent the insertion of vec directives.
; CHECK-DIRECTIVE-LABEL: BEGIN REGION
; CHECK-DIRECTIVE-NEXT: @llvm.directive.region.entry


; CHECK-LABEL: @doit
;
; FIXME: We currently ignore @llvm.assume calls completely inside VPlan HIR CG.
; It'll be better to scalarize them instead so that later passes would have a
; chance to use the info the assumes provide.
; CHECK-NOT: @llvm.assume
;
; Verify that vectorization happened:
; CHECK: load <4 x i32>
; CHECK: load <4 x i32>
; CHECK: store <4 x i32>
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
