; This test checks to see if we correctly convert arithmatic instructions like udiv, sdiv, urem and srem
; to SVML library functions
;
; DPC++ Source-code used to generate the IR,
;
; <sample_input.cpp>
; Compilation command: clang++ --gcc-toolchain=${ICS_GCCBIN}/.. -fsycl sample_input.cpp -fsycl -lOpenCL -lsycl
; IR intercepted by setting a breakpoint in IntelVPlanDriver.cpp::runOnFunction in GDB and capturing
; the output of Fn.getParent()->dump()
;

;#include <CL/sycl.hpp>
;#include <array>
;#include <numeric>
;
;using namespace cl::sycl;
;
;int main() {
;  queue myQueue;
;  const size_t array_size = 1024 * 128;
;  std::array<cl::sycl::cl_float, array_size> A, B, C;
;  std::iota(begin(B), end(B), 0.2);
;  std::iota(begin(C), end(C), 0.4);
;  unsigned uFrame1 = 12, uFrame2 = 6;
;  int frame1 = 5, frame2 = 16;
;  long frame3 = 5, frame4 = 6;
;  const auto N = A.size();
;
;  buffer<float, 1> bufA(A.data(), range<1>{N});
;  buffer<float, 1> bufB(B.data(), range<1>{N});
;  buffer<float, 1> bufC(C.data(), range<1>{N});
;  myQueue.submit([&](handler &cgh) {
;    auto A = bufA.get_access<access::mode::read>(cgh);
;    auto B = bufA.get_access<access::mode::read>(cgh);
;    auto C = bufA.get_access<access::mode::write>(cgh);
;    cgh.parallel_for<class add>(range<1>{N}, [=](id<1> i) {
;      for (int j = 0; j < N; ++j) {
;        C[i] = A[i] + B[i] + (uFrame1 / uFrame2) + (uFrame1 % uFrame2) +
;               (uFrame1 / -2);
;        C[i] =
;            A[i] + B[i] + (frame1 / frame2) + (frame1 % frame2) + (frame2 / 8);
;        C[i] = A[i] + B[i] + (frame3 / frame4) + (frame3 % frame4);
;      }
;    });
;  });
;}


; RUN: opt -S -replace-with-math-library-functions -vector-library=SVML %s | FileCheck %s

; CHECK: [[UDIV:%.*]] = call i32 @_Z4udivjj(i32 {{.*}}, i32 {{.*}})
; CHECK-NEXT: [[FUDIV:%.*]] = uitofp i32 [[UDIV]] to float

; CHECK: [[UREM:%.*]] = call i32 @_Z4uremjj(i32 {{.*}}, i32 {{.*}})
; CHECK-NEXT: [[FUREM:%.*]] = uitofp i32 [[UREM]] to float

; CHECK: [[IDIV:%.*]] = call i32 @_Z4idivii(i32 {{.*}}, i32 {{.*}})
; CHECK-NEXT: [[FIDIV:%.*]] = sitofp i32 [[IDIV]] to float

; CHECK: [[IREM:%.*]] = call i32 @_Z4iremii(i32 {{.*}}, i32 {{.*}})
; CHECK-NEXT: [[FUREM:%.*]] = sitofp i32 [[IREM]] to float

; Check that we do not convert 'div' operations with power-of-2 divisors
; CHECK: [[SDIV:%.*]] = sdiv i32 {{.*}}, 8
; CHECK-NEXT: [[SUDIV:%.*]] = sitofp i32 [[SDIV]] to float

; Check that we do not convert 'div' operations with i64 operands
; CHECK: [[DIV64:%.*]] = sdiv i64 [[OP1:%.*]], [[OP2:%.*]]
; CHECK-NEXT: [[SUDIVF:%.*]] = sitofp i64 [[DIV64]] to float

; Check that we do not convert 'srem' operations with i64 operands
; CHECK: [[REM64:%.*]] = srem i64 [[OP1:%.*]], [[OP2:%.*]]
; CHECK-NEXT: [[SUREM64:%.*]] = sitofp i64 [[REM64]] to float

;; Check that no replacement happens when a given cl_opt is passed.
; RUN: opt %s -S -disable-mf-replacement -replace-with-math-library-functions \
; RUN: | FileCheck %s --check-prefix=CHECK-NO-REPLACEMENT

