; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -VPlanDriver -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-loop-optreport=high -intel-ir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=LLVM

; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-loop-optreport=high -hir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output -print-after=VPlanDriverHIR 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @serial_call() nounwind

define void @test_serialized(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL: Global loop optimization report for : test_serialized
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15300: LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM:           remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15482: vectorized math library calls: 0
; LLVM-NEXT:      remark #15484: vector function calls: 0
; LLVM-NEXT:      remark #15485: serialized function calls: 2
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Call serialization isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_serialized
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15436: loop was not vectorized:
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
; LLVM-NEXT:      remark #15300: LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM:           remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15482: vectorized math library calls: 0
; LLVM-NEXT:      remark #15484: vector function calls: 1
; LLVM-NEXT:      remark #15485: serialized function calls: 0
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Vector-variant isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vector_variant
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15436: loop was not vectorized:
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
; LLVM-NEXT:      remark #15300: LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM-NEXT:      remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15482: vectorized math library calls: 3
; LLVM-NEXT:      remark #15484: vector function calls: 0
; LLVM-NEXT:      remark #15485: serialized function calls: 0
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_sqrt
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
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
; HIR-NEXT:      remark #15300: LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR:           remark #15475: --- begin vector loop cost summary ---
; HIR-NEXT:      remark #15482: vectorized math library calls: 3
; HIR-NEXT:      remark #15484: vector function calls: 0
; HIR-NEXT:      remark #15485: serialized function calls: 0
; HIR-NEXT:      remark #15488: --- end vector loop cost summary ---
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
; LLVM-NEXT:      remark #15300: LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM:           remark #15447: --- begin vector loop memory reference summary ---
; LLVM-NEXT:      remark #15450: unmasked unaligned unit stride loads: 1
; LLVM-NEXT:      remark #15451: unmasked unaligned unit stride stores: 1
; LLVM-NEXT:      remark #15456: masked unaligned unit stride loads: 1
; LLVM-NEXT:      remark #15457: masked unaligned unit stride stores: 1
; LLVM-NEXT:      remark #15458: masked indexed (or gather) loads: 1
; LLVM-NEXT:      remark #15459: masked indexed (or scatter) stores: 1
; LLVM-NEXT:      remark #15462: unmasked indexed (or gather) loads: 1
; LLVM-NEXT:      remark #15463: unmasked indexed (or scatter) stores: 1
; LLVM:           remark #15474: --- end vector loop memory reference summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_nonvls_mem
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
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
; HIR-NEXT:      remark #15300: LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR:           remark #15447: --- begin vector loop memory reference summary ---
; HIR-NEXT:      remark #15450: unmasked unaligned unit stride loads: 1
; HIR-NEXT:      remark #15451: unmasked unaligned unit stride stores: 1
; HIR-NEXT:      remark #15456: masked unaligned unit stride loads: 1
; HIR-NEXT:      remark #15457: masked unaligned unit stride stores: 1
; HIR-NEXT:      remark #15458: masked indexed (or gather) loads: 1
; HIR-NEXT:      remark #15459: masked indexed (or scatter) stores: 1
; HIR-NEXT:      remark #15462: unmasked indexed (or gather) loads: 1
; HIR-NEXT:      remark #15463: unmasked indexed (or scatter) stores: 1
; HIR:           remark #15474: --- end vector loop memory reference summary ---
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
; LLVM-NEXT:      remark #15300: LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM:           remark #15447: --- begin vector loop memory reference summary ---
; LLVM-NEXT:      remark #15450: unmasked unaligned unit stride loads: 0
; LLVM-NEXT:      remark #15451: unmasked unaligned unit stride stores: 0
; LLVM-NEXT:      remark #15456: masked unaligned unit stride loads: 0
; LLVM-NEXT:      remark #15457: masked unaligned unit stride stores: 0
; LLVM-NEXT:      remark #15458: masked indexed (or gather) loads: 0
; LLVM-NEXT:      remark #15459: masked indexed (or scatter) stores: 0
; Gaps aren't supported by VLS yet.
; LLVM-NEXT:      remark #15462: unmasked indexed (or gather) loads: 2
; LLVM-NEXT:      remark #15463: unmasked indexed (or scatter) stores: 2
; LLVM-NEXT:      remark #15554: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15555: Masked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15556: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15557: Masked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15474: --- end vector loop memory reference summary ---
; LLVM:       LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_vls_mem
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   %.vls.load8 = undef;
; HIR-NEXT:        |   %.vec = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>];
; HIR-NEXT:        |   %.vec2 = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>] = 41;
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1] = %.vec;
; HIR-NEXT:        |   %.vls.load = (<8 x i64>*)(%ptr2)[2 * i1];
; HIR-NEXT:        |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-NEXT:        |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-NEXT:        |   %shuffle = shufflevector 41,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>;
; HIR-NEXT:        |   %shuffle4 = shufflevector undef,  %shuffle,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; HIR-NEXT:        |   %shuffle5 = shufflevector %vls.extract,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>;
; HIR-NEXT:        |   %shuffle6 = shufflevector %shuffle4,  %shuffle5,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; HIR-NEXT:        |   (<8 x i64>*)(%ptr2)[2 * i1] = %shuffle6;
; HIR-NEXT:        |   %.vec7 = %vls.extract == 67;
; HIR-NEXT:        |   %vls.mask = shufflevector %.vec7,  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>;
; HIR-NEXT:        |   %.vls.load8 = (<8 x i64>*)(%ptr3)[2 * i1]; Mask = @{%vls.mask}
; HIR-NEXT:        |   %vls.extract9 = shufflevector %.vls.load8,  %.vls.load8,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-NEXT:        |   %vls.extract10 = shufflevector %.vls.load8,  %.vls.load8,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-NEXT:        |   %shuffle11 = shufflevector 41,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>;
; HIR-NEXT:        |   %shuffle12 = shufflevector undef,  %shuffle11,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; HIR-NEXT:        |   %shuffle13 = shufflevector 42,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>;
; HIR-NEXT:        |   %shuffle14 = shufflevector %shuffle12,  %shuffle13,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; HIR-NEXT:        |   %vls.mask15 = shufflevector %.vec7,  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>;
; HIR-NEXT:        |   (<8 x i64>*)(%ptr4)[2 * i1] = %shuffle14; Mask = @{%vls.mask15}
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vls_mem
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15300: LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR:           remark #15447: --- begin vector loop memory reference summary ---
; HIR-NEXT:      remark #15450: unmasked unaligned unit stride loads: 0
; HIR-NEXT:      remark #15451: unmasked unaligned unit stride stores: 0
; HIR-NEXT:      remark #15456: masked unaligned unit stride loads: 0
; HIR-NEXT:      remark #15457: masked unaligned unit stride stores: 0
; HIR-NEXT:      remark #15458: masked indexed (or gather) loads: 0
; HIR-NEXT:      remark #15459: masked indexed (or scatter) stores: 0
; HIR-NEXT:      remark #15462: unmasked indexed (or gather) loads: 2
; HIR-NEXT:      remark #15463: unmasked indexed (or scatter) stores: 2
; HIR-NEXT:      remark #15554: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; HIR-NEXT:      remark #15555: Masked VLS-optimized loads (each part of the group counted separately): 2
; HIR-NEXT:      remark #15556: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; HIR-NEXT:      remark #15557: Masked VLS-optimized stores (each part of the group counted separately): 2
; HIR-NEXT:      remark #15474: --- end vector loop memory reference summary ---
; HIR:       LOOP END
; HIR-NEXT:  =================================================================
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

attributes #0 = { nounwind "vector-variants"="_ZGVbN4l_vec_func" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
attributes #1 = { nounwind readnone "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
