; Test that VPlan CFG builder is able to handle a loop nest where
; inner loop early exits to outer loop.

; Incoming HIR
;  BEGIN REGION { }
;        %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;
;        + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;        |   %ld = (%arr)[i1];
;        |   %storemerge.lcssa = 0;
;        |   if (%ld == 42)
;        |   {
;        |      + DO i2 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
;        |      |   if (i2 == 5)
;        |      |   {
;        |      |      goto inner.early.exit;
;        |      |   }
;        |      + END LOOP
;        |
;        |      %storemerge.lcssa = 1;
;        |      goto outer.latch;
;        |      inner.early.exit:
;        |      %storemerge.lcssa = 2;
;        |   }
;        |   outer.latch:
;        |   %conv44 = sitofp.i32.float(%storemerge.lcssa);
;        + END LOOP
;
;        @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;        ret ;
;  END REGION

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -vplan-print-after-hir-decomposer < %s 2>&1 | FileCheck %s

; CHECK-LABEL: VPlan after VPlanHIRDecomposer:
; CHECK:      External Defs Start:
; CHECK-DAG:   [[ARR:%.*]] = {%arr}
; CHECK-DAG:   [[STOREMERGE:%.*]] = {%storemerge.lcssa}
; CHECK-NEXT: External Defs End:
; CHECK-NEXT:   [[BB1:BB[0-9]+]]: # preds:
; CHECK-NEXT:    br [[BB2:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB2]]: # preds: [[BB1]]
; CHECK-NEXT:    br [[BB3:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB3]]: # preds: [[BB2]], [[BB11:BB[0-9]+]]
; CHECK-NEXT:    i64 [[IV1:%.*]] = phi  [ i64 0, [[BB2]] ],  [ i64 [[IV1_NEXT:%.*]], [[BB11]] ]
; CHECK-NEXT:    i32* [[GEP:%.*]] = subscript inbounds i32* %arr i64 [[IV1]]
; CHECK-NEXT:    i32 [[LD:%.*]] = load i32* [[GEP]]
; CHECK-NEXT:    i32 [[COPY0:%.*]] = hir-copy i32 0 , OriginPhiId: -1
; CHECK-NEXT:    i1 [[CMP1:%.*]] = icmp eq i32 [[LD]] i32 42
; CHECK-NEXT:    br i1 [[CMP1]], [[BB4:BB[0-9]+]], [[BB11]]
; CHECK-EMPTY:
; CHECK-NEXT:     [[BB4]]: # preds: [[BB3]]
; CHECK-NEXT:      br [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:     [[BB5]]: # preds: [[BB4]]
; CHECK-NEXT:      br [[BB7:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:     [[BB7]]: # preds: [[BB5]], [[BB10:BB[0-9]+]]
; CHECK-NEXT:      i32 [[IV2:%.*]] = phi  [ i32 0, [[BB5]] ],  [ i32 [[IV2_NEXT:%.*]], [[BB10]] ]
; CHECK-NEXT:      i1 [[CMP2:%.*]] = icmp eq i32 [[IV2]] i32 5
; CHECK-NEXT:      br i1 [[CMP2]], [[BB8:BB[0-9]+]], [[BB10]]
; CHECK-EMPTY:
; CHECK-NEXT:       [[BB10]]: # preds: [[BB7]]
; CHECK-NEXT:        i32 [[IV2_NEXT:%.*]] = add i32 [[IV2]] i32 1
; CHECK-NEXT:        i1 [[CMP3:%.*]] = icmp slt i32 [[IV2_NEXT]] i32 50
; CHECK-NEXT:        br i1 [[CMP3]], [[BB7]], [[BB11:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:       [[BB11]]: # preds: [[BB10]]
; CHECK-NEXT:        br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:       [[BB6]]: # preds: [[BB11]]
; CHECK-NEXT:        i32 [[COPY1:%.*]] = hir-copy i32 1 , OriginPhiId: -1
; CHECK-NEXT:        br <External Basic Block>
; CHECK-EMPTY:
; CHECK-NEXT:     [[BB8]]: # preds: [[BB7]]
; CHECK-NEXT:      br <External Basic Block>
; CHECK-EMPTY:
; CHECK-NEXT:     [[BB9:BB[0-9]+]]: # preds: [[BB8]]
; CHECK-NEXT:      i32 [[COPY2:%.*]] = hir-copy i32 2 , OriginPhiId: -1
; CHECK-NEXT:      br [[BB12:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB12]]: # preds: [[BB6]], [[BB9]], [[BB3]]
; CHECK-NEXT:    i32 [[PHI:%.*]] = phi [ i32 [[COPY2]], [[BB9]] ], [ i32 [[COPY1]], [[BB6]] ], [ i32 [[COPY0]], [[BB3]] ]
; CHECK-NEXT:    float [[CONV:%.*]] = sitofp i32 [[PHI]] to float
; CHECK-NEXT:    i64 [[IV1_NEXT:%.*]] = add i64 [[IV1]] i64 1
; CHECK-NEXT:    i1 [[CMP4:%.*]] = icmp slt i64 [[IV1_NEXT]] i64 100
; CHECK-NEXT:    br i1 [[CMP4]], [[BB3]], [[BB13:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB13]]: # preds: [[BB12]]
; CHECK-NEXT:    br [[BB14:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:   [[BB14]]: # preds: [[BB13]]
; CHECK-NEXT:    br <External Block>

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

define void @foo(i32* %arr) {
DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.ph

outer.ph:                                         ; preds = %DIR.OMP.SIMD.1
  br label %outer.body

outer.body:                                       ; preds = %outer.latch, %outer.ph
  %outer.iv = phi i64 [ 0, %outer.ph ], [ %outer.iv.next, %outer.latch ]
  %gep = getelementptr inbounds i32, i32* %arr, i64 %outer.iv
  %ld = load i32, i32* %gep, align 4
  %cmp = icmp eq i32 %ld, 42
  br i1 %cmp, label %inner.ph, label %outer.latch

inner.ph:                                         ; preds = %outer.body
  br label %inner.body

inner.body:                                       ; preds = %inner.latch, %inner.ph
  %inner.iv = phi i32 [ 0, %inner.ph ], [ %inner.iv.next, %inner.latch ]
  %cmp2 = icmp eq i32 %inner.iv, 5
  br i1 %cmp2, label %inner.early.exit, label %inner.latch

inner.latch:                                      ; preds = %inner.body
  %inner.iv.next = add nuw nsw i32 %inner.iv, 1
  %exitcond.not = icmp eq i32 %inner.iv.next, 50
  br i1 %exitcond.not, label %inner.exit, label %inner.body

inner.early.exit:                                 ; preds = %inner.body
  br label %outer.latch

inner.exit:                                       ; preds = %inner.latch
  br label %outer.latch

outer.latch:                                      ; preds = %inner.exit, %inner.early.exit, %outer.body
  %storemerge.lcssa = phi i32 [ 2, %inner.early.exit ], [ 1, %inner.exit ], [ 0, %outer.body ]
  %conv44 = sitofp i32 %storemerge.lcssa to float
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %exitcond35.not = icmp eq i64 %outer.iv.next, 100
  br i1 %exitcond35.not, label %outer.exit, label %outer.body

outer.exit:                                       ; preds = %outer.latch
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
