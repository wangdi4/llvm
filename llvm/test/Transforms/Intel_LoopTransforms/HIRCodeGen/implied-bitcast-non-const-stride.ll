;RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-cg -force-hir-cg -print-after=hir-cg 2>&1 %s | FileCheck %s
;RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -force-hir-cg -print-after=hir-cg 2>&1 %s | FileCheck %s

; Verify that we are able to generate code correctly in opaque ptr mode when
; there is an implied bitcast (i8* to i32*) between GEPs. First GEP was 
; emitted in i8 type because stride is non-const.

; CHECK: [[GEP1:%.*]] = getelementptr inbounds i8, ptr %arr, i64 {{.*}}
; CHECK:  = getelementptr inbounds i32, ptr [[GEP1]], i64 {{.*}}


define void @foo(ptr %arr, i64 %mul.2, i1 %rel.5, i64 %inner.tc, i64 %outer.tc) {
entry:
  br label %outer.loop

outer.loop:                                             ; preds = %bb26, %entry
  %indvars.iv12 = phi i64 [ 1, %entry ], [ %indvars.iv.next13, %bb26 ]
  br i1 %rel.5, label %bb26, label %inner.loop.preheader

inner.loop.preheader:                                   ; preds = %outer.loop
  %sub1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.2, ptr nonnull elementtype(i32) %arr, i64 %indvars.iv12)
  %t2 = trunc i64 %indvars.iv12 to i32
  br label %inner.loop

inner.loop:                                             ; preds = %inner.loop, %inner.loop.preheader
  %indvars.iv = phi i64 [ 1, %inner.loop.preheader ], [ %indvars.iv.next, %inner.loop ]
  %t5 = trunc i64 %indvars.iv to i32
  %sub2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %sub1, i64 %indvars.iv)
  store i32 %t2, ptr %sub2, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %inner.tc
  br i1 %exitcond, label %bb26.loopexit, label %inner.loop

bb26.loopexit:                                    ; preds = %inner.loop
  br label %bb26

bb26:                                             ; preds = %bb26.loopexit, %outer.loop
  %indvars.iv.next13 = add nuw nsw i64 %indvars.iv12, 1
  %exitcond17 = icmp eq i64 %indvars.iv.next13, %outer.tc
  br i1 %exitcond17, label %bb22.loopexit, label %outer.loop

bb22.loopexit:                                    ; preds = %bb26
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #2 = { nounwind readnone speculatable }
