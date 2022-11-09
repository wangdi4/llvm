; Check that VPlan's CallVecDecisions doesn't choose vector library based
; vectorization scenario for an invalid library function call. If the
; prototype of called function doesn't match standard library version,
; then we should serialize the call instead of vectorizing with vector
; library.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vector-library=SVML -print-after=hir-vplan-vec -vplan-print-after-call-vec-decisions -disable-output %s 2>&1 | FileCheck %s

; CHECK: VPlan after CallVecDecisions analysis
; CHECK:   [DA: Div] float {{%.*}} = call float* {{%.*}} float* {{%.*}} float (...)* @atan2pi [Serial]

; CHECK:         + DO i1 = 0, 35, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:    |   %.copy = 0.000000e+00;
; CHECK-NEXT:    |   %.vec = i1 + <i64 0, i64 1> + 1 >u 14;
; CHECK-NEXT:    |   %serial.temp = undef;
; CHECK-NEXT:    |   %mask.0. = extractelement %.vec,  0;
; CHECK-NEXT:    |   if (%mask.0. == 1)
; CHECK-NEXT:    |   {
; CHECK-NEXT:    |      %atan2pi = @atan2pi(&((float*)(%A)[i1 + 1]),  &((float*)(%B)[i1 + 1]));
; CHECK-NEXT:    |      %serial.temp = insertelement %serial.temp,  %atan2pi,  0;
; CHECK-NEXT:    |   }
; CHECK-NEXT:    |   %mask.1. = extractelement %.vec,  1;
; CHECK-NEXT:    |   if (%mask.1. == 1)
; CHECK-NEXT:    |   {
; CHECK-NEXT:    |      %extract.1. = extractelement &((<2 x float*>)(%A)[i1 + <i64 0, i64 1> + 1]),  1;
; CHECK-NEXT:    |      %extract.1.3 = extractelement &((<2 x float*>)(%B)[i1 + <i64 0, i64 1> + 1]),  1;
; CHECK-NEXT:    |      %atan2pi4 = @atan2pi(%extract.1.,  %extract.1.3);
; CHECK-NEXT:    |      %serial.temp = insertelement %serial.temp,  %atan2pi4,  1;
; CHECK-NEXT:    |   }
; CHECK-NEXT:    |   %.copy6 = %serial.temp;
; CHECK-NEXT:    |   %select = (%.vec == <i1 true, i1 true>) ? %.copy6 : %.copy;
; CHECK-NEXT:    |   (<2 x float>*)(%R)[i1 + 1] = %select;
; CHECK-NEXT:    + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* noalias dereferenceable(4) %A, float* noalias dereferenceable(4) %B, float* noalias nocapture writeonly dereferenceable(4) %R) local_unnamed_addr {
bb2.preheader:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %if.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end ], [ 1, %bb2.preheader ]
  %A.ptr = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %B.ptr = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %cond = icmp ugt i64 %indvars.iv, 14
  br i1 %cond, label %if.then, label %if.end

if.then:
  %func_result.i = tail call float (...) @atan2pi(float* nonnull %A.ptr, float* nonnull %B.ptr) #1
  br label %if.end

if.end:
  %res = phi float [ %func_result.i, %if.then ], [ 0.000000, %bb2 ]
  %R.ptr = getelementptr inbounds float, float* %R, i64 %indvars.iv
  store float %res, float* %R.ptr, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 38
  br i1 %exitcond.not, label %loopexit, label %bb2

loopexit:                                       ; preds = %if.end
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare float @atan2pi(...) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { nounwind }
