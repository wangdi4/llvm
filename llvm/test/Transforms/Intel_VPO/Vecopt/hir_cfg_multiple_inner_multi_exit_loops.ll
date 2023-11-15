; Test soft bail out for the case with several multi exit inner loops.

; Incoming HIR
; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; 
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %ld = (%arr)[i1];
;       |   %storemerge.lcssa = 0;
;       |   if (%ld == 42)
;       |   {
;       |
;       |      + DO i2 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
;       |      |   if (i2 == 5)
;       |      |   {
;       |      |      goto inner.early.exit;
;       |      |   }
;       |      + END LOOP
;       |
;       |
;       |      + DO i2 = 0, 89, 1   <DO_MULTI_EXIT_LOOP>
;       |      |   if (i2 == 10)
;       |      |   {
;       |      |      goto inner2.early.exit;
;       |      |   }
;       |      + END LOOP
;       |
;       |      %storemerge.lcssa = 1;
;       |      goto outer.latch;
;       |      inner2.early.exit:
;       |      %storemerge.lcssa = 2;
;       |      goto outer.latch;
;       |      inner.early.exit:
;       |      %storemerge.lcssa = 2;
;       |   }
;       |   outer.latch:
;       |   %conv44 = sitofp.i32.float(%storemerge.lcssa);
;       + END LOOP
; 
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION

; REQUIRES: asserts
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec -disable-output -debug-only=VPlanDriver < %s 2>&1 | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI

; CHECK: VD: Not vectorizing: Cannot support multiple multi-exit loops.
; OPTRPTHI: remark #15436: loop was not vectorized: HIR: Cannot support more than one multiple-exit loop.

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define void @foo(ptr %arr) {
DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.ph

outer.ph:
  br label %outer.body

outer.body:
  %outer.iv = phi i64 [ 0, %outer.ph ], [ %outer.iv.next, %outer.latch ]
  %gep = getelementptr inbounds i32, ptr %arr, i64 %outer.iv
  %ld = load i32, ptr %gep, align 4
  %cmp = icmp eq i32 %ld, 42
  br i1 %cmp, label %inner.ph, label %outer.latch

inner.ph:
  br label %inner.body

inner.body:
  %inner.iv = phi i32 [ 0, %inner.ph ], [ %inner.iv.next, %inner.latch ]
  %cmp2 = icmp eq i32 %inner.iv, 5
  br i1 %cmp2, label %inner.early.exit, label %inner.latch

inner.latch:
  %inner.iv.next = add nuw nsw i32 %inner.iv, 1
  %exitcond.not = icmp eq i32 %inner.iv.next, 50
  br i1 %exitcond.not, label %inner.exit, label %inner.body

inner.early.exit:
  br label %outer.latch

inner.exit:
  br label %inner2.ph

inner2.ph:
  br label %inner2.body

inner2.body:
  %inner2.iv = phi i32 [ 0, %inner2.ph ], [ %inner2.iv.next, %inner2.latch ]
  %cmp3 = icmp eq i32 %inner2.iv, 10
  br i1 %cmp3, label %inner2.early.exit, label %inner2.latch

inner2.latch:
  %inner2.iv.next = add nuw nsw i32 %inner2.iv, 1
  %exitcond2.not = icmp eq i32 %inner2.iv.next, 90
  br i1 %exitcond2.not, label %inner2.exit, label %inner2.body

inner2.early.exit:
  br label %outer.latch

inner2.exit:
  br label %outer.latch

outer.latch:
  %storemerge.lcssa = phi i32 [ 2, %inner.early.exit ], [ 2, %inner2.early.exit], [ 1, %inner2.exit ], [ 0, %outer.body ]
  %conv44 = sitofp i32 %storemerge.lcssa to float
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %exitcond35.not = icmp eq i64 %outer.iv.next, 100
  br i1 %exitcond35.not, label %outer.exit, label %outer.body

outer.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
