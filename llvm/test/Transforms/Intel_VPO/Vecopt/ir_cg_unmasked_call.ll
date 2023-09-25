; RUN: opt -passes="vplan-vec" -S < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i1 @some_cond(i64)

declare i1 @ballot(i1) #0
declare <2 x i8> @ballot.vec(<2 x i8>, <2 x i8>)

; Exact type for the active mask will depend on the library implementation, but
; it doesn't matter for the VPlan implementation as long as the library
; implementation for ballot is in sync with that of the first argument (mask)
; for the unmasked functions.
declare void @unmasked_scalar(i1, i64) #1
declare void @unmasked_vector(<2 x i8>, <2 x i64>)

define void @test() local_unnamed_addr #1 {
; CHECK-LABEL: @test(
; CHECK:         [[TMP1:%.*]] = call i1 @some_cond(i64 [[UNI_PHI3:%.*]])
; CHECK-NEXT:    [[TMP2:%.*]] = insertelement <2 x i1> undef, i1 [[TMP1]], i32 0
; CHECK-NEXT:    [[TMP3:%.*]] = call i1 @some_cond(i64 [[VEC_PHI_EXTRACT_1_:%.*]])
; CHECK-NEXT:    [[TMP4:%.*]] = insertelement <2 x i1> [[TMP2]], i1 [[TMP3]], i32 1
; CHECK-NEXT:    br label [[VPLANNEDBB4:%.*]]
; CHECK:       VPlannedBB3:
; CHECK-NEXT:    [[MASKEXT:%.*]] = sext <2 x i1> %3 to <2 x i8>
; CHECK-NEXT:    [[TMP5:%.*]] = call <2 x i8> @ballot.vec(<2 x i8> <i8 1, i8 1>, <2 x i8> [[MASKEXT]])
; CHECK-NEXT:    [[TMP6:%.*]] = trunc <2 x i8> [[TMP5]] to <2 x i1>
; CHECK-NEXT:    [[TMP7:%.*]] = bitcast <2 x i1> [[TMP4]] to i2
; CHECK-NEXT:    [[TMP8:%.*]] = icmp ne i2 [[TMP7]], 0
; CHECK-NEXT:    [[TMP9:%.*]] = zext <2 x i1> [[TMP6]] to <2 x i8>
; CHECK-NEXT:    br i1 [[TMP8]], label [[PRED_CALL_IF:%.*]], label %[[TMP10:.*]]
; CHECK:       pred.call.if:
; CHECK-NEXT:    call void @unmasked_vector(<2 x i8> [[TMP9]], <2 x i64> [[VEC_PHI:%.*]])
; CHECK-NEXT:    br label %[[TMP10]]
; CHECK:       [[TMP10]]:
; CHECK-NEXT:    br label [[PRED_CALL_CONTINUE:%.*]]
; CHECK:       pred.call.continue:
; CHECK-NEXT:    br label [[VPLANNEDBB5:%.*]]
; CHECK:       VPlannedBB4:
; CHECK-NEXT:    [[TMP11:%.*]] = add nuw nsw <2 x i64> [[VEC_PHI]], <i64 2, i64 2>
; CHECK-NEXT:    [[TMP12:%.*]] = add nuw nsw i64 [[UNI_PHI3]], 2
; CHECK-NEXT:    [[TMP13:%.*]] = icmp uge i64 [[TMP12]], 2
; CHECK-NEXT:    br i1 true, label [[VPLANNEDBB6:%.*]], label [[VECTOR_BODY:%.*]], [[LOOP0:!llvm.loop !.*]]
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %cond = call i1 @some_cond(i64 %iv)
  br i1 %cond, label %if.then, label %latch

if.then:
  %active_mask = call i1 @ballot(i1 true)
  call void @unmasked_scalar(i1 %active_mask, i64 %iv)
  br label %latch

latch:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 2
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbM2v_ballot(ballot.vec)" "opencl-vec-uniform-return" }
attributes #1 = { "vector-variants"="_ZGVbN2vv_unmasked_scalar(unmasked_vector)" "unmasked" }
