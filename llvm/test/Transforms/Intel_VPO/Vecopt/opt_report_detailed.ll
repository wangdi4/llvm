; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -VPlanDriver -vector-library=SVML -vplan-force-vf=4 -intel-loop-optreport=high -intel-ir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=LLVM

; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vector-library=SVML -vplan-force-vf=4 -intel-loop-optreport=high -hir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output -print-after=VPlanDriverHIR 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR

; This only has checks for the VLS test below.
; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vector-library=SVML -vplan-force-vf=4 -intel-loop-optreport=high -hir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always -enable-vplan-vls-cg -enable-vp-value-codegen-hir=0 < %s -disable-output -print-after=VPlanDriverHIR 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR-MIXED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @serial_call() nounwind

define void @test_serialized(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL: Global loop optimization report for : test_serialized
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM:           Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 0
; LLVM-NEXT:      Remark: vector function calls: 0
; LLVM-NEXT:      Remark: serialized function calls: 2
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Call serialization isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_serialized
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: loop was not vectorized:
; HIR-NEXT:  LOOP END
; HIR-NEXT:  =================================================================

entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  call void @serial_call()
  %iv.next = add nuw nsw i64 %iv, 1
  call void @serial_call()
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @vec_func(i64) #0
declare void @_ZGVBN4v_vec_func(i64)

define void @test_vector_variant(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL: Global loop optimization report for : test_vector_variant
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM:           Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 0
; LLVM-NEXT:      Remark: vector function calls: 1
; LLVM-NEXT:      Remark: serialized function calls: 0
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Vector-variant isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vector_variant
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: loop was not vectorized:
; HIR-NEXT:  LOOP END
; HIR-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  call void @vec_func(i64 %iv)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare double @llvm.sqrt.f64(double %val) #1
declare double @_Z4sqrtd(double %val) #1
declare double @sqrt(double %val) #1
define void @test_sqrt(i32* nocapture %arr) local_unnamed_addr #1 {
; LLVM-LABEL: Global loop optimization report for : test_sqrt
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM-NEXT:      Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 3
; LLVM-NEXT:      Remark: vector function calls: 0
; LLVM-NEXT:      Remark: serialized function calls: 0
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_sqrt
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <novectorize>
; HIR-NEXT:        |   %.vec = sitofp.<4 x i64>.<4 x double>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; HIR-NEXT:        |   %llvm.sqrt.v4f64 = @llvm.sqrt.v4f64(%.vec);
; HIR-NEXT:        |   %_Z4sqrtDv4_d = @_Z4sqrtDv4_d(%.vec);
; HIR-NEXT:        |   %__svml_sqrt4 = @__svml_sqrt4(%.vec);
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_sqrt
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: LOOP WAS VECTORIZED
; HIR-NEXT:      Remark: vectorization support: vector length 4
; HIR:           Remark: --- begin vector loop cost summary ---
; HIR-NEXT:      Remark: vectorized math library calls: 3
; HIR-NEXT:      Remark: vector function calls: 0
; HIR-NEXT:      Remark: serialized function calls: 0
; HIR-NEXT:      Remark: --- end vector loop cost summary ---
; HIR:       LOOP END
; HIR-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %d = sitofp i64 %iv to double
  ; Intrinsic
  %sqrt = call double @llvm.sqrt.f64(double %d)
  ; Vectorizable, but vector function isn't prefixed with __svml
  %sqrt2 = call double @_Z4sqrtd(double %d)
  ; Vector function is prefixed with __svml
  %sqrt3 = call double @sqrt(double %d)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @test_nonvls_mem(i64* %ptr, i64 *%ptr2) #1 {
; LLVM-LABEL: Global loop optimization report for : test_nonvls_mem
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM:           Remark: --- begin vector loop memory reference summary ---
; LLVM-NEXT:      Remark: unmasked unaligned unit stride loads: 1
; LLVM-NEXT:      Remark: unmasked unaligned unit stride stores: 1
; LLVM-NEXT:      Remark: masked unaligned unit stride loads: 1
; LLVM-NEXT:      Remark: masked unaligned unit stride stores: 1
; LLVM-NEXT:      Remark: masked indexed (or gather) loads: 1
; LLVM-NEXT:      Remark: masked indexed (or scatter) stores: 1
; LLVM-NEXT:      Remark: unmasked indexed (or gather) loads: 1
; LLVM-NEXT:      Remark: unmasked indexed (or scatter) stores: 1
; LLVM:           Remark: --- end vector loop memory reference summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_nonvls_mem
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <novectorize>
; HIR-NEXT:        |   %.vec9 = undef;
; HIR-NEXT:        |   %.vec5 = undef;
; HIR-NEXT:        |   %.vec = i1 + <i64 0, i64 1, i64 2, i64 3>  *  i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-NEXT:        |   %.vec2 = (<4 x i64>*)(%ptr)[i1];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[i1] = 42;
; HIR-NEXT:        |   %.vec3 = (<4 x i64>*)(%ptr2)[%.vec];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[%.vec] = 42;
; HIR-NEXT:        |   %.vec4 = %.vec2 == 76;
; HIR-NEXT:        |   %reverse = shufflevector %.vec4,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %.vec5 = (<4 x i64>*)(%ptr)[-1 * i1 + -3]; Mask = @{%reverse}
; HIR-NEXT:        |   %reverse6 = shufflevector %.vec5,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %reverse7 = shufflevector %.vec4,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %reverse8 = shufflevector 42,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[-1 * i1 + -3] = %reverse8; Mask = @{%reverse7}
; HIR-NEXT:        |   %.vec9 = (<4 x i64>*)(%ptr2)[%.vec]; Mask = @{%.vec4}
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[%.vec] = 77; Mask = @{%.vec4}
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_nonvls_mem
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: LOOP WAS VECTORIZED
; HIR-NEXT:      Remark: vectorization support: vector length 4
; HIR:           Remark: --- begin vector loop memory reference summary ---
; HIR-NEXT:      Remark: unmasked unaligned unit stride loads: 1
; HIR-NEXT:      Remark: unmasked unaligned unit stride stores: 1
; HIR-NEXT:      Remark: masked unaligned unit stride loads: 1
; HIR-NEXT:      Remark: masked unaligned unit stride stores: 1
; HIR-NEXT:      Remark: masked indexed (or gather) loads: 1
; HIR-NEXT:      Remark: masked indexed (or scatter) stores: 1
; HIR-NEXT:      Remark: unmasked indexed (or gather) loads: 1
; HIR-NEXT:      Remark: unmasked indexed (or scatter) stores: 1
; HIR:           Remark: --- end vector loop memory reference summary ---
; HIR:       LOOP END
; HIR-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %non.linear = mul nsw nuw i64 %iv, %iv
  %neg = sub nsw nuw i64 0, %iv
  %linear.gep = getelementptr i64, i64 *%ptr, i64 %iv
  %reverse.linear.gep = getelementptr i64, i64 *%ptr, i64 %neg
  %nonlinear.gep = getelementptr i64, i64 *%ptr2, i64 %non.linear

  %unmasked.unit.load = load i64, i64 *%linear.gep
  store i64 42, i64 *%linear.gep ; unmasked.unit.store
  %cond = icmp eq i64 %unmasked.unit.load, 76

  %unmasked.gather = load i64, i64 *%nonlinear.gep
  store i64 42, i64 *%nonlinear.gep ; unmasked.scatter

  br i1 %cond, label %if, label %latch

if:
  %masked.reverse.unit.load = load i64, i64 *%reverse.linear.gep
  store i64 42, i64 *%reverse.linear.gep ; masked.reverse.unit.store

  %masked.gather = load i64, i64 *%nonlinear.gep
  store i64 77, i64 *%nonlinear.gep ; masked.scatter
  br label %latch

latch:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @test_vls_mem(i64 *%ptr, i64 *%ptr2, i64 *%ptr3, i64 *%ptr4) #1 {
; LLVM-LABEL: Global loop optimization report for : test_vls_mem
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM:           Remark: --- begin vector loop memory reference summary ---
; LLVM-NEXT:      Remark: unmasked unaligned unit stride loads: 0
; LLVM-NEXT:      Remark: unmasked unaligned unit stride stores: 0
; LLVM-NEXT:      Remark: masked unaligned unit stride loads: 0
; LLVM-NEXT:      Remark: masked unaligned unit stride stores: 0
; LLVM-NEXT:      Remark: masked indexed (or gather) loads: 0
; LLVM-NEXT:      Remark: masked indexed (or scatter) stores: 0
; Gaps aren't supported by VLS yet.
; LLVM-NEXT:      Remark: unmasked indexed (or gather) loads: 2
; LLVM-NEXT:      Remark: unmasked indexed (or scatter) stores: 2
; LLVM-NEXT:      Remark: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      Remark: Masked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      Remark: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      Remark: Masked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      Remark: --- end vector loop memory reference summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_vls_mem
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <novectorize>
; HIR-NEXT:        |   %.vec7 = undef;
; HIR-NEXT:        |   %.vec6 = undef;
; HIR-NEXT:        |   %.vec = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>];
; HIR-NEXT:        |   %.vec2 = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>] = 41;
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1] = %.vec;
; HIR-NEXT:        |   %.vec3 = (<4 x i64>*)(%ptr2)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3>];
; HIR-NEXT:        |   %.vec4 = (<4 x i64>*)(%ptr2)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3> + 1];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3>] = 41;
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3> + 1] = %.vec3;
; HIR-NEXT:        |   %.vec5 = %.vec3 == 67;
; HIR-NEXT:        |   %.vec6 = (<4 x i64>*)(%ptr3)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3>]; Mask = @{%.vec5}
; HIR-NEXT:        |   %.vec7 = (<4 x i64>*)(%ptr3)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3> + 1]; Mask = @{%.vec5}
; HIR-NEXT:        |   (<4 x i64>*)(%ptr4)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3>] = 41; Mask = @{%.vec5}
; HIR-NEXT:        |   (<4 x i64>*)(%ptr4)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3> + 1] = 42; Mask = @{%.vec5}
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vls_mem
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: LOOP WAS VECTORIZED
; HIR-NEXT:      Remark: vectorization support: vector length 4
; HIR:           Remark: --- begin vector loop memory reference summary ---
; TODO: HIR VLS doesn't work in full VPValue-based HIR CG.
;       This is being tracked in CMPLRLLVM-20246.
; HIR-NEXT:      Remark: unmasked unaligned unit stride loads: 0
; HIR-NEXT:      Remark: unmasked unaligned unit stride stores: 0
; HIR-NEXT:      Remark: masked unaligned unit stride loads: 0
; HIR-NEXT:      Remark: masked unaligned unit stride stores: 0
; HIR-NEXT:      Remark: masked indexed (or gather) loads: 2
; HIR-NEXT:      Remark: masked indexed (or scatter) stores: 2
; HIR-NEXT:      Remark: unmasked indexed (or gather) loads: 4
; HIR-NEXT:      Remark: unmasked indexed (or scatter) stores: 4
; HIR-NEXT:      Remark: Unmasked VLS-optimized loads (each part of the group counted separately): 0
; HIR-NEXT:      Remark: Masked VLS-optimized loads (each part of the group counted separately): 0
; HIR-NEXT:      Remark: Unmasked VLS-optimized stores (each part of the group counted separately): 0
; HIR-NEXT:      Remark: Masked VLS-optimized stores (each part of the group counted separately): 0
; HIR-NEXT:      Remark: --- end vector loop memory reference summary ---
; HIR:       LOOP END
; HIR-NEXT:  =================================================================