; Check that we do not do any instruction replacement.
; CHECK-NO-REPLACEMENT: {{.*}} = udiv i32 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = urem i32 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = sdiv i32 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = srem i32 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = sdiv i32 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = sdiv i64 {{.*}}, {{.*}}
; CHECK-NO-REPLACEMENT: {{.*}} = srem i64 {{.*}}, {{.*}}

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }
; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr
declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
define void @"sycl_handler_add"(float addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, float addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, float addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32, i32, i32, i32, i64, i64) {
  %19 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %18
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(float addrspace(1)* %0, %"class.cl::sycl::range"* %1, %"class.cl::sycl::range"* %2, %"class.cl::sycl::range"* %3, float addrspace(1)* %4, %"class.cl::sycl::range"* %5, %"class.cl::sycl::range"* %6, %"class.cl::sycl::range"* %7, float addrspace(1)* %8, %"class.cl::sycl::range"* %9, %"class.cl::sycl::range"* %10, %"class.cl::sycl::range"* %11, i32 %12, i32 %13, i32 %14, i32 %15, i64 %16, i64 %17) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %20 = sext i32 %index to i64
  %add = add nuw i64 %20, %19
  %21 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %3, i64 0, i32 0, i32 0, i64 0
  %22 = load i64, i64* %21, align 8
  %23 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %7, i64 0, i32 0, i32 0, i64 0
  %24 = load i64, i64* %23, align 8
  %25 = getelementptr inbounds %"class.cl::sycl::range", %"class.cl::sycl::range"* %11, i64 0, i32 0, i32 0, i64 0
  %26 = load i64, i64* %25, align 8
  br label %27

27:                                               ; preds = %27, %simd.loop
  %.09.i = phi i32 [ 0, %simd.loop ], [ %67, %27 ]
  %28 = add i64 %24, %add
  %29 = getelementptr inbounds float, float addrspace(1)* %4, i64 %28
  %30 = load float, float addrspace(1)* %29, align 4
  %31 = add i64 %26, %add
  %32 = getelementptr inbounds float, float addrspace(1)* %8, i64 %31
  %33 = load float, float addrspace(1)* %32, align 4
  %34 = fadd float %30, %33
  %35 = udiv i32 %12, %13
  %36 = uitofp i32 %35 to float
  %37 = fadd float %34, %36
  %38 = urem i32 %12, %13
  %39 = uitofp i32 %38 to float
  %40 = fadd float %37, %39
  %41 = icmp ugt i32 %12, -3
  %42 = uitofp i1 %41 to float
  %43 = fadd float %40, %42
  %44 = add i64 %22, %add
  %45 = getelementptr inbounds float, float addrspace(1)* %0, i64 %44
  store float %43, float addrspace(1)* %45, align 4
  %46 = load float, float addrspace(1)* %29, align 4
  %47 = load float, float addrspace(1)* %32, align 4
  %48 = fadd float %46, %47
  %49 = sdiv i32 %14, %15
  %50 = sitofp i32 %49 to float
  %51 = fadd float %48, %50
  %52 = srem i32 %14, %15
  %53 = sitofp i32 %52 to float
  %54 = fadd float %51, %53
  %55 = sdiv i32 %15, 8
  %56 = sitofp i32 %55 to float
  %57 = fadd float %54, %56
  store float %57, float addrspace(1)* %45, align 4
  %58 = load float, float addrspace(1)* %29, align 4
  %59 = load float, float addrspace(1)* %32, align 4
  %60 = fadd float %58, %59
  %61 = sdiv i64 %16, %17
  %62 = sitofp i64 %61 to float
  %63 = fadd float %60, %62
  %64 = srem i64 %16, %17
  %65 = sitofp i64 %64 to float
  %66 = fadd float %63, %65
  store float %66, float addrspace(1)* %45, align 4
  %67 = add nuw nsw i32 %.09.i, 1
  %exitcond.i = icmp eq i32 %67, 131072
  br i1 %exitcond.i, label %"VPBB.exit", label %27

"VPBB.exit": ; preds = %27
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %"VPBB.exit"
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !18

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.unroll.disable"}


;; Check that no replacement happens for vector input operands.
; RUN: opt %s -S -replace-with-math-library-functions \
; RUN: | FileCheck %s --check-prefix=CHECK-VEC-NO-REPLACEMENT

%VEC3 = type { <3 x i32> }
%RANGE = type { %ARRAY }
%ARRAY = type { [1 x i64] }

define void @test(%VEC3 addrspace(1)* noalias %0, %RANGE* byval(%RANGE) %1, %RANGE* byval(%RANGE) %2, %RANGE* byval(%RANGE) %3) {
  %.sroa.0.0..sroa_idx = getelementptr inbounds %RANGE, %RANGE* %3, i64 0, i32 0, i32 0, i64 0
  %.sroa.0.0.copyload = load i64, i64* %.sroa.0.0..sroa_idx, align 8
  %5 = getelementptr inbounds %VEC3, %VEC3 addrspace(1)* %0, i64 %.sroa.0.0.copyload
  %6 = call i64 @_Z13get_global_idj(i32 0)
  %7 = trunc i64 %6 to i32
  %8 = insertelement <3 x i32> undef, i32 %7, i32 0
  %9 = getelementptr inbounds %VEC3, %VEC3 addrspace(1)* %5, i64 %6
  %10 = shufflevector <3 x i32> %8, <3 x i32> undef, <3 x i32> zeroinitializer
  %11 = bitcast %VEC3 addrspace(1)* %9 to <4 x i32> addrspace(1)*
  %12 = load <4 x i32>, <4 x i32> addrspace(1)* %11, align 16
  %13 = shufflevector <4 x i32> %12, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %14 = sdiv <3 x i32> %10, %13
; CHECK-VEC-NO-REPLACEMENT: {{.*}} = sdiv <3 x i32> {{.*}}, {{.*}}
  %15 = shufflevector <3 x i32> %14, <3 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  store <4 x i32> %15, <4 x i32> addrspace(1)* %11, align 16
  ret void
}
