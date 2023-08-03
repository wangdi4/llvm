; Test to check that VPlan clears wrap flags for instructions participating
; in reduction idioms.

; REQUIRES: asserts
; RUN: opt -passes=vplan-vec -vplan-print-after-clear-redn-wrap-flags -vplan-dump-details -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

define i32 @foo(ptr noalias nocapture %A, ptr noalias nocapture %B, i32 %inc) {
;
; CHECK-LABEL:  VPlan after clearing wrap flags for reductions:
; CHECK-NEXT:  VPlan IR for: foo:loop.#{{[0-9]+}}
; CHECK:         [[BB1:BB[0-9]+]]: # preds: [[BB0:BB[0-9]+]]
; CHECK-NEXT:     i32 [[VP_REDURED_INIT:%.*]] = reduction-init i32 0 i32 0
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

; CHECK:         [[BB2:BB[0-9]+]]: # preds: [[BB1]], [[BB2]]
; CHECK:          i32 [[VP_REDU:%.*]] = phi  [ i32 [[VP_REDURED_INIT]], [[BB1]] ],  [ i32 [[VP0:%.*]], [[BB2]] ]
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

; CHECK:          i32 [[VP3:%.*]] = add i32 [[VP_REDU]] i32 [[VP1:%vp.*]]
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

; CHECK:          i32 [[VP0]] = add i32 [[VP3]] i32 [[VP2:%vp.*]]
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

; CHECK:         [[BB3:BB[0-9]+]]: # preds: [[BB2]]
; CHECK-NEXT:     i32 [[VP_REDURED_FINAL:%.*]] = reduction-final{u_add} i32 [[VP0]]
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details
;
; CHECK:  define i32 @foo
; CHECK:       vector.body:
; CHECK:         [[VEC_PHI30:%.*]] = phi <4 x i32> [ zeroinitializer, [[VPLANNEDBB10:%.*]] ], [ [[TMP3:%.*]], [[VECTOR_BODY0:%.*]] ]
; CHECK:         [[TMP2:%.*]] = add <4 x i32> [[VEC_PHI30]], [[WIDE_LOAD0:%.*]]
; CHECK-NEXT:    [[TMP3]] = add <4 x i32> [[TMP2]], [[WIDE_LOAD50:%.*]]
;
entry:
  br label %simd.begin

simd.begin:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop

loop:
  %iv = phi i64 [0, %simd.begin], [%iv_inc, %loop]
  %redu = phi i32 [0, %simd.begin], [%3, %loop]
  %gepa = getelementptr inbounds i32, ptr %A, i64 %iv
  %gepb = getelementptr inbounds i32, ptr %B, i64 %iv
  %0 = load i32, ptr %gepa
  %1 = load i32, ptr %gepb
  %2 = add nuw nsw i32 %redu, %0
  %3 = add nuw nsw i32 %2, %1
  %iv_inc = add nuw nsw i64 %iv, 1
  %4 = icmp ult i64 %iv_inc, 1024
  br i1 %4, label %loop, label %exit

exit:
  %lcssa = phi i32 [%3, %loop]
  br label %simd.exit

simd.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %fin

fin:
  ret i32 %lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