; HIR-MIXED-LABEL: Function: test_vls_mem
; HIR-MIXED:       BEGIN REGION { modified }
; HIR-MIXED-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <novectorize>
; HIR-MIXED-NEXT:        |   %ld.x2.mask.2.vec = undef;
; HIR-MIXED-NEXT:        |   %ld.x2.mask.1.vec = undef;
; HIR-MIXED-NEXT:        |   %ld.x3.1.vec = (<4 x i64>*)(%ptr)[3 * i1 + <i64 0, i64 3, i64 6, i64 9>];
; HIR-MIXED-NEXT:        |   %ld.x3.2.vec = (<4 x i64>*)(%ptr)[3 * i1 + <i64 0, i64 3, i64 6, i64 9> + 1];
; HIR-MIXED-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + <i64 0, i64 3, i64 6, i64 9>] = 41;
; HIR-MIXED-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + <i64 0, i64 3, i64 6, i64 9> + 1] = %ld.x3.1.vec;
; HIR-MIXED-NEXT:        |   %ld.x2.1.vls.load = (<8 x i64>*)(%ptr2)[2 * i1];
; HIR-MIXED-NEXT:        |   %vls.shuf = shufflevector %ld.x2.1.vls.load,  undef,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-MIXED-NEXT:        |   %vls.shuf2 = shufflevector %ld.x2.1.vls.load,  undef,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-MIXED-NEXT:        |   %comb.shuf = shufflevector 41,  %vls.shuf,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; HIR-MIXED-NEXT:        |   %vls.interleave = shufflevector %comb.shuf,  undef,  <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>;
; HIR-MIXED-NEXT:        |   (<8 x i64>*)(%ptr2)[2 * i1] = %vls.interleave;
; HIR-MIXED-NEXT:        |   %wide.cmp. = %vls.shuf == 67;
; HIR-MIXED-NEXT:        |   %ld.x2.mask.1.vec = (<4 x i64>*)(%ptr3)[2 * i1 + <i64 0, i64 2, i64 4, i64 6>]; Mask = @{%wide.cmp.}
; HIR-MIXED-NEXT:        |   %ld.x2.mask.2.vec = (<4 x i64>*)(%ptr3)[2 * i1 + <i64 0, i64 2, i64 4, i64 6> + 1]; Mask = @{%wide.cmp.}
; HIR-MIXED-NEXT:        |   (<4 x i64>*)(%ptr4)[2 * i1 + <i64 0, i64 2, i64 4, i64 6>] = 41; Mask = @{%wide.cmp.}
; HIR-MIXED-NEXT:        |   (<4 x i64>*)(%ptr4)[2 * i1 + <i64 0, i64 2, i64 4, i64 6> + 1] = 42; Mask = @{%wide.cmp.}
; HIR-MIXED-NEXT:        + END LOOP
; HIR-MIXED:             ret ;
; HIR-MIXED-NEXT:  END REGION

