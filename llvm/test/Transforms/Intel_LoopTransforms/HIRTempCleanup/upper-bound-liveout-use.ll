; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>" 2>&1 | FileCheck %s

; Verify that use of %t301.out in inner loop's upper bound is considered an
; inner loop use and is not replaced because the rval %t301 is redefined
; inside the loop.

; Techinically, it is possible to replace the liveout temp in the upper but
; the current logic incorrectly removes %t301.out as loop livein and then
; verifier complains. It may not be profitable to replace the temp just in
; the upper as it can lead to mismatch in temp used in subscripts vs upper
; resulting in conservative DD analysis.

; CHECK: + DO i1 = 0, %t298 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 33>
; CHECK: |   %t301.out = %t301;
; CHECK: |   (%t23)[i1] = 0.000000e+00;
; CHECK: |   if (%t79 >= %t301)
; CHECK: |   {
; CHECK: |      %t306 = (@module_mp_fast_sbm_mp_dropradii_)[0][i1];
; CHECK: |      %t309 = 0.000000e+00;
; CHECK: |
; CHECK: |      + DO i2 = 0, -1 * sext.i32.i64(%t301.out) + %t295 + -1, 1   <DO_LOOP>
; CHECK: |      |   if ((%t28)[i2 + sext.i32.i64(%t301.out) + -1] <= %t306)
; CHECK: |      |   {
; CHECK: |      |      %t309 = (%t27)[i2 + sext.i32.i64(%t301.out) + -1]  +  %t309;
; CHECK: |      |      (%t23)[i1] = %t309;
; CHECK: |      |      %t301 = i2 + sext.i32.i64(%t301.out) + 1;
; CHECK: |      |   }
; CHECK: |      + END LOOP
; CHECK: |   }
; CHECK: + END LOOP


@module_mp_fast_sbm_mp_dropradii_ = external hidden unnamed_addr global [33 x float], align 8

define void @foo(i32 %t294, i64 %t295, i64 %t298, i32 %t79, ptr %t23, ptr %t27, ptr %t28) {
entry:
  br label %loop

loop:                                              ; preds = %t327, %entry
  %t300 = phi i64 [ 1, %entry ], [ %t329, %t327 ]
  %t301 = phi i32 [ %t294, %entry ], [ %t328, %t327 ]
  %t302 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %t23, i64 %t300) #6
  store float 0.000000e+00, ptr %t302, align 1
  %t303 = icmp slt i32 %t79, %t301
  br i1 %t303, label %t327, label %t304

t304:                                              ; preds = %loop
  %t305 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) @module_mp_fast_sbm_mp_dropradii_, i64 %t300) #6
  %t306 = load float, ptr %t305, align 1
  %t307 = sext i32 %t301 to i64
  br label %t308

t308:                                              ; preds = %t321, %t304
  %t309 = phi float [ 0.000000e+00, %t304 ], [ %t322, %t321 ]
  %t310 = phi i64 [ %t307, %t304 ], [ %t315, %t321 ]
  %t311 = phi i32 [ %t301, %t304 ], [ %t323, %t321 ]
  %t312 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %t28, i64 %t310) #6
  %t313 = load float, ptr %t312, align 1
  %t314 = fcmp fast ugt float %t313, %t306
  %t315 = add nsw i64 %t310, 1
  br i1 %t314, label %t321, label %t316

t316:                                              ; preds = %t308
  %t317 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %t27, i64 %t310) #6
  %t318 = load float, ptr %t317, align 1
  %t319 = fadd fast float %t318, %t309
  store float %t319, ptr %t302, align 1
  %t320 = trunc i64 %t315 to i32
  br label %t321

t321:                                              ; preds = %t316, %t308
  %t322 = phi float [ %t319, %t316 ], [ %t309, %t308 ]
  %t323 = phi i32 [ %t320, %t316 ], [ %t311, %t308 ]
  %t324 = icmp eq i64 %t315, %t295
  br i1 %t324, label %t325, label %t308

t325:                                              ; preds = %t321
  %t326 = phi i32 [ %t323, %t321 ]
  br label %t327

t327:                                              ; preds = %t325, %loop
  %t328 = phi i32 [ %t301, %loop ], [ %t326, %t325 ]
  %t329 = add nuw nsw i64 %t300, 1
  %t330 = icmp eq i64 %t329, %t298
  br i1 %t330, label %exit, label %loop

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #4 = { nounwind readnone speculatable }
