; Verify that input IR is not changed in case VF=1 is selected.

; We don't have auto-vectorization for the LLVM-IR path yet, so we need an
; explicit SIMD region which in turn requires hir-cg/VPODirectiveCleanup for the
; HIR-path to re-use the exact same checks.

; RUN: opt -S < %s -vplan-vec -vplan-force-vf=1 | FileCheck %s
; RUN: opt -S < %s -passes="vplan-vec" -vplan-force-vf=1 | FileCheck %s
; RUN: opt -S < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:        -hir-cg  -vplan-force-vf=1 | FileCheck %s
; RUN: opt -S < %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg" \
; RUN:        -vplan-force-vf=1 | FileCheck %s

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define void @doit() local_unnamed_addr #0 {
; CHECK-LABEL: @doit(
; CHECK-NEXT:  entry:
; CHECK:         [[TOK:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
; CHECK:         br label [[FOR_BODY:%.*]]
; CHECK:       for.body:
; CHECK-NEXT:    [[INDVARS_IV:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[INDVARS_IV_NEXT:%.*]], [[FOR_BODY]] ]
; CHECK-NEXT:    [[LD_IDX:%.*]] = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 [[INDVARS_IV]]
; CHECK-NEXT:    [[LD:%.*]] = load i32, i32* [[LD_IDX]]
; CHECK-NEXT:    [[ST_IDX:%.*]] = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 [[INDVARS_IV]]
; CHECK-NEXT:    store i32 [[LD]], i32* [[ST_IDX]]
; CHECK-NEXT:    [[INDVARS_IV_NEXT]] = add nuw nsw i64 [[INDVARS_IV]], 1
; CHECK-NEXT:    [[EXITCOND:%.*]] = icmp eq i64 [[INDVARS_IV_NEXT]], 1024
; CHECK:         br i1 [[EXITCOND]], label [[FOR_END:%.*]], label [[FOR_BODY]]
; CHECK:       for.end:
; CHECK:         call void @llvm.directive.region.exit(token [[TOK:%.*]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK:         ret void
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ld.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %indvars.iv
  %ld = load i32, i32* %ld.idx
  %st.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %indvars.iv
  store i32 %ld, i32* %st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