; HIR-MIXED-LABEL: Report from: HIR Loop optimizations framework for : test_vls_mem
; HIR-MIXED-EMPTY:
; HIR-MIXED-NEXT:  LOOP BEGIN
; HIR-MIXED-NEXT:      Remark: LOOP WAS VECTORIZED
; HIR-MIXED-NEXT:      Remark: vectorization support: vector length 4
; HIR-MIXED:           Remark: --- begin vector loop memory reference summary ---
; Only VLS remarks are implemented for HIR mixed CG.
; HIR-MIXED:           Remark: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; HIR-MIXED-NEXT:      Remark: Masked VLS-optimized loads (each part of the group counted separately): 0
; HIR-MIXED-NEXT:      Remark: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; HIR-MIXED-NEXT:      Remark: Masked VLS-optimized stores (each part of the group counted separately): 0
; HIR-MIXED-NEXT:      Remark: --- end vector loop memory reference summary ---
; HIR-MIXED:       LOOP END
; HIR-MIXED-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %iv.x3 = mul nsw nuw i64 %iv, 3
  %iv.x2 = mul nsw nuw i64 %iv, 2

  ; Group with gaps but no predicate
  %gep.x3.1 = getelementptr i64, i64 *%ptr, i64 %iv.x3
  %gep.x3.2 = getelementptr i64, i64 *%gep.x3.1, i64 1
  %ld.x3.1 = load i64, i64 *%gep.x3.1
  %ld.x3.2 = load i64, i64 *%gep.x3.2
  store i64 41, i64 *%gep.x3.1
  store i64 %ld.x3.1, i64 *%gep.x3.2

  ; Group without gaps and without predicate
  %gep.x2.1 = getelementptr i64, i64 *%ptr2, i64 %iv.x2
  %gep.x2.2 = getelementptr i64, i64 *%gep.x2.1, i64 1
  %ld.x2.1 = load i64, i64 *%gep.x2.1
  %ld.x2.2 = load i64, i64 *%gep.x2.2
  store i64 41, i64 *%gep.x2.1
  store i64 %ld.x2.1, i64 *%gep.x2.2

  ; Group without gaps but under a predicate
  %cond = icmp eq i64 %ld.x2.1, 67
  %gep.x2.mask.1 = getelementptr i64, i64 *%ptr3, i64 %iv.x2
  %gep.x2.mask.2 = getelementptr i64, i64 *%gep.x2.mask.1, i64 1
  %gep.st.x2.mask.1 = getelementptr i64, i64 *%ptr4, i64 %iv.x2
  %gep.st.x2.mask.2 = getelementptr i64, i64 *%gep.st.x2.mask.1, i64 1
  br i1 %cond, label %if, label %latch

; Current VLS (at least LLVM IR) works in single BB only. If that will ever be
; generalized to multiple BBs one load/store should be hoisted/sinked into
; unpredicated block.
if:
  %ld.x2.mask.1 = load i64, i64 *%gep.x2.mask.1
  %ld.x2.mask.2 = load i64, i64 *%gep.x2.mask.2
  store i64 41, i64 *%gep.st.x2.mask.1
  store i64 42, i64 *%gep.st.x2.mask.2
  br label %latch

latch:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind "vector-variants"="_ZGVbN4v_vec_func" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
attributes #1 = { nounwind "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
