; This test verifies that we fixup the inner loop exit condition to check
; for the case where the inner loop should not be entered.
; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -debug-only=VPlanPredicator 2>&1 | FileCheck %s
;
; The expected VPlan dump looks like the following after predication.
;  REGION: region1 (BP: NULL)
;  BB9 (BP: NULL) :
;   <Empty Block>
;  SUCCESSORS(1):loop14
;  no PREDECESSORS
;
;  REGION: loop14 (BP: NULL)
;  BB2 (BP: NULL) :
;   <Empty Block>
;  SUCCESSORS(1):BB3
;  no PREDECESSORS
;
;  BB3 (BP: NULL) :
;   i64 %vp51392 = phi  [ i64 %vp55984, BB12 ],  [ i64 0, BB2 ]
;   i32 %vp52640 = trunc i64 %vp51392 to i32
;  SUCCESSORS(1):region16
;  PREDECESSORS(2): BB12 BB2
;
;  REGION: region16 (BP: NULL)
;  BB11 (BP: NULL) :
;   <Empty Block>
;  SUCCESSORS(1):loop15
;  no PREDECESSORS
;
;  REGION: loop15 (BP: NULL)
;  BB4 (BP: NULL) :
;   i1 %vp4352 = block-predicate i1 %cmp1
;  SUCCESSORS(1):BB6
;  no PREDECESSORS
;
;  BB6 (BP: NULL) :
;   i32 %vp53680 = phi  [ i32 %vp55184, BB13 ],  [ i32 0, BB4 ]
;   i1 %vp51808 = block-predicate i1 %cmp1
;   i32 %vp54928 = call i32 (...)* @baz
;   i32 %vp55184 = add i32 %vp53680 i32 1
;   i1 %vp55440 = icmp i32 %vp55184 i32 1024
;  SUCCESSORS(1):BB13
;  PREDECESSORS(2): BB13 BB4
;
;  BB13 (BP: NULL) :
;   i1 %vp52016 = block-predicate i1 %cmp1
;   i1 %vp8608 = all-zero-check i1 %cmp1
;   i1 %vp8784 = not i1 %vp8608
;   i1 %vp8960 = and i1 %vp8784 i1 %vp55440
;  SUCCESSORS(2):BB6(i1 %vp8960), BB7(!i1 %vp8960)
;  PREDECESSORS(1): BB6
;
;  BB7 (BP: NULL) :
;   i1 %vp1088 = block-predicate i1 %cmp1
;  no SUCCESSORS
;  PREDECESSORS(1): BB13
;
;  SUCCESSORS(1):BB5
;  END Region(loop15)
;
;  BB5 (BP: NULL) :
;   i1 %vp51600 = not i1 %cmp1
;   i64 %vp55984 = add i64 %vp51392 i64 1
;   i1 %vp56240 = icmp i64 %vp55984 i64 1024
;  no SUCCESSORS
;  PREDECESSORS(1): loop15
;
;  SUCCESSORS(1):BB12
;  END Region(region16)
;
;  BB12 (BP: NULL) :
;   <Empty Block>
;   Condition(BB5): i1 %vp56240 = icmp i64 %vp55984 i64 1024
;  SUCCESSORS(2):BB8(i1 %vp56240), BB3(!i1 %vp56240)
;  PREDECESSORS(1): region16
;
;  BB8 (BP: NULL) :
;   <Empty Block>
;  no SUCCESSORS
;  PREDECESSORS(1): BB12
;
;  SUCCESSORS(1):BB10
;  END Region(loop14)
;
;  BB10 (BP: NULL) :
;   <Empty Block>
;  no SUCCESSORS
;  PREDECESSORS(1): loop14
;
;  END Region(region1)
;
; CHECK-LABEL: VPlan IR for: Predicator: After predication
; CHECK: REGION: region{{.*}}
; CHECK: REGION: loop{{.*}}
; CHECK: REGION: [[INNERLOOP:loop[0-9]+]]
; CHECK-NEXT: BB{{.*}}
; CHECK-NEXT: i1 %vp{{.*}} = block-predicate i1 [[PHPRED:%.*]]
; CHECK: [[INNERHEADER:BB[0-9]+]] (BP: {{.*}})
; CHECK: i32 {{.*}} = call {{.*}}* @baz
; CHECK-NEXT: i32 [[INDINC:.*]] = add i32 %vp{{.*}} i32 1
; CHECK-NEXT: i1 [[EXITCOND:.*]] = icmp i32 [[INDINC]] i32 1024
; CHECK: BB{{[0-9]+}} (BP: {{.*}})
; CHECK-NEXT: i1 {{.*}} = block-predicate i1 [[PHPRED]]
; CHECK-NEXT: i1 [[ALLZERO:.*]] = all-zero-check i1 [[PHPRED]]
; CHECK-NEXT: i1 [[NOTALLZERO:.*]] = not i1 [[ALLZERO]]
; CHECK-NEXT: i1 [[NEWEXITCOND:.*]] = and i1 [[NOTALLZERO]] i1 [[EXITCOND]]
; CHECK-NEXT: SUCCESSORS(2):[[INNERHEADER]](i1 [[NEWEXITCOND]]), BB{{[0-9]+}}(!i1 [[NEWEXITCOND]])
; CHECK: END Region([[INNERLOOP]]

define void @foo(i32 %n1, i32 %n2) {
outer.for.lr.ph:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %outer.for.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %cmp1 = icmp sgt i32 %n1, %n2
  br label %outer.for

outer.for:                               ; preds = %outer.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %outer.for.inc ], [ 0, %DIR.OMP.SIMD.2 ]
  %1 = trunc i64 %indvars.iv to i32
  br i1 %cmp1, label %for.body.preheader, label %outer.for.inc

for.body.preheader:                               ; preds = %outer.for
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i1.014 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %call = call i32 (...) @baz() #1
  %inc = add nuw nsw i32 %i1.014, 1
  %exitcond = icmp ult i32 %inc, 1024
  br i1 %exitcond, label %for.body, label %outer.for.inc.loopexit

outer.for.inc.loopexit:                       ; preds = %for.body
  br label %outer.for.inc

outer.for.inc:                                ; preds = %outer.for.inc.loopexit, %outer.for
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond15 = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond15, label %DIR.OMP.END.SIMD.4, label %outer.for

DIR.OMP.END.SIMD.4:                               ; preds = %outer.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @baz(...)
