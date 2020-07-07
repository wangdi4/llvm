; This test checks that insertvalue and extractvalue instructions are correctly
; handled (serialized) by the vectorizer.
;
; RUN: opt -VPlanDriver -vplan-force-vf=2 -S %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare dso_local { double, double } @bar1(i64) local_unnamed_addr
declare dso_local { double, <4 x i32>, [10 x i32]} @bar2(i64) local_unnamed_addr


define void @test_pointer_induction_escape() {
;
; CHECK:       vector.body:
; CHECK:         [[VEC_PHI_EXTRACT_1_0:%.*]] = extractelement <2 x i64> [[VEC_PHI0:%.*]], i32 1
; CHECK-NEXT:    [[TMP1:%.*]] = call { double, double } @bar1(i64 [[UNI_PHI0:%.*]])
; CHECK-NEXT:    [[TMP2:%.*]] = call { double, double } @bar1(i64 [[VEC_PHI_EXTRACT_1_0]])
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE0:%.*]] = extractvalue { double, double } [[TMP1]], 0
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE10:%.*]] = extractvalue { double, double } [[TMP2]], 0
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE20:%.*]] = extractvalue { double, double } [[TMP1]], 1
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE30:%.*]] = extractvalue { double, double } [[TMP2]], 1
; CHECK-NEXT:    [[TMP3:%.*]] = call { double, <4 x i32>, [10 x i32] } @bar2(i64 [[UNI_PHI0]])
; CHECK-NEXT:    [[TMP4:%.*]] = call { double, <4 x i32>, [10 x i32] } @bar2(i64 [[VEC_PHI_EXTRACT_1_0]])
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE40:%.*]] = extractvalue { double, <4 x i32>, [10 x i32] } [[TMP3]], 1
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE50:%.*]] = extractvalue { double, <4 x i32>, [10 x i32] } [[TMP4]], 1
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE60:%.*]] = extractvalue { double, <4 x i32>, [10 x i32] } [[TMP3]], 2
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE70:%.*]] = extractvalue { double, <4 x i32>, [10 x i32] } [[TMP4]], 2
; CHECK-NEXT:    [[SERIAL_INSERTVALUE0:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } { i64 1, double undef, <4 x i32> undef, [10 x i32] undef }, <4 x i32> [[SERIAL_EXTRACTVALUE40]], 2
; CHECK-NEXT:    [[SERIAL_INSERTVALUE80:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } { i64 1, double undef, <4 x i32> undef, [10 x i32] undef }, <4 x i32> [[SERIAL_EXTRACTVALUE50]], 2
; CHECK-NEXT:    [[SERIAL_INSERTVALUE90:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } [[SERIAL_INSERTVALUE0]], [10 x i32] [[SERIAL_EXTRACTVALUE60]], 3
; CHECK-NEXT:    [[SERIAL_INSERTVALUE100:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } [[SERIAL_INSERTVALUE80]], [10 x i32] [[SERIAL_EXTRACTVALUE70]], 3
; CHECK-NEXT:    [[TMP5:%.*]] = icmp eq <2 x i64> [[VEC_PHI0]], <i64 64, i64 64>
; CHECK-NEXT:    [[TMP6:%.*]] = add <2 x i64> [[VEC_PHI0]], <i64 2, i64 2>
; CHECK-NEXT:    [[DOTEXTRACT_1_0:%.*]] = extractelement <2 x i64> [[TMP6]], i32 1
; CHECK-NEXT:    [[DOTEXTRACT_0_0:%.*]] = extractelement <2 x i64> [[TMP6]], i32 0
; CHECK-NEXT:    [[PREDICATE0:%.*]] = extractelement <2 x i1> [[TMP5]], i64 0
; CHECK-NEXT:    [[TMP7:%.*]] = icmp eq i1 [[PREDICATE0]], true
; CHECK-NEXT:    br i1 [[TMP7]], label %[[PRED_CALL_IF0:.*]], label %[[TMP9:.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[PRED_CALL_IF0]]:
; CHECK-NEXT:    [[TMP8:%.*]] = call { double, double } @bar1(i64 [[DOTEXTRACT_0_0]])
; CHECK-NEXT:    br label %[[TMP9]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[TMP9]]:
; CHECK-NEXT:    [[TMP10:%.*]] = phi { double, double } [ undef, %vector.body ], [ [[TMP8]], %[[PRED_CALL_IF0]] ]
; CHECK-NEXT:    br label %[[PRED_CALL_CONTINUE0:.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[PRED_CALL_CONTINUE0]]:
; CHECK-NEXT:    [[PREDICATE110:%.*]] = extractelement <2 x i1> [[TMP5]], i64 1
; CHECK-NEXT:    [[TMP11:%.*]] = icmp eq i1 [[PREDICATE110]], true
; CHECK-NEXT:    br i1 [[TMP11]], label %[[PRED_CALL_IF170:.*]], label %[[TMP13:.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[PRED_CALL_IF170]]:
; CHECK-NEXT:    [[TMP12:%.*]] = call { double, double } @bar1(i64 [[DOTEXTRACT_1_0]])
; CHECK-NEXT:    br label %[[TMP13]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[TMP13]]:
; CHECK-NEXT:    [[TMP14:%.*]] = phi { double, double } [ undef, %[[PRED_CALL_CONTINUE0]] ], [ [[TMP12]], %[[PRED_CALL_IF170]] ]
; CHECK-NEXT:    br label %[[PRED_CALL_CONTINUE180:.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  [[PRED_CALL_CONTINUE180]]:
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE120:%.*]] = extractvalue { double, double } [[TMP10]], 1
; CHECK-NEXT:    [[SERIAL_EXTRACTVALUE130:%.*]] = extractvalue { double, double } [[TMP14]], 1
; CHECK-NEXT:    [[SERIAL_INSERTVALUE140:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } [[SERIAL_INSERTVALUE0]], double [[SERIAL_EXTRACTVALUE120]], 1
; CHECK-NEXT:    [[SERIAL_INSERTVALUE150:%.*]] = insertvalue { i64, double, <4 x i32>, [10 x i32] } [[SERIAL_INSERTVALUE80]], double [[SERIAL_EXTRACTVALUE130]], 1
;
  br label %simd.begin.region
simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %simd.loop
simd.loop:
  %iv.current = phi i64 [ 0, %simd.begin.region], [%iv.next, %simd.loop.end]
  %call1 = tail call { double, double } @bar1(i64 %iv.current)

  ; Test extractvalue
  %e1 = extractvalue { double, double } %call1, 0
  %e2 = extractvalue { double, double } %call1, 1
  %call2 = tail call { double, <4 x i32>, [10 x i32]} @bar2(i64 %iv.current)
  %e.vec = extractvalue { double, <4 x i32>, [10 x i32]} %call2, 1
  %e.arr = extractvalue { double, <4 x i32>, [10 x i32]} %call2, 2

  ; Test insertvalue
  %agg1 = insertvalue {i64 , double, <4 x i32>, [10 x i32]} undef, i64 1, 0
  %agg2 = insertvalue {i64 , double, <4 x i32>, [10 x i32]} %agg1, <4 x i32> %e.vec, 2
  %agg3 = insertvalue {i64 , double, <4 x i32>, [10 x i32]} %agg2, [10 x i32] %e.arr, 3

  ;branch into an if-part.
  %cmp = icmp eq i64 %iv.current, 64
  br i1 %cmp, label %block.iv.64, label %next

block.iv.64:
  %lookahead = add i64%iv.current, 2
  %call3 = tail call { double, double } @bar1(i64 %lookahead)
  %e3 = extractvalue { double, double } %call3, 1
  %agg4 = insertvalue {i64 , double, <4 x i32>, [10 x i32]} %agg2, double %e3, 1
  br label %next

next:
  %iv.next = add nuw nsw i64 %iv.current, 1
  %icmp = icmp eq i64 %iv.next, 10
  br label %simd.loop.end
simd.loop.end:
  br i1 %icmp, label %simd.end.region, label %simd.loop
simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.end
for.end:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

