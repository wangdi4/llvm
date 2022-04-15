;RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -analyze -hir-framework -hir-cg -force-hir-cg -print-after=hir-cg -hir-details-dims 2>&1 %s | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -force-hir-cg -print-after=hir-cg -hir-details-dims 2>&1 %s | FileCheck %s

;RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -force-hir-cg -print-after=hir-cg -hir-details-dims 2>&1 %s | FileCheck %s --check-prefix=OPAQUE

; Verify that the framework handles intel.subscript.nonexact() correctly.
; The stride 544 of higher dimension is not an exact multiple of the element
; size of 64 for %base_type. To correctly generate code for this
; dimension CG needs to bitcast base ptr to i8* type to compute offset
; in bytes.

; CHECK: + DO i1 = 0, 11, 1   <DO_LOOP>
; CHECK: |   %cast = sitofp.i32.float(2 * i1 + 2);
; CHECK: |
; CHECK: |   + DO i2 = 0, 14, 1   <DO_LOOP>
; CHECK: |   |   (getelementptr inbounds ([12 x %ext_type], [12 x %ext_type]* @D, i64 0, i64 0, i32 0))[0:i1:544(%base_type*:0)].0[0:i2:4([15 x float]:15)] = %cast;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: After


; CHECK: = load i64, i64* %i1
; CHECK: [[IV:%.*]] = load i64, i64* %i1.i64, align 4
; CHECK:  [[MUL:%.*]] = mul nsw i64 544, [[IV]]
; CHECK: = getelementptr inbounds i8, i8* bitcast ([12 x %ext_type]* @D to i8*), i64 [[MUL]]


; Verify that we are able to generate code successfully in the absence of
; bitcasts and that the test passes with opaque pointers.

; OPAQUE: = load i64, ptr %i1
; OPAQUE: [[IV:%.*]] = load i64, ptr %i1.i64, align 4
; OPAQUE:  [[MUL:%.*]] = mul nsw i64 544, [[IV]]
; OPAQUE: [[GEP1:%.*]] = getelementptr inbounds i8, ptr @D
; OPAQUE: = getelementptr inbounds %base_type, ptr [[GEP1]]


%base_type = type <{ [15 x float], float }>
%ext_type = type <{ %base_type, [4 x [2 x [15 x i32]]] }>

@D = external hidden global [12 x %ext_type]

define void @foo() {
entry:
  br label %bb2.i

bb2.i:                                            ; preds = %loop_exit7.i, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %loop_exit7.i ]
  %noexact.sub = tail call %base_type* @llvm.intel.subscript.nonexact.p0base_type.i64.i64.p0base_type.i64(i8 0, i64 1, i64 544, %base_type* elementtype(%base_type) getelementptr inbounds ([12 x %ext_type], [12 x %ext_type]* @D, i64 0, i64 0, i32 0), i64 %indvars.iv)
  %gep = getelementptr inbounds %base_type, %base_type* %noexact.sub, i64 0, i32 0, i64 0
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %t3 = shl i32 %indvars.iv.tr, 1
  %cast = sitofp i32 %t3 to float
  br label %loop_body6.i

loop_body6.i:                                     ; preds = %loop_body6.i, %bb2.i
  %iv = phi i64 [ 1, %bb2.i ], [ %add.1.i, %loop_body6.i ]
  %subs = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %gep, i64 %iv)
  store float %cast, float* %subs, align 1
  %add.1.i = add nuw nsw i64 %iv, 1
  %exitcond193.not = icmp eq i64 %add.1.i, 16
  br i1 %exitcond193.not, label %loop_exit7.i, label %loop_body6.i

loop_exit7.i:                                     ; preds = %loop_body6.i
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond195 = icmp eq i64 %indvars.iv.next, 13
  br i1 %exitcond195, label %types_mp_sub_.exit, label %bb2.i

types_mp_sub_.exit:
  ret void
}

declare %base_type* @llvm.intel.subscript.nonexact.p0base_type.i64.i64.p0base_type.i64(i8, i64, i64, %base_type*, i64) #0

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

attributes #0 = { nounwind readnone speculatable }

