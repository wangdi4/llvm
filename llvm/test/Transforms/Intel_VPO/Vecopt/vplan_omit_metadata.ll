; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>,hir-cg' -vplan-force-vf=4 -hir-details -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR-CHECK
; RUN: opt -passes='vplan-vec' -vplan-force-vf=4 -vplan-build-vect-candidates=1 -print-before=vplan-vec -print-after=vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=LLVM-CHECK
; LIT test to check that metadata such as nonnull, dereferenceable,
; dereferenceable_or_null get dropped from the vector reference after
; vectorization.
define void @foo(ptr nocapture %arr) {
;                   *** IR Dump Before VPlan HIR Vectorizer (hir-vplan-vec) ***
; HIR-CHECK:                   {{.*}} = (%arr)[i1]
; HIR-CHECK:                         <RVAL-REG> {{.*}}(LINEAR ptr %arr){{.*}} !nonnull
;                   *** IR Dump After VPlan HIR Vectorizer (hir-vplan-vec) ***
; HIR-CHECK:                   {{.*}} = (<4 x ptr>*)(%arr)[i1]
; HIR-CHECK:                         <RVAL-REG> {{.*}}(<4 x ptr>*)(LINEAR ptr %arr)
; HIR-CHECK-NOT:   !nonnull
; HIR-CHECK-NOT:   !dereferenceable
; HIR-CHECK-SAME:  inbounds {{.*}}
;
;                    *** IR Dump Before VPlan Vectorizer (vplan-vec) ***
; LLVM-CHECK:         [[I1_04:%.*]] = phi i64 [ 0, {{.*}} ], [ [[INC:%.*]], {{.*}} ]
; LLVM-CHECK-NEXT:    [[ARRAYIDX:%.*]] = getelementptr inbounds ptr, ptr [[ARR:%.*]], i64 [[I1_04]]
; LLVM-CHECK-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[ARRAYIDX]], align 8, !nonnull !0, !dereferenceable !1, !dereferenceable_or_null !1
;                    *** IR Dump After VPlan Vectorizer (vplan-vec) ***
; LLVM-CHECK:         [[UNI_PHI:%uni.*]] = phi i64 [ 0, {{.*}} ], [ [[TMP3:%.*]], {{.*}} ]
; LLVM-CHECK:         [[SCALAR_GEP:%.*]] = getelementptr inbounds ptr, ptr [[ARR:%.*]], i64 [[UNI_PHI]]
; LLVM-CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <4 x ptr>, ptr [[SCALAR_GEP]]
; LLVM-CHECK-NOT:     !nonnull
; LLVM-CHECK-NOT:     !dereferenceable
; LLVM-CHECK-SAME:    align 8
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i1.04 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %arr, i64 %i1.04
  %0 = load ptr, ptr %arrayidx, align 8, !nonnull !0, !dereferenceable !1, !dereferenceable_or_null !1
  %add.ptr = getelementptr inbounds i64, ptr %0, i64 1
  store ptr %add.ptr, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %i1.04, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

!0 = !{}
!1 = !{i64 8}
