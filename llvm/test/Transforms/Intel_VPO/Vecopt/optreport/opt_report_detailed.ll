; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -enable-new-pm=0 -vplan-vec -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-opt-report=high -intel-ir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=LLVM
; RUN: opt -passes='vplan-vec,intel-ir-optreport-emitter' -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-opt-report=high -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=LLVM

; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-opt-report=high -hir-optreport-emitter -enable-intel-advanced-opts -vplan-vls-level=always -print-after=hir-vplan-vec < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>,hir-optreport-emitter' -vector-library=SVML -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops=false -intel-opt-report=high -enable-intel-advanced-opts -vplan-vls-level=always < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @serial_call() nounwind

define void @test_serialized(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL:  Global optimization report for : test_serialized
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM-NEXT:      remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15476: scalar cost: 2.000000
; LLVM-NEXT:      remark #15477: vector cost: 4.500000
; LLVM-NEXT:      remark #15478: estimated potential speedup: 0.437500
; LLVM-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
; LLVM-NEXT:      remark #15482: vectorized math library calls: 0
; LLVM-NEXT:      remark #15484: vector function calls: 0
; LLVM-NEXT:      remark #15485: serialized function calls: 2
; LLVM-NEXT:      remark #15558: Call to function 'serial_call' was serialized due to no suitable vector variants were found.
; LLVM-NEXT:      remark #15558: Call to function 'serial_call' was serialized due to no suitable vector variants were found.
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_serialized
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        |   @serial_call();
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_serialized
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR-NEXT:      remark #15475: --- begin vector loop cost summary ---
; HIR-NEXT:      remark #15476: scalar cost: 2.000000
; HIR-NEXT:      remark #15477: vector cost: 4.500000
; HIR-NEXT:      remark #15478: estimated potential speedup: 0.437500
; HIR-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
; HIR-NEXT:      remark #15482: vectorized math library calls: 0
; HIR-NEXT:      remark #15484: vector function calls: 0
; HIR-NEXT:      remark #15485: serialized function calls: 2
; HIR-NEXT:      remark #15488: --- end vector loop cost summary ---
; HIR:       LOOP END
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
; LLVM-LABEL:  Global optimization report for : test_vector_variant
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM-NEXT:      remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15476: scalar cost: 2.000000
; LLVM-NEXT:      remark #15477: vector cost: 4.500000
; LLVM-NEXT:      remark #15478: estimated potential speedup: 0.437500
; LLVM-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
; LLVM-NEXT:      remark #15482: vectorized math library calls: 0
; LLVM-NEXT:      remark #15484: vector function calls: 1
; LLVM-NEXT:      remark #15485: serialized function calls: 0
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Vector-variant isn't supported for HIR yet. They are serialized for now.
; HIR-LABEL: Function: test_vector_variant
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   @_ZGVbN4l_vec_func(i1);
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vector_variant
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR-NEXT:      remark #15475: --- begin vector loop cost summary ---
; HIR-NEXT:      remark #15476: scalar cost: 2.000000
; HIR-NEXT:      remark #15477: vector cost: 4.500000
; HIR-NEXT:      remark #15478: estimated potential speedup: 0.437500
; HIR-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
; HIR-NEXT:      remark #15482: vectorized math library calls: 0
; HIR-NEXT:      remark #15484: vector function calls: 1
; HIR-NEXT:      remark #15485: serialized function calls: 0
; HIR-NEXT:      remark #15488: --- end vector loop cost summary ---
; HIR:       LOOP END
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
; LLVM-LABEL:  Global optimization report for : test_sqrt
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; LLVM-NEXT:      remark #15305: vectorization support: vector length 4
; LLVM-NEXT:      remark #15475: --- begin vector loop cost summary ---
; LLVM-NEXT:      remark #15476: scalar cost: 31.000000
; LLVM-NEXT:      remark #15477: vector cost: 12.000000
; LLVM-NEXT:      remark #15478: estimated potential speedup: 2.578125
; LLVM-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
; LLVM-NEXT:      remark #15482: vectorized math library calls: 3
; LLVM-NEXT:      remark #15484: vector function calls: 0
; LLVM-NEXT:      remark #15485: serialized function calls: 0
; LLVM-NEXT:      remark #15488: --- end vector loop cost summary ---
; LLVM:       LOOP END
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
; HIR-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; HIR-NEXT:      remark #15305: vectorization support: vector length 4
; HIR-NEXT:      remark #15475: --- begin vector loop cost summary ---
; HIR-NEXT:      remark #15476: scalar cost: 31.000000
; HIR-NEXT:      remark #15477: vector cost: 12.000000
; HIR-NEXT:      remark #15478: estimated potential speedup: 2.578125
; HIR-NEXT:      remark #15309: vectorization support: normalized vectorization overhead 0.000000
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
; LLVM-LABEL:  Global optimization report for : test_nonvls_mem
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
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
; LLVM-NEXT:      remark #15567: Gathers are generated due to non-unit stride index of the corresponding loads.
; LLVM-NEXT:      remark #15568: Scatters are generated due to non-unit stride index of the corresponding stores.
; LLVM:           remark #15474: --- end vector loop memory reference summary ---
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_nonvls_mem
; HIR-EMPTY:
; HIR-NEXT:  BEGIN REGION { modified }
; HIR-NEXT:        + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   %.vec9 = undef;
; HIR-NEXT:        |   %.vec5 = undef;
; HIR-NEXT:        |   %.vec = i1 + <i64 0, i64 1, i64 2, i64 3>  *  i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-NEXT:        |   %.scal = i1  *  i1;
; HIR-NEXT:        |   %.vec2 = (<4 x i64>*)(%ptr)[i1];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[i1] = 42;
; HIR-NEXT:        |   %.vec3 = (<4 x i64>*)(%ptr2)[%.vec];
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[%.vec] = 42;
; HIR-NEXT:        |   %.vec4 = %.vec2 == 76;
; HIR-NEXT:        |   %reverse = shufflevector %.vec4,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %.vec5 = (<4 x i64>*)(%ptr)[-1 * i1 + -3], Mask = @{%reverse};
; HIR-NEXT:        |   %reverse6 = shufflevector %.vec5,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %reverse7 = shufflevector %.vec4,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   %reverse8 = shufflevector 42,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; HIR-NEXT:        |   (<4 x i64>*)(%ptr)[-1 * i1 + -3] = %reverse8, Mask = @{%reverse7};
; HIR-NEXT:        |   %.vec9 = (<4 x i64>*)(%ptr2)[%.vec], Mask = @{%.vec4};
; HIR-NEXT:        |   (<4 x i64>*)(%ptr2)[%.vec] = 77, Mask = @{%.vec4};
; HIR-NEXT:        + END LOOP
; HIR:             ret ;
; HIR-NEXT:  END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_nonvls_mem
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
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
; HIR-NEXT:      remark #15567: Gathers are generated due to non-unit stride index of the corresponding loads.
; HIR-NEXT:      remark #15568: Scatters are generated due to non-unit stride index of the corresponding stores.
; HIR:           remark #15474: --- end vector loop memory reference summary ---
; HIR-NEXT:  LOOP END
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
; LLVM-LABEL:  Global optimization report for : test_vls_mem
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
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
; LLVM-NEXT:      remark #15567: Gathers are generated due to non-unit stride index of the corresponding loads.
; LLVM-NEXT:      remark #15568: Scatters are generated due to non-unit stride index of the corresponding stores.
; LLVM-NEXT:      remark #15554: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15555: Masked VLS-optimized loads (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15556: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15557: Masked VLS-optimized stores (each part of the group counted separately): 2
; LLVM-NEXT:      remark #15474: --- end vector loop memory reference summary ---
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Function: test_vls_mem
; HIR-EMPTY:
; HIR-NEXT: BEGIN REGION { modified }
; HIR-NEXT:       + DO i1 = 0, 299, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:       |   %.vls.load7 = undef;
; HIR-NEXT:       |   %.vec = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>];
; HIR-NEXT:       |   %.vec2 = (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1];
; HIR-NEXT:       |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>] = 41;
; HIR-NEXT:       |   (<4 x i64>*)(%ptr)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1] = %.vec;
; HIR-NEXT:       |   %.vls.load = (<8 x i64>*)(%ptr2)[2 * i1];
; HIR-NEXT:       |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-NEXT:       |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-NEXT:       |   %.extended = shufflevector 41,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:       |   %shuffle = shufflevector undef,  %.extended,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; HIR-NEXT:       |   %.extended4 = shufflevector %vls.extract,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:       |   %shuffle5 = shufflevector %shuffle,  %.extended4,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; HIR-NEXT:       |   (<8 x i64>*)(%ptr2)[2 * i1] = %shuffle5;
; HIR-NEXT:       |   %.vec6 = %vls.extract == 67;
; HIR-NEXT:       |   %vls.mask = shufflevector %.vec6,  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>;
; HIR-NEXT:       |   %.vls.load7 = (<8 x i64>*)(%ptr3)[2 * i1], Mask = @{%vls.mask};
; HIR-NEXT:       |   %vls.extract8 = shufflevector %.vls.load7,  %.vls.load7,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-NEXT:       |   %vls.extract9 = shufflevector %.vls.load7,  %.vls.load7,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-NEXT:       |   %.extended10 = shufflevector 41,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:       |   %shuffle11 = shufflevector undef,  %.extended10,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; HIR-NEXT:       |   %.extended12 = shufflevector 42,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; HIR-NEXT:       |   %shuffle13 = shufflevector %shuffle11,  %.extended12,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; HIR-NEXT:       |   %vls.mask14 = shufflevector %.vec6,  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>;
; HIR-NEXT:       |   (<8 x i64>*)(%ptr4)[2 * i1] = %shuffle13, Mask = @{%vls.mask14};
; HIR-NEXT:       + END LOOP
; HIR:            ret ;
; HIR-NEXT: END REGION

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vls_mem
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
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
; HIR-NEXT:      remark #15567: Gathers are generated due to non-unit stride index of the corresponding loads.
; HIR-NEXT:      remark #15568: Scatters are generated due to non-unit stride index of the corresponding stores.
; HIR-NEXT:      remark #15554: Unmasked VLS-optimized loads (each part of the group counted separately): 2
; HIR-NEXT:      remark #15555: Masked VLS-optimized loads (each part of the group counted separately): 2
; HIR-NEXT:      remark #15556: Unmasked VLS-optimized stores (each part of the group counted separately): 2
; HIR-NEXT:      remark #15557: Masked VLS-optimized stores (each part of the group counted separately): 2
; HIR-NEXT:      remark #15474: --- end vector loop memory reference summary ---
; HIR-NEXT:  LOOP END
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
