; RUN: opt < %s -passes='module(intel-loop-attrs)' -force-intel-prefer-func-level-region -S 2>&1 | FileCheck %s

; This test case checks that the attribute "prefer-function-level-region"
; was added to the function @foo since it has a large number of parameters
; that are dope vectors and they are used in many nested loops. It is the
; same test case as intel_loopattrs09.ll, but it checks the ir.

; CHECK: define internal void @foo
; CHECK-SAME: #0
; CHECK: attributes #0 = {
; CHECK-SAME: "prefer-function-level-region"

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$" = type { %"MOD_PARAM$.btT_BOUNDS"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"MOD_PARAM$.btT_BOUNDS" = type { %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", i32, i32, [4 x [4 x i32]], %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank3$", %"QNCA_a0$i32*$rank3$", %"QNCA_a0$i32*$rank3$", %"QNCA_a0$i32*$rank3$" }
%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$i32*$rank3$" = type { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank4$" = type { double*, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank3$" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

@mod_scalars_mp_ntfirst_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_scalars_mp_iic_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_scalars_mp_g_ = available_externally local_unnamed_addr global double 0.000000e+00, align 8
@mod_scalars_mp_dtfast_ = available_externally local_unnamed_addr global [1 x double] zeroinitializer, align 8
@mod_scalars_mp_rho0_ = available_externally local_unnamed_addr global double 0.000000e+00, align 8
@mod_scalars_mp_nfast_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_scalars_mp_weight_ = available_externally local_unnamed_addr global [1 x [257 x [2 x double]]] zeroinitializer, align 8
@mod_scalars_mp_iif_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_param_mp_mm_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_scalars_mp_predictor_2d_step_ = available_externally local_unnamed_addr global [1 x i32] zeroinitializer, align 8
@mod_param_mp_bounds_ = available_externally local_unnamed_addr global %"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$" zeroinitializer

; Function Attrs: nofree nosync nounwind uwtable
define internal void @foo(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3, i32* noalias nocapture readonly dereferenceable(4) %4, i32* noalias nocapture readonly dereferenceable(4) %5, i32* noalias nocapture readonly dereferenceable(4) %6, i32* noalias nocapture readonly dereferenceable(4) %7, i32* noalias nocapture readonly dereferenceable(4) %8, i32* noalias nocapture readonly dereferenceable(4) %9, i32* noalias nocapture readonly dereferenceable(4) %10, i32* noalias nocapture readonly dereferenceable(4) %11, i32* noalias nocapture readonly dereferenceable(4) %12, i32* noalias nocapture readonly dereferenceable(4) %13, i32* noalias nocapture readonly dereferenceable(4) %14, i32* noalias nocapture readonly dereferenceable(4) %15, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %16, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %17, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %18, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %19, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %20, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %21, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %22, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %23, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %24, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %25, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %26, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %27, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %28, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %29, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %30, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %31, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %32, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %33, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %34, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %35, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %36, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %37, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %38, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %39, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %40, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %41, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %42, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %43, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %44, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %45, %"QNCA_a0$double*$rank4$"* noalias nocapture readonly dereferenceable(144) "assumed_shape" "ptrnoalias" %46, %"QNCA_a0$double*$rank4$"* noalias nocapture readonly dereferenceable(144) "assumed_shape" "ptrnoalias" %47, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %48, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %49, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %50, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %51, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %52, %"QNCA_a0$double*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %53) #0 {
  %55 = load i32, i32* %2, align 1
  %56 = load i32, i32* %4, align 1
  %57 = load i32, i32* %7, align 1
  %58 = load i32, i32* %8, align 1
  %59 = load i32, i32* %9, align 1
  %60 = load i32, i32* %10, align 1
  %61 = sub i32 1, %57
  %62 = add i32 %61, %58
  %63 = sext i32 %62 to i64
  %64 = icmp sgt i64 %63, 0
  %65 = select i1 %64, i64 %63, i64 0
  %66 = shl nuw nsw i64 %65, 3
  %67 = sub i32 1, %59
  %68 = add i32 %67, %60
  %69 = sext i32 %68 to i64
  %70 = icmp sgt i64 %69, 0
  %71 = select i1 %70, i64 %69, i64 0
  %72 = mul nsw i64 %66, %71
  %73 = lshr exact i64 %72, 3
  %74 = alloca double, i64 %73, align 8
  %75 = alloca double, i64 %73, align 8
  %76 = alloca double, i64 %73, align 8
  %77 = alloca double, i64 %73, align 8
  %78 = alloca double, i64 %73, align 8
  %79 = alloca double, i64 %73, align 8
  %80 = alloca double, i64 %73, align 8
  %81 = alloca double, i64 %73, align 8
  %82 = alloca double, i64 %73, align 8
  %83 = alloca double, i64 %73, align 8
  %84 = alloca double, i64 %73, align 8
  %85 = alloca double, i64 %73, align 8
  %86 = alloca double, i64 %73, align 8
  %87 = alloca double, i64 %73, align 8
  %88 = alloca double, i64 %73, align 8
  %89 = alloca double, i64 %73, align 8
  %90 = alloca double, i64 %73, align 8
  %91 = alloca double, i64 %73, align 8
  %92 = alloca double, i64 %73, align 8
  %93 = alloca double, i64 %73, align 8
  %94 = shl nsw i64 %63, 3
  %95 = load %"MOD_PARAM$.btT_BOUNDS"*, %"MOD_PARAM$.btT_BOUNDS"** getelementptr inbounds (%"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$", %"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$"* @mod_param_mp_bounds_, i64 0, i32 0), align 8
  %96 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$", %"QNCA_a0$%\22MOD_PARAM$.btT_BOUNDS\22*$rank1$"* @mod_param_mp_bounds_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %97 = load i64, i64* %96, align 1
  %98 = load i32, i32* %0, align 1
  %99 = sext i32 %98 to i64
  %100 = tail call %"MOD_PARAM$.btT_BOUNDS"* @"llvm.intel.subscript.p0s_MOD_PARAM$.btT_BOUNDSs.i64.i64.p0s_MOD_PARAM$.btT_BOUNDSs.i64"(i8 0, i64 %97, i64 1920, %"MOD_PARAM$.btT_BOUNDS"* elementtype(%"MOD_PARAM$.btT_BOUNDS") %95, i64 %99)
  %101 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 8
  %102 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %101, i64 0, i32 0
  %103 = load i32*, i32** %102, align 1
  %104 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %101, i64 0, i32 6, i64 0
  %105 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %104, i64 0, i32 1
  %106 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %105, i32 0)
  %107 = load i64, i64* %106, align 1
  %108 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %104, i64 0, i32 2
  %109 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %108, i32 0)
  %110 = load i64, i64* %109, align 1
  %111 = load i32, i32* %1, align 1
  %112 = sext i32 %111 to i64
  %113 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %110, i64 %107, i32* elementtype(i32) %103, i64 %112)
  %114 = load i32, i32* %113, align 1
  %115 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 12
  %116 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %115, i64 0, i32 0
  %117 = load i32*, i32** %116, align 1
  %118 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %115, i64 0, i32 6, i64 0
  %119 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %118, i64 0, i32 1
  %120 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %119, i32 0)
  %121 = load i64, i64* %120, align 1
  %122 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %118, i64 0, i32 2
  %123 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %122, i32 0)
  %124 = load i64, i64* %123, align 1
  %125 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %124, i64 %121, i32* elementtype(i32) %117, i64 %112)
  %126 = load i32, i32* %125, align 1
  %127 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 14
  %128 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %127, i64 0, i32 0
  %129 = load i32*, i32** %128, align 1
  %130 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %127, i64 0, i32 6, i64 0
  %131 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %130, i64 0, i32 1
  %132 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %131, i32 0)
  %133 = load i64, i64* %132, align 1
  %134 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %130, i64 0, i32 2
  %135 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %134, i32 0)
  %136 = load i64, i64* %135, align 1
  %137 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %136, i64 %133, i32* elementtype(i32) %129, i64 %112)
  %138 = load i32, i32* %137, align 1
  %139 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 9
  %140 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %139, i64 0, i32 0
  %141 = load i32*, i32** %140, align 1
  %142 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %139, i64 0, i32 6, i64 0
  %143 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %142, i64 0, i32 1
  %144 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %143, i32 0)
  %145 = load i64, i64* %144, align 1
  %146 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %142, i64 0, i32 2
  %147 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %146, i32 0)
  %148 = load i64, i64* %147, align 1
  %149 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %148, i64 %145, i32* elementtype(i32) %141, i64 %112)
  %150 = load i32, i32* %149, align 1
  %151 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 15
  %152 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %151, i64 0, i32 0
  %153 = load i32*, i32** %152, align 1
  %154 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %151, i64 0, i32 6, i64 0
  %155 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %154, i64 0, i32 1
  %156 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %155, i32 0)
  %157 = load i64, i64* %156, align 1
  %158 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %154, i64 0, i32 2
  %159 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %158, i32 0)
  %160 = load i64, i64* %159, align 1
  %161 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %160, i64 %157, i32* elementtype(i32) %153, i64 %112)
  %162 = load i32, i32* %161, align 1
  %163 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 10
  %164 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %163, i64 0, i32 0
  %165 = load i32*, i32** %164, align 1
  %166 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %163, i64 0, i32 6, i64 0
  %167 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %166, i64 0, i32 1
  %168 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %167, i32 0)
  %169 = load i64, i64* %168, align 1
  %170 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %166, i64 0, i32 2
  %171 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %170, i32 0)
  %172 = load i64, i64* %171, align 1
  %173 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %172, i64 %169, i32* elementtype(i32) %165, i64 %112)
  %174 = load i32, i32* %173, align 1
  %175 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 17
  %176 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %175, i64 0, i32 0
  %177 = load i32*, i32** %176, align 1
  %178 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %175, i64 0, i32 6, i64 0
  %179 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %178, i64 0, i32 1
  %180 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %179, i32 0)
  %181 = load i64, i64* %180, align 1
  %182 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %178, i64 0, i32 2
  %183 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %182, i32 0)
  %184 = load i64, i64* %183, align 1
  %185 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %184, i64 %181, i32* elementtype(i32) %177, i64 %112)
  %186 = load i32, i32* %185, align 1
  %187 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 19
  %188 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %187, i64 0, i32 0
  %189 = load i32*, i32** %188, align 1
  %190 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %187, i64 0, i32 6, i64 0
  %191 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %190, i64 0, i32 1
  %192 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %191, i32 0)
  %193 = load i64, i64* %192, align 1
  %194 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %190, i64 0, i32 2
  %195 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %194, i32 0)
  %196 = load i64, i64* %195, align 1
  %197 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %196, i64 %193, i32* elementtype(i32) %189, i64 %112)
  %198 = load i32, i32* %197, align 1
  %199 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 11
  %200 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %199, i64 0, i32 0
  %201 = load i32*, i32** %200, align 1
  %202 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %199, i64 0, i32 6, i64 0
  %203 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %202, i64 0, i32 1
  %204 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %203, i32 0)
  %205 = load i64, i64* %204, align 1
  %206 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %202, i64 0, i32 2
  %207 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %206, i32 0)
  %208 = load i64, i64* %207, align 1
  %209 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %208, i64 %205, i32* elementtype(i32) %201, i64 %112)
  %210 = load i32, i32* %209, align 1
  %211 = getelementptr inbounds %"MOD_PARAM$.btT_BOUNDS", %"MOD_PARAM$.btT_BOUNDS"* %100, i64 0, i32 20
  %212 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %211, i64 0, i32 0
  %213 = load i32*, i32** %212, align 1
  %214 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %211, i64 0, i32 6, i64 0
  %215 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %214, i64 0, i32 1
  %216 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %215, i32 0)
  %217 = load i64, i64* %216, align 1
  %218 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %214, i64 0, i32 2
  %219 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %218, i32 0)
  %220 = load i64, i64* %219, align 1
  %221 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %220, i64 %217, i32* elementtype(i32) %213, i64 %112)
  %222 = load i32, i32* %221, align 1
  %223 = load i32, i32* %12, align 1
  %224 = sub nsw i32 3, %223
  %225 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_scalars_mp_predictor_2d_step_, i64 0, i64 0), i64 %99)
  %226 = load i32, i32* %225, align 1
  %227 = xor i32 %226, -1
  %228 = add i32 %198, -1
  %229 = icmp slt i32 %198, 4
  %230 = select i1 %229, i32 2, i32 %228
  %231 = add nsw i32 %230, -2
  %232 = add nsw i32 %210, 1
  %233 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_param_mp_mm_, i64 0, i64 0), i64 %99)
  %234 = load i32, i32* %233, align 1
  %235 = icmp slt i32 %210, %234
  %236 = select i1 %235, i32 %232, i32 %234
  %237 = add nsw i32 %236, 1
  %238 = icmp slt i32 %237, %231
  br i1 %238, label %346, label %239

239:                                              ; preds = %54
  %240 = add i32 %138, -3
  %241 = add nsw i32 %150, 2
  %242 = icmp slt i32 %241, %240
  %243 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %244 = load double*, double** %243, align 1
  %245 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %246 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %245, i64 0, i32 1
  %247 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %246, i32 0)
  %248 = load i64, i64* %247, align 1
  %249 = sext i32 %55 to i64
  %250 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %246, i32 1)
  %251 = load i64, i64* %250, align 1
  %252 = sext i32 %56 to i64
  %253 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %246, i32 2)
  %254 = load i64, i64* %253, align 1
  %255 = load i32, i32* %11, align 1
  %256 = sext i32 %255 to i64
  %257 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %254, double* elementtype(double) %244, i64 %256)
  %258 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %259 = load double*, double** %258, align 1
  %260 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %261 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %260, i64 0, i32 1
  %262 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %261, i32 0)
  %263 = load i64, i64* %262, align 1
  %264 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %261, i32 1)
  %265 = load i64, i64* %264, align 1
  %266 = sext i32 %57 to i64
  %267 = sext i32 %59 to i64
  %268 = sext i32 %240 to i64
  %269 = add nsw i32 %150, 3
  %270 = zext i32 %231 to i64
  %271 = add nsw i32 %236, 2
  br label %272

272:                                              ; preds = %289, %239
  %273 = phi i64 [ %270, %239 ], [ %290, %289 ]
  br i1 %242, label %289, label %274

274:                                              ; preds = %272
  %275 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %252, i64 %251, double* elementtype(double) %257, i64 %273)
  %276 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %252, i64 %265, double* elementtype(double) %259, i64 %273)
  %277 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %267, i64 %94, double* elementtype(double) nonnull %91, i64 %273)
  br label %278

278:                                              ; preds = %278, %274
  %279 = phi i64 [ %268, %274 ], [ %286, %278 ]
  %280 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %249, i64 %248, double* elementtype(double) %275, i64 %279)
  %281 = load double, double* %280, align 1
  %282 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %249, i64 %263, double* elementtype(double) %276, i64 %279)
  %283 = load double, double* %282, align 1
  %284 = fadd fast double %283, %281
  %285 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %266, i64 8, double* elementtype(double) nonnull %277, i64 %279)
  store double %284, double* %285, align 1
  %286 = add nsw i64 %279, 1
  %287 = trunc i64 %286 to i32
  %288 = icmp eq i32 %269, %287
  br i1 %288, label %289, label %278

289:                                              ; preds = %278, %272
  %290 = add nuw nsw i64 %273, 1
  %291 = trunc i64 %290 to i32
  %292 = icmp eq i32 %271, %291
  br i1 %292, label %293, label %272

293:                                              ; preds = %289
  %294 = add i32 %138, -2
  %295 = icmp slt i32 %241, %294
  %296 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %20, i64 0, i32 0
  %297 = load double*, double** %296, align 1
  %298 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %20, i64 0, i32 6, i64 0
  %299 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %298, i64 0, i32 1
  %300 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %299, i32 0)
  %301 = load i64, i64* %300, align 1
  %302 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %299, i32 1)
  %303 = load i64, i64* %302, align 1
  %304 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %305 = load double*, double** %304, align 1
  %306 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %307 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %306, i64 0, i32 1
  %308 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %307, i32 0)
  %309 = load i64, i64* %308, align 1
  %310 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %307, i32 1)
  %311 = load i64, i64* %310, align 1
  %312 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %307, i32 2)
  %313 = load i64, i64* %312, align 1
  %314 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %313, double* elementtype(double) %305, i64 %256)
  %315 = sext i32 %294 to i64
  br label %316

316:                                              ; preds = %342, %293
  %317 = phi i64 [ %270, %293 ], [ %343, %342 ]
  br i1 %295, label %342, label %318

318:                                              ; preds = %316
  %319 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %252, i64 %303, double* elementtype(double) %297, i64 %317)
  %320 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %267, i64 %94, double* elementtype(double) nonnull %91, i64 %317)
  %321 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %252, i64 %311, double* elementtype(double) %314, i64 %317)
  %322 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %267, i64 %94, double* elementtype(double) nonnull %88, i64 %317)
  br label %323

323:                                              ; preds = %323, %318
  %324 = phi i64 [ %315, %318 ], [ %339, %323 ]
  %325 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %249, i64 %301, double* elementtype(double) %319, i64 %324)
  %326 = load double, double* %325, align 1
  %327 = fmul fast double %326, 5.000000e-01
  %328 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %266, i64 8, double* elementtype(double) nonnull %320, i64 %324)
  %329 = load double, double* %328, align 1
  %330 = add nsw i64 %324, -1
  %331 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %266, i64 8, double* elementtype(double) nonnull %320, i64 %330)
  %332 = load double, double* %331, align 1
  %333 = fadd fast double %332, %329
  %334 = fmul fast double %327, %333
  %335 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %249, i64 %309, double* elementtype(double) %321, i64 %324)
  %336 = load double, double* %335, align 1
  %337 = fmul fast double %334, %336
  %338 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %266, i64 8, double* elementtype(double) nonnull %322, i64 %324)
  store double %337, double* %338, align 1
  %339 = add nsw i64 %324, 1
  %340 = trunc i64 %339 to i32
  %341 = icmp eq i32 %269, %340
  br i1 %341, label %342, label %323

342:                                              ; preds = %323, %316
  %343 = add nuw nsw i64 %317, 1
  %344 = trunc i64 %343 to i32
  %345 = icmp eq i32 %271, %344
  br i1 %345, label %346, label %316

346:                                              ; preds = %342, %54
  %347 = add nsw i32 %230, -1
  %348 = icmp slt i32 %237, %347
  br i1 %348, label %413, label %349

349:                                              ; preds = %346
  %350 = add i32 %138, -3
  %351 = add nsw i32 %150, 2
  %352 = icmp slt i32 %351, %350
  %353 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %19, i64 0, i32 0
  %354 = load double*, double** %353, align 1
  %355 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %19, i64 0, i32 6, i64 0
  %356 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %355, i64 0, i32 1
  %357 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %356, i32 0)
  %358 = load i64, i64* %357, align 1
  %359 = sext i32 %55 to i64
  %360 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %356, i32 1)
  %361 = load i64, i64* %360, align 1
  %362 = sext i32 %56 to i64
  %363 = sext i32 %57 to i64
  %364 = sext i32 %59 to i64
  %365 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %366 = load double*, double** %365, align 1
  %367 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %368 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %367, i64 0, i32 1
  %369 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %368, i32 0)
  %370 = load i64, i64* %369, align 1
  %371 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %368, i32 1)
  %372 = load i64, i64* %371, align 1
  %373 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %368, i32 2)
  %374 = load i64, i64* %373, align 1
  %375 = load i32, i32* %11, align 1
  %376 = sext i32 %375 to i64
  %377 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %374, double* elementtype(double) %366, i64 %376)
  %378 = sext i32 %350 to i64
  %379 = add nsw i32 %150, 3
  %380 = sext i32 %347 to i64
  %381 = add nsw i32 %236, 2
  br label %382

382:                                              ; preds = %409, %349
  %383 = phi i64 [ %380, %349 ], [ %410, %409 ]
  br i1 %352, label %409, label %384

384:                                              ; preds = %382
  %385 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %362, i64 %361, double* elementtype(double) %354, i64 %383)
  %386 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %364, i64 %94, double* elementtype(double) nonnull %91, i64 %383)
  %387 = add nsw i64 %383, -1
  %388 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %364, i64 %94, double* elementtype(double) nonnull %91, i64 %387)
  %389 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %362, i64 %372, double* elementtype(double) %377, i64 %383)
  %390 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %364, i64 %94, double* elementtype(double) nonnull %87, i64 %383)
  br label %391

391:                                              ; preds = %391, %384
  %392 = phi i64 [ %378, %384 ], [ %406, %391 ]
  %393 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %359, i64 %358, double* elementtype(double) %385, i64 %392)
  %394 = load double, double* %393, align 1
  %395 = fmul fast double %394, 5.000000e-01
  %396 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %363, i64 8, double* elementtype(double) nonnull %386, i64 %392)
  %397 = load double, double* %396, align 1
  %398 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %363, i64 8, double* elementtype(double) nonnull %388, i64 %392)
  %399 = load double, double* %398, align 1
  %400 = fadd fast double %399, %397
  %401 = fmul fast double %395, %400
  %402 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %359, i64 %370, double* elementtype(double) %389, i64 %392)
  %403 = load double, double* %402, align 1
  %404 = fmul fast double %401, %403
  %405 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %363, i64 8, double* elementtype(double) nonnull %390, i64 %392)
  store double %404, double* %405, align 1
  %406 = add nsw i64 %392, 1
  %407 = trunc i64 %406 to i32
  %408 = icmp eq i32 %379, %407
  br i1 %408, label %409, label %391

409:                                              ; preds = %391, %382
  %410 = add nsw i64 %383, 1
  %411 = trunc i64 %410 to i32
  %412 = icmp eq i32 %381, %411
  br i1 %412, label %413, label %382

413:                                              ; preds = %409, %346
  %414 = and i32 %226, 1
  %415 = icmp eq i32 %414, 0
  %416 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_scalars_mp_iif_, i64 0, i64 0), i64 %99)
  %417 = load i32, i32* %416, align 1
  %418 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 4112, double* elementtype(double) getelementptr inbounds ([1 x [257 x [2 x double]]], [1 x [257 x [2 x double]]]* @mod_scalars_mp_weight_, i64 0, i64 0, i64 0, i64 0), i64 %99)
  %419 = icmp eq i32 %417, 1
  br i1 %415, label %846, label %844

420:                                              ; preds = %620, %443
  %421 = phi i64 [ %624, %620 ], [ %444, %443 ]
  %422 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %600, i64 %599, double* elementtype(double) %592, i64 %421)
  br label %423

423:                                              ; preds = %423, %420
  %424 = phi i64 [ %621, %420 ], [ %426, %423 ]
  %425 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %597, i64 %596, double* elementtype(double) %422, i64 %424)
  store double 0.000000e+00, double* %425, align 1
  %426 = add nsw i64 %424, 1
  %427 = icmp eq i64 %426, %627
  br i1 %427, label %428, label %423

428:                                              ; preds = %423
  br i1 %601, label %443, label %429

429:                                              ; preds = %428
  %430 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %600, i64 %609, double* elementtype(double) %603, i64 %421)
  %431 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %611, i64 %94, double* elementtype(double) nonnull %88, i64 %421)
  %432 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %600, i64 %619, double* elementtype(double) %613, i64 %421)
  br label %433

433:                                              ; preds = %433, %429
  %434 = phi i64 [ %623, %429 ], [ %440, %433 ]
  %435 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %597, i64 %607, double* elementtype(double) %430, i64 %434)
  store double 0.000000e+00, double* %435, align 1
  %436 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %610, i64 8, double* elementtype(double) nonnull %431, i64 %434)
  %437 = load double, double* %436, align 1
  %438 = fmul fast double %437, %588
  %439 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %597, i64 %617, double* elementtype(double) %432, i64 %434)
  store double %438, double* %439, align 1
  %440 = add nsw i64 %434, 1
  %441 = trunc i64 %440 to i32
  %442 = icmp eq i32 %622, %441
  br i1 %442, label %443, label %433

443:                                              ; preds = %433, %428
  %444 = add nsw i64 %421, 1
  %445 = icmp eq i64 %444, %626
  br i1 %445, label %446, label %420

446:                                              ; preds = %650, %628, %584, %443
  %447 = icmp slt i32 %222, %174
  br i1 %447, label %847, label %448

448:                                              ; preds = %446
  %449 = icmp slt i32 %162, %126
  %450 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %41, i64 0, i32 0
  %451 = load double*, double** %450, align 1
  %452 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %41, i64 0, i32 6, i64 0
  %453 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %452, i64 0, i32 1
  %454 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %453, i32 0)
  %455 = load i64, i64* %454, align 1
  %456 = sext i32 %55 to i64
  %457 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %453, i32 1)
  %458 = load i64, i64* %457, align 1
  %459 = sext i32 %56 to i64
  %460 = sext i32 %57 to i64
  %461 = sext i32 %59 to i64
  %462 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 0
  %463 = load double*, double** %462, align 1
  %464 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 6, i64 0
  %465 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %464, i64 0, i32 1
  %466 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %465, i32 0)
  %467 = load i64, i64* %466, align 1
  %468 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %465, i32 1)
  %469 = load i64, i64* %468, align 1
  %470 = sext i32 %126 to i64
  %471 = add nsw i32 %162, 1
  %472 = sext i32 %174 to i64
  %473 = add nsw i32 %222, 1
  %474 = sext i32 %473 to i64
  %475 = sext i32 %471 to i64
  br label %476

476:                                              ; preds = %491, %448
  %477 = phi i64 [ %472, %448 ], [ %492, %491 ]
  br i1 %449, label %491, label %478

478:                                              ; preds = %476
  %479 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %459, i64 %458, double* elementtype(double) %451, i64 %477)
  %480 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %461, i64 %94, double* elementtype(double) nonnull %87, i64 %477)
  %481 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %459, i64 %469, double* elementtype(double) %463, i64 %477)
  br label %482

482:                                              ; preds = %482, %478
  %483 = phi i64 [ %470, %478 ], [ %489, %482 ]
  %484 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %456, i64 %455, double* elementtype(double) %479, i64 %483)
  store double 0.000000e+00, double* %484, align 1
  %485 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %460, i64 8, double* elementtype(double) nonnull %480, i64 %483)
  %486 = load double, double* %485, align 1
  %487 = fmul fast double %486, %588
  %488 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %456, i64 %467, double* elementtype(double) %481, i64 %483)
  store double %487, double* %488, align 1
  %489 = add nsw i64 %483, 1
  %490 = icmp eq i64 %489, %475
  br i1 %490, label %491, label %482

491:                                              ; preds = %482, %476
  %492 = add nsw i64 %477, 1
  %493 = icmp eq i64 %492, %474
  br i1 %493, label %847, label %476

494:                                              ; preds = %715, %528
  %495 = phi i64 [ %719, %715 ], [ %529, %528 ]
  %496 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %681, double* elementtype(double) %674, i64 %495)
  %497 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %690, double* elementtype(double) %695, i64 %495)
  br label %498

498:                                              ; preds = %498, %494
  %499 = phi i64 [ %716, %494 ], [ %506, %498 ]
  %500 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %678, double* elementtype(double) %496, i64 %499)
  %501 = load double, double* %500, align 1
  %502 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %688, double* elementtype(double) %497, i64 %499)
  %503 = load double, double* %502, align 1
  %504 = fmul fast double %503, %658
  %505 = fadd fast double %504, %501
  store double %505, double* %500, align 1
  %506 = add nsw i64 %499, 1
  %507 = icmp eq i64 %506, %722
  br i1 %507, label %508, label %498

508:                                              ; preds = %498
  br i1 %696, label %528, label %509

509:                                              ; preds = %508
  %510 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %704, double* elementtype(double) %698, i64 %495)
  %511 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %706, i64 %94, double* elementtype(double) nonnull %88, i64 %495)
  %512 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %714, double* elementtype(double) %708, i64 %495)
  br label %513

513:                                              ; preds = %513, %509
  %514 = phi i64 [ %718, %509 ], [ %525, %513 ]
  %515 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %702, double* elementtype(double) %510, i64 %514)
  %516 = load double, double* %515, align 1
  %517 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %705, i64 8, double* elementtype(double) nonnull %511, i64 %514)
  %518 = load double, double* %517, align 1
  %519 = fmul fast double %518, %658
  %520 = fadd fast double %519, %516
  store double %520, double* %515, align 1
  %521 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %712, double* elementtype(double) %512, i64 %514)
  %522 = load double, double* %521, align 1
  %523 = fmul fast double %518, %670
  %524 = fadd fast double %522, %523
  store double %524, double* %521, align 1
  %525 = add nsw i64 %514, 1
  %526 = trunc i64 %525 to i32
  %527 = icmp eq i32 %717, %526
  br i1 %527, label %528, label %513

528:                                              ; preds = %513, %508
  %529 = add nsw i64 %495, 1
  %530 = icmp eq i64 %529, %721
  br i1 %530, label %531, label %494

531:                                              ; preds = %746, %653, %528
  %532 = icmp slt i32 %222, %174
  br i1 %532, label %847, label %533

533:                                              ; preds = %531
  %534 = icmp slt i32 %162, %126
  %535 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %41, i64 0, i32 0
  %536 = load double*, double** %535, align 1
  %537 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %41, i64 0, i32 6, i64 0
  %538 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %537, i64 0, i32 1
  %539 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %538, i32 0)
  %540 = load i64, i64* %539, align 1
  %541 = sext i32 %55 to i64
  %542 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %538, i32 1)
  %543 = load i64, i64* %542, align 1
  %544 = sext i32 %56 to i64
  %545 = sext i32 %57 to i64
  %546 = sext i32 %59 to i64
  %547 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 0
  %548 = load double*, double** %547, align 1
  %549 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 6, i64 0
  %550 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %549, i64 0, i32 1
  %551 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %550, i32 0)
  %552 = load i64, i64* %551, align 1
  %553 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %550, i32 1)
  %554 = load i64, i64* %553, align 1
  %555 = sext i32 %126 to i64
  %556 = add nsw i32 %162, 1
  %557 = sext i32 %174 to i64
  %558 = add nsw i32 %222, 1
  %559 = sext i32 %558 to i64
  %560 = sext i32 %556 to i64
  br label %561

561:                                              ; preds = %581, %533
  %562 = phi i64 [ %557, %533 ], [ %582, %581 ]
  br i1 %534, label %581, label %563

563:                                              ; preds = %561
  %564 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %544, i64 %543, double* elementtype(double) %536, i64 %562)
  %565 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %546, i64 %94, double* elementtype(double) nonnull %87, i64 %562)
  %566 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %544, i64 %554, double* elementtype(double) %548, i64 %562)
  br label %567

567:                                              ; preds = %567, %563
  %568 = phi i64 [ %555, %563 ], [ %579, %567 ]
  %569 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %541, i64 %540, double* elementtype(double) %564, i64 %568)
  %570 = load double, double* %569, align 1
  %571 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %545, i64 8, double* elementtype(double) nonnull %565, i64 %568)
  %572 = load double, double* %571, align 1
  %573 = fmul fast double %572, %658
  %574 = fadd fast double %573, %570
  store double %574, double* %569, align 1
  %575 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %541, i64 %552, double* elementtype(double) %566, i64 %568)
  %576 = load double, double* %575, align 1
  %577 = fmul fast double %572, %670
  %578 = fadd fast double %576, %577
  store double %578, double* %575, align 1
  %579 = add nsw i64 %568, 1
  %580 = icmp eq i64 %579, %560
  br i1 %580, label %581, label %567

581:                                              ; preds = %567, %561
  %582 = add nsw i64 %562, 1
  %583 = icmp eq i64 %582, %559
  br i1 %583, label %847, label %561

584:                                              ; preds = %844
  %585 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 2)
  %586 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %585, i64 2)
  %587 = load double, double* %586, align 1
  %588 = fmul fast double %587, 0xBFB5555555555555
  br i1 %845, label %446, label %589

589:                                              ; preds = %584
  %590 = icmp slt i32 %162, %126
  %591 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %43, i64 0, i32 0
  %592 = load double*, double** %591, align 1
  %593 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %43, i64 0, i32 6, i64 0
  %594 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %593, i64 0, i32 1
  %595 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %594, i32 0)
  %596 = load i64, i64* %595, align 1
  %597 = sext i32 %55 to i64
  %598 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %594, i32 1)
  %599 = load i64, i64* %598, align 1
  %600 = sext i32 %56 to i64
  %601 = icmp slt i32 %162, %114
  %602 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %39, i64 0, i32 0
  %603 = load double*, double** %602, align 1
  %604 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %39, i64 0, i32 6, i64 0
  %605 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %604, i64 0, i32 1
  %606 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %605, i32 0)
  %607 = load i64, i64* %606, align 1
  %608 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %605, i32 1)
  %609 = load i64, i64* %608, align 1
  %610 = sext i32 %57 to i64
  %611 = sext i32 %59 to i64
  %612 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 0
  %613 = load double*, double** %612, align 1
  %614 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 6, i64 0
  %615 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %614, i64 0, i32 1
  %616 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %615, i32 0)
  %617 = load i64, i64* %616, align 1
  %618 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %615, i32 1)
  %619 = load i64, i64* %618, align 1
  br i1 %590, label %628, label %620

620:                                              ; preds = %589
  %621 = sext i32 %126 to i64
  %622 = add nsw i32 %162, 1
  %623 = sext i32 %114 to i64
  %624 = sext i32 %186 to i64
  %625 = add nsw i32 %222, 1
  %626 = sext i32 %625 to i64
  %627 = sext i32 %622 to i64
  br label %420

628:                                              ; preds = %589
  br i1 %601, label %446, label %629

629:                                              ; preds = %628
  %630 = sext i32 %114 to i64
  %631 = add nsw i32 %162, 1
  %632 = sext i32 %186 to i64
  %633 = add nsw i32 %222, 1
  %634 = sext i32 %633 to i64
  br label %635

635:                                              ; preds = %650, %629
  %636 = phi i64 [ %632, %629 ], [ %651, %650 ]
  %637 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %600, i64 %609, double* elementtype(double) %603, i64 %636)
  %638 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %611, i64 %94, double* elementtype(double) nonnull %88, i64 %636)
  %639 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %600, i64 %619, double* elementtype(double) %613, i64 %636)
  br label %640

640:                                              ; preds = %640, %635
  %641 = phi i64 [ %630, %635 ], [ %647, %640 ]
  %642 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %597, i64 %607, double* elementtype(double) %637, i64 %641)
  store double 0.000000e+00, double* %642, align 1
  %643 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %610, i64 8, double* elementtype(double) nonnull %638, i64 %641)
  %644 = load double, double* %643, align 1
  %645 = fmul fast double %644, %588
  %646 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %597, i64 %617, double* elementtype(double) %639, i64 %641)
  store double %645, double* %646, align 1
  %647 = add nsw i64 %641, 1
  %648 = trunc i64 %647 to i32
  %649 = icmp eq i32 %631, %648
  br i1 %649, label %650, label %640

650:                                              ; preds = %640
  %651 = add nsw i64 %636, 1
  %652 = icmp eq i64 %651, %634
  br i1 %652, label %446, label %635

653:                                              ; preds = %844
  %654 = add nsw i32 %417, -1
  %655 = sext i32 %654 to i64
  %656 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 %655)
  %657 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %656, i64 1)
  %658 = load double, double* %657, align 1
  %659 = sext i32 %417 to i64
  %660 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 %659)
  %661 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %660, i64 2)
  %662 = load double, double* %661, align 1
  %663 = fmul fast double %662, 0x3FE5555555555555
  %664 = add nsw i32 %417, 1
  %665 = sext i32 %664 to i64
  %666 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 %665)
  %667 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %666, i64 2)
  %668 = load double, double* %667, align 1
  %669 = fmul fast double %668, 0x3FB5555555555555
  %670 = fsub fast double %663, %669
  br i1 %845, label %531, label %671

671:                                              ; preds = %653
  %672 = icmp slt i32 %162, %126
  %673 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %43, i64 0, i32 0
  %674 = load double*, double** %673, align 1
  %675 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %43, i64 0, i32 6, i64 0
  %676 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %675, i64 0, i32 1
  %677 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %676, i32 0)
  %678 = load i64, i64* %677, align 1
  %679 = sext i32 %55 to i64
  %680 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %676, i32 1)
  %681 = load i64, i64* %680, align 1
  %682 = sext i32 %56 to i64
  %683 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %684 = load double*, double** %683, align 1
  %685 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %686 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %685, i64 0, i32 1
  %687 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %686, i32 0)
  %688 = load i64, i64* %687, align 1
  %689 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %686, i32 1)
  %690 = load i64, i64* %689, align 1
  %691 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %686, i32 2)
  %692 = load i64, i64* %691, align 1
  %693 = load i32, i32* %11, align 1
  %694 = sext i32 %693 to i64
  %695 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %692, double* elementtype(double) %684, i64 %694)
  %696 = icmp slt i32 %162, %114
  %697 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %39, i64 0, i32 0
  %698 = load double*, double** %697, align 1
  %699 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %39, i64 0, i32 6, i64 0
  %700 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %699, i64 0, i32 1
  %701 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %700, i32 0)
  %702 = load i64, i64* %701, align 1
  %703 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %700, i32 1)
  %704 = load i64, i64* %703, align 1
  %705 = sext i32 %57 to i64
  %706 = sext i32 %59 to i64
  %707 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 0
  %708 = load double*, double** %707, align 1
  %709 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 6, i64 0
  %710 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %709, i64 0, i32 1
  %711 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %710, i32 0)
  %712 = load i64, i64* %711, align 1
  %713 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %710, i32 1)
  %714 = load i64, i64* %713, align 1
  br i1 %672, label %723, label %715

715:                                              ; preds = %671
  %716 = sext i32 %126 to i64
  %717 = add nsw i32 %162, 1
  %718 = sext i32 %114 to i64
  %719 = sext i32 %186 to i64
  %720 = add nsw i32 %222, 1
  %721 = sext i32 %720 to i64
  %722 = sext i32 %717 to i64
  br label %494

723:                                              ; preds = %671
  %724 = sext i32 %114 to i64
  %725 = add nsw i32 %162, 1
  %726 = sext i32 %186 to i64
  %727 = add nsw i32 %222, 1
  %728 = sext i32 %727 to i64
  br label %729

729:                                              ; preds = %746, %723
  %730 = phi i64 [ %726, %723 ], [ %747, %746 ]
  br i1 %696, label %746, label %749

731:                                              ; preds = %749, %731
  %732 = phi i64 [ %724, %749 ], [ %743, %731 ]
  %733 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %702, double* elementtype(double) %750, i64 %732)
  %734 = load double, double* %733, align 1
  %735 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %705, i64 8, double* elementtype(double) nonnull %751, i64 %732)
  %736 = load double, double* %735, align 1
  %737 = fmul fast double %736, %658
  %738 = fadd fast double %737, %734
  store double %738, double* %733, align 1
  %739 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %679, i64 %712, double* elementtype(double) %752, i64 %732)
  %740 = load double, double* %739, align 1
  %741 = fmul fast double %736, %670
  %742 = fadd fast double %740, %741
  store double %742, double* %739, align 1
  %743 = add nsw i64 %732, 1
  %744 = trunc i64 %743 to i32
  %745 = icmp eq i32 %725, %744
  br i1 %745, label %746, label %731

746:                                              ; preds = %731, %729
  %747 = add nsw i64 %730, 1
  %748 = icmp eq i64 %747, %728
  br i1 %748, label %531, label %729

749:                                              ; preds = %729
  %750 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %704, double* elementtype(double) %698, i64 %730)
  %751 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %706, i64 %94, double* elementtype(double) nonnull %88, i64 %730)
  %752 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %682, i64 %714, double* elementtype(double) %708, i64 %730)
  br label %731

753:                                              ; preds = %846
  %754 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 1)
  %755 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %754, i64 2)
  %756 = load double, double* %755, align 1
  br label %763

757:                                              ; preds = %846
  %758 = sext i32 %417 to i64
  %759 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* elementtype(double) %418, i64 %758)
  %760 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %759, i64 2)
  %761 = load double, double* %760, align 1
  %762 = fmul fast double %761, 0x3FDAAAAAAAAAAAAB
  br label %763

763:                                              ; preds = %757, %753
  %764 = phi double [ %756, %753 ], [ %762, %757 ]
  %765 = icmp slt i32 %222, %186
  br i1 %765, label %804, label %766

766:                                              ; preds = %763
  %767 = icmp slt i32 %162, %114
  %768 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 0
  %769 = load double*, double** %768, align 1
  %770 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %40, i64 0, i32 6, i64 0
  %771 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %770, i64 0, i32 1
  %772 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %771, i32 0)
  %773 = load i64, i64* %772, align 1
  %774 = sext i32 %55 to i64
  %775 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %771, i32 1)
  %776 = load i64, i64* %775, align 1
  %777 = sext i32 %56 to i64
  %778 = sext i32 %57 to i64
  %779 = sext i32 %59 to i64
  %780 = sext i32 %114 to i64
  %781 = add nsw i32 %162, 1
  %782 = sext i32 %186 to i64
  %783 = add nsw i32 %222, 1
  %784 = sext i32 %783 to i64
  br label %785

785:                                              ; preds = %801, %766
  %786 = phi i64 [ %782, %766 ], [ %802, %801 ]
  br i1 %767, label %801, label %787

787:                                              ; preds = %785
  %788 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %777, i64 %776, double* elementtype(double) %769, i64 %786)
  %789 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %779, i64 %94, double* elementtype(double) nonnull %88, i64 %786)
  br label %790

790:                                              ; preds = %790, %787
  %791 = phi i64 [ %780, %787 ], [ %798, %790 ]
  %792 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %774, i64 %773, double* elementtype(double) %788, i64 %791)
  %793 = load double, double* %792, align 1
  %794 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %778, i64 8, double* elementtype(double) nonnull %789, i64 %791)
  %795 = load double, double* %794, align 1
  %796 = fmul fast double %795, %764
  %797 = fadd fast double %796, %793
  store double %797, double* %792, align 1
  %798 = add nsw i64 %791, 1
  %799 = trunc i64 %798 to i32
  %800 = icmp eq i32 %781, %799
  br i1 %800, label %801, label %790

801:                                              ; preds = %790, %785
  %802 = add nsw i64 %786, 1
  %803 = icmp eq i64 %802, %784
  br i1 %803, label %804, label %785

804:                                              ; preds = %801, %763
  %805 = icmp slt i32 %222, %174
  br i1 %805, label %847, label %806

806:                                              ; preds = %804
  %807 = icmp slt i32 %162, %126
  %808 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 0
  %809 = load double*, double** %808, align 1
  %810 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %42, i64 0, i32 6, i64 0
  %811 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %810, i64 0, i32 1
  %812 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %811, i32 0)
  %813 = load i64, i64* %812, align 1
  %814 = sext i32 %55 to i64
  %815 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %811, i32 1)
  %816 = load i64, i64* %815, align 1
  %817 = sext i32 %56 to i64
  %818 = sext i32 %57 to i64
  %819 = sext i32 %59 to i64
  %820 = sext i32 %126 to i64
  %821 = add nsw i32 %162, 1
  %822 = sext i32 %174 to i64
  %823 = add nsw i32 %222, 1
  %824 = sext i32 %823 to i64
  %825 = sext i32 %821 to i64
  br label %826

826:                                              ; preds = %841, %806
  %827 = phi i64 [ %822, %806 ], [ %842, %841 ]
  br i1 %807, label %841, label %828

828:                                              ; preds = %826
  %829 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %817, i64 %816, double* elementtype(double) %809, i64 %827)
  %830 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %819, i64 %94, double* elementtype(double) nonnull %87, i64 %827)
  br label %831

831:                                              ; preds = %831, %828
  %832 = phi i64 [ %820, %828 ], [ %839, %831 ]
  %833 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %814, i64 %813, double* elementtype(double) %829, i64 %832)
  %834 = load double, double* %833, align 1
  %835 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %818, i64 8, double* elementtype(double) nonnull %830, i64 %832)
  %836 = load double, double* %835, align 1
  %837 = fmul fast double %836, %764
  %838 = fadd fast double %837, %834
  store double %838, double* %833, align 1
  %839 = add nsw i64 %832, 1
  %840 = icmp eq i64 %839, %825
  br i1 %840, label %841, label %831

841:                                              ; preds = %831, %826
  %842 = add nsw i64 %827, 1
  %843 = icmp eq i64 %842, %824
  br i1 %843, label %847, label %826

844:                                              ; preds = %413
  %845 = icmp slt i32 %222, %186
  br i1 %419, label %584, label %653

846:                                              ; preds = %413
  br i1 %419, label %753, label %757

847:                                              ; preds = %841, %804, %581, %531, %491, %446
  %848 = load i32, i32* %416, align 1
  %849 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_scalars_mp_nfast_, i64 0, i64 0), i64 %99)
  %850 = load i32, i32* %849, align 1
  %851 = icmp sgt i32 %848, %850
  br i1 %851, label %4232, label %852

852:                                              ; preds = %847
  %853 = load double, double* @mod_scalars_mp_rho0_, align 8
  %854 = fdiv fast double 1.000000e+03, %853
  %855 = icmp eq i32 %848, 1
  br i1 %855, label %1249, label %1315

856:                                              ; preds = %1253, %925
  %857 = phi i64 [ %1314, %1253 ], [ %926, %925 ]
  br i1 %1255, label %858, label %860

858:                                              ; preds = %856
  %859 = add nsw i64 %857, 1
  br label %925

860:                                              ; preds = %856
  %861 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %88, i64 %857)
  %862 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %87, i64 %857)
  %863 = add nsw i64 %857, 1
  %864 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %87, i64 %863)
  %865 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %76, i64 %857)
  %866 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1266, double* elementtype(double) %1271, i64 %857)
  %867 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1279, double* elementtype(double) %1273, i64 %857)
  %868 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1287, double* elementtype(double) %1281, i64 %857)
  %869 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %75, i64 %857)
  %870 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1295, double* elementtype(double) %1289, i64 %857)
  %871 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %92, i64 %857)
  %872 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %74, i64 %857)
  %873 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1303, double* elementtype(double) %1297, i64 %857)
  %874 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %81, i64 %857)
  %875 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %80, i64 %857)
  %876 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1267, i64 %1311, double* elementtype(double) %1305, i64 %857)
  %877 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1257, i64 %94, double* elementtype(double) nonnull %79, i64 %857)
  br label %878

878:                                              ; preds = %878, %860
  %879 = phi i64 [ %1312, %860 ], [ %882, %878 ]
  %880 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %861, i64 %879)
  %881 = load double, double* %880, align 1
  %882 = add nsw i64 %879, 1
  %883 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %861, i64 %882)
  %884 = load double, double* %883, align 1
  %885 = fsub fast double %881, %884
  %886 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %862, i64 %879)
  %887 = load double, double* %886, align 1
  %888 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %864, i64 %879)
  %889 = load double, double* %888, align 1
  %890 = fsub fast double %887, %889
  %891 = fadd fast double %890, %885
  %892 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %865, i64 %879)
  store double %891, double* %892, align 1
  %893 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1263, double* elementtype(double) %866, i64 %879)
  %894 = load double, double* %893, align 1
  %895 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1277, double* elementtype(double) %867, i64 %879)
  %896 = load double, double* %895, align 1
  %897 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1285, double* elementtype(double) %868, i64 %879)
  %898 = load double, double* %897, align 1
  %899 = fmul fast double %891, %1251
  %900 = fmul fast double %899, %896
  %901 = fmul fast double %900, %898
  %902 = fadd fast double %901, %894
  %903 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %869, i64 %879)
  store double %902, double* %903, align 1
  %904 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1293, double* elementtype(double) %870, i64 %879)
  %905 = load double, double* %904, align 1
  %906 = fadd fast double %905, %902
  %907 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %871, i64 %879)
  store double %906, double* %907, align 1
  %908 = fadd fast double %902, %894
  %909 = fmul fast double %908, 5.000000e-01
  %910 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %872, i64 %879)
  store double %909, double* %910, align 1
  %911 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1301, double* elementtype(double) %873, i64 %879)
  %912 = load double, double* %911, align 1
  %913 = fadd fast double %912, %854
  %914 = fmul fast double %913, %909
  %915 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %874, i64 %879)
  store double %914, double* %915, align 1
  %916 = fmul fast double %914, %909
  %917 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %875, i64 %879)
  store double %916, double* %917, align 1
  %918 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1264, i64 %1309, double* elementtype(double) %876, i64 %879)
  %919 = load double, double* %918, align 1
  %920 = fsub fast double %912, %919
  %921 = fmul fast double %920, %909
  %922 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1256, i64 8, double* elementtype(double) nonnull %877, i64 %879)
  store double %921, double* %922, align 1
  %923 = trunc i64 %882 to i32
  %924 = icmp eq i32 %1313, %923
  br i1 %924, label %925, label %878

925:                                              ; preds = %878, %858
  %926 = phi i64 [ %859, %858 ], [ %863, %878 ]
  %927 = trunc i64 %926 to i32
  %928 = icmp eq i32 %232, %927
  br i1 %928, label %1319, label %856

929:                                              ; preds = %1178, %1003
  %930 = phi i64 [ %1245, %1178 ], [ %1004, %1003 ]
  br i1 %1182, label %931, label %933

931:                                              ; preds = %929
  %932 = add nsw i64 %930, 1
  br label %1003

933:                                              ; preds = %929
  %934 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %88, i64 %930)
  %935 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %87, i64 %930)
  %936 = add nsw i64 %930, 1
  %937 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %87, i64 %936)
  %938 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %76, i64 %930)
  %939 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1193, double* elementtype(double) %1198, i64 %930)
  %940 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1206, double* elementtype(double) %1200, i64 %930)
  %941 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1214, double* elementtype(double) %1208, i64 %930)
  %942 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %75, i64 %930)
  %943 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1223, double* elementtype(double) %1217, i64 %930)
  %944 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %92, i64 %930)
  %945 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1193, double* elementtype(double) %1226, i64 %930)
  %946 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %74, i64 %930)
  %947 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1234, double* elementtype(double) %1228, i64 %930)
  %948 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %81, i64 %930)
  %949 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %80, i64 %930)
  %950 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1194, i64 %1242, double* elementtype(double) %1236, i64 %930)
  %951 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1184, i64 %94, double* elementtype(double) nonnull %79, i64 %930)
  br label %952

952:                                              ; preds = %952, %933
  %953 = phi i64 [ %1243, %933 ], [ %956, %952 ]
  %954 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %934, i64 %953)
  %955 = load double, double* %954, align 1
  %956 = add nsw i64 %953, 1
  %957 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %934, i64 %956)
  %958 = load double, double* %957, align 1
  %959 = fsub fast double %955, %958
  %960 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %935, i64 %953)
  %961 = load double, double* %960, align 1
  %962 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %937, i64 %953)
  %963 = load double, double* %962, align 1
  %964 = fsub fast double %961, %963
  %965 = fadd fast double %964, %959
  %966 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %938, i64 %953)
  store double %965, double* %966, align 1
  %967 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1190, double* elementtype(double) %939, i64 %953)
  %968 = load double, double* %967, align 1
  %969 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1204, double* elementtype(double) %940, i64 %953)
  %970 = load double, double* %969, align 1
  %971 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1212, double* elementtype(double) %941, i64 %953)
  %972 = load double, double* %971, align 1
  %973 = fmul fast double %1215, %965
  %974 = fmul fast double %973, %970
  %975 = fmul fast double %974, %972
  %976 = fadd fast double %975, %968
  %977 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %942, i64 %953)
  store double %976, double* %977, align 1
  %978 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1221, double* elementtype(double) %943, i64 %953)
  %979 = load double, double* %978, align 1
  %980 = fadd fast double %979, %976
  %981 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %944, i64 %953)
  store double %980, double* %981, align 1
  %982 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1190, double* elementtype(double) %945, i64 %953)
  %983 = load double, double* %982, align 1
  %984 = fmul fast double %983, 0x3FE5C28F5C28F5C2
  %985 = fadd fast double %976, %968
  %986 = fmul fast double %985, 1.600000e-01
  %987 = fadd fast double %984, %986
  %988 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %946, i64 %953)
  store double %987, double* %988, align 1
  %989 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1232, double* elementtype(double) %947, i64 %953)
  %990 = load double, double* %989, align 1
  %991 = fadd fast double %990, %854
  %992 = fmul fast double %991, %987
  %993 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %948, i64 %953)
  store double %992, double* %993, align 1
  %994 = fmul fast double %992, %987
  %995 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %949, i64 %953)
  store double %994, double* %995, align 1
  %996 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1191, i64 %1240, double* elementtype(double) %950, i64 %953)
  %997 = load double, double* %996, align 1
  %998 = fsub fast double %990, %997
  %999 = fmul fast double %998, %987
  %1000 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1183, i64 8, double* elementtype(double) nonnull %951, i64 %953)
  store double %999, double* %1000, align 1
  %1001 = trunc i64 %956 to i32
  %1002 = icmp eq i32 %1244, %1001
  br i1 %1002, label %1003, label %952

1003:                                             ; preds = %952, %931
  %1004 = phi i64 [ %932, %931 ], [ %936, %952 ]
  %1005 = trunc i64 %1004 to i32
  %1006 = icmp eq i32 %232, %1005
  br i1 %1006, label %1319, label %929

1007:                                             ; preds = %1097, %1089
  %1008 = phi i64 [ %1175, %1097 ], [ %1090, %1089 ]
  br i1 %1099, label %1009, label %1011

1009:                                             ; preds = %1007
  %1010 = add nsw i64 %1008, 1
  br label %1089

1011:                                             ; preds = %1007
  %1012 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %88, i64 %1008)
  %1013 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %87, i64 %1008)
  %1014 = add nsw i64 %1008, 1
  %1015 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %87, i64 %1014)
  %1016 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1110, double* elementtype(double) %1115, i64 %1008)
  %1017 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1123, double* elementtype(double) %1117, i64 %1008)
  %1018 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1131, double* elementtype(double) %1125, i64 %1008)
  %1019 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1139, double* elementtype(double) %1142, i64 %1008)
  %1020 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1139, double* elementtype(double) %1144, i64 %1008)
  %1021 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %75, i64 %1008)
  %1022 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1153, double* elementtype(double) %1147, i64 %1008)
  %1023 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %92, i64 %1008)
  %1024 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1110, double* elementtype(double) %1156, i64 %1008)
  %1025 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %74, i64 %1008)
  %1026 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1164, double* elementtype(double) %1158, i64 %1008)
  %1027 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %81, i64 %1008)
  %1028 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %80, i64 %1008)
  %1029 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1111, i64 %1172, double* elementtype(double) %1166, i64 %1008)
  %1030 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1101, i64 %94, double* elementtype(double) nonnull %79, i64 %1008)
  br label %1031

1031:                                             ; preds = %1031, %1011
  %1032 = phi i64 [ %1173, %1011 ], [ %1035, %1031 ]
  %1033 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1012, i64 %1032)
  %1034 = load double, double* %1033, align 1
  %1035 = add nsw i64 %1032, 1
  %1036 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1012, i64 %1035)
  %1037 = load double, double* %1036, align 1
  %1038 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1013, i64 %1032)
  %1039 = load double, double* %1038, align 1
  %1040 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1015, i64 %1032)
  %1041 = load double, double* %1040, align 1
  %1042 = fadd fast double %1034, %1039
  %1043 = fadd fast double %1037, %1041
  %1044 = fsub fast double %1042, %1043
  %1045 = fmul fast double %1044, 0x3FDAAAAAAAAAAAAA
  %1046 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1107, double* elementtype(double) %1016, i64 %1032)
  %1047 = load double, double* %1046, align 1
  %1048 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1121, double* elementtype(double) %1017, i64 %1032)
  %1049 = load double, double* %1048, align 1
  %1050 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1129, double* elementtype(double) %1018, i64 %1032)
  %1051 = load double, double* %1050, align 1
  %1052 = fmul fast double %1051, %1049
  %1053 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1137, double* elementtype(double) %1019, i64 %1032)
  %1054 = load double, double* %1053, align 1
  %1055 = fmul fast double %1054, 0x3FE5555555555555
  %1056 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1137, double* elementtype(double) %1020, i64 %1032)
  %1057 = load double, double* %1056, align 1
  %1058 = fmul fast double %1145, %1057
  %1059 = fadd fast double %1055, %1045
  %1060 = fmul fast double %1059, %1095
  %1061 = fsub fast double %1060, %1058
  %1062 = fmul fast double %1052, %1061
  %1063 = fadd fast double %1062, %1047
  %1064 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1021, i64 %1032)
  store double %1063, double* %1064, align 1
  %1065 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1151, double* elementtype(double) %1022, i64 %1032)
  %1066 = load double, double* %1065, align 1
  %1067 = fadd fast double %1063, %1066
  %1068 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1023, i64 %1032)
  store double %1067, double* %1068, align 1
  %1069 = fmul fast double %1063, 6.000000e-01
  %1070 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1107, double* elementtype(double) %1024, i64 %1032)
  %1071 = load double, double* %1070, align 1
  %1072 = fmul fast double %1071, 4.000000e-01
  %1073 = fadd fast double %1072, %1069
  %1074 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1025, i64 %1032)
  store double %1073, double* %1074, align 1
  %1075 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1162, double* elementtype(double) %1026, i64 %1032)
  %1076 = load double, double* %1075, align 1
  %1077 = fadd fast double %1076, %854
  %1078 = fmul fast double %1077, %1073
  %1079 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1027, i64 %1032)
  store double %1078, double* %1079, align 1
  %1080 = fmul fast double %1078, %1073
  %1081 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1028, i64 %1032)
  store double %1080, double* %1081, align 1
  %1082 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1108, i64 %1170, double* elementtype(double) %1029, i64 %1032)
  %1083 = load double, double* %1082, align 1
  %1084 = fsub fast double %1076, %1083
  %1085 = fmul fast double %1084, %1073
  %1086 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1100, i64 8, double* elementtype(double) nonnull %1030, i64 %1032)
  store double %1085, double* %1086, align 1
  %1087 = trunc i64 %1035 to i32
  %1088 = icmp eq i32 %1174, %1087
  br i1 %1088, label %1089, label %1031

1089:                                             ; preds = %1031, %1009
  %1090 = phi i64 [ %1010, %1009 ], [ %1014, %1031 ]
  %1091 = trunc i64 %1090 to i32
  %1092 = icmp eq i32 %232, %1091
  br i1 %1092, label %1319, label %1007

1093:                                             ; preds = %1246
  %1094 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %1095 = load double, double* %1094, align 1
  %1096 = icmp slt i32 %210, %228
  br i1 %1096, label %1319, label %1097

1097:                                             ; preds = %1093
  %1098 = add nsw i32 %138, -1
  %1099 = icmp slt i32 %150, %1098
  %1100 = sext i32 %57 to i64
  %1101 = sext i32 %59 to i64
  %1102 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %1103 = load double*, double** %1102, align 1
  %1104 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %1105 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1104, i64 0, i32 1
  %1106 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1105, i32 0)
  %1107 = load i64, i64* %1106, align 1
  %1108 = sext i32 %55 to i64
  %1109 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1105, i32 1)
  %1110 = load i64, i64* %1109, align 1
  %1111 = sext i32 %56 to i64
  %1112 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1105, i32 2)
  %1113 = load i64, i64* %1112, align 1
  %1114 = sext i32 %223 to i64
  %1115 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1113, double* elementtype(double) %1103, i64 %1114)
  %1116 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %1117 = load double*, double** %1116, align 1
  %1118 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %1119 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1118, i64 0, i32 1
  %1120 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1119, i32 0)
  %1121 = load i64, i64* %1120, align 1
  %1122 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1119, i32 1)
  %1123 = load i64, i64* %1122, align 1
  %1124 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %1125 = load double*, double** %1124, align 1
  %1126 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %1127 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1126, i64 0, i32 1
  %1128 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1127, i32 0)
  %1129 = load i64, i64* %1128, align 1
  %1130 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1127, i32 1)
  %1131 = load i64, i64* %1130, align 1
  %1132 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %50, i64 0, i32 0
  %1133 = load double*, double** %1132, align 1
  %1134 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %50, i64 0, i32 6, i64 0
  %1135 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1134, i64 0, i32 1
  %1136 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1135, i32 0)
  %1137 = load i64, i64* %1136, align 1
  %1138 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1135, i32 1)
  %1139 = load i64, i64* %1138, align 1
  %1140 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1135, i32 2)
  %1141 = load i64, i64* %1140, align 1
  %1142 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1141, double* elementtype(double) %1133, i64 %1114)
  %1143 = sext i32 %224 to i64
  %1144 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1141, double* elementtype(double) %1133, i64 %1143)
  %1145 = fmul fast double %1095, 0x3FB5555555555555
  %1146 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %1147 = load double*, double** %1146, align 1
  %1148 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %1149 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1148, i64 0, i32 1
  %1150 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1149, i32 0)
  %1151 = load i64, i64* %1150, align 1
  %1152 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1149, i32 1)
  %1153 = load i64, i64* %1152, align 1
  %1154 = load i32, i32* %11, align 1
  %1155 = sext i32 %1154 to i64
  %1156 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1113, double* elementtype(double) %1103, i64 %1155)
  %1157 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 0
  %1158 = load double*, double** %1157, align 1
  %1159 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 6, i64 0
  %1160 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1159, i64 0, i32 1
  %1161 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1160, i32 0)
  %1162 = load i64, i64* %1161, align 1
  %1163 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1160, i32 1)
  %1164 = load i64, i64* %1163, align 1
  %1165 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 0
  %1166 = load double*, double** %1165, align 1
  %1167 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 6, i64 0
  %1168 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1167, i64 0, i32 1
  %1169 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1168, i32 0)
  %1170 = load i64, i64* %1169, align 1
  %1171 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1168, i32 1)
  %1172 = load i64, i64* %1171, align 1
  %1173 = sext i32 %1098 to i64
  %1174 = add nsw i32 %150, 1
  %1175 = sext i32 %228 to i64
  br label %1007

1176:                                             ; preds = %1315
  %1177 = icmp slt i32 %210, %228
  br i1 %1177, label %1319, label %1178

1178:                                             ; preds = %1176
  %1179 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %1180 = load double, double* %1179, align 1
  %1181 = add nsw i32 %138, -1
  %1182 = icmp slt i32 %150, %1181
  %1183 = sext i32 %57 to i64
  %1184 = sext i32 %59 to i64
  %1185 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %1186 = load double*, double** %1185, align 1
  %1187 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %1188 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1187, i64 0, i32 1
  %1189 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1188, i32 0)
  %1190 = load i64, i64* %1189, align 1
  %1191 = sext i32 %55 to i64
  %1192 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1188, i32 1)
  %1193 = load i64, i64* %1192, align 1
  %1194 = sext i32 %56 to i64
  %1195 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1188, i32 2)
  %1196 = load i64, i64* %1195, align 1
  %1197 = sext i32 %223 to i64
  %1198 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1196, double* elementtype(double) %1186, i64 %1197)
  %1199 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %1200 = load double*, double** %1199, align 1
  %1201 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %1202 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1201, i64 0, i32 1
  %1203 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1202, i32 0)
  %1204 = load i64, i64* %1203, align 1
  %1205 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1202, i32 1)
  %1206 = load i64, i64* %1205, align 1
  %1207 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %1208 = load double*, double** %1207, align 1
  %1209 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %1210 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1209, i64 0, i32 1
  %1211 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1210, i32 0)
  %1212 = load i64, i64* %1211, align 1
  %1213 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1210, i32 1)
  %1214 = load i64, i64* %1213, align 1
  %1215 = fmul fast double %1180, 2.000000e+00
  %1216 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %1217 = load double*, double** %1216, align 1
  %1218 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %1219 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1218, i64 0, i32 1
  %1220 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1219, i32 0)
  %1221 = load i64, i64* %1220, align 1
  %1222 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1219, i32 1)
  %1223 = load i64, i64* %1222, align 1
  %1224 = load i32, i32* %11, align 1
  %1225 = sext i32 %1224 to i64
  %1226 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1196, double* elementtype(double) %1186, i64 %1225)
  %1227 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 0
  %1228 = load double*, double** %1227, align 1
  %1229 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 6, i64 0
  %1230 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1229, i64 0, i32 1
  %1231 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1230, i32 0)
  %1232 = load i64, i64* %1231, align 1
  %1233 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1230, i32 1)
  %1234 = load i64, i64* %1233, align 1
  %1235 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 0
  %1236 = load double*, double** %1235, align 1
  %1237 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 6, i64 0
  %1238 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1237, i64 0, i32 1
  %1239 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1238, i32 0)
  %1240 = load i64, i64* %1239, align 1
  %1241 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1238, i32 1)
  %1242 = load i64, i64* %1241, align 1
  %1243 = sext i32 %1181 to i64
  %1244 = add nsw i32 %150, 1
  %1245 = sext i32 %228 to i64
  br label %929

1246:                                             ; preds = %1315
  %1247 = and i32 %227, 1
  %1248 = icmp eq i32 %1247, 0
  br i1 %1248, label %1319, label %1093

1249:                                             ; preds = %852
  %1250 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %1251 = load double, double* %1250, align 1
  %1252 = icmp slt i32 %210, %228
  br i1 %1252, label %1319, label %1253

1253:                                             ; preds = %1249
  %1254 = add nsw i32 %138, -1
  %1255 = icmp slt i32 %150, %1254
  %1256 = sext i32 %57 to i64
  %1257 = sext i32 %59 to i64
  %1258 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %1259 = load double*, double** %1258, align 1
  %1260 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %1261 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1260, i64 0, i32 1
  %1262 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1261, i32 0)
  %1263 = load i64, i64* %1262, align 1
  %1264 = sext i32 %55 to i64
  %1265 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1261, i32 1)
  %1266 = load i64, i64* %1265, align 1
  %1267 = sext i32 %56 to i64
  %1268 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1261, i32 2)
  %1269 = load i64, i64* %1268, align 1
  %1270 = sext i32 %223 to i64
  %1271 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1269, double* elementtype(double) %1259, i64 %1270)
  %1272 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %1273 = load double*, double** %1272, align 1
  %1274 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %1275 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1274, i64 0, i32 1
  %1276 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1275, i32 0)
  %1277 = load i64, i64* %1276, align 1
  %1278 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1275, i32 1)
  %1279 = load i64, i64* %1278, align 1
  %1280 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %1281 = load double*, double** %1280, align 1
  %1282 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %1283 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1282, i64 0, i32 1
  %1284 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1283, i32 0)
  %1285 = load i64, i64* %1284, align 1
  %1286 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1283, i32 1)
  %1287 = load i64, i64* %1286, align 1
  %1288 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %1289 = load double*, double** %1288, align 1
  %1290 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %1291 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1290, i64 0, i32 1
  %1292 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1291, i32 0)
  %1293 = load i64, i64* %1292, align 1
  %1294 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1291, i32 1)
  %1295 = load i64, i64* %1294, align 1
  %1296 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 0
  %1297 = load double*, double** %1296, align 1
  %1298 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %38, i64 0, i32 6, i64 0
  %1299 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1298, i64 0, i32 1
  %1300 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1299, i32 0)
  %1301 = load i64, i64* %1300, align 1
  %1302 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1299, i32 1)
  %1303 = load i64, i64* %1302, align 1
  %1304 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 0
  %1305 = load double*, double** %1304, align 1
  %1306 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 6, i64 0
  %1307 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1306, i64 0, i32 1
  %1308 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1307, i32 0)
  %1309 = load i64, i64* %1308, align 1
  %1310 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1307, i32 1)
  %1311 = load i64, i64* %1310, align 1
  %1312 = sext i32 %1254 to i64
  %1313 = add nsw i32 %150, 1
  %1314 = sext i32 %228 to i64
  br label %856

1315:                                             ; preds = %852
  %1316 = load i32, i32* %225, align 1
  %1317 = and i32 %1316, 1
  %1318 = icmp eq i32 %1317, 0
  br i1 %1318, label %1246, label %1176

1319:                                             ; preds = %1249, %1246, %1176, %1093, %1089, %1003, %925
  %1320 = icmp slt i32 %210, %174
  br i1 %1320, label %1661, label %1321

1321:                                             ; preds = %1319
  %1322 = icmp slt i32 %150, %114
  %1323 = sext i32 %57 to i64
  %1324 = sext i32 %59 to i64
  %1325 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %1326 = load double*, double** %1325, align 1
  %1327 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %1328 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1327, i64 0, i32 1
  %1329 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1328, i32 0)
  %1330 = load i64, i64* %1329, align 1
  %1331 = sext i32 %55 to i64
  %1332 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1328, i32 1)
  %1333 = load i64, i64* %1332, align 1
  %1334 = sext i32 %56 to i64
  %1335 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1328, i32 2)
  %1336 = load i64, i64* %1335, align 1
  %1337 = load i32, i32* %13, align 1
  %1338 = sext i32 %1337 to i64
  %1339 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1336, double* elementtype(double) %1326, i64 %1338)
  %1340 = sext i32 %114 to i64
  %1341 = add nsw i32 %150, 1
  %1342 = sext i32 %174 to i64
  %1343 = sext i32 %232 to i64
  br label %1344

1344:                                             ; preds = %1357, %1321
  %1345 = phi i64 [ %1342, %1321 ], [ %1358, %1357 ]
  br i1 %1322, label %1357, label %1346

1346:                                             ; preds = %1344
  %1347 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %75, i64 %1345)
  %1348 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1333, double* elementtype(double) %1339, i64 %1345)
  br label %1349

1349:                                             ; preds = %1349, %1346
  %1350 = phi i64 [ %1340, %1346 ], [ %1354, %1349 ]
  %1351 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1347, i64 %1350)
  %1352 = load double, double* %1351, align 1
  %1353 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1330, double* elementtype(double) %1348, i64 %1350)
  store double %1352, double* %1353, align 1
  %1354 = add nsw i64 %1350, 1
  %1355 = trunc i64 %1354 to i32
  %1356 = icmp eq i32 %1341, %1355
  br i1 %1356, label %1357, label %1349

1357:                                             ; preds = %1349, %1344
  %1358 = add nsw i64 %1345, 1
  %1359 = icmp eq i64 %1358, %1343
  br i1 %1359, label %1360, label %1344

1360:                                             ; preds = %1357
  %1361 = load i32, i32* %225, align 1
  %1362 = and i32 %1361, 1
  %1363 = icmp eq i32 %1362, 0
  br i1 %1363, label %1394, label %1380

1364:                                             ; preds = %1380, %1377
  %1365 = phi i64 [ %1342, %1380 ], [ %1378, %1377 ]
  br i1 %1322, label %1377, label %1366

1366:                                             ; preds = %1364
  %1367 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %76, i64 %1365)
  %1368 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1388, double* elementtype(double) %1393, i64 %1365)
  br label %1369

1369:                                             ; preds = %1369, %1366
  %1370 = phi i64 [ %1340, %1366 ], [ %1374, %1369 ]
  %1371 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1367, i64 %1370)
  %1372 = load double, double* %1371, align 1
  %1373 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1386, double* elementtype(double) %1368, i64 %1370)
  store double %1372, double* %1373, align 1
  %1374 = add nsw i64 %1370, 1
  %1375 = trunc i64 %1374 to i32
  %1376 = icmp eq i32 %1341, %1375
  br i1 %1376, label %1377, label %1369

1377:                                             ; preds = %1369, %1364
  %1378 = add nsw i64 %1365, 1
  %1379 = icmp eq i64 %1378, %1343
  br i1 %1379, label %1394, label %1364

1380:                                             ; preds = %1360
  %1381 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %50, i64 0, i32 0
  %1382 = load double*, double** %1381, align 1
  %1383 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %50, i64 0, i32 6, i64 0
  %1384 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1383, i64 0, i32 1
  %1385 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1384, i32 0)
  %1386 = load i64, i64* %1385, align 1
  %1387 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1384, i32 1)
  %1388 = load i64, i64* %1387, align 1
  %1389 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1384, i32 2)
  %1390 = load i64, i64* %1389, align 1
  %1391 = load i32, i32* %11, align 1
  %1392 = sext i32 %1391 to i64
  %1393 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1390, double* elementtype(double) %1382, i64 %1392)
  br label %1364

1394:                                             ; preds = %1377, %1360
  %1395 = load double, double* @mod_scalars_mp_g_, align 8
  %1396 = fmul fast double %1395, 5.000000e-01
  %1397 = icmp slt i32 %150, %138
  %1398 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %20, i64 0, i32 0
  %1399 = load double*, double** %1398, align 1
  %1400 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %20, i64 0, i32 6, i64 0
  %1401 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1400, i64 0, i32 1
  %1402 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1401, i32 0)
  %1403 = load i64, i64* %1402, align 1
  %1404 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1401, i32 1)
  %1405 = load i64, i64* %1404, align 1
  %1406 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %1407 = load double*, double** %1406, align 1
  %1408 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %1409 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1408, i64 0, i32 1
  %1410 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1409, i32 0)
  %1411 = load i64, i64* %1410, align 1
  %1412 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1409, i32 1)
  %1413 = load i64, i64* %1412, align 1
  %1414 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 0
  %1415 = load double*, double** %1414, align 1
  %1416 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %37, i64 0, i32 6, i64 0
  %1417 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1416, i64 0, i32 1
  %1418 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1417, i32 0)
  %1419 = load i64, i64* %1418, align 1
  %1420 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1417, i32 1)
  %1421 = load i64, i64* %1420, align 1
  %1422 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %19, i64 0, i32 0
  %1423 = load double*, double** %1422, align 1
  %1424 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %19, i64 0, i32 6, i64 0
  %1425 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1424, i64 0, i32 1
  %1426 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1425, i32 0)
  %1427 = load i64, i64* %1426, align 1
  %1428 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1425, i32 1)
  %1429 = load i64, i64* %1428, align 1
  %1430 = sext i32 %138 to i64
  %1431 = sext i32 %198 to i64
  br label %1432

1432:                                             ; preds = %1559, %1394
  %1433 = phi i64 [ %1342, %1394 ], [ %1560, %1559 ]
  br i1 %1397, label %1492, label %1434

1434:                                             ; preds = %1432
  %1435 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1405, double* elementtype(double) %1399, i64 %1433)
  %1436 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1413, double* elementtype(double) %1407, i64 %1433)
  %1437 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %81, i64 %1433)
  %1438 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %79, i64 %1433)
  %1439 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1421, double* elementtype(double) %1415, i64 %1433)
  %1440 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %74, i64 %1433)
  %1441 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %80, i64 %1433)
  %1442 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %78, i64 %1433)
  br label %1443

1443:                                             ; preds = %1443, %1434
  %1444 = phi i64 [ %1430, %1434 ], [ %1489, %1443 ]
  %1445 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1403, double* elementtype(double) %1435, i64 %1444)
  %1446 = load double, double* %1445, align 1
  %1447 = fmul fast double %1446, %1396
  %1448 = add nsw i64 %1444, -1
  %1449 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1411, double* elementtype(double) %1436, i64 %1448)
  %1450 = load double, double* %1449, align 1
  %1451 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1411, double* elementtype(double) %1436, i64 %1444)
  %1452 = load double, double* %1451, align 1
  %1453 = fadd fast double %1452, %1450
  %1454 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1437, i64 %1448)
  %1455 = load double, double* %1454, align 1
  %1456 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1437, i64 %1444)
  %1457 = load double, double* %1456, align 1
  %1458 = fsub fast double %1455, %1457
  %1459 = fmul fast double %1458, %1453
  %1460 = fsub fast double %1450, %1452
  %1461 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1438, i64 %1448)
  %1462 = load double, double* %1461, align 1
  %1463 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1438, i64 %1444)
  %1464 = load double, double* %1463, align 1
  %1465 = fadd fast double %1464, %1462
  %1466 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1419, double* elementtype(double) %1439, i64 %1448)
  %1467 = load double, double* %1466, align 1
  %1468 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1419, double* elementtype(double) %1439, i64 %1444)
  %1469 = load double, double* %1468, align 1
  %1470 = fsub fast double %1467, %1469
  %1471 = fmul fast double %1470, 0x3FD5555555555555
  %1472 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1440, i64 %1448)
  %1473 = load double, double* %1472, align 1
  %1474 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1440, i64 %1444)
  %1475 = load double, double* %1474, align 1
  %1476 = fsub fast double %1473, %1475
  %1477 = fmul fast double %1471, %1476
  %1478 = fadd fast double %1465, %1477
  %1479 = fmul fast double %1478, %1460
  %1480 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1441, i64 %1448)
  %1481 = load double, double* %1480, align 1
  %1482 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1441, i64 %1444)
  %1483 = load double, double* %1482, align 1
  %1484 = fadd fast double %1481, %1459
  %1485 = fsub fast double %1484, %1483
  %1486 = fadd fast double %1485, %1479
  %1487 = fmul fast double %1447, %1486
  %1488 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1442, i64 %1444)
  store double %1487, double* %1488, align 1
  %1489 = add nsw i64 %1444, 1
  %1490 = trunc i64 %1489 to i32
  %1491 = icmp eq i32 %1341, %1490
  br i1 %1491, label %1492, label %1443

1492:                                             ; preds = %1443, %1432
  %1493 = icmp slt i64 %1433, %1431
  %1494 = select i1 %1493, i1 true, i1 %1322
  br i1 %1494, label %1559, label %1495

1495:                                             ; preds = %1492
  %1496 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1429, double* elementtype(double) %1423, i64 %1433)
  %1497 = add nsw i64 %1433, -1
  %1498 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1413, double* elementtype(double) %1407, i64 %1497)
  %1499 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1413, double* elementtype(double) %1407, i64 %1433)
  %1500 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %81, i64 %1497)
  %1501 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %81, i64 %1433)
  %1502 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %79, i64 %1497)
  %1503 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %79, i64 %1433)
  %1504 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1421, double* elementtype(double) %1415, i64 %1497)
  %1505 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1421, double* elementtype(double) %1415, i64 %1433)
  %1506 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %74, i64 %1497)
  %1507 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %74, i64 %1433)
  %1508 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %80, i64 %1497)
  %1509 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %80, i64 %1433)
  %1510 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %77, i64 %1433)
  br label %1511

1511:                                             ; preds = %1511, %1495
  %1512 = phi i64 [ %1340, %1495 ], [ %1556, %1511 ]
  %1513 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1427, double* elementtype(double) %1496, i64 %1512)
  %1514 = load double, double* %1513, align 1
  %1515 = fmul fast double %1514, %1396
  %1516 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1411, double* elementtype(double) %1498, i64 %1512)
  %1517 = load double, double* %1516, align 1
  %1518 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1411, double* elementtype(double) %1499, i64 %1512)
  %1519 = load double, double* %1518, align 1
  %1520 = fadd fast double %1519, %1517
  %1521 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1500, i64 %1512)
  %1522 = load double, double* %1521, align 1
  %1523 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1501, i64 %1512)
  %1524 = load double, double* %1523, align 1
  %1525 = fsub fast double %1522, %1524
  %1526 = fmul fast double %1525, %1520
  %1527 = fsub fast double %1517, %1519
  %1528 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1502, i64 %1512)
  %1529 = load double, double* %1528, align 1
  %1530 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1503, i64 %1512)
  %1531 = load double, double* %1530, align 1
  %1532 = fadd fast double %1531, %1529
  %1533 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1419, double* elementtype(double) %1504, i64 %1512)
  %1534 = load double, double* %1533, align 1
  %1535 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1419, double* elementtype(double) %1505, i64 %1512)
  %1536 = load double, double* %1535, align 1
  %1537 = fsub fast double %1534, %1536
  %1538 = fmul fast double %1537, 0x3FD5555555555555
  %1539 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1506, i64 %1512)
  %1540 = load double, double* %1539, align 1
  %1541 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1507, i64 %1512)
  %1542 = load double, double* %1541, align 1
  %1543 = fsub fast double %1540, %1542
  %1544 = fmul fast double %1538, %1543
  %1545 = fadd fast double %1532, %1544
  %1546 = fmul fast double %1545, %1527
  %1547 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1508, i64 %1512)
  %1548 = load double, double* %1547, align 1
  %1549 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1509, i64 %1512)
  %1550 = load double, double* %1549, align 1
  %1551 = fadd fast double %1548, %1526
  %1552 = fsub fast double %1551, %1550
  %1553 = fadd fast double %1552, %1546
  %1554 = fmul fast double %1515, %1553
  %1555 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1510, i64 %1512)
  store double %1554, double* %1555, align 1
  %1556 = add nsw i64 %1512, 1
  %1557 = trunc i64 %1556 to i32
  %1558 = icmp eq i32 %1341, %1557
  br i1 %1558, label %1559, label %1511

1559:                                             ; preds = %1511, %1492
  %1560 = add nsw i64 %1433, 1
  %1561 = icmp eq i64 %1560, %1343
  br i1 %1561, label %1562, label %1432

1562:                                             ; preds = %1559
  %1563 = add i32 %138, -1
  %1564 = icmp slt i32 %1341, %1563
  %1565 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %1566 = load double*, double** %1565, align 1
  %1567 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %1568 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1567, i64 0, i32 1
  %1569 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1568, i32 0)
  %1570 = load i64, i64* %1569, align 1
  %1571 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1568, i32 1)
  %1572 = load i64, i64* %1571, align 1
  %1573 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1568, i32 2)
  %1574 = load i64, i64* %1573, align 1
  %1575 = load i32, i32* %11, align 1
  %1576 = sext i32 %1575 to i64
  %1577 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1574, double* elementtype(double) %1566, i64 %1576)
  %1578 = sext i32 %1563 to i64
  %1579 = add nsw i32 %150, 2
  br label %1580

1580:                                             ; preds = %1613, %1562
  %1581 = phi i64 [ %1342, %1562 ], [ %1614, %1613 ]
  br i1 %1564, label %1613, label %1582

1582:                                             ; preds = %1580
  %1583 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1572, double* elementtype(double) %1577, i64 %1581)
  %1584 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %82, i64 %1581)
  %1585 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %88, i64 %1581)
  %1586 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %93, i64 %1581)
  br label %1587

1587:                                             ; preds = %1587, %1582
  %1588 = phi i64 [ %1578, %1582 ], [ %1596, %1587 ]
  %1589 = add nsw i64 %1588, -1
  %1590 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1570, double* elementtype(double) %1583, i64 %1589)
  %1591 = load double, double* %1590, align 1
  %1592 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1570, double* elementtype(double) %1583, i64 %1588)
  %1593 = load double, double* %1592, align 1
  %1594 = fmul fast double %1593, -2.000000e+00
  %1595 = fadd fast double %1594, %1591
  %1596 = add nsw i64 %1588, 1
  %1597 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1570, double* elementtype(double) %1583, i64 %1596)
  %1598 = load double, double* %1597, align 1
  %1599 = fadd fast double %1595, %1598
  %1600 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1584, i64 %1588)
  store double %1599, double* %1600, align 1
  %1601 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1585, i64 %1589)
  %1602 = load double, double* %1601, align 1
  %1603 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1585, i64 %1588)
  %1604 = load double, double* %1603, align 1
  %1605 = fmul fast double %1604, -2.000000e+00
  %1606 = fadd fast double %1605, %1602
  %1607 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1585, i64 %1596)
  %1608 = load double, double* %1607, align 1
  %1609 = fadd fast double %1606, %1608
  %1610 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1586, i64 %1588)
  store double %1609, double* %1610, align 1
  %1611 = trunc i64 %1596 to i32
  %1612 = icmp eq i32 %1579, %1611
  br i1 %1612, label %1613, label %1587

1613:                                             ; preds = %1587, %1580
  %1614 = add nsw i64 %1581, 1
  %1615 = icmp eq i64 %1614, %1343
  br i1 %1615, label %1616, label %1580

1616:                                             ; preds = %1613
  %1617 = icmp slt i32 %150, %1563
  br label %1618

1618:                                             ; preds = %1658, %1616
  %1619 = phi i64 [ %1342, %1616 ], [ %1659, %1658 ]
  br i1 %1617, label %1658, label %1620

1620:                                             ; preds = %1618
  %1621 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1334, i64 %1572, double* elementtype(double) %1577, i64 %1619)
  %1622 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %82, i64 %1619)
  %1623 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %88, i64 %1619)
  %1624 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %93, i64 %1619)
  %1625 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1324, i64 %94, double* elementtype(double) nonnull %85, i64 %1619)
  br label %1626

1626:                                             ; preds = %1626, %1620
  %1627 = phi i64 [ %1578, %1620 ], [ %1630, %1626 ]
  %1628 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1570, double* elementtype(double) %1621, i64 %1627)
  %1629 = load double, double* %1628, align 1
  %1630 = add nsw i64 %1627, 1
  %1631 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1331, i64 %1570, double* elementtype(double) %1621, i64 %1630)
  %1632 = load double, double* %1631, align 1
  %1633 = fadd fast double %1632, %1629
  %1634 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1622, i64 %1627)
  %1635 = load double, double* %1634, align 1
  %1636 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1622, i64 %1630)
  %1637 = load double, double* %1636, align 1
  %1638 = fadd fast double %1637, %1635
  %1639 = fmul fast double %1638, 0xBFC5555555555555
  %1640 = fadd fast double %1633, %1639
  %1641 = fmul fast double %1640, 2.500000e-01
  %1642 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1623, i64 %1627)
  %1643 = load double, double* %1642, align 1
  %1644 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1623, i64 %1630)
  %1645 = load double, double* %1644, align 1
  %1646 = fadd fast double %1645, %1643
  %1647 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1624, i64 %1627)
  %1648 = load double, double* %1647, align 1
  %1649 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1624, i64 %1630)
  %1650 = load double, double* %1649, align 1
  %1651 = fadd fast double %1650, %1648
  %1652 = fmul fast double %1651, 0xBFC5555555555555
  %1653 = fadd fast double %1646, %1652
  %1654 = fmul fast double %1641, %1653
  %1655 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1323, i64 8, double* elementtype(double) nonnull %1625, i64 %1627)
  store double %1654, double* %1655, align 1
  %1656 = trunc i64 %1630 to i32
  %1657 = icmp eq i32 %1341, %1656
  br i1 %1657, label %1658, label %1626

1658:                                             ; preds = %1626, %1618
  %1659 = add nsw i64 %1619, 1
  %1660 = icmp eq i64 %1659, %1343
  br i1 %1660, label %1661, label %1618

1661:                                             ; preds = %1658, %1319
  %1662 = add nsw i32 %174, -1
  %1663 = icmp sgt i32 %174, 1
  %1664 = select i1 %1663, i32 %1662, i32 1
  %1665 = load i32, i32* %233, align 1
  %1666 = icmp slt i32 %210, %1665
  %1667 = select i1 %1666, i32 %232, i32 %1665
  %1668 = icmp slt i32 %1667, %1664
  br i1 %1668, label %1722, label %1669

1669:                                             ; preds = %1661
  %1670 = icmp slt i32 %150, %138
  %1671 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %1672 = load double*, double** %1671, align 1
  %1673 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %1674 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1673, i64 0, i32 1
  %1675 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1674, i32 0)
  %1676 = load i64, i64* %1675, align 1
  %1677 = sext i32 %55 to i64
  %1678 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1674, i32 1)
  %1679 = load i64, i64* %1678, align 1
  %1680 = sext i32 %56 to i64
  %1681 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1674, i32 2)
  %1682 = load i64, i64* %1681, align 1
  %1683 = load i32, i32* %11, align 1
  %1684 = sext i32 %1683 to i64
  %1685 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1682, double* elementtype(double) %1672, i64 %1684)
  %1686 = sext i32 %57 to i64
  %1687 = sext i32 %59 to i64
  %1688 = sext i32 %138 to i64
  %1689 = add nsw i32 %150, 1
  %1690 = zext i32 %1664 to i64
  %1691 = add nsw i32 %1667, 1
  %1692 = sext i32 %1691 to i64
  %1693 = sext i32 %1689 to i64
  br label %1694

1694:                                             ; preds = %1719, %1669
  %1695 = phi i64 [ %1690, %1669 ], [ %1720, %1719 ]
  br i1 %1670, label %1696, label %1698

1696:                                             ; preds = %1694
  %1697 = add nsw i64 %1695, 1
  br label %1719

1698:                                             ; preds = %1694
  %1699 = add nsw i64 %1695, -1
  %1700 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1680, i64 %1679, double* elementtype(double) %1685, i64 %1699)
  %1701 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1680, i64 %1679, double* elementtype(double) %1685, i64 %1695)
  %1702 = add nsw i64 %1695, 1
  %1703 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1680, i64 %1679, double* elementtype(double) %1685, i64 %1702)
  %1704 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1687, i64 %94, double* elementtype(double) nonnull %82, i64 %1695)
  br label %1705

1705:                                             ; preds = %1705, %1698
  %1706 = phi i64 [ %1688, %1698 ], [ %1717, %1705 ]
  %1707 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1677, i64 %1676, double* elementtype(double) %1700, i64 %1706)
  %1708 = load double, double* %1707, align 1
  %1709 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1677, i64 %1676, double* elementtype(double) %1701, i64 %1706)
  %1710 = load double, double* %1709, align 1
  %1711 = fmul fast double %1710, -2.000000e+00
  %1712 = fadd fast double %1711, %1708
  %1713 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1677, i64 %1676, double* elementtype(double) %1703, i64 %1706)
  %1714 = load double, double* %1713, align 1
  %1715 = fadd fast double %1712, %1714
  %1716 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1686, i64 8, double* elementtype(double) nonnull %1704, i64 %1706)
  store double %1715, double* %1716, align 1
  %1717 = add nsw i64 %1706, 1
  %1718 = icmp eq i64 %1717, %1693
  br i1 %1718, label %1719, label %1705

1719:                                             ; preds = %1705, %1696
  %1720 = phi i64 [ %1697, %1696 ], [ %1702, %1705 ]
  %1721 = icmp eq i64 %1720, %1692
  br i1 %1721, label %1722, label %1694

1722:                                             ; preds = %1719, %1661
  %1723 = icmp ne i32 %174, 1
  %1724 = icmp slt i32 %150, %138
  %1725 = select i1 %1723, i1 true, i1 %1724
  br i1 %1725, label %1742, label %1726

1726:                                             ; preds = %1722
  %1727 = sext i32 %57 to i64
  %1728 = sext i32 %59 to i64
  %1729 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1728, i64 %94, double* elementtype(double) nonnull %82, i64 1)
  %1730 = zext i32 %1662 to i64
  %1731 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1728, i64 %94, double* elementtype(double) nonnull %82, i64 %1730)
  %1732 = sext i32 %138 to i64
  %1733 = add nsw i32 %150, 1
  %1734 = sext i32 %1733 to i64
  br label %1735

1735:                                             ; preds = %1735, %1726
  %1736 = phi i64 [ %1732, %1726 ], [ %1740, %1735 ]
  %1737 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1727, i64 8, double* elementtype(double) nonnull %1729, i64 %1736)
  %1738 = load double, double* %1737, align 1
  %1739 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1727, i64 8, double* elementtype(double) nonnull %1731, i64 %1736)
  store double %1738, double* %1739, align 1
  %1740 = add nsw i64 %1736, 1
  %1741 = icmp eq i64 %1740, %1734
  br i1 %1741, label %1742, label %1735

1742:                                             ; preds = %1735, %1722
  %1743 = icmp ne i32 %210, %1665
  %1744 = select i1 %1743, i1 true, i1 %1724
  br i1 %1744, label %1762, label %1745

1745:                                             ; preds = %1742
  %1746 = sext i32 %57 to i64
  %1747 = sext i32 %59 to i64
  %1748 = sext i32 %210 to i64
  %1749 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1747, i64 %94, double* elementtype(double) nonnull %82, i64 %1748)
  %1750 = sext i32 %232 to i64
  %1751 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1747, i64 %94, double* elementtype(double) nonnull %82, i64 %1750)
  %1752 = sext i32 %138 to i64
  %1753 = add nsw i32 %150, 1
  %1754 = sext i32 %1753 to i64
  br label %1755

1755:                                             ; preds = %1755, %1745
  %1756 = phi i64 [ %1752, %1745 ], [ %1760, %1755 ]
  %1757 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1746, i64 8, double* elementtype(double) nonnull %1749, i64 %1756)
  %1758 = load double, double* %1757, align 1
  %1759 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1746, i64 8, double* elementtype(double) nonnull %1751, i64 %1756)
  store double %1758, double* %1759, align 1
  %1760 = add nsw i64 %1756, 1
  %1761 = icmp eq i64 %1760, %1754
  br i1 %1761, label %1762, label %1755

1762:                                             ; preds = %1755, %1742
  %1763 = icmp slt i32 %232, %174
  br i1 %1763, label %1863, label %1764

1764:                                             ; preds = %1762
  %1765 = add i32 %138, -1
  %1766 = icmp slt i32 %150, %1765
  %1767 = sext i32 %57 to i64
  %1768 = sext i32 %59 to i64
  %1769 = sext i32 %1765 to i64
  %1770 = add nsw i32 %150, 1
  %1771 = sext i32 %174 to i64
  %1772 = add nsw i32 %210, 2
  br label %1773

1773:                                             ; preds = %1794, %1764
  %1774 = phi i64 [ %1771, %1764 ], [ %1795, %1794 ]
  br i1 %1766, label %1794, label %1775

1775:                                             ; preds = %1773
  %1776 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %87, i64 %1774)
  %1777 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %93, i64 %1774)
  br label %1778

1778:                                             ; preds = %1778, %1775
  %1779 = phi i64 [ %1769, %1775 ], [ %1787, %1778 ]
  %1780 = add nsw i64 %1779, -1
  %1781 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1776, i64 %1780)
  %1782 = load double, double* %1781, align 1
  %1783 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1776, i64 %1779)
  %1784 = load double, double* %1783, align 1
  %1785 = fmul fast double %1784, -2.000000e+00
  %1786 = fadd fast double %1785, %1782
  %1787 = add nsw i64 %1779, 1
  %1788 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1776, i64 %1787)
  %1789 = load double, double* %1788, align 1
  %1790 = fadd fast double %1786, %1789
  %1791 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1777, i64 %1779)
  store double %1790, double* %1791, align 1
  %1792 = trunc i64 %1787 to i32
  %1793 = icmp eq i32 %1770, %1792
  br i1 %1793, label %1794, label %1778

1794:                                             ; preds = %1778, %1773
  %1795 = add nsw i64 %1774, 1
  %1796 = trunc i64 %1795 to i32
  %1797 = icmp eq i32 %1772, %1796
  br i1 %1797, label %1798, label %1773

1798:                                             ; preds = %1794
  %1799 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %1800 = load double*, double** %1799, align 1
  %1801 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %1802 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1801, i64 0, i32 1
  %1803 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1802, i32 0)
  %1804 = load i64, i64* %1803, align 1
  %1805 = sext i32 %55 to i64
  %1806 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1802, i32 1)
  %1807 = load i64, i64* %1806, align 1
  %1808 = sext i32 %56 to i64
  %1809 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1802, i32 2)
  %1810 = load i64, i64* %1809, align 1
  %1811 = load i32, i32* %11, align 1
  %1812 = sext i32 %1811 to i64
  %1813 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1810, double* elementtype(double) %1800, i64 %1812)
  %1814 = sext i32 %138 to i64
  %1815 = sext i32 %1770 to i64
  br label %1816

1816:                                             ; preds = %1859, %1798
  %1817 = phi i64 [ %1771, %1798 ], [ %1860, %1859 ]
  br i1 %1724, label %1859, label %1818

1818:                                             ; preds = %1816
  %1819 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1808, i64 %1807, double* elementtype(double) %1813, i64 %1817)
  %1820 = add nsw i64 %1817, -1
  %1821 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1808, i64 %1807, double* elementtype(double) %1813, i64 %1820)
  %1822 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %82, i64 %1817)
  %1823 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %82, i64 %1820)
  %1824 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %87, i64 %1817)
  %1825 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %93, i64 %1817)
  %1826 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1768, i64 %94, double* elementtype(double) nonnull %86, i64 %1817)
  br label %1827

1827:                                             ; preds = %1827, %1818
  %1828 = phi i64 [ %1814, %1818 ], [ %1857, %1827 ]
  %1829 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1805, i64 %1804, double* elementtype(double) %1819, i64 %1828)
  %1830 = load double, double* %1829, align 1
  %1831 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1805, i64 %1804, double* elementtype(double) %1821, i64 %1828)
  %1832 = load double, double* %1831, align 1
  %1833 = fadd fast double %1832, %1830
  %1834 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1822, i64 %1828)
  %1835 = load double, double* %1834, align 1
  %1836 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1823, i64 %1828)
  %1837 = load double, double* %1836, align 1
  %1838 = fadd fast double %1837, %1835
  %1839 = fmul fast double %1838, 0xBFC5555555555555
  %1840 = fadd fast double %1833, %1839
  %1841 = fmul fast double %1840, 2.500000e-01
  %1842 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1824, i64 %1828)
  %1843 = load double, double* %1842, align 1
  %1844 = add nsw i64 %1828, -1
  %1845 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1824, i64 %1844)
  %1846 = load double, double* %1845, align 1
  %1847 = fadd fast double %1846, %1843
  %1848 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1825, i64 %1828)
  %1849 = load double, double* %1848, align 1
  %1850 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1825, i64 %1844)
  %1851 = load double, double* %1850, align 1
  %1852 = fadd fast double %1851, %1849
  %1853 = fmul fast double %1852, 0xBFC5555555555555
  %1854 = fadd fast double %1847, %1853
  %1855 = fmul fast double %1841, %1854
  %1856 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1767, i64 8, double* elementtype(double) nonnull %1826, i64 %1828)
  store double %1855, double* %1856, align 1
  %1857 = add nsw i64 %1828, 1
  %1858 = icmp eq i64 %1857, %1815
  br i1 %1858, label %1859, label %1827

1859:                                             ; preds = %1827, %1816
  %1860 = add nsw i64 %1817, 1
  %1861 = trunc i64 %1860 to i32
  %1862 = icmp eq i32 %1772, %1861
  br i1 %1862, label %1863, label %1816

1863:                                             ; preds = %1859, %1762
  %1864 = icmp slt i32 %210, %198
  br i1 %1864, label %1914, label %1865

1865:                                             ; preds = %1863
  %1866 = add nsw i32 %114, -1
  %1867 = add nsw i32 %150, 1
  %1868 = icmp slt i32 %1867, %1866
  %1869 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %1870 = load double*, double** %1869, align 1
  %1871 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %1872 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1871, i64 0, i32 1
  %1873 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1872, i32 0)
  %1874 = load i64, i64* %1873, align 1
  %1875 = sext i32 %55 to i64
  %1876 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1872, i32 1)
  %1877 = load i64, i64* %1876, align 1
  %1878 = sext i32 %56 to i64
  %1879 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1872, i32 2)
  %1880 = load i64, i64* %1879, align 1
  %1881 = load i32, i32* %11, align 1
  %1882 = sext i32 %1881 to i64
  %1883 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1880, double* elementtype(double) %1870, i64 %1882)
  %1884 = sext i32 %57 to i64
  %1885 = sext i32 %59 to i64
  %1886 = sext i32 %1866 to i64
  %1887 = add nsw i32 %150, 2
  %1888 = sext i32 %198 to i64
  %1889 = sext i32 %232 to i64
  br label %1890

1890:                                             ; preds = %1911, %1865
  %1891 = phi i64 [ %1888, %1865 ], [ %1912, %1911 ]
  br i1 %1868, label %1911, label %1892

1892:                                             ; preds = %1890
  %1893 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1878, i64 %1877, double* elementtype(double) %1883, i64 %1891)
  %1894 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1885, i64 %94, double* elementtype(double) nonnull %82, i64 %1891)
  br label %1895

1895:                                             ; preds = %1895, %1892
  %1896 = phi i64 [ %1886, %1892 ], [ %1904, %1895 ]
  %1897 = add nsw i64 %1896, -1
  %1898 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1875, i64 %1874, double* elementtype(double) %1893, i64 %1897)
  %1899 = load double, double* %1898, align 1
  %1900 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1875, i64 %1874, double* elementtype(double) %1893, i64 %1896)
  %1901 = load double, double* %1900, align 1
  %1902 = fmul fast double %1901, -2.000000e+00
  %1903 = fadd fast double %1902, %1899
  %1904 = add nsw i64 %1896, 1
  %1905 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1875, i64 %1874, double* elementtype(double) %1893, i64 %1904)
  %1906 = load double, double* %1905, align 1
  %1907 = fadd fast double %1903, %1906
  %1908 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1884, i64 8, double* elementtype(double) nonnull %1894, i64 %1896)
  store double %1907, double* %1908, align 1
  %1909 = trunc i64 %1904 to i32
  %1910 = icmp eq i32 %1887, %1909
  br i1 %1910, label %1911, label %1895

1911:                                             ; preds = %1895, %1890
  %1912 = add nsw i64 %1891, 1
  %1913 = icmp eq i64 %1912, %1889
  br i1 %1913, label %1914, label %1890

1914:                                             ; preds = %1911, %1863
  %1915 = icmp slt i32 %210, %228
  br i1 %1915, label %1954, label %1916

1916:                                             ; preds = %1914
  %1917 = add nsw i32 %150, 1
  %1918 = icmp slt i32 %1917, %114
  %1919 = sext i32 %57 to i64
  %1920 = sext i32 %59 to i64
  %1921 = sext i32 %114 to i64
  %1922 = add nsw i32 %150, 2
  %1923 = sext i32 %228 to i64
  br label %1924

1924:                                             ; preds = %1950, %1916
  %1925 = phi i64 [ %1923, %1916 ], [ %1951, %1950 ]
  br i1 %1918, label %1926, label %1928

1926:                                             ; preds = %1924
  %1927 = add nsw i64 %1925, 1
  br label %1950

1928:                                             ; preds = %1924
  %1929 = add nsw i64 %1925, -1
  %1930 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1920, i64 %94, double* elementtype(double) nonnull %88, i64 %1929)
  %1931 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1920, i64 %94, double* elementtype(double) nonnull %88, i64 %1925)
  %1932 = add nsw i64 %1925, 1
  %1933 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1920, i64 %94, double* elementtype(double) nonnull %88, i64 %1932)
  %1934 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1920, i64 %94, double* elementtype(double) nonnull %93, i64 %1925)
  br label %1935

1935:                                             ; preds = %1935, %1928
  %1936 = phi i64 [ %1921, %1928 ], [ %1947, %1935 ]
  %1937 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1919, i64 8, double* elementtype(double) nonnull %1930, i64 %1936)
  %1938 = load double, double* %1937, align 1
  %1939 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1919, i64 8, double* elementtype(double) nonnull %1931, i64 %1936)
  %1940 = load double, double* %1939, align 1
  %1941 = fmul fast double %1940, -2.000000e+00
  %1942 = fadd fast double %1941, %1938
  %1943 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1919, i64 8, double* elementtype(double) nonnull %1933, i64 %1936)
  %1944 = load double, double* %1943, align 1
  %1945 = fadd fast double %1942, %1944
  %1946 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1919, i64 8, double* elementtype(double) nonnull %1934, i64 %1936)
  store double %1945, double* %1946, align 1
  %1947 = add nsw i64 %1936, 1
  %1948 = trunc i64 %1947 to i32
  %1949 = icmp eq i32 %1922, %1948
  br i1 %1949, label %1950, label %1935

1950:                                             ; preds = %1935, %1926
  %1951 = phi i64 [ %1927, %1926 ], [ %1932, %1935 ]
  %1952 = trunc i64 %1951 to i32
  %1953 = icmp eq i32 %232, %1952
  br i1 %1953, label %1954, label %1924

1954:                                             ; preds = %1950, %1914
  br i1 %1864, label %2026, label %1955

1955:                                             ; preds = %1954
  %1956 = add nsw i32 %150, 1
  %1957 = icmp slt i32 %1956, %114
  %1958 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %1959 = load double*, double** %1958, align 1
  %1960 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %1961 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %1960, i64 0, i32 1
  %1962 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1961, i32 0)
  %1963 = load i64, i64* %1962, align 1
  %1964 = sext i32 %55 to i64
  %1965 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1961, i32 1)
  %1966 = load i64, i64* %1965, align 1
  %1967 = sext i32 %56 to i64
  %1968 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %1961, i32 2)
  %1969 = load i64, i64* %1968, align 1
  %1970 = load i32, i32* %11, align 1
  %1971 = sext i32 %1970 to i64
  %1972 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %1969, double* elementtype(double) %1959, i64 %1971)
  %1973 = sext i32 %57 to i64
  %1974 = sext i32 %59 to i64
  %1975 = sext i32 %114 to i64
  %1976 = add nsw i32 %150, 2
  %1977 = sext i32 %198 to i64
  %1978 = sext i32 %232 to i64
  br label %1979

1979:                                             ; preds = %2023, %1955
  %1980 = phi i64 [ %1977, %1955 ], [ %2024, %2023 ]
  br i1 %1957, label %2023, label %1981

1981:                                             ; preds = %1979
  %1982 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1967, i64 %1966, double* elementtype(double) %1972, i64 %1980)
  %1983 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %82, i64 %1980)
  %1984 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %88, i64 %1980)
  %1985 = add nsw i64 %1980, -1
  %1986 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %88, i64 %1985)
  %1987 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %93, i64 %1980)
  %1988 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %93, i64 %1985)
  %1989 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %1974, i64 %94, double* elementtype(double) nonnull %83, i64 %1980)
  br label %1990

1990:                                             ; preds = %1990, %1981
  %1991 = phi i64 [ %1975, %1981 ], [ %2020, %1990 ]
  %1992 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1964, i64 %1963, double* elementtype(double) %1982, i64 %1991)
  %1993 = load double, double* %1992, align 1
  %1994 = add nsw i64 %1991, -1
  %1995 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1964, i64 %1963, double* elementtype(double) %1982, i64 %1994)
  %1996 = load double, double* %1995, align 1
  %1997 = fadd fast double %1996, %1993
  %1998 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1983, i64 %1991)
  %1999 = load double, double* %1998, align 1
  %2000 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1983, i64 %1994)
  %2001 = load double, double* %2000, align 1
  %2002 = fadd fast double %2001, %1999
  %2003 = fmul fast double %2002, 0xBFC5555555555555
  %2004 = fadd fast double %1997, %2003
  %2005 = fmul fast double %2004, 2.500000e-01
  %2006 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1984, i64 %1991)
  %2007 = load double, double* %2006, align 1
  %2008 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1986, i64 %1991)
  %2009 = load double, double* %2008, align 1
  %2010 = fadd fast double %2009, %2007
  %2011 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1987, i64 %1991)
  %2012 = load double, double* %2011, align 1
  %2013 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1988, i64 %1991)
  %2014 = load double, double* %2013, align 1
  %2015 = fadd fast double %2014, %2012
  %2016 = fmul fast double %2015, 0xBFC5555555555555
  %2017 = fadd fast double %2010, %2016
  %2018 = fmul fast double %2005, %2017
  %2019 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %1973, i64 8, double* elementtype(double) nonnull %1989, i64 %1991)
  store double %2018, double* %2019, align 1
  %2020 = add nsw i64 %1991, 1
  %2021 = trunc i64 %2020 to i32
  %2022 = icmp eq i32 %1976, %2021
  br i1 %2022, label %2023, label %1990

2023:                                             ; preds = %1990, %1979
  %2024 = add nsw i64 %1980, 1
  %2025 = icmp eq i64 %2024, %1978
  br i1 %2025, label %2026, label %1979

2026:                                             ; preds = %2023, %1954
  %2027 = icmp slt i32 %1667, %230
  br i1 %2027, label %2095, label %2028

2028:                                             ; preds = %2026
  %2029 = icmp slt i32 %150, %114
  %2030 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %2031 = load double*, double** %2030, align 1
  %2032 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %2033 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2032, i64 0, i32 1
  %2034 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2033, i32 0)
  %2035 = load i64, i64* %2034, align 1
  %2036 = sext i32 %55 to i64
  %2037 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2033, i32 1)
  %2038 = load i64, i64* %2037, align 1
  %2039 = sext i32 %56 to i64
  %2040 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2033, i32 2)
  %2041 = load i64, i64* %2040, align 1
  %2042 = load i32, i32* %11, align 1
  %2043 = sext i32 %2042 to i64
  %2044 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2041, double* elementtype(double) %2031, i64 %2043)
  %2045 = sext i32 %57 to i64
  %2046 = sext i32 %59 to i64
  %2047 = sext i32 %114 to i64
  %2048 = add nsw i32 %150, 1
  %2049 = sext i32 %230 to i64
  %2050 = add nsw i32 %1667, 1
  %2051 = sext i32 %2050 to i64
  %2052 = sext i32 %2048 to i64
  br label %2053

2053:                                             ; preds = %2092, %2028
  %2054 = phi i64 [ %2049, %2028 ], [ %2093, %2092 ]
  br i1 %2029, label %2055, label %2057

2055:                                             ; preds = %2053
  %2056 = add nsw i64 %2054, 1
  br label %2092

2057:                                             ; preds = %2053
  %2058 = add nsw i64 %2054, -1
  %2059 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2039, i64 %2038, double* elementtype(double) %2044, i64 %2058)
  %2060 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2039, i64 %2038, double* elementtype(double) %2044, i64 %2054)
  %2061 = add nsw i64 %2054, 1
  %2062 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2039, i64 %2038, double* elementtype(double) %2044, i64 %2061)
  %2063 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2046, i64 %94, double* elementtype(double) nonnull %82, i64 %2054)
  %2064 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2046, i64 %94, double* elementtype(double) nonnull %87, i64 %2058)
  %2065 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2046, i64 %94, double* elementtype(double) nonnull %87, i64 %2054)
  %2066 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2046, i64 %94, double* elementtype(double) nonnull %87, i64 %2061)
  %2067 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2046, i64 %94, double* elementtype(double) nonnull %93, i64 %2054)
  br label %2068

2068:                                             ; preds = %2068, %2057
  %2069 = phi i64 [ %2047, %2057 ], [ %2090, %2068 ]
  %2070 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2036, i64 %2035, double* elementtype(double) %2059, i64 %2069)
  %2071 = load double, double* %2070, align 1
  %2072 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2036, i64 %2035, double* elementtype(double) %2060, i64 %2069)
  %2073 = load double, double* %2072, align 1
  %2074 = fmul fast double %2073, -2.000000e+00
  %2075 = fadd fast double %2074, %2071
  %2076 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2036, i64 %2035, double* elementtype(double) %2062, i64 %2069)
  %2077 = load double, double* %2076, align 1
  %2078 = fadd fast double %2075, %2077
  %2079 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2045, i64 8, double* elementtype(double) nonnull %2063, i64 %2069)
  store double %2078, double* %2079, align 1
  %2080 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2045, i64 8, double* elementtype(double) nonnull %2064, i64 %2069)
  %2081 = load double, double* %2080, align 1
  %2082 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2045, i64 8, double* elementtype(double) nonnull %2065, i64 %2069)
  %2083 = load double, double* %2082, align 1
  %2084 = fmul fast double %2083, -2.000000e+00
  %2085 = fadd fast double %2084, %2081
  %2086 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2045, i64 8, double* elementtype(double) nonnull %2066, i64 %2069)
  %2087 = load double, double* %2086, align 1
  %2088 = fadd fast double %2085, %2087
  %2089 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2045, i64 8, double* elementtype(double) nonnull %2067, i64 %2069)
  store double %2088, double* %2089, align 1
  %2090 = add nsw i64 %2069, 1
  %2091 = icmp eq i64 %2090, %2052
  br i1 %2091, label %2092, label %2068

2092:                                             ; preds = %2068, %2055
  %2093 = phi i64 [ %2056, %2055 ], [ %2061, %2068 ]
  %2094 = icmp eq i64 %2093, %2051
  br i1 %2094, label %2095, label %2053

2095:                                             ; preds = %2092, %2026
  %2096 = icmp slt i32 %150, %114
  %2097 = select i1 %1723, i1 true, i1 %2096
  br i1 %2097, label %2118, label %2098

2098:                                             ; preds = %2095
  %2099 = sext i32 %57 to i64
  %2100 = sext i32 %59 to i64
  %2101 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2100, i64 %94, double* elementtype(double) nonnull %82, i64 2)
  %2102 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2100, i64 %94, double* elementtype(double) nonnull %82, i64 1)
  %2103 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2100, i64 %94, double* elementtype(double) nonnull %93, i64 2)
  %2104 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2100, i64 %94, double* elementtype(double) nonnull %93, i64 1)
  %2105 = sext i32 %114 to i64
  %2106 = add nsw i32 %150, 1
  %2107 = sext i32 %2106 to i64
  br label %2108

2108:                                             ; preds = %2108, %2098
  %2109 = phi i64 [ %2105, %2098 ], [ %2116, %2108 ]
  %2110 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2099, i64 8, double* elementtype(double) nonnull %2101, i64 %2109)
  %2111 = load double, double* %2110, align 1
  %2112 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2099, i64 8, double* elementtype(double) nonnull %2102, i64 %2109)
  store double %2111, double* %2112, align 1
  %2113 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2099, i64 8, double* elementtype(double) nonnull %2103, i64 %2109)
  %2114 = load double, double* %2113, align 1
  %2115 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2099, i64 8, double* elementtype(double) nonnull %2104, i64 %2109)
  store double %2114, double* %2115, align 1
  %2116 = add nsw i64 %2109, 1
  %2117 = icmp eq i64 %2116, %2107
  br i1 %2117, label %2118, label %2108

2118:                                             ; preds = %2108, %2095
  %2119 = select i1 %1743, i1 true, i1 %2096
  br i1 %2119, label %2142, label %2120

2120:                                             ; preds = %2118
  %2121 = sext i32 %57 to i64
  %2122 = sext i32 %59 to i64
  %2123 = sext i32 %210 to i64
  %2124 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2122, i64 %94, double* elementtype(double) nonnull %82, i64 %2123)
  %2125 = sext i32 %232 to i64
  %2126 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2122, i64 %94, double* elementtype(double) nonnull %82, i64 %2125)
  %2127 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2122, i64 %94, double* elementtype(double) nonnull %93, i64 %2123)
  %2128 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2122, i64 %94, double* elementtype(double) nonnull %93, i64 %2125)
  %2129 = sext i32 %114 to i64
  %2130 = add nsw i32 %150, 1
  %2131 = sext i32 %2130 to i64
  br label %2132

2132:                                             ; preds = %2132, %2120
  %2133 = phi i64 [ %2129, %2120 ], [ %2140, %2132 ]
  %2134 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2121, i64 8, double* elementtype(double) nonnull %2124, i64 %2133)
  %2135 = load double, double* %2134, align 1
  %2136 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2121, i64 8, double* elementtype(double) nonnull %2126, i64 %2133)
  store double %2135, double* %2136, align 1
  %2137 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2121, i64 8, double* elementtype(double) nonnull %2127, i64 %2133)
  %2138 = load double, double* %2137, align 1
  %2139 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2121, i64 8, double* elementtype(double) nonnull %2128, i64 %2133)
  store double %2138, double* %2139, align 1
  %2140 = add nsw i64 %2133, 1
  %2141 = icmp eq i64 %2140, %2131
  br i1 %2141, label %2142, label %2132

2142:                                             ; preds = %2132, %2118
  br i1 %1915, label %2215, label %2143

2143:                                             ; preds = %2142
  %2144 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %2145 = load double*, double** %2144, align 1
  %2146 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %2147 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2146, i64 0, i32 1
  %2148 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2147, i32 0)
  %2149 = load i64, i64* %2148, align 1
  %2150 = sext i32 %55 to i64
  %2151 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2147, i32 1)
  %2152 = load i64, i64* %2151, align 1
  %2153 = sext i32 %56 to i64
  %2154 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2147, i32 2)
  %2155 = load i64, i64* %2154, align 1
  %2156 = load i32, i32* %11, align 1
  %2157 = sext i32 %2156 to i64
  %2158 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2155, double* elementtype(double) %2145, i64 %2157)
  %2159 = sext i32 %57 to i64
  %2160 = sext i32 %59 to i64
  %2161 = sext i32 %114 to i64
  %2162 = add nsw i32 %150, 1
  %2163 = sext i32 %228 to i64
  %2164 = sext i32 %2162 to i64
  br label %2165

2165:                                             ; preds = %2211, %2143
  %2166 = phi i64 [ %2163, %2143 ], [ %2212, %2211 ]
  br i1 %2096, label %2167, label %2169

2167:                                             ; preds = %2165
  %2168 = add nsw i64 %2166, 1
  br label %2211

2169:                                             ; preds = %2165
  %2170 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2153, i64 %2152, double* elementtype(double) %2158, i64 %2166)
  %2171 = add nsw i64 %2166, 1
  %2172 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2153, i64 %2152, double* elementtype(double) %2158, i64 %2171)
  %2173 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %82, i64 %2166)
  %2174 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %82, i64 %2171)
  %2175 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %87, i64 %2166)
  %2176 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %87, i64 %2171)
  %2177 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %93, i64 %2166)
  %2178 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %93, i64 %2171)
  %2179 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2160, i64 %94, double* elementtype(double) nonnull %84, i64 %2166)
  br label %2180

2180:                                             ; preds = %2180, %2169
  %2181 = phi i64 [ %2161, %2169 ], [ %2209, %2180 ]
  %2182 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2150, i64 %2149, double* elementtype(double) %2170, i64 %2181)
  %2183 = load double, double* %2182, align 1
  %2184 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2150, i64 %2149, double* elementtype(double) %2172, i64 %2181)
  %2185 = load double, double* %2184, align 1
  %2186 = fadd fast double %2185, %2183
  %2187 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2173, i64 %2181)
  %2188 = load double, double* %2187, align 1
  %2189 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2174, i64 %2181)
  %2190 = load double, double* %2189, align 1
  %2191 = fadd fast double %2190, %2188
  %2192 = fmul fast double %2191, 0xBFC5555555555555
  %2193 = fadd fast double %2186, %2192
  %2194 = fmul fast double %2193, 2.500000e-01
  %2195 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2175, i64 %2181)
  %2196 = load double, double* %2195, align 1
  %2197 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2176, i64 %2181)
  %2198 = load double, double* %2197, align 1
  %2199 = fadd fast double %2198, %2196
  %2200 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2177, i64 %2181)
  %2201 = load double, double* %2200, align 1
  %2202 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2178, i64 %2181)
  %2203 = load double, double* %2202, align 1
  %2204 = fadd fast double %2203, %2201
  %2205 = fmul fast double %2204, 0xBFC5555555555555
  %2206 = fadd fast double %2199, %2205
  %2207 = fmul fast double %2194, %2206
  %2208 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2159, i64 8, double* elementtype(double) nonnull %2179, i64 %2181)
  store double %2207, double* %2208, align 1
  %2209 = add nsw i64 %2181, 1
  %2210 = icmp eq i64 %2209, %2164
  br i1 %2210, label %2211, label %2180

2211:                                             ; preds = %2180, %2167
  %2212 = phi i64 [ %2168, %2167 ], [ %2171, %2180 ]
  %2213 = trunc i64 %2212 to i32
  %2214 = icmp eq i32 %232, %2213
  br i1 %2214, label %2215, label %2165

2215:                                             ; preds = %2211, %2142
  br i1 %1320, label %2256, label %2216

2216:                                             ; preds = %2215
  %2217 = sext i32 %57 to i64
  %2218 = sext i32 %59 to i64
  %2219 = sext i32 %138 to i64
  %2220 = add nsw i32 %150, 1
  %2221 = sext i32 %174 to i64
  %2222 = sext i32 %232 to i64
  %2223 = sext i32 %2220 to i64
  br label %2224

2224:                                             ; preds = %2253, %2216
  %2225 = phi i64 [ %2221, %2216 ], [ %2254, %2253 ]
  br i1 %1724, label %2226, label %2228

2226:                                             ; preds = %2224
  %2227 = add nsw i64 %2225, 1
  br label %2253

2228:                                             ; preds = %2224
  %2229 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2218, i64 %94, double* elementtype(double) nonnull %85, i64 %2225)
  %2230 = add nsw i64 %2225, 1
  %2231 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2218, i64 %94, double* elementtype(double) nonnull %86, i64 %2230)
  %2232 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2218, i64 %94, double* elementtype(double) nonnull %86, i64 %2225)
  %2233 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2218, i64 %94, double* elementtype(double) nonnull %78, i64 %2225)
  br label %2234

2234:                                             ; preds = %2234, %2228
  %2235 = phi i64 [ %2219, %2228 ], [ %2251, %2234 ]
  %2236 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2217, i64 8, double* elementtype(double) nonnull %2229, i64 %2235)
  %2237 = load double, double* %2236, align 1
  %2238 = add nsw i64 %2235, -1
  %2239 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2217, i64 8, double* elementtype(double) nonnull %2229, i64 %2238)
  %2240 = load double, double* %2239, align 1
  %2241 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2217, i64 8, double* elementtype(double) nonnull %2231, i64 %2235)
  %2242 = load double, double* %2241, align 1
  %2243 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2217, i64 8, double* elementtype(double) nonnull %2232, i64 %2235)
  %2244 = load double, double* %2243, align 1
  %2245 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2217, i64 8, double* elementtype(double) nonnull %2233, i64 %2235)
  %2246 = load double, double* %2245, align 1
  %2247 = fadd fast double %2237, %2242
  %2248 = fsub fast double %2240, %2247
  %2249 = fadd fast double %2248, %2244
  %2250 = fadd fast double %2249, %2246
  store double %2250, double* %2245, align 1
  %2251 = add nsw i64 %2235, 1
  %2252 = icmp eq i64 %2251, %2223
  br i1 %2252, label %2253, label %2234

2253:                                             ; preds = %2234, %2226
  %2254 = phi i64 [ %2227, %2226 ], [ %2230, %2234 ]
  %2255 = icmp eq i64 %2254, %2222
  br i1 %2255, label %2256, label %2224

2256:                                             ; preds = %2253, %2215
  br i1 %1864, label %2294, label %2257

2257:                                             ; preds = %2256
  %2258 = sext i32 %57 to i64
  %2259 = sext i32 %59 to i64
  %2260 = sext i32 %114 to i64
  %2261 = add nsw i32 %150, 1
  %2262 = sext i32 %198 to i64
  %2263 = sext i32 %232 to i64
  %2264 = sext i32 %2261 to i64
  br label %2265

2265:                                             ; preds = %2291, %2257
  %2266 = phi i64 [ %2262, %2257 ], [ %2292, %2291 ]
  br i1 %2096, label %2291, label %2267

2267:                                             ; preds = %2265
  %2268 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2259, i64 %94, double* elementtype(double) nonnull %83, i64 %2266)
  %2269 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2259, i64 %94, double* elementtype(double) nonnull %84, i64 %2266)
  %2270 = add nsw i64 %2266, -1
  %2271 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2259, i64 %94, double* elementtype(double) nonnull %84, i64 %2270)
  %2272 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2259, i64 %94, double* elementtype(double) nonnull %77, i64 %2266)
  br label %2273

2273:                                             ; preds = %2273, %2267
  %2274 = phi i64 [ %2260, %2267 ], [ %2275, %2273 ]
  %2275 = add nsw i64 %2274, 1
  %2276 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2258, i64 8, double* elementtype(double) nonnull %2268, i64 %2275)
  %2277 = load double, double* %2276, align 1
  %2278 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2258, i64 8, double* elementtype(double) nonnull %2268, i64 %2274)
  %2279 = load double, double* %2278, align 1
  %2280 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2258, i64 8, double* elementtype(double) nonnull %2269, i64 %2274)
  %2281 = load double, double* %2280, align 1
  %2282 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2258, i64 8, double* elementtype(double) nonnull %2271, i64 %2274)
  %2283 = load double, double* %2282, align 1
  %2284 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2258, i64 8, double* elementtype(double) nonnull %2272, i64 %2274)
  %2285 = load double, double* %2284, align 1
  %2286 = fadd fast double %2277, %2281
  %2287 = fsub fast double %2279, %2286
  %2288 = fadd fast double %2287, %2283
  %2289 = fadd fast double %2288, %2285
  store double %2289, double* %2284, align 1
  %2290 = icmp eq i64 %2275, %2264
  br i1 %2290, label %2291, label %2273

2291:                                             ; preds = %2273, %2265
  %2292 = add nsw i64 %2266, 1
  %2293 = icmp eq i64 %2292, %2263
  br i1 %2293, label %2294, label %2265

2294:                                             ; preds = %2291, %2256
  br i1 %1915, label %2406, label %2295

2295:                                             ; preds = %2294
  %2296 = add i32 %138, -1
  %2297 = icmp slt i32 %150, %2296
  %2298 = sext i32 %57 to i64
  %2299 = sext i32 %59 to i64
  %2300 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %16, i64 0, i32 0
  %2301 = load double*, double** %2300, align 1
  %2302 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %16, i64 0, i32 6, i64 0
  %2303 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2302, i64 0, i32 1
  %2304 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2303, i32 0)
  %2305 = load i64, i64* %2304, align 1
  %2306 = sext i32 %55 to i64
  %2307 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2303, i32 1)
  %2308 = load i64, i64* %2307, align 1
  %2309 = sext i32 %56 to i64
  %2310 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %2311 = load double*, double** %2310, align 1
  %2312 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %2313 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2312, i64 0, i32 1
  %2314 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2313, i32 0)
  %2315 = load i64, i64* %2314, align 1
  %2316 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2313, i32 1)
  %2317 = load i64, i64* %2316, align 1
  %2318 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2313, i32 2)
  %2319 = load i64, i64* %2318, align 1
  %2320 = load i32, i32* %11, align 1
  %2321 = sext i32 %2320 to i64
  %2322 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2319, double* elementtype(double) %2311, i64 %2321)
  %2323 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 0
  %2324 = load double*, double** %2323, align 1
  %2325 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 6, i64 0
  %2326 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2325, i64 0, i32 1
  %2327 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2326, i32 0)
  %2328 = load i64, i64* %2327, align 1
  %2329 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2326, i32 1)
  %2330 = load i64, i64* %2329, align 1
  %2331 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %2332 = load double*, double** %2331, align 1
  %2333 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %2334 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2333, i64 0, i32 1
  %2335 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2334, i32 0)
  %2336 = load i64, i64* %2335, align 1
  %2337 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2334, i32 1)
  %2338 = load i64, i64* %2337, align 1
  %2339 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2334, i32 2)
  %2340 = load i64, i64* %2339, align 1
  %2341 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2340, double* elementtype(double) %2332, i64 %2321)
  %2342 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %26, i64 0, i32 0
  %2343 = load double*, double** %2342, align 1
  %2344 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %26, i64 0, i32 6, i64 0
  %2345 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2344, i64 0, i32 1
  %2346 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2345, i32 0)
  %2347 = load i64, i64* %2346, align 1
  %2348 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2345, i32 1)
  %2349 = load i64, i64* %2348, align 1
  %2350 = sext i32 %2296 to i64
  %2351 = add nsw i32 %150, 1
  %2352 = sext i32 %228 to i64
  br label %2353

2353:                                             ; preds = %2402, %2295
  %2354 = phi i64 [ %2352, %2295 ], [ %2403, %2402 ]
  br i1 %2297, label %2355, label %2357

2355:                                             ; preds = %2353
  %2356 = add nsw i64 %2354, 1
  br label %2402

2357:                                             ; preds = %2353
  %2358 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2299, i64 %94, double* elementtype(double) nonnull %91, i64 %2354)
  %2359 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2308, double* elementtype(double) %2301, i64 %2354)
  %2360 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2317, double* elementtype(double) %2322, i64 %2354)
  %2361 = add nsw i64 %2354, 1
  %2362 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2317, double* elementtype(double) %2322, i64 %2361)
  %2363 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2330, double* elementtype(double) %2324, i64 %2354)
  %2364 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2338, double* elementtype(double) %2341, i64 %2354)
  %2365 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2309, i64 %2349, double* elementtype(double) %2343, i64 %2354)
  %2366 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2299, i64 %94, double* elementtype(double) nonnull %85, i64 %2354)
  %2367 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2299, i64 %94, double* elementtype(double) nonnull %84, i64 %2354)
  br label %2368

2368:                                             ; preds = %2368, %2357
  %2369 = phi i64 [ %2350, %2357 ], [ %2385, %2368 ]
  %2370 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2298, i64 8, double* elementtype(double) nonnull %2358, i64 %2369)
  %2371 = load double, double* %2370, align 1
  %2372 = fmul fast double %2371, 5.000000e-01
  %2373 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2305, double* elementtype(double) %2359, i64 %2369)
  %2374 = load double, double* %2373, align 1
  %2375 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2315, double* elementtype(double) %2360, i64 %2369)
  %2376 = load double, double* %2375, align 1
  %2377 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2315, double* elementtype(double) %2362, i64 %2369)
  %2378 = load double, double* %2377, align 1
  %2379 = fadd fast double %2378, %2376
  %2380 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2328, double* elementtype(double) %2363, i64 %2369)
  %2381 = load double, double* %2380, align 1
  %2382 = fmul fast double %2381, %2379
  %2383 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2336, double* elementtype(double) %2364, i64 %2369)
  %2384 = load double, double* %2383, align 1
  %2385 = add nsw i64 %2369, 1
  %2386 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2336, double* elementtype(double) %2364, i64 %2385)
  %2387 = load double, double* %2386, align 1
  %2388 = fadd fast double %2387, %2384
  %2389 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2306, i64 %2347, double* elementtype(double) %2365, i64 %2369)
  %2390 = load double, double* %2389, align 1
  %2391 = fmul fast double %2390, %2388
  %2392 = fsub fast double %2382, %2391
  %2393 = fmul fast double %2392, 5.000000e-01
  %2394 = fadd fast double %2393, %2374
  %2395 = fmul fast double %2372, %2394
  %2396 = fmul fast double %2395, %2379
  %2397 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2298, i64 8, double* elementtype(double) nonnull %2366, i64 %2369)
  store double %2396, double* %2397, align 1
  %2398 = fmul fast double %2395, %2388
  %2399 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2298, i64 8, double* elementtype(double) nonnull %2367, i64 %2369)
  store double %2398, double* %2399, align 1
  %2400 = trunc i64 %2385 to i32
  %2401 = icmp eq i32 %2351, %2400
  br i1 %2401, label %2402, label %2368

2402:                                             ; preds = %2368, %2355
  %2403 = phi i64 [ %2356, %2355 ], [ %2361, %2368 ]
  %2404 = trunc i64 %2403 to i32
  %2405 = icmp eq i32 %232, %2404
  br i1 %2405, label %2406, label %2353

2406:                                             ; preds = %2402, %2294
  br i1 %1320, label %2437, label %2407

2407:                                             ; preds = %2406
  %2408 = sext i32 %57 to i64
  %2409 = sext i32 %59 to i64
  %2410 = sext i32 %138 to i64
  %2411 = add nsw i32 %150, 1
  %2412 = sext i32 %174 to i64
  %2413 = sext i32 %232 to i64
  %2414 = sext i32 %2411 to i64
  br label %2415

2415:                                             ; preds = %2434, %2407
  %2416 = phi i64 [ %2412, %2407 ], [ %2435, %2434 ]
  br i1 %1724, label %2434, label %2417

2417:                                             ; preds = %2415
  %2418 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2409, i64 %94, double* elementtype(double) nonnull %85, i64 %2416)
  %2419 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2409, i64 %94, double* elementtype(double) nonnull %78, i64 %2416)
  br label %2420

2420:                                             ; preds = %2420, %2417
  %2421 = phi i64 [ %2410, %2417 ], [ %2432, %2420 ]
  %2422 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2408, i64 8, double* elementtype(double) nonnull %2418, i64 %2421)
  %2423 = load double, double* %2422, align 1
  %2424 = add nsw i64 %2421, -1
  %2425 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2408, i64 8, double* elementtype(double) nonnull %2418, i64 %2424)
  %2426 = load double, double* %2425, align 1
  %2427 = fadd fast double %2426, %2423
  %2428 = fmul fast double %2427, 5.000000e-01
  %2429 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2408, i64 8, double* elementtype(double) nonnull %2419, i64 %2421)
  %2430 = load double, double* %2429, align 1
  %2431 = fadd fast double %2428, %2430
  store double %2431, double* %2429, align 1
  %2432 = add nsw i64 %2421, 1
  %2433 = icmp eq i64 %2432, %2414
  br i1 %2433, label %2434, label %2420

2434:                                             ; preds = %2420, %2415
  %2435 = add nsw i64 %2416, 1
  %2436 = icmp eq i64 %2435, %2413
  br i1 %2436, label %2437, label %2415

2437:                                             ; preds = %2434, %2406
  br i1 %1864, label %2469, label %2438

2438:                                             ; preds = %2437
  %2439 = sext i32 %57 to i64
  %2440 = sext i32 %59 to i64
  %2441 = sext i32 %114 to i64
  %2442 = add nsw i32 %150, 1
  %2443 = sext i32 %198 to i64
  %2444 = sext i32 %232 to i64
  %2445 = sext i32 %2442 to i64
  br label %2446

2446:                                             ; preds = %2466, %2438
  %2447 = phi i64 [ %2443, %2438 ], [ %2467, %2466 ]
  br i1 %2096, label %2466, label %2448

2448:                                             ; preds = %2446
  %2449 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2440, i64 %94, double* elementtype(double) nonnull %84, i64 %2447)
  %2450 = add nsw i64 %2447, -1
  %2451 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2440, i64 %94, double* elementtype(double) nonnull %84, i64 %2450)
  %2452 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2440, i64 %94, double* elementtype(double) nonnull %77, i64 %2447)
  br label %2453

2453:                                             ; preds = %2453, %2448
  %2454 = phi i64 [ %2441, %2448 ], [ %2464, %2453 ]
  %2455 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2439, i64 8, double* elementtype(double) nonnull %2449, i64 %2454)
  %2456 = load double, double* %2455, align 1
  %2457 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2439, i64 8, double* elementtype(double) nonnull %2451, i64 %2454)
  %2458 = load double, double* %2457, align 1
  %2459 = fadd fast double %2458, %2456
  %2460 = fmul fast double %2459, 5.000000e-01
  %2461 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2439, i64 8, double* elementtype(double) nonnull %2452, i64 %2454)
  %2462 = load double, double* %2461, align 1
  %2463 = fsub fast double %2462, %2460
  store double %2463, double* %2461, align 1
  %2464 = add nsw i64 %2454, 1
  %2465 = icmp eq i64 %2464, %2445
  br i1 %2465, label %2466, label %2453

2466:                                             ; preds = %2453, %2446
  %2467 = add nsw i64 %2447, 1
  %2468 = icmp eq i64 %2467, %2444
  br i1 %2468, label %2469, label %2446

2469:                                             ; preds = %2466, %2437
  br i1 %1763, label %2509, label %2470

2470:                                             ; preds = %2469
  %2471 = add nsw i32 %150, 1
  %2472 = icmp slt i32 %2471, %114
  %2473 = sext i32 %57 to i64
  %2474 = sext i32 %59 to i64
  %2475 = sext i32 %114 to i64
  %2476 = add nsw i32 %150, 2
  %2477 = sext i32 %174 to i64
  %2478 = add nsw i32 %210, 2
  br label %2479

2479:                                             ; preds = %2505, %2470
  %2480 = phi i64 [ %2477, %2470 ], [ %2506, %2505 ]
  br i1 %2472, label %2505, label %2481

2481:                                             ; preds = %2479
  %2482 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2474, i64 %94, double* elementtype(double) nonnull %91, i64 %2480)
  %2483 = add nsw i64 %2480, -1
  %2484 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2474, i64 %94, double* elementtype(double) nonnull %91, i64 %2483)
  %2485 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2474, i64 %94, double* elementtype(double) nonnull %90, i64 %2480)
  br label %2486

2486:                                             ; preds = %2486, %2481
  %2487 = phi i64 [ %2475, %2481 ], [ %2502, %2486 ]
  %2488 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2473, i64 8, double* elementtype(double) nonnull %2482, i64 %2487)
  %2489 = load double, double* %2488, align 1
  %2490 = add nsw i64 %2487, -1
  %2491 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2473, i64 8, double* elementtype(double) nonnull %2482, i64 %2490)
  %2492 = load double, double* %2491, align 1
  %2493 = fadd fast double %2492, %2489
  %2494 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2473, i64 8, double* elementtype(double) nonnull %2484, i64 %2487)
  %2495 = load double, double* %2494, align 1
  %2496 = fadd fast double %2493, %2495
  %2497 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2473, i64 8, double* elementtype(double) nonnull %2484, i64 %2490)
  %2498 = load double, double* %2497, align 1
  %2499 = fadd fast double %2496, %2498
  %2500 = fmul fast double %2499, 2.500000e-01
  %2501 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2473, i64 8, double* elementtype(double) nonnull %2485, i64 %2487)
  store double %2500, double* %2501, align 1
  %2502 = add nsw i64 %2487, 1
  %2503 = trunc i64 %2502 to i32
  %2504 = icmp eq i32 %2476, %2503
  br i1 %2504, label %2505, label %2486

2505:                                             ; preds = %2486, %2479
  %2506 = add nsw i64 %2480, 1
  %2507 = trunc i64 %2506 to i32
  %2508 = icmp eq i32 %2478, %2507
  br i1 %2508, label %2509, label %2479

2509:                                             ; preds = %2505, %2469
  br i1 %1915, label %2686, label %2510

2510:                                             ; preds = %2509
  %2511 = add i32 %138, -1
  %2512 = icmp slt i32 %150, %2511
  %2513 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %36, i64 0, i32 0
  %2514 = load double*, double** %2513, align 1
  %2515 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %36, i64 0, i32 6, i64 0
  %2516 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2515, i64 0, i32 1
  %2517 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2516, i32 0)
  %2518 = load i64, i64* %2517, align 1
  %2519 = sext i32 %55 to i64
  %2520 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2516, i32 1)
  %2521 = load i64, i64* %2520, align 1
  %2522 = sext i32 %56 to i64
  %2523 = sext i32 %57 to i64
  %2524 = sext i32 %59 to i64
  %2525 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 0
  %2526 = load double*, double** %2525, align 1
  %2527 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 6, i64 0
  %2528 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2527, i64 0, i32 1
  %2529 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2528, i32 0)
  %2530 = load i64, i64* %2529, align 1
  %2531 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2528, i32 1)
  %2532 = load i64, i64* %2531, align 1
  %2533 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %2534 = load double*, double** %2533, align 1
  %2535 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %2536 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2535, i64 0, i32 1
  %2537 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2536, i32 0)
  %2538 = load i64, i64* %2537, align 1
  %2539 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2536, i32 1)
  %2540 = load i64, i64* %2539, align 1
  %2541 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %2542 = load double*, double** %2541, align 1
  %2543 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %2544 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2543, i64 0, i32 1
  %2545 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2544, i32 0)
  %2546 = load i64, i64* %2545, align 1
  %2547 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2544, i32 1)
  %2548 = load i64, i64* %2547, align 1
  %2549 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2544, i32 2)
  %2550 = load i64, i64* %2549, align 1
  %2551 = load i32, i32* %11, align 1
  %2552 = sext i32 %2551 to i64
  %2553 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2550, double* elementtype(double) %2542, i64 %2552)
  %2554 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %28, i64 0, i32 0
  %2555 = load double*, double** %2554, align 1
  %2556 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %28, i64 0, i32 6, i64 0
  %2557 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2556, i64 0, i32 1
  %2558 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2557, i32 0)
  %2559 = load i64, i64* %2558, align 1
  %2560 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2557, i32 1)
  %2561 = load i64, i64* %2560, align 1
  %2562 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %2563 = load double*, double** %2562, align 1
  %2564 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %2565 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2564, i64 0, i32 1
  %2566 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2565, i32 0)
  %2567 = load i64, i64* %2566, align 1
  %2568 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2565, i32 1)
  %2569 = load i64, i64* %2568, align 1
  %2570 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %2571 = load double*, double** %2570, align 1
  %2572 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %2573 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2572, i64 0, i32 1
  %2574 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2573, i32 0)
  %2575 = load i64, i64* %2574, align 1
  %2576 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2573, i32 1)
  %2577 = load i64, i64* %2576, align 1
  %2578 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2573, i32 2)
  %2579 = load i64, i64* %2578, align 1
  %2580 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2579, double* elementtype(double) %2571, i64 %2552)
  %2581 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %32, i64 0, i32 0
  %2582 = load double*, double** %2581, align 1
  %2583 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %32, i64 0, i32 6, i64 0
  %2584 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2583, i64 0, i32 1
  %2585 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2584, i32 0)
  %2586 = load i64, i64* %2585, align 1
  %2587 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2584, i32 1)
  %2588 = load i64, i64* %2587, align 1
  %2589 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %31, i64 0, i32 0
  %2590 = load double*, double** %2589, align 1
  %2591 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %31, i64 0, i32 6, i64 0
  %2592 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2591, i64 0, i32 1
  %2593 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2592, i32 0)
  %2594 = load i64, i64* %2593, align 1
  %2595 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2592, i32 1)
  %2596 = load i64, i64* %2595, align 1
  %2597 = sext i32 %2511 to i64
  %2598 = add nsw i32 %150, 1
  %2599 = sext i32 %228 to i64
  br label %2600

2600:                                             ; preds = %2682, %2510
  %2601 = phi i64 [ %2599, %2510 ], [ %2683, %2682 ]
  br i1 %2512, label %2602, label %2604

2602:                                             ; preds = %2600
  %2603 = add nsw i64 %2601, 1
  br label %2682

2604:                                             ; preds = %2600
  %2605 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2521, double* elementtype(double) %2514, i64 %2601)
  %2606 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2524, i64 %94, double* elementtype(double) nonnull %91, i64 %2601)
  %2607 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2532, double* elementtype(double) %2526, i64 %2601)
  %2608 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2540, double* elementtype(double) %2534, i64 %2601)
  %2609 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2548, double* elementtype(double) %2553, i64 %2601)
  %2610 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2561, double* elementtype(double) %2555, i64 %2601)
  %2611 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2569, double* elementtype(double) %2563, i64 %2601)
  %2612 = add nsw i64 %2601, 1
  %2613 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2569, double* elementtype(double) %2563, i64 %2612)
  %2614 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2577, double* elementtype(double) %2580, i64 %2612)
  %2615 = add nsw i64 %2601, -1
  %2616 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2569, double* elementtype(double) %2563, i64 %2615)
  %2617 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2577, double* elementtype(double) %2580, i64 %2601)
  %2618 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2588, double* elementtype(double) %2582, i64 %2601)
  %2619 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2524, i64 %94, double* elementtype(double) nonnull %85, i64 %2601)
  %2620 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2522, i64 %2596, double* elementtype(double) %2590, i64 %2601)
  %2621 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2524, i64 %94, double* elementtype(double) nonnull %84, i64 %2601)
  br label %2622

2622:                                             ; preds = %2622, %2604
  %2623 = phi i64 [ %2597, %2604 ], [ %2632, %2622 ]
  %2624 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2518, double* elementtype(double) %2605, i64 %2623)
  %2625 = load double, double* %2624, align 1
  %2626 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2523, i64 8, double* elementtype(double) nonnull %2606, i64 %2623)
  %2627 = load double, double* %2626, align 1
  %2628 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2530, double* elementtype(double) %2607, i64 %2623)
  %2629 = load double, double* %2628, align 1
  %2630 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2538, double* elementtype(double) %2608, i64 %2623)
  %2631 = load double, double* %2630, align 1
  %2632 = add nsw i64 %2623, 1
  %2633 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2538, double* elementtype(double) %2608, i64 %2632)
  %2634 = load double, double* %2633, align 1
  %2635 = fadd fast double %2634, %2631
  %2636 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2546, double* elementtype(double) %2609, i64 %2632)
  %2637 = load double, double* %2636, align 1
  %2638 = fmul fast double %2637, %2635
  %2639 = add nsw i64 %2623, -1
  %2640 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2538, double* elementtype(double) %2608, i64 %2639)
  %2641 = load double, double* %2640, align 1
  %2642 = fadd fast double %2641, %2631
  %2643 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2546, double* elementtype(double) %2609, i64 %2623)
  %2644 = load double, double* %2643, align 1
  %2645 = fmul fast double %2642, %2644
  %2646 = fsub fast double %2638, %2645
  %2647 = fmul fast double %2646, %2629
  %2648 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2559, double* elementtype(double) %2610, i64 %2623)
  %2649 = load double, double* %2648, align 1
  %2650 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2567, double* elementtype(double) %2611, i64 %2623)
  %2651 = load double, double* %2650, align 1
  %2652 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2567, double* elementtype(double) %2613, i64 %2623)
  %2653 = load double, double* %2652, align 1
  %2654 = fadd fast double %2653, %2651
  %2655 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2575, double* elementtype(double) %2614, i64 %2623)
  %2656 = load double, double* %2655, align 1
  %2657 = fmul fast double %2656, %2654
  %2658 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2567, double* elementtype(double) %2616, i64 %2623)
  %2659 = load double, double* %2658, align 1
  %2660 = fadd fast double %2659, %2651
  %2661 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2575, double* elementtype(double) %2617, i64 %2623)
  %2662 = load double, double* %2661, align 1
  %2663 = fmul fast double %2660, %2662
  %2664 = fsub fast double %2657, %2663
  %2665 = fmul fast double %2664, %2649
  %2666 = fsub fast double %2647, %2665
  %2667 = fmul fast double %2625, 5.000000e-01
  %2668 = fmul fast double %2667, %2627
  %2669 = fmul fast double %2668, %2666
  %2670 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2586, double* elementtype(double) %2618, i64 %2623)
  %2671 = load double, double* %2670, align 1
  %2672 = fmul fast double %2671, %2671
  %2673 = fmul fast double %2672, %2669
  %2674 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2523, i64 8, double* elementtype(double) nonnull %2619, i64 %2623)
  store double %2673, double* %2674, align 1
  %2675 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2519, i64 %2594, double* elementtype(double) %2620, i64 %2623)
  %2676 = load double, double* %2675, align 1
  %2677 = fmul fast double %2676, %2676
  %2678 = fmul fast double %2677, %2669
  %2679 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2523, i64 8, double* elementtype(double) nonnull %2621, i64 %2623)
  store double %2678, double* %2679, align 1
  %2680 = trunc i64 %2632 to i32
  %2681 = icmp eq i32 %2598, %2680
  br i1 %2681, label %2682, label %2622

2682:                                             ; preds = %2622, %2602
  %2683 = phi i64 [ %2603, %2602 ], [ %2612, %2622 ]
  %2684 = trunc i64 %2683 to i32
  %2685 = icmp eq i32 %232, %2684
  br i1 %2685, label %2686, label %2600

2686:                                             ; preds = %2682, %2509
  br i1 %1763, label %2865, label %2687

2687:                                             ; preds = %2686
  %2688 = add nsw i32 %150, 1
  %2689 = icmp slt i32 %2688, %114
  %2690 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %35, i64 0, i32 0
  %2691 = load double*, double** %2690, align 1
  %2692 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %35, i64 0, i32 6, i64 0
  %2693 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2692, i64 0, i32 1
  %2694 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2693, i32 0)
  %2695 = load i64, i64* %2694, align 1
  %2696 = sext i32 %55 to i64
  %2697 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2693, i32 1)
  %2698 = load i64, i64* %2697, align 1
  %2699 = sext i32 %56 to i64
  %2700 = sext i32 %57 to i64
  %2701 = sext i32 %59 to i64
  %2702 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %29, i64 0, i32 0
  %2703 = load double*, double** %2702, align 1
  %2704 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %29, i64 0, i32 6, i64 0
  %2705 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2704, i64 0, i32 1
  %2706 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2705, i32 0)
  %2707 = load i64, i64* %2706, align 1
  %2708 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2705, i32 1)
  %2709 = load i64, i64* %2708, align 1
  %2710 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %2711 = load double*, double** %2710, align 1
  %2712 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %2713 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2712, i64 0, i32 1
  %2714 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2713, i32 0)
  %2715 = load i64, i64* %2714, align 1
  %2716 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2713, i32 1)
  %2717 = load i64, i64* %2716, align 1
  %2718 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %2719 = load double*, double** %2718, align 1
  %2720 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %2721 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2720, i64 0, i32 1
  %2722 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2721, i32 0)
  %2723 = load i64, i64* %2722, align 1
  %2724 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2721, i32 1)
  %2725 = load i64, i64* %2724, align 1
  %2726 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2721, i32 2)
  %2727 = load i64, i64* %2726, align 1
  %2728 = load i32, i32* %11, align 1
  %2729 = sext i32 %2728 to i64
  %2730 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2727, double* elementtype(double) %2719, i64 %2729)
  %2731 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %30, i64 0, i32 0
  %2732 = load double*, double** %2731, align 1
  %2733 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %30, i64 0, i32 6, i64 0
  %2734 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2733, i64 0, i32 1
  %2735 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2734, i32 0)
  %2736 = load i64, i64* %2735, align 1
  %2737 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2734, i32 1)
  %2738 = load i64, i64* %2737, align 1
  %2739 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %2740 = load double*, double** %2739, align 1
  %2741 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %2742 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2741, i64 0, i32 1
  %2743 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2742, i32 0)
  %2744 = load i64, i64* %2743, align 1
  %2745 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2742, i32 1)
  %2746 = load i64, i64* %2745, align 1
  %2747 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %2748 = load double*, double** %2747, align 1
  %2749 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %2750 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2749, i64 0, i32 1
  %2751 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2750, i32 0)
  %2752 = load i64, i64* %2751, align 1
  %2753 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2750, i32 1)
  %2754 = load i64, i64* %2753, align 1
  %2755 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2750, i32 2)
  %2756 = load i64, i64* %2755, align 1
  %2757 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %2756, double* elementtype(double) %2748, i64 %2729)
  %2758 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %33, i64 0, i32 0
  %2759 = load double*, double** %2758, align 1
  %2760 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %33, i64 0, i32 6, i64 0
  %2761 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2760, i64 0, i32 1
  %2762 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2761, i32 0)
  %2763 = load i64, i64* %2762, align 1
  %2764 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2761, i32 1)
  %2765 = load i64, i64* %2764, align 1
  %2766 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %34, i64 0, i32 0
  %2767 = load double*, double** %2766, align 1
  %2768 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %34, i64 0, i32 6, i64 0
  %2769 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2768, i64 0, i32 1
  %2770 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2769, i32 0)
  %2771 = load i64, i64* %2770, align 1
  %2772 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2769, i32 1)
  %2773 = load i64, i64* %2772, align 1
  %2774 = sext i32 %114 to i64
  %2775 = add nsw i32 %150, 2
  %2776 = sext i32 %174 to i64
  %2777 = add nsw i32 %210, 2
  br label %2778

2778:                                             ; preds = %2861, %2687
  %2779 = phi i64 [ %2776, %2687 ], [ %2862, %2861 ]
  br i1 %2689, label %2861, label %2780

2780:                                             ; preds = %2778
  %2781 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2698, double* elementtype(double) %2691, i64 %2779)
  %2782 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2701, i64 %94, double* elementtype(double) nonnull %90, i64 %2779)
  %2783 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2709, double* elementtype(double) %2703, i64 %2779)
  %2784 = add nsw i64 %2779, -1
  %2785 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2717, double* elementtype(double) %2711, i64 %2784)
  %2786 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2717, double* elementtype(double) %2711, i64 %2779)
  %2787 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2725, double* elementtype(double) %2730, i64 %2779)
  %2788 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2738, double* elementtype(double) %2732, i64 %2779)
  %2789 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2746, double* elementtype(double) %2740, i64 %2779)
  %2790 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2754, double* elementtype(double) %2757, i64 %2779)
  %2791 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2746, double* elementtype(double) %2740, i64 %2784)
  %2792 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2754, double* elementtype(double) %2757, i64 %2784)
  %2793 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2765, double* elementtype(double) %2759, i64 %2779)
  %2794 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2701, i64 %94, double* elementtype(double) nonnull %86, i64 %2779)
  %2795 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2699, i64 %2773, double* elementtype(double) %2767, i64 %2779)
  %2796 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2701, i64 %94, double* elementtype(double) nonnull %83, i64 %2779)
  br label %2797

2797:                                             ; preds = %2797, %2780
  %2798 = phi i64 [ %2774, %2780 ], [ %2858, %2797 ]
  %2799 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2695, double* elementtype(double) %2781, i64 %2798)
  %2800 = load double, double* %2799, align 1
  %2801 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2700, i64 8, double* elementtype(double) nonnull %2782, i64 %2798)
  %2802 = load double, double* %2801, align 1
  %2803 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2707, double* elementtype(double) %2783, i64 %2798)
  %2804 = load double, double* %2803, align 1
  %2805 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2715, double* elementtype(double) %2785, i64 %2798)
  %2806 = load double, double* %2805, align 1
  %2807 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2715, double* elementtype(double) %2786, i64 %2798)
  %2808 = load double, double* %2807, align 1
  %2809 = fadd fast double %2808, %2806
  %2810 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2723, double* elementtype(double) %2787, i64 %2798)
  %2811 = load double, double* %2810, align 1
  %2812 = fmul fast double %2811, %2809
  %2813 = add nsw i64 %2798, -1
  %2814 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2715, double* elementtype(double) %2785, i64 %2813)
  %2815 = load double, double* %2814, align 1
  %2816 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2715, double* elementtype(double) %2786, i64 %2813)
  %2817 = load double, double* %2816, align 1
  %2818 = fadd fast double %2817, %2815
  %2819 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2723, double* elementtype(double) %2787, i64 %2813)
  %2820 = load double, double* %2819, align 1
  %2821 = fmul fast double %2818, %2820
  %2822 = fsub fast double %2812, %2821
  %2823 = fmul fast double %2822, %2804
  %2824 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2736, double* elementtype(double) %2788, i64 %2798)
  %2825 = load double, double* %2824, align 1
  %2826 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2744, double* elementtype(double) %2789, i64 %2813)
  %2827 = load double, double* %2826, align 1
  %2828 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2744, double* elementtype(double) %2789, i64 %2798)
  %2829 = load double, double* %2828, align 1
  %2830 = fadd fast double %2829, %2827
  %2831 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2752, double* elementtype(double) %2790, i64 %2798)
  %2832 = load double, double* %2831, align 1
  %2833 = fmul fast double %2832, %2830
  %2834 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2744, double* elementtype(double) %2791, i64 %2813)
  %2835 = load double, double* %2834, align 1
  %2836 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2744, double* elementtype(double) %2791, i64 %2798)
  %2837 = load double, double* %2836, align 1
  %2838 = fadd fast double %2837, %2835
  %2839 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2752, double* elementtype(double) %2792, i64 %2798)
  %2840 = load double, double* %2839, align 1
  %2841 = fmul fast double %2838, %2840
  %2842 = fsub fast double %2833, %2841
  %2843 = fmul fast double %2842, %2825
  %2844 = fadd fast double %2843, %2823
  %2845 = fmul fast double %2800, 5.000000e-01
  %2846 = fmul fast double %2845, %2802
  %2847 = fmul fast double %2846, %2844
  %2848 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2763, double* elementtype(double) %2793, i64 %2798)
  %2849 = load double, double* %2848, align 1
  %2850 = fmul fast double %2849, %2849
  %2851 = fmul fast double %2850, %2847
  %2852 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2700, i64 8, double* elementtype(double) nonnull %2794, i64 %2798)
  store double %2851, double* %2852, align 1
  %2853 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2696, i64 %2771, double* elementtype(double) %2795, i64 %2798)
  %2854 = load double, double* %2853, align 1
  %2855 = fmul fast double %2854, %2854
  %2856 = fmul fast double %2855, %2847
  %2857 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2700, i64 8, double* elementtype(double) nonnull %2796, i64 %2798)
  store double %2856, double* %2857, align 1
  %2858 = add nsw i64 %2798, 1
  %2859 = trunc i64 %2858 to i32
  %2860 = icmp eq i32 %2775, %2859
  br i1 %2860, label %2861, label %2797

2861:                                             ; preds = %2797, %2778
  %2862 = add nsw i64 %2779, 1
  %2863 = trunc i64 %2862 to i32
  %2864 = icmp eq i32 %2777, %2863
  br i1 %2864, label %2865, label %2778

2865:                                             ; preds = %2861, %2686
  br i1 %1320, label %2939, label %2866

2866:                                             ; preds = %2865
  %2867 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %2868 = load double*, double** %2867, align 1
  %2869 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %2870 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2869, i64 0, i32 1
  %2871 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2870, i32 0)
  %2872 = load i64, i64* %2871, align 1
  %2873 = sext i32 %55 to i64
  %2874 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2870, i32 1)
  %2875 = load i64, i64* %2874, align 1
  %2876 = sext i32 %56 to i64
  %2877 = sext i32 %57 to i64
  %2878 = sext i32 %59 to i64
  %2879 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %2880 = load double*, double** %2879, align 1
  %2881 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %2882 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2881, i64 0, i32 1
  %2883 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2882, i32 0)
  %2884 = load i64, i64* %2883, align 1
  %2885 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2882, i32 1)
  %2886 = load i64, i64* %2885, align 1
  %2887 = sext i32 %138 to i64
  %2888 = add nsw i32 %150, 1
  %2889 = sext i32 %174 to i64
  %2890 = sext i32 %232 to i64
  %2891 = sext i32 %2888 to i64
  br label %2892

2892:                                             ; preds = %2936, %2866
  %2893 = phi i64 [ %2889, %2866 ], [ %2937, %2936 ]
  br i1 %1724, label %2894, label %2896

2894:                                             ; preds = %2892
  %2895 = add nsw i64 %2893, 1
  br label %2936

2896:                                             ; preds = %2892
  %2897 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2876, i64 %2875, double* elementtype(double) %2868, i64 %2893)
  %2898 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2878, i64 %94, double* elementtype(double) nonnull %85, i64 %2893)
  %2899 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2876, i64 %2886, double* elementtype(double) %2880, i64 %2893)
  %2900 = add nsw i64 %2893, 1
  %2901 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2878, i64 %94, double* elementtype(double) nonnull %86, i64 %2900)
  %2902 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2878, i64 %94, double* elementtype(double) nonnull %86, i64 %2893)
  %2903 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2878, i64 %94, double* elementtype(double) nonnull %78, i64 %2893)
  br label %2904

2904:                                             ; preds = %2904, %2896
  %2905 = phi i64 [ %2887, %2896 ], [ %2934, %2904 ]
  %2906 = add nsw i64 %2905, -1
  %2907 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2873, i64 %2872, double* elementtype(double) %2897, i64 %2906)
  %2908 = load double, double* %2907, align 1
  %2909 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2873, i64 %2872, double* elementtype(double) %2897, i64 %2905)
  %2910 = load double, double* %2909, align 1
  %2911 = fadd fast double %2910, %2908
  %2912 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2877, i64 8, double* elementtype(double) nonnull %2898, i64 %2905)
  %2913 = load double, double* %2912, align 1
  %2914 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2877, i64 8, double* elementtype(double) nonnull %2898, i64 %2906)
  %2915 = load double, double* %2914, align 1
  %2916 = fsub fast double %2913, %2915
  %2917 = fmul fast double %2916, %2911
  %2918 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2873, i64 %2884, double* elementtype(double) %2899, i64 %2906)
  %2919 = load double, double* %2918, align 1
  %2920 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2873, i64 %2884, double* elementtype(double) %2899, i64 %2905)
  %2921 = load double, double* %2920, align 1
  %2922 = fadd fast double %2921, %2919
  %2923 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2877, i64 8, double* elementtype(double) nonnull %2901, i64 %2905)
  %2924 = load double, double* %2923, align 1
  %2925 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2877, i64 8, double* elementtype(double) nonnull %2902, i64 %2905)
  %2926 = load double, double* %2925, align 1
  %2927 = fsub fast double %2924, %2926
  %2928 = fmul fast double %2927, %2922
  %2929 = fadd fast double %2928, %2917
  %2930 = fmul fast double %2929, 5.000000e-01
  %2931 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2877, i64 8, double* elementtype(double) nonnull %2903, i64 %2905)
  %2932 = load double, double* %2931, align 1
  %2933 = fadd fast double %2930, %2932
  store double %2933, double* %2931, align 1
  %2934 = add nsw i64 %2905, 1
  %2935 = icmp eq i64 %2934, %2891
  br i1 %2935, label %2936, label %2904

2936:                                             ; preds = %2904, %2894
  %2937 = phi i64 [ %2895, %2894 ], [ %2900, %2904 ]
  %2938 = icmp eq i64 %2937, %2890
  br i1 %2938, label %2939, label %2892

2939:                                             ; preds = %2936, %2865
  br i1 %1864, label %3012, label %2940

2940:                                             ; preds = %2939
  %2941 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %2942 = load double*, double** %2941, align 1
  %2943 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %2944 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2943, i64 0, i32 1
  %2945 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2944, i32 0)
  %2946 = load i64, i64* %2945, align 1
  %2947 = sext i32 %55 to i64
  %2948 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2944, i32 1)
  %2949 = load i64, i64* %2948, align 1
  %2950 = sext i32 %56 to i64
  %2951 = sext i32 %57 to i64
  %2952 = sext i32 %59 to i64
  %2953 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %2954 = load double*, double** %2953, align 1
  %2955 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %2956 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %2955, i64 0, i32 1
  %2957 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2956, i32 0)
  %2958 = load i64, i64* %2957, align 1
  %2959 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %2956, i32 1)
  %2960 = load i64, i64* %2959, align 1
  %2961 = sext i32 %114 to i64
  %2962 = add nsw i32 %150, 1
  %2963 = sext i32 %198 to i64
  %2964 = sext i32 %232 to i64
  %2965 = sext i32 %2962 to i64
  br label %2966

2966:                                             ; preds = %3009, %2940
  %2967 = phi i64 [ %2963, %2940 ], [ %3010, %3009 ]
  br i1 %2096, label %3009, label %2968

2968:                                             ; preds = %2966
  %2969 = add nsw i64 %2967, -1
  %2970 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2950, i64 %2949, double* elementtype(double) %2942, i64 %2969)
  %2971 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2950, i64 %2949, double* elementtype(double) %2942, i64 %2967)
  %2972 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2952, i64 %94, double* elementtype(double) nonnull %83, i64 %2967)
  %2973 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2950, i64 %2960, double* elementtype(double) %2954, i64 %2969)
  %2974 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2950, i64 %2960, double* elementtype(double) %2954, i64 %2967)
  %2975 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2952, i64 %94, double* elementtype(double) nonnull %84, i64 %2967)
  %2976 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2952, i64 %94, double* elementtype(double) nonnull %84, i64 %2969)
  %2977 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %2952, i64 %94, double* elementtype(double) nonnull %77, i64 %2967)
  br label %2978

2978:                                             ; preds = %2978, %2968
  %2979 = phi i64 [ %2961, %2968 ], [ %2985, %2978 ]
  %2980 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2947, i64 %2946, double* elementtype(double) %2970, i64 %2979)
  %2981 = load double, double* %2980, align 1
  %2982 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2947, i64 %2946, double* elementtype(double) %2971, i64 %2979)
  %2983 = load double, double* %2982, align 1
  %2984 = fadd fast double %2983, %2981
  %2985 = add nsw i64 %2979, 1
  %2986 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2951, i64 8, double* elementtype(double) nonnull %2972, i64 %2985)
  %2987 = load double, double* %2986, align 1
  %2988 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2951, i64 8, double* elementtype(double) nonnull %2972, i64 %2979)
  %2989 = load double, double* %2988, align 1
  %2990 = fsub fast double %2987, %2989
  %2991 = fmul fast double %2990, %2984
  %2992 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2947, i64 %2958, double* elementtype(double) %2973, i64 %2979)
  %2993 = load double, double* %2992, align 1
  %2994 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2947, i64 %2958, double* elementtype(double) %2974, i64 %2979)
  %2995 = load double, double* %2994, align 1
  %2996 = fadd fast double %2995, %2993
  %2997 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2951, i64 8, double* elementtype(double) nonnull %2975, i64 %2979)
  %2998 = load double, double* %2997, align 1
  %2999 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2951, i64 8, double* elementtype(double) nonnull %2976, i64 %2979)
  %3000 = load double, double* %2999, align 1
  %3001 = fsub fast double %2998, %3000
  %3002 = fmul fast double %3001, %2996
  %3003 = fsub fast double %2991, %3002
  %3004 = fmul fast double %3003, 5.000000e-01
  %3005 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %2951, i64 8, double* elementtype(double) nonnull %2977, i64 %2979)
  %3006 = load double, double* %3005, align 1
  %3007 = fadd fast double %3004, %3006
  store double %3007, double* %3005, align 1
  %3008 = icmp eq i64 %2985, %2965
  br i1 %3008, label %3009, label %2978

3009:                                             ; preds = %2978, %2966
  %3010 = add nsw i64 %2967, 1
  %3011 = icmp eq i64 %3010, %2964
  br i1 %3011, label %3012, label %2966

3012:                                             ; preds = %3009, %2939
  %3013 = load i32, i32* %416, align 1
  %3014 = icmp eq i32 %3013, 1
  %3015 = load i32, i32* %225, align 1
  %3016 = and i32 %3015, 1
  %3017 = icmp ne i32 %3016, 0
  %3018 = and i1 %3014, %3017
  br i1 %3018, label %3449, label %3455

3019:                                             ; preds = %3359, %3035
  %3020 = phi i64 [ %3390, %3359 ], [ %3036, %3035 ]
  br i1 %1724, label %3035, label %3021

3021:                                             ; preds = %3019
  %3022 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3369, i64 %3368, double* elementtype(double) %3361, i64 %3020)
  %3023 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3371, i64 %94, double* elementtype(double) nonnull %78, i64 %3020)
  %3024 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3369, i64 %3379, double* elementtype(double) %3387, i64 %3020)
  br label %3025

3025:                                             ; preds = %3025, %3021
  %3026 = phi i64 [ %3388, %3021 ], [ %3033, %3025 ]
  %3027 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3366, i64 %3365, double* elementtype(double) %3022, i64 %3026)
  %3028 = load double, double* %3027, align 1
  %3029 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3370, i64 8, double* elementtype(double) nonnull %3023, i64 %3026)
  %3030 = load double, double* %3029, align 1
  %3031 = fsub fast double %3028, %3030
  store double %3031, double* %3027, align 1
  store double %3028, double* %3029, align 1
  %3032 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3366, i64 %3377, double* elementtype(double) %3024, i64 %3026)
  store double %3031, double* %3032, align 1
  %3033 = add nsw i64 %3026, 1
  %3034 = icmp eq i64 %3033, %3392
  br i1 %3034, label %3035, label %3025

3035:                                             ; preds = %3025, %3019
  %3036 = add nsw i64 %3020, 1
  %3037 = icmp eq i64 %3036, %3391
  br i1 %3037, label %3038, label %3019

3038:                                             ; preds = %3358, %3035
  br i1 %1864, label %3474, label %3039

3039:                                             ; preds = %3038
  %3040 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 0
  %3041 = load double*, double** %3040, align 1
  %3042 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 6, i64 0
  %3043 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3042, i64 0, i32 1
  %3044 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3043, i32 0)
  %3045 = load i64, i64* %3044, align 1
  %3046 = sext i32 %55 to i64
  %3047 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3043, i32 1)
  %3048 = load i64, i64* %3047, align 1
  %3049 = sext i32 %56 to i64
  %3050 = sext i32 %57 to i64
  %3051 = sext i32 %59 to i64
  %3052 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 0
  %3053 = load double*, double** %3052, align 1
  %3054 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 6, i64 0
  %3055 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3054, i64 0, i32 1
  %3056 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3055, i32 0)
  %3057 = load i64, i64* %3056, align 1
  %3058 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3055, i32 1)
  %3059 = load i64, i64* %3058, align 1
  %3060 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3055, i32 2)
  %3061 = load i64, i64* %3060, align 1
  %3062 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3055, i32 3)
  %3063 = load i64, i64* %3062, align 1
  %3064 = load i32, i32* %14, align 1
  %3065 = sext i32 %3064 to i64
  %3066 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3063, double* elementtype(double) %3053, i64 %3065)
  %3067 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3061, double* elementtype(double) %3066, i64 0)
  %3068 = sext i32 %114 to i64
  %3069 = add nsw i32 %150, 1
  %3070 = sext i32 %198 to i64
  %3071 = sext i32 %232 to i64
  %3072 = sext i32 %3069 to i64
  br label %3073

3073:                                             ; preds = %3089, %3039
  %3074 = phi i64 [ %3070, %3039 ], [ %3090, %3089 ]
  br i1 %2096, label %3089, label %3075

3075:                                             ; preds = %3073
  %3076 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3049, i64 %3048, double* elementtype(double) %3041, i64 %3074)
  %3077 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3051, i64 %94, double* elementtype(double) nonnull %77, i64 %3074)
  %3078 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3049, i64 %3059, double* elementtype(double) %3067, i64 %3074)
  br label %3079

3079:                                             ; preds = %3079, %3075
  %3080 = phi i64 [ %3068, %3075 ], [ %3087, %3079 ]
  %3081 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3046, i64 %3045, double* elementtype(double) %3076, i64 %3080)
  %3082 = load double, double* %3081, align 1
  %3083 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3050, i64 8, double* elementtype(double) nonnull %3077, i64 %3080)
  %3084 = load double, double* %3083, align 1
  %3085 = fsub fast double %3082, %3084
  store double %3085, double* %3081, align 1
  store double %3082, double* %3083, align 1
  %3086 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3046, i64 %3057, double* elementtype(double) %3078, i64 %3080)
  store double %3085, double* %3086, align 1
  %3087 = add nsw i64 %3080, 1
  %3088 = icmp eq i64 %3087, %3072
  br i1 %3088, label %3089, label %3079

3089:                                             ; preds = %3079, %3073
  %3090 = add nsw i64 %3074, 1
  %3091 = icmp eq i64 %3090, %3071
  br i1 %3091, label %3474, label %3073

3092:                                             ; preds = %3281, %3115
  %3093 = phi i64 [ %3316, %3281 ], [ %3116, %3115 ]
  br i1 %1724, label %3115, label %3094

3094:                                             ; preds = %3092
  %3095 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3291, i64 %3290, double* elementtype(double) %3283, i64 %3093)
  %3096 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3293, i64 %94, double* elementtype(double) nonnull %78, i64 %3093)
  %3097 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3291, i64 %3301, double* elementtype(double) %3309, i64 %3093)
  %3098 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3291, i64 %3301, double* elementtype(double) %3313, i64 %3093)
  br label %3099

3099:                                             ; preds = %3099, %3094
  %3100 = phi i64 [ %3314, %3094 ], [ %3113, %3099 ]
  %3101 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3288, i64 %3287, double* elementtype(double) %3095, i64 %3100)
  %3102 = load double, double* %3101, align 1
  %3103 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3292, i64 8, double* elementtype(double) nonnull %3096, i64 %3100)
  %3104 = load double, double* %3103, align 1
  %3105 = fsub fast double %3102, %3104
  store double %3105, double* %3101, align 1
  %3106 = fmul fast double %3105, 1.500000e+00
  %3107 = fadd fast double %3106, %3104
  %3108 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3288, i64 %3299, double* elementtype(double) %3097, i64 %3100)
  %3109 = load double, double* %3108, align 1
  %3110 = fmul fast double %3109, -5.000000e-01
  %3111 = fadd fast double %3107, %3110
  store double %3111, double* %3103, align 1
  %3112 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3288, i64 %3299, double* elementtype(double) %3098, i64 %3100)
  store double %3105, double* %3112, align 1
  %3113 = add nsw i64 %3100, 1
  %3114 = icmp eq i64 %3113, %3318
  br i1 %3114, label %3115, label %3099

3115:                                             ; preds = %3099, %3092
  %3116 = add nsw i64 %3093, 1
  %3117 = icmp eq i64 %3116, %3317
  br i1 %3117, label %3118, label %3092

3118:                                             ; preds = %3280, %3115
  br i1 %1864, label %3474, label %3119

3119:                                             ; preds = %3118
  %3120 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 0
  %3121 = load double*, double** %3120, align 1
  %3122 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 6, i64 0
  %3123 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3122, i64 0, i32 1
  %3124 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3123, i32 0)
  %3125 = load i64, i64* %3124, align 1
  %3126 = sext i32 %55 to i64
  %3127 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3123, i32 1)
  %3128 = load i64, i64* %3127, align 1
  %3129 = sext i32 %56 to i64
  %3130 = sext i32 %57 to i64
  %3131 = sext i32 %59 to i64
  %3132 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 0
  %3133 = load double*, double** %3132, align 1
  %3134 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 6, i64 0
  %3135 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3134, i64 0, i32 1
  %3136 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3135, i32 0)
  %3137 = load i64, i64* %3136, align 1
  %3138 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3135, i32 1)
  %3139 = load i64, i64* %3138, align 1
  %3140 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3135, i32 2)
  %3141 = load i64, i64* %3140, align 1
  %3142 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3135, i32 3)
  %3143 = load i64, i64* %3142, align 1
  %3144 = load i32, i32* %15, align 1
  %3145 = sext i32 %3144 to i64
  %3146 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3143, double* elementtype(double) %3133, i64 %3145)
  %3147 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3141, double* elementtype(double) %3146, i64 0)
  %3148 = load i32, i32* %14, align 1
  %3149 = sext i32 %3148 to i64
  %3150 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3143, double* elementtype(double) %3133, i64 %3149)
  %3151 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3141, double* elementtype(double) %3150, i64 0)
  %3152 = sext i32 %114 to i64
  %3153 = add nsw i32 %150, 1
  %3154 = sext i32 %198 to i64
  %3155 = sext i32 %232 to i64
  %3156 = sext i32 %3153 to i64
  br label %3157

3157:                                             ; preds = %3180, %3119
  %3158 = phi i64 [ %3154, %3119 ], [ %3181, %3180 ]
  br i1 %2096, label %3180, label %3159

3159:                                             ; preds = %3157
  %3160 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3129, i64 %3128, double* elementtype(double) %3121, i64 %3158)
  %3161 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3131, i64 %94, double* elementtype(double) nonnull %77, i64 %3158)
  %3162 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3129, i64 %3139, double* elementtype(double) %3147, i64 %3158)
  %3163 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3129, i64 %3139, double* elementtype(double) %3151, i64 %3158)
  br label %3164

3164:                                             ; preds = %3164, %3159
  %3165 = phi i64 [ %3152, %3159 ], [ %3178, %3164 ]
  %3166 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3126, i64 %3125, double* elementtype(double) %3160, i64 %3165)
  %3167 = load double, double* %3166, align 1
  %3168 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3130, i64 8, double* elementtype(double) nonnull %3161, i64 %3165)
  %3169 = load double, double* %3168, align 1
  %3170 = fsub fast double %3167, %3169
  store double %3170, double* %3166, align 1
  %3171 = fmul fast double %3170, 1.500000e+00
  %3172 = fadd fast double %3171, %3169
  %3173 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3126, i64 %3137, double* elementtype(double) %3162, i64 %3165)
  %3174 = load double, double* %3173, align 1
  %3175 = fmul fast double %3174, -5.000000e-01
  %3176 = fadd fast double %3172, %3175
  store double %3176, double* %3168, align 1
  %3177 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3126, i64 %3137, double* elementtype(double) %3163, i64 %3165)
  store double %3170, double* %3177, align 1
  %3178 = add nsw i64 %3165, 1
  %3179 = icmp eq i64 %3178, %3156
  br i1 %3179, label %3180, label %3164

3180:                                             ; preds = %3164, %3157
  %3181 = add nsw i64 %3158, 1
  %3182 = icmp eq i64 %3181, %3155
  br i1 %3182, label %3474, label %3157

3183:                                             ; preds = %3320, %3209
  %3184 = phi i64 [ %3355, %3320 ], [ %3210, %3209 ]
  br i1 %1724, label %3209, label %3185

3185:                                             ; preds = %3183
  %3186 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3330, i64 %3329, double* elementtype(double) %3322, i64 %3184)
  %3187 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3332, i64 %94, double* elementtype(double) nonnull %78, i64 %3184)
  %3188 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3330, i64 %3340, double* elementtype(double) %3348, i64 %3184)
  %3189 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3330, i64 %3340, double* elementtype(double) %3352, i64 %3184)
  br label %3190

3190:                                             ; preds = %3190, %3185
  %3191 = phi i64 [ %3353, %3185 ], [ %3207, %3190 ]
  %3192 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3327, i64 %3326, double* elementtype(double) %3186, i64 %3191)
  %3193 = load double, double* %3192, align 1
  %3194 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3331, i64 8, double* elementtype(double) nonnull %3187, i64 %3191)
  %3195 = load double, double* %3194, align 1
  %3196 = fsub fast double %3193, %3195
  store double %3196, double* %3192, align 1
  %3197 = fmul fast double %3196, 0x3FFEAAAAAAAAAAAB
  %3198 = fadd fast double %3197, %3195
  %3199 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3327, i64 %3338, double* elementtype(double) %3188, i64 %3191)
  %3200 = load double, double* %3199, align 1
  %3201 = fmul fast double %3200, 0xBFF5555555555555
  %3202 = fadd fast double %3198, %3201
  %3203 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3327, i64 %3338, double* elementtype(double) %3189, i64 %3191)
  %3204 = load double, double* %3203, align 1
  %3205 = fmul fast double %3204, 0x3FDAAAAAAAAAAAAB
  %3206 = fadd fast double %3202, %3205
  store double %3206, double* %3194, align 1
  store double %3196, double* %3203, align 1
  %3207 = add nsw i64 %3191, 1
  %3208 = icmp eq i64 %3207, %3357
  br i1 %3208, label %3209, label %3190

3209:                                             ; preds = %3190, %3183
  %3210 = add nsw i64 %3184, 1
  %3211 = icmp eq i64 %3210, %3356
  br i1 %3211, label %3212, label %3183

3212:                                             ; preds = %3319, %3209
  br i1 %1864, label %3474, label %3213

3213:                                             ; preds = %3212
  %3214 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 0
  %3215 = load double*, double** %3214, align 1
  %3216 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 6, i64 0
  %3217 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3216, i64 0, i32 1
  %3218 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3217, i32 0)
  %3219 = load i64, i64* %3218, align 1
  %3220 = sext i32 %55 to i64
  %3221 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3217, i32 1)
  %3222 = load i64, i64* %3221, align 1
  %3223 = sext i32 %56 to i64
  %3224 = sext i32 %57 to i64
  %3225 = sext i32 %59 to i64
  %3226 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 0
  %3227 = load double*, double** %3226, align 1
  %3228 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %47, i64 0, i32 6, i64 0
  %3229 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3228, i64 0, i32 1
  %3230 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3229, i32 0)
  %3231 = load i64, i64* %3230, align 1
  %3232 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3229, i32 1)
  %3233 = load i64, i64* %3232, align 1
  %3234 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3229, i32 2)
  %3235 = load i64, i64* %3234, align 1
  %3236 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3229, i32 3)
  %3237 = load i64, i64* %3236, align 1
  %3238 = load i32, i32* %15, align 1
  %3239 = sext i32 %3238 to i64
  %3240 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3237, double* elementtype(double) %3227, i64 %3239)
  %3241 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3235, double* elementtype(double) %3240, i64 0)
  %3242 = load i32, i32* %14, align 1
  %3243 = sext i32 %3242 to i64
  %3244 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3237, double* elementtype(double) %3227, i64 %3243)
  %3245 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3235, double* elementtype(double) %3244, i64 0)
  %3246 = sext i32 %114 to i64
  %3247 = add nsw i32 %150, 1
  %3248 = sext i32 %198 to i64
  %3249 = sext i32 %232 to i64
  %3250 = sext i32 %3247 to i64
  br label %3251

3251:                                             ; preds = %3277, %3213
  %3252 = phi i64 [ %3248, %3213 ], [ %3278, %3277 ]
  br i1 %2096, label %3277, label %3253

3253:                                             ; preds = %3251
  %3254 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3223, i64 %3222, double* elementtype(double) %3215, i64 %3252)
  %3255 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3225, i64 %94, double* elementtype(double) nonnull %77, i64 %3252)
  %3256 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3223, i64 %3233, double* elementtype(double) %3241, i64 %3252)
  %3257 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3223, i64 %3233, double* elementtype(double) %3245, i64 %3252)
  br label %3258

3258:                                             ; preds = %3258, %3253
  %3259 = phi i64 [ %3246, %3253 ], [ %3275, %3258 ]
  %3260 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3220, i64 %3219, double* elementtype(double) %3254, i64 %3259)
  %3261 = load double, double* %3260, align 1
  %3262 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3224, i64 8, double* elementtype(double) nonnull %3255, i64 %3259)
  %3263 = load double, double* %3262, align 1
  %3264 = fsub fast double %3261, %3263
  store double %3264, double* %3260, align 1
  %3265 = fmul fast double %3264, 0x3FFEAAAAAAAAAAAB
  %3266 = fadd fast double %3265, %3263
  %3267 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3220, i64 %3231, double* elementtype(double) %3256, i64 %3259)
  %3268 = load double, double* %3267, align 1
  %3269 = fmul fast double %3268, 0xBFF5555555555555
  %3270 = fadd fast double %3266, %3269
  %3271 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3220, i64 %3231, double* elementtype(double) %3257, i64 %3259)
  %3272 = load double, double* %3271, align 1
  %3273 = fmul fast double %3272, 0x3FDAAAAAAAAAAAAB
  %3274 = fadd fast double %3270, %3273
  store double %3274, double* %3262, align 1
  store double %3264, double* %3271, align 1
  %3275 = add nsw i64 %3259, 1
  %3276 = icmp eq i64 %3275, %3250
  br i1 %3276, label %3277, label %3258

3277:                                             ; preds = %3258, %3251
  %3278 = add nsw i64 %3252, 1
  %3279 = icmp eq i64 %3278, %3249
  br i1 %3279, label %3474, label %3251

3280:                                             ; preds = %3393
  br i1 %1320, label %3118, label %3281

3281:                                             ; preds = %3280
  %3282 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 0
  %3283 = load double*, double** %3282, align 1
  %3284 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 6, i64 0
  %3285 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3284, i64 0, i32 1
  %3286 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3285, i32 0)
  %3287 = load i64, i64* %3286, align 1
  %3288 = sext i32 %55 to i64
  %3289 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3285, i32 1)
  %3290 = load i64, i64* %3289, align 1
  %3291 = sext i32 %56 to i64
  %3292 = sext i32 %57 to i64
  %3293 = sext i32 %59 to i64
  %3294 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 0
  %3295 = load double*, double** %3294, align 1
  %3296 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 6, i64 0
  %3297 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3296, i64 0, i32 1
  %3298 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3297, i32 0)
  %3299 = load i64, i64* %3298, align 1
  %3300 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3297, i32 1)
  %3301 = load i64, i64* %3300, align 1
  %3302 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3297, i32 2)
  %3303 = load i64, i64* %3302, align 1
  %3304 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3297, i32 3)
  %3305 = load i64, i64* %3304, align 1
  %3306 = load i32, i32* %15, align 1
  %3307 = sext i32 %3306 to i64
  %3308 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3305, double* elementtype(double) %3295, i64 %3307)
  %3309 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3303, double* elementtype(double) %3308, i64 0)
  %3310 = load i32, i32* %14, align 1
  %3311 = sext i32 %3310 to i64
  %3312 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3305, double* elementtype(double) %3295, i64 %3311)
  %3313 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3303, double* elementtype(double) %3312, i64 0)
  %3314 = sext i32 %138 to i64
  %3315 = add nsw i32 %150, 1
  %3316 = sext i32 %174 to i64
  %3317 = sext i32 %232 to i64
  %3318 = sext i32 %3315 to i64
  br label %3092

3319:                                             ; preds = %3393
  br i1 %1320, label %3212, label %3320

3320:                                             ; preds = %3319
  %3321 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 0
  %3322 = load double*, double** %3321, align 1
  %3323 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 6, i64 0
  %3324 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3323, i64 0, i32 1
  %3325 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3324, i32 0)
  %3326 = load i64, i64* %3325, align 1
  %3327 = sext i32 %55 to i64
  %3328 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3324, i32 1)
  %3329 = load i64, i64* %3328, align 1
  %3330 = sext i32 %56 to i64
  %3331 = sext i32 %57 to i64
  %3332 = sext i32 %59 to i64
  %3333 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 0
  %3334 = load double*, double** %3333, align 1
  %3335 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 6, i64 0
  %3336 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3335, i64 0, i32 1
  %3337 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3336, i32 0)
  %3338 = load i64, i64* %3337, align 1
  %3339 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3336, i32 1)
  %3340 = load i64, i64* %3339, align 1
  %3341 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3336, i32 2)
  %3342 = load i64, i64* %3341, align 1
  %3343 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3336, i32 3)
  %3344 = load i64, i64* %3343, align 1
  %3345 = load i32, i32* %15, align 1
  %3346 = sext i32 %3345 to i64
  %3347 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3344, double* elementtype(double) %3334, i64 %3346)
  %3348 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3342, double* elementtype(double) %3347, i64 0)
  %3349 = load i32, i32* %14, align 1
  %3350 = sext i32 %3349 to i64
  %3351 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3344, double* elementtype(double) %3334, i64 %3350)
  %3352 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3342, double* elementtype(double) %3351, i64 0)
  %3353 = sext i32 %138 to i64
  %3354 = add nsw i32 %150, 1
  %3355 = sext i32 %174 to i64
  %3356 = sext i32 %232 to i64
  %3357 = sext i32 %3354 to i64
  br label %3183

3358:                                             ; preds = %3449
  br i1 %1320, label %3038, label %3359

3359:                                             ; preds = %3358
  %3360 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 0
  %3361 = load double*, double** %3360, align 1
  %3362 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 6, i64 0
  %3363 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3362, i64 0, i32 1
  %3364 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3363, i32 0)
  %3365 = load i64, i64* %3364, align 1
  %3366 = sext i32 %55 to i64
  %3367 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3363, i32 1)
  %3368 = load i64, i64* %3367, align 1
  %3369 = sext i32 %56 to i64
  %3370 = sext i32 %57 to i64
  %3371 = sext i32 %59 to i64
  %3372 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 0
  %3373 = load double*, double** %3372, align 1
  %3374 = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %46, i64 0, i32 6, i64 0
  %3375 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3374, i64 0, i32 1
  %3376 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3375, i32 0)
  %3377 = load i64, i64* %3376, align 1
  %3378 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3375, i32 1)
  %3379 = load i64, i64* %3378, align 1
  %3380 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3375, i32 2)
  %3381 = load i64, i64* %3380, align 1
  %3382 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3375, i32 3)
  %3383 = load i64, i64* %3382, align 1
  %3384 = load i32, i32* %14, align 1
  %3385 = sext i32 %3384 to i64
  %3386 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %3383, double* elementtype(double) %3373, i64 %3385)
  %3387 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 %3381, double* elementtype(double) %3386, i64 0)
  %3388 = sext i32 %138 to i64
  %3389 = add nsw i32 %150, 1
  %3390 = sext i32 %174 to i64
  %3391 = sext i32 %232 to i64
  %3392 = sext i32 %3389 to i64
  br label %3019

3393:                                             ; preds = %3449
  %3394 = add nsw i32 %3453, 1
  %3395 = icmp eq i32 %3451, %3394
  br i1 %3395, label %3280, label %3319

3396:                                             ; preds = %3456, %3410
  %3397 = phi i64 [ %3471, %3456 ], [ %3411, %3410 ]
  br i1 %1724, label %3410, label %3398

3398:                                             ; preds = %3396
  %3399 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3458, i64 %94, double* elementtype(double) nonnull %78, i64 %3397)
  %3400 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3468, i64 %3467, double* elementtype(double) %3460, i64 %3397)
  br label %3401

3401:                                             ; preds = %3401, %3398
  %3402 = phi i64 [ %3469, %3398 ], [ %3408, %3401 ]
  %3403 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3457, i64 8, double* elementtype(double) nonnull %3399, i64 %3402)
  %3404 = load double, double* %3403, align 1
  %3405 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3465, i64 %3464, double* elementtype(double) %3400, i64 %3402)
  %3406 = load double, double* %3405, align 1
  %3407 = fadd fast double %3406, %3404
  store double %3407, double* %3403, align 1
  %3408 = add nsw i64 %3402, 1
  %3409 = icmp eq i64 %3408, %3473
  br i1 %3409, label %3410, label %3401

3410:                                             ; preds = %3401, %3396
  %3411 = add nsw i64 %3397, 1
  %3412 = icmp eq i64 %3411, %3472
  br i1 %3412, label %3413, label %3396

3413:                                             ; preds = %3455, %3410
  br i1 %1864, label %3474, label %3414

3414:                                             ; preds = %3413
  %3415 = sext i32 %57 to i64
  %3416 = sext i32 %59 to i64
  %3417 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 0
  %3418 = load double*, double** %3417, align 1
  %3419 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %45, i64 0, i32 6, i64 0
  %3420 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3419, i64 0, i32 1
  %3421 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3420, i32 0)
  %3422 = load i64, i64* %3421, align 1
  %3423 = sext i32 %55 to i64
  %3424 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3420, i32 1)
  %3425 = load i64, i64* %3424, align 1
  %3426 = sext i32 %56 to i64
  %3427 = sext i32 %114 to i64
  %3428 = add nsw i32 %150, 1
  %3429 = sext i32 %198 to i64
  %3430 = sext i32 %232 to i64
  %3431 = sext i32 %3428 to i64
  br label %3432

3432:                                             ; preds = %3446, %3414
  %3433 = phi i64 [ %3429, %3414 ], [ %3447, %3446 ]
  br i1 %2096, label %3446, label %3434

3434:                                             ; preds = %3432
  %3435 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3416, i64 %94, double* elementtype(double) nonnull %77, i64 %3433)
  %3436 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3426, i64 %3425, double* elementtype(double) %3418, i64 %3433)
  br label %3437

3437:                                             ; preds = %3437, %3434
  %3438 = phi i64 [ %3427, %3434 ], [ %3444, %3437 ]
  %3439 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3415, i64 8, double* elementtype(double) nonnull %3435, i64 %3438)
  %3440 = load double, double* %3439, align 1
  %3441 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3423, i64 %3422, double* elementtype(double) %3436, i64 %3438)
  %3442 = load double, double* %3441, align 1
  %3443 = fadd fast double %3442, %3440
  store double %3443, double* %3439, align 1
  %3444 = add nsw i64 %3438, 1
  %3445 = icmp eq i64 %3444, %3431
  br i1 %3445, label %3446, label %3437

3446:                                             ; preds = %3437, %3432
  %3447 = add nsw i64 %3433, 1
  %3448 = icmp eq i64 %3447, %3430
  br i1 %3448, label %3474, label %3432

3449:                                             ; preds = %3012
  %3450 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_scalars_mp_iic_, i64 0, i64 0), i64 %99)
  %3451 = load i32, i32* %3450, align 1
  %3452 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([1 x i32], [1 x i32]* @mod_scalars_mp_ntfirst_, i64 0, i64 0), i64 %99)
  %3453 = load i32, i32* %3452, align 1
  %3454 = icmp eq i32 %3451, %3453
  br i1 %3454, label %3358, label %3393

3455:                                             ; preds = %3012
  br i1 %1320, label %3413, label %3456

3456:                                             ; preds = %3455
  %3457 = sext i32 %57 to i64
  %3458 = sext i32 %59 to i64
  %3459 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 0
  %3460 = load double*, double** %3459, align 1
  %3461 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %44, i64 0, i32 6, i64 0
  %3462 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3461, i64 0, i32 1
  %3463 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3462, i32 0)
  %3464 = load i64, i64* %3463, align 1
  %3465 = sext i32 %55 to i64
  %3466 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3462, i32 1)
  %3467 = load i64, i64* %3466, align 1
  %3468 = sext i32 %56 to i64
  %3469 = sext i32 %138 to i64
  %3470 = add nsw i32 %150, 1
  %3471 = sext i32 %174 to i64
  %3472 = sext i32 %232 to i64
  %3473 = sext i32 %3470 to i64
  br label %3396

3474:                                             ; preds = %3446, %3413, %3277, %3212, %3180, %3118, %3089, %3038
  br i1 %1915, label %3526, label %3475

3475:                                             ; preds = %3474
  %3476 = add i32 %138, -1
  %3477 = icmp slt i32 %150, %3476
  %3478 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 0
  %3479 = load double*, double** %3478, align 1
  %3480 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %53, i64 0, i32 6, i64 0
  %3481 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3480, i64 0, i32 1
  %3482 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3481, i32 0)
  %3483 = load i64, i64* %3482, align 1
  %3484 = sext i32 %55 to i64
  %3485 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3481, i32 1)
  %3486 = load i64, i64* %3485, align 1
  %3487 = sext i32 %56 to i64
  %3488 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3481, i32 2)
  %3489 = load i64, i64* %3488, align 1
  %3490 = sext i32 %223 to i64
  %3491 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3489, double* elementtype(double) %3479, i64 %3490)
  %3492 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 0
  %3493 = load double*, double** %3492, align 1
  %3494 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %17, i64 0, i32 6, i64 0
  %3495 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3494, i64 0, i32 1
  %3496 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3495, i32 0)
  %3497 = load i64, i64* %3496, align 1
  %3498 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3495, i32 1)
  %3499 = load i64, i64* %3498, align 1
  %3500 = sext i32 %57 to i64
  %3501 = sext i32 %59 to i64
  %3502 = sext i32 %3476 to i64
  %3503 = add nsw i32 %150, 1
  %3504 = sext i32 %228 to i64
  br label %3505

3505:                                             ; preds = %3522, %3475
  %3506 = phi i64 [ %3504, %3475 ], [ %3523, %3522 ]
  br i1 %3477, label %3522, label %3507

3507:                                             ; preds = %3505
  %3508 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3487, i64 %3486, double* elementtype(double) %3491, i64 %3506)
  %3509 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3487, i64 %3499, double* elementtype(double) %3493, i64 %3506)
  %3510 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3501, i64 %94, double* elementtype(double) nonnull %89, i64 %3506)
  br label %3511

3511:                                             ; preds = %3511, %3507
  %3512 = phi i64 [ %3502, %3507 ], [ %3519, %3511 ]
  %3513 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3484, i64 %3483, double* elementtype(double) %3508, i64 %3512)
  %3514 = load double, double* %3513, align 1
  %3515 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3484, i64 %3497, double* elementtype(double) %3509, i64 %3512)
  %3516 = load double, double* %3515, align 1
  %3517 = fadd fast double %3516, %3514
  %3518 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3500, i64 8, double* elementtype(double) nonnull %3510, i64 %3512)
  store double %3517, double* %3518, align 1
  %3519 = add nsw i64 %3512, 1
  %3520 = trunc i64 %3519 to i32
  %3521 = icmp eq i32 %3503, %3520
  br i1 %3521, label %3522, label %3511

3522:                                             ; preds = %3511, %3505
  %3523 = add nsw i64 %3506, 1
  %3524 = trunc i64 %3523 to i32
  %3525 = icmp eq i32 %232, %3524
  br i1 %3525, label %3526, label %3505

3526:                                             ; preds = %3522, %3474
  %3527 = load i32, i32* %416, align 1
  %3528 = icmp eq i32 %3527, 1
  br i1 %3528, label %4101, label %4146

3529:                                             ; preds = %4105, %3575
  %3530 = phi i64 [ %4143, %4105 ], [ %3576, %3575 ]
  br i1 %1724, label %3575, label %3531

3531:                                             ; preds = %3529
  %3532 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4115, i64 %4114, double* elementtype(double) %4107, i64 %3530)
  %3533 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4115, i64 %4123, double* elementtype(double) %4117, i64 %3530)
  %3534 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4125, i64 %94, double* elementtype(double) nonnull %92, i64 %3530)
  %3535 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4115, i64 %4133, double* elementtype(double) %4137, i64 %3530)
  %3536 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4125, i64 %94, double* elementtype(double) nonnull %89, i64 %3530)
  %3537 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4125, i64 %94, double* elementtype(double) nonnull %78, i64 %3530)
  %3538 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4115, i64 %4133, double* elementtype(double) %4140, i64 %3530)
  br label %3539

3539:                                             ; preds = %3539, %3531
  %3540 = phi i64 [ %4141, %3531 ], [ %3573, %3539 ]
  %3541 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4111, double* elementtype(double) %3532, i64 %3540)
  %3542 = load double, double* %3541, align 1
  %3543 = add nsw i64 %3540, -1
  %3544 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4111, double* elementtype(double) %3532, i64 %3543)
  %3545 = load double, double* %3544, align 1
  %3546 = fadd fast double %3545, %3542
  %3547 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4121, double* elementtype(double) %3533, i64 %3540)
  %3548 = load double, double* %3547, align 1
  %3549 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4121, double* elementtype(double) %3533, i64 %3543)
  %3550 = load double, double* %3549, align 1
  %3551 = fadd fast double %3550, %3548
  %3552 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4124, i64 8, double* elementtype(double) nonnull %3534, i64 %3540)
  %3553 = load double, double* %3552, align 1
  %3554 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4124, i64 8, double* elementtype(double) nonnull %3534, i64 %3543)
  %3555 = load double, double* %3554, align 1
  %3556 = fadd fast double %3555, %3553
  %3557 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4131, double* elementtype(double) %3535, i64 %3540)
  %3558 = load double, double* %3557, align 1
  %3559 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4124, i64 8, double* elementtype(double) nonnull %3536, i64 %3540)
  %3560 = load double, double* %3559, align 1
  %3561 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4124, i64 8, double* elementtype(double) nonnull %3536, i64 %3543)
  %3562 = load double, double* %3561, align 1
  %3563 = fadd fast double %3562, %3560
  %3564 = fmul fast double %3563, %3558
  %3565 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4124, i64 8, double* elementtype(double) nonnull %3537, i64 %3540)
  %3566 = load double, double* %3565, align 1
  %3567 = fmul fast double %3546, %4104
  %3568 = fmul fast double %3567, %3551
  %3569 = fmul fast double %3568, %3566
  %3570 = fadd fast double %3564, %3569
  %3571 = fdiv fast double %3570, %3556
  %3572 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4112, i64 %4131, double* elementtype(double) %3538, i64 %3540)
  store double %3571, double* %3572, align 1
  %3573 = add nsw i64 %3540, 1
  %3574 = icmp eq i64 %3573, %4145
  br i1 %3574, label %3575, label %3539

3575:                                             ; preds = %3539, %3529
  %3576 = add nsw i64 %3530, 1
  %3577 = icmp eq i64 %3576, %4144
  br i1 %3577, label %3578, label %3529

3578:                                             ; preds = %4101, %3575
  br i1 %1864, label %4150, label %3579

3579:                                             ; preds = %3578
  %3580 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %3581 = load double*, double** %3580, align 1
  %3582 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %3583 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3582, i64 0, i32 1
  %3584 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3583, i32 0)
  %3585 = load i64, i64* %3584, align 1
  %3586 = sext i32 %55 to i64
  %3587 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3583, i32 1)
  %3588 = load i64, i64* %3587, align 1
  %3589 = sext i32 %56 to i64
  %3590 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %3591 = load double*, double** %3590, align 1
  %3592 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %3593 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3592, i64 0, i32 1
  %3594 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3593, i32 0)
  %3595 = load i64, i64* %3594, align 1
  %3596 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3593, i32 1)
  %3597 = load i64, i64* %3596, align 1
  %3598 = sext i32 %57 to i64
  %3599 = sext i32 %59 to i64
  %3600 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %3601 = load double*, double** %3600, align 1
  %3602 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %3603 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3602, i64 0, i32 1
  %3604 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3603, i32 0)
  %3605 = load i64, i64* %3604, align 1
  %3606 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3603, i32 1)
  %3607 = load i64, i64* %3606, align 1
  %3608 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3603, i32 2)
  %3609 = load i64, i64* %3608, align 1
  %3610 = sext i32 %223 to i64
  %3611 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3609, double* elementtype(double) %3601, i64 %3610)
  %3612 = load i32, i32* %13, align 1
  %3613 = sext i32 %3612 to i64
  %3614 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3609, double* elementtype(double) %3601, i64 %3613)
  %3615 = sext i32 %114 to i64
  %3616 = add nsw i32 %150, 1
  %3617 = sext i32 %198 to i64
  %3618 = sext i32 %232 to i64
  %3619 = sext i32 %3616 to i64
  br label %3620

3620:                                             ; preds = %3670, %3579
  %3621 = phi i64 [ %3617, %3579 ], [ %3671, %3670 ]
  br i1 %2096, label %3670, label %3622

3622:                                             ; preds = %3620
  %3623 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3588, double* elementtype(double) %3581, i64 %3621)
  %3624 = add nsw i64 %3621, -1
  %3625 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3588, double* elementtype(double) %3581, i64 %3624)
  %3626 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3597, double* elementtype(double) %3591, i64 %3621)
  %3627 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3597, double* elementtype(double) %3591, i64 %3624)
  %3628 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3599, i64 %94, double* elementtype(double) nonnull %92, i64 %3621)
  %3629 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3599, i64 %94, double* elementtype(double) nonnull %92, i64 %3624)
  %3630 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3607, double* elementtype(double) %3611, i64 %3621)
  %3631 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3599, i64 %94, double* elementtype(double) nonnull %89, i64 %3621)
  %3632 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3599, i64 %94, double* elementtype(double) nonnull %89, i64 %3624)
  %3633 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3599, i64 %94, double* elementtype(double) nonnull %77, i64 %3621)
  %3634 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3589, i64 %3607, double* elementtype(double) %3614, i64 %3621)
  br label %3635

3635:                                             ; preds = %3635, %3622
  %3636 = phi i64 [ %3615, %3622 ], [ %3668, %3635 ]
  %3637 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3585, double* elementtype(double) %3623, i64 %3636)
  %3638 = load double, double* %3637, align 1
  %3639 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3585, double* elementtype(double) %3625, i64 %3636)
  %3640 = load double, double* %3639, align 1
  %3641 = fadd fast double %3640, %3638
  %3642 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3595, double* elementtype(double) %3626, i64 %3636)
  %3643 = load double, double* %3642, align 1
  %3644 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3595, double* elementtype(double) %3627, i64 %3636)
  %3645 = load double, double* %3644, align 1
  %3646 = fadd fast double %3645, %3643
  %3647 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3598, i64 8, double* elementtype(double) nonnull %3628, i64 %3636)
  %3648 = load double, double* %3647, align 1
  %3649 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3598, i64 8, double* elementtype(double) nonnull %3629, i64 %3636)
  %3650 = load double, double* %3649, align 1
  %3651 = fadd fast double %3650, %3648
  %3652 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3605, double* elementtype(double) %3630, i64 %3636)
  %3653 = load double, double* %3652, align 1
  %3654 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3598, i64 8, double* elementtype(double) nonnull %3631, i64 %3636)
  %3655 = load double, double* %3654, align 1
  %3656 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3598, i64 8, double* elementtype(double) nonnull %3632, i64 %3636)
  %3657 = load double, double* %3656, align 1
  %3658 = fadd fast double %3657, %3655
  %3659 = fmul fast double %3658, %3653
  %3660 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3598, i64 8, double* elementtype(double) nonnull %3633, i64 %3636)
  %3661 = load double, double* %3660, align 1
  %3662 = fmul fast double %3641, %4104
  %3663 = fmul fast double %3662, %3646
  %3664 = fmul fast double %3663, %3661
  %3665 = fadd fast double %3659, %3664
  %3666 = fdiv fast double %3665, %3651
  %3667 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3586, i64 %3605, double* elementtype(double) %3634, i64 %3636)
  store double %3666, double* %3667, align 1
  %3668 = add nsw i64 %3636, 1
  %3669 = icmp eq i64 %3668, %3619
  br i1 %3669, label %3670, label %3635

3670:                                             ; preds = %3635, %3620
  %3671 = add nsw i64 %3621, 1
  %3672 = icmp eq i64 %3671, %3618
  br i1 %3672, label %4150, label %3620

3673:                                             ; preds = %4057, %3719
  %3674 = phi i64 [ %4095, %4057 ], [ %3720, %3719 ]
  br i1 %1724, label %3719, label %3675

3675:                                             ; preds = %3673
  %3676 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4067, i64 %4066, double* elementtype(double) %4059, i64 %3674)
  %3677 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4067, i64 %4075, double* elementtype(double) %4069, i64 %3674)
  %3678 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4077, i64 %94, double* elementtype(double) nonnull %92, i64 %3674)
  %3679 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4067, i64 %4085, double* elementtype(double) %4089, i64 %3674)
  %3680 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4077, i64 %94, double* elementtype(double) nonnull %89, i64 %3674)
  %3681 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4077, i64 %94, double* elementtype(double) nonnull %78, i64 %3674)
  %3682 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4067, i64 %4085, double* elementtype(double) %4092, i64 %3674)
  br label %3683

3683:                                             ; preds = %3683, %3675
  %3684 = phi i64 [ %4093, %3675 ], [ %3717, %3683 ]
  %3685 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4063, double* elementtype(double) %3676, i64 %3684)
  %3686 = load double, double* %3685, align 1
  %3687 = add nsw i64 %3684, -1
  %3688 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4063, double* elementtype(double) %3676, i64 %3687)
  %3689 = load double, double* %3688, align 1
  %3690 = fadd fast double %3689, %3686
  %3691 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4073, double* elementtype(double) %3677, i64 %3684)
  %3692 = load double, double* %3691, align 1
  %3693 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4073, double* elementtype(double) %3677, i64 %3687)
  %3694 = load double, double* %3693, align 1
  %3695 = fadd fast double %3694, %3692
  %3696 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4076, i64 8, double* elementtype(double) nonnull %3678, i64 %3684)
  %3697 = load double, double* %3696, align 1
  %3698 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4076, i64 8, double* elementtype(double) nonnull %3678, i64 %3687)
  %3699 = load double, double* %3698, align 1
  %3700 = fadd fast double %3699, %3697
  %3701 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4083, double* elementtype(double) %3679, i64 %3684)
  %3702 = load double, double* %3701, align 1
  %3703 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4076, i64 8, double* elementtype(double) nonnull %3680, i64 %3684)
  %3704 = load double, double* %3703, align 1
  %3705 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4076, i64 8, double* elementtype(double) nonnull %3680, i64 %3687)
  %3706 = load double, double* %3705, align 1
  %3707 = fadd fast double %3706, %3704
  %3708 = fmul fast double %3707, %3702
  %3709 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4076, i64 8, double* elementtype(double) nonnull %3681, i64 %3684)
  %3710 = load double, double* %3709, align 1
  %3711 = fmul fast double %3690, %4056
  %3712 = fmul fast double %3711, %3695
  %3713 = fmul fast double %3712, %3710
  %3714 = fadd fast double %3708, %3713
  %3715 = fdiv fast double %3714, %3700
  %3716 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4064, i64 %4083, double* elementtype(double) %3682, i64 %3684)
  store double %3715, double* %3716, align 1
  %3717 = add nsw i64 %3684, 1
  %3718 = icmp eq i64 %3717, %4097
  br i1 %3718, label %3719, label %3683

3719:                                             ; preds = %3683, %3673
  %3720 = add nsw i64 %3674, 1
  %3721 = icmp eq i64 %3720, %4096
  br i1 %3721, label %3722, label %3673

3722:                                             ; preds = %4054, %3719
  br i1 %1864, label %4150, label %3723

3723:                                             ; preds = %3722
  %3724 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %3725 = load double*, double** %3724, align 1
  %3726 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %3727 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3726, i64 0, i32 1
  %3728 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3727, i32 0)
  %3729 = load i64, i64* %3728, align 1
  %3730 = sext i32 %55 to i64
  %3731 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3727, i32 1)
  %3732 = load i64, i64* %3731, align 1
  %3733 = sext i32 %56 to i64
  %3734 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %3735 = load double*, double** %3734, align 1
  %3736 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %3737 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3736, i64 0, i32 1
  %3738 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3737, i32 0)
  %3739 = load i64, i64* %3738, align 1
  %3740 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3737, i32 1)
  %3741 = load i64, i64* %3740, align 1
  %3742 = sext i32 %57 to i64
  %3743 = sext i32 %59 to i64
  %3744 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %3745 = load double*, double** %3744, align 1
  %3746 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %3747 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3746, i64 0, i32 1
  %3748 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3747, i32 0)
  %3749 = load i64, i64* %3748, align 1
  %3750 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3747, i32 1)
  %3751 = load i64, i64* %3750, align 1
  %3752 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3747, i32 2)
  %3753 = load i64, i64* %3752, align 1
  %3754 = sext i32 %223 to i64
  %3755 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3753, double* elementtype(double) %3745, i64 %3754)
  %3756 = load i32, i32* %13, align 1
  %3757 = sext i32 %3756 to i64
  %3758 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3753, double* elementtype(double) %3745, i64 %3757)
  %3759 = sext i32 %114 to i64
  %3760 = add nsw i32 %150, 1
  %3761 = sext i32 %198 to i64
  %3762 = sext i32 %232 to i64
  %3763 = sext i32 %3760 to i64
  br label %3764

3764:                                             ; preds = %3814, %3723
  %3765 = phi i64 [ %3761, %3723 ], [ %3815, %3814 ]
  br i1 %2096, label %3814, label %3766

3766:                                             ; preds = %3764
  %3767 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3732, double* elementtype(double) %3725, i64 %3765)
  %3768 = add nsw i64 %3765, -1
  %3769 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3732, double* elementtype(double) %3725, i64 %3768)
  %3770 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3741, double* elementtype(double) %3735, i64 %3765)
  %3771 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3741, double* elementtype(double) %3735, i64 %3768)
  %3772 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3743, i64 %94, double* elementtype(double) nonnull %92, i64 %3765)
  %3773 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3743, i64 %94, double* elementtype(double) nonnull %92, i64 %3768)
  %3774 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3751, double* elementtype(double) %3755, i64 %3765)
  %3775 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3743, i64 %94, double* elementtype(double) nonnull %89, i64 %3765)
  %3776 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3743, i64 %94, double* elementtype(double) nonnull %89, i64 %3768)
  %3777 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3743, i64 %94, double* elementtype(double) nonnull %77, i64 %3765)
  %3778 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3733, i64 %3751, double* elementtype(double) %3758, i64 %3765)
  br label %3779

3779:                                             ; preds = %3779, %3766
  %3780 = phi i64 [ %3759, %3766 ], [ %3812, %3779 ]
  %3781 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3729, double* elementtype(double) %3767, i64 %3780)
  %3782 = load double, double* %3781, align 1
  %3783 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3729, double* elementtype(double) %3769, i64 %3780)
  %3784 = load double, double* %3783, align 1
  %3785 = fadd fast double %3784, %3782
  %3786 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3739, double* elementtype(double) %3770, i64 %3780)
  %3787 = load double, double* %3786, align 1
  %3788 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3739, double* elementtype(double) %3771, i64 %3780)
  %3789 = load double, double* %3788, align 1
  %3790 = fadd fast double %3789, %3787
  %3791 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3742, i64 8, double* elementtype(double) nonnull %3772, i64 %3780)
  %3792 = load double, double* %3791, align 1
  %3793 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3742, i64 8, double* elementtype(double) nonnull %3773, i64 %3780)
  %3794 = load double, double* %3793, align 1
  %3795 = fadd fast double %3794, %3792
  %3796 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3749, double* elementtype(double) %3774, i64 %3780)
  %3797 = load double, double* %3796, align 1
  %3798 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3742, i64 8, double* elementtype(double) nonnull %3775, i64 %3780)
  %3799 = load double, double* %3798, align 1
  %3800 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3742, i64 8, double* elementtype(double) nonnull %3776, i64 %3780)
  %3801 = load double, double* %3800, align 1
  %3802 = fadd fast double %3801, %3799
  %3803 = fmul fast double %3802, %3797
  %3804 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3742, i64 8, double* elementtype(double) nonnull %3777, i64 %3780)
  %3805 = load double, double* %3804, align 1
  %3806 = fmul fast double %3785, %4056
  %3807 = fmul fast double %3806, %3790
  %3808 = fmul fast double %3807, %3805
  %3809 = fadd fast double %3803, %3808
  %3810 = fdiv fast double %3809, %3795
  %3811 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3730, i64 %3749, double* elementtype(double) %3778, i64 %3780)
  store double %3810, double* %3811, align 1
  %3812 = add nsw i64 %3780, 1
  %3813 = icmp eq i64 %3812, %3763
  br i1 %3813, label %3814, label %3779

3814:                                             ; preds = %3779, %3764
  %3815 = add nsw i64 %3765, 1
  %3816 = icmp eq i64 %3815, %3762
  br i1 %3816, label %4150, label %3764

3817:                                             ; preds = %4000, %3873
  %3818 = phi i64 [ %4051, %4000 ], [ %3874, %3873 ]
  br i1 %1724, label %3873, label %3819

3819:                                             ; preds = %3817
  %3820 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4009, double* elementtype(double) %4002, i64 %3818)
  %3821 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4018, double* elementtype(double) %4012, i64 %3818)
  %3822 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4020, i64 %94, double* elementtype(double) nonnull %92, i64 %3818)
  %3823 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4028, double* elementtype(double) %4032, i64 %3818)
  %3824 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4020, i64 %94, double* elementtype(double) nonnull %89, i64 %3818)
  %3825 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4020, i64 %94, double* elementtype(double) nonnull %78, i64 %3818)
  %3826 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4040, double* elementtype(double) %4043, i64 %3818)
  %3827 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4040, double* elementtype(double) %4045, i64 %3818)
  %3828 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4010, i64 %4028, double* elementtype(double) %4048, i64 %3818)
  br label %3829

3829:                                             ; preds = %3829, %3819
  %3830 = phi i64 [ %4049, %3819 ], [ %3871, %3829 ]
  %3831 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4006, double* elementtype(double) %3820, i64 %3830)
  %3832 = load double, double* %3831, align 1
  %3833 = add nsw i64 %3830, -1
  %3834 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4006, double* elementtype(double) %3820, i64 %3833)
  %3835 = load double, double* %3834, align 1
  %3836 = fadd fast double %3835, %3832
  %3837 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4016, double* elementtype(double) %3821, i64 %3830)
  %3838 = load double, double* %3837, align 1
  %3839 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4016, double* elementtype(double) %3821, i64 %3833)
  %3840 = load double, double* %3839, align 1
  %3841 = fadd fast double %3840, %3838
  %3842 = fmul fast double %3841, %3836
  %3843 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4019, i64 8, double* elementtype(double) nonnull %3822, i64 %3830)
  %3844 = load double, double* %3843, align 1
  %3845 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4019, i64 8, double* elementtype(double) nonnull %3822, i64 %3833)
  %3846 = load double, double* %3845, align 1
  %3847 = fadd fast double %3846, %3844
  %3848 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4026, double* elementtype(double) %3823, i64 %3830)
  %3849 = load double, double* %3848, align 1
  %3850 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4019, i64 8, double* elementtype(double) nonnull %3824, i64 %3830)
  %3851 = load double, double* %3850, align 1
  %3852 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4019, i64 8, double* elementtype(double) nonnull %3824, i64 %3833)
  %3853 = load double, double* %3852, align 1
  %3854 = fadd fast double %3853, %3851
  %3855 = fmul fast double %3854, %3849
  %3856 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4019, i64 8, double* elementtype(double) nonnull %3825, i64 %3830)
  %3857 = load double, double* %3856, align 1
  %3858 = fmul fast double %3857, %3997
  %3859 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4038, double* elementtype(double) %3826, i64 %3830)
  %3860 = load double, double* %3859, align 1
  %3861 = fmul fast double %3860, %3998
  %3862 = fadd fast double %3861, %3858
  %3863 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4038, double* elementtype(double) %3827, i64 %3830)
  %3864 = load double, double* %3863, align 1
  %3865 = fmul fast double %3864, %3999
  %3866 = fsub fast double %3862, %3865
  %3867 = fmul fast double %3842, %3866
  %3868 = fadd fast double %3867, %3855
  %3869 = fdiv fast double %3868, %3847
  %3870 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4007, i64 %4026, double* elementtype(double) %3828, i64 %3830)
  store double %3869, double* %3870, align 1
  %3871 = add nsw i64 %3830, 1
  %3872 = icmp eq i64 %3871, %4053
  br i1 %3872, label %3873, label %3829

3873:                                             ; preds = %3829, %3817
  %3874 = add nsw i64 %3818, 1
  %3875 = icmp eq i64 %3874, %4052
  br i1 %3875, label %3876, label %3817

3876:                                             ; preds = %3994, %3873
  br i1 %1864, label %4150, label %3877

3877:                                             ; preds = %3876
  %3878 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %3879 = load double*, double** %3878, align 1
  %3880 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %3881 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3880, i64 0, i32 1
  %3882 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3881, i32 0)
  %3883 = load i64, i64* %3882, align 1
  %3884 = sext i32 %55 to i64
  %3885 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3881, i32 1)
  %3886 = load i64, i64* %3885, align 1
  %3887 = sext i32 %56 to i64
  %3888 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %3889 = load double*, double** %3888, align 1
  %3890 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %3891 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3890, i64 0, i32 1
  %3892 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3891, i32 0)
  %3893 = load i64, i64* %3892, align 1
  %3894 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3891, i32 1)
  %3895 = load i64, i64* %3894, align 1
  %3896 = sext i32 %57 to i64
  %3897 = sext i32 %59 to i64
  %3898 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 0
  %3899 = load double*, double** %3898, align 1
  %3900 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %52, i64 0, i32 6, i64 0
  %3901 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3900, i64 0, i32 1
  %3902 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3901, i32 0)
  %3903 = load i64, i64* %3902, align 1
  %3904 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3901, i32 1)
  %3905 = load i64, i64* %3904, align 1
  %3906 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3901, i32 2)
  %3907 = load i64, i64* %3906, align 1
  %3908 = sext i32 %223 to i64
  %3909 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3907, double* elementtype(double) %3899, i64 %3908)
  %3910 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %49, i64 0, i32 0
  %3911 = load double*, double** %3910, align 1
  %3912 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %49, i64 0, i32 6, i64 0
  %3913 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %3912, i64 0, i32 1
  %3914 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3913, i32 0)
  %3915 = load i64, i64* %3914, align 1
  %3916 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3913, i32 1)
  %3917 = load i64, i64* %3916, align 1
  %3918 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %3913, i32 2)
  %3919 = load i64, i64* %3918, align 1
  %3920 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3919, double* elementtype(double) %3911, i64 %3908)
  %3921 = sext i32 %224 to i64
  %3922 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3919, double* elementtype(double) %3911, i64 %3921)
  %3923 = load i32, i32* %13, align 1
  %3924 = sext i32 %3923 to i64
  %3925 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %3907, double* elementtype(double) %3899, i64 %3924)
  %3926 = sext i32 %114 to i64
  %3927 = add nsw i32 %150, 1
  %3928 = sext i32 %198 to i64
  %3929 = sext i32 %232 to i64
  %3930 = sext i32 %3927 to i64
  br label %3931

3931:                                             ; preds = %3991, %3877
  %3932 = phi i64 [ %3928, %3877 ], [ %3992, %3991 ]
  br i1 %2096, label %3991, label %3933

3933:                                             ; preds = %3931
  %3934 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3886, double* elementtype(double) %3879, i64 %3932)
  %3935 = add nsw i64 %3932, -1
  %3936 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3886, double* elementtype(double) %3879, i64 %3935)
  %3937 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3895, double* elementtype(double) %3889, i64 %3932)
  %3938 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3895, double* elementtype(double) %3889, i64 %3935)
  %3939 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3897, i64 %94, double* elementtype(double) nonnull %92, i64 %3932)
  %3940 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3897, i64 %94, double* elementtype(double) nonnull %92, i64 %3935)
  %3941 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3905, double* elementtype(double) %3909, i64 %3932)
  %3942 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3897, i64 %94, double* elementtype(double) nonnull %89, i64 %3932)
  %3943 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3897, i64 %94, double* elementtype(double) nonnull %89, i64 %3935)
  %3944 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3897, i64 %94, double* elementtype(double) nonnull %77, i64 %3932)
  %3945 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3917, double* elementtype(double) %3920, i64 %3932)
  %3946 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3917, double* elementtype(double) %3922, i64 %3932)
  %3947 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %3887, i64 %3905, double* elementtype(double) %3925, i64 %3932)
  br label %3948

3948:                                             ; preds = %3948, %3933
  %3949 = phi i64 [ %3926, %3933 ], [ %3989, %3948 ]
  %3950 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3883, double* elementtype(double) %3934, i64 %3949)
  %3951 = load double, double* %3950, align 1
  %3952 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3883, double* elementtype(double) %3936, i64 %3949)
  %3953 = load double, double* %3952, align 1
  %3954 = fadd fast double %3953, %3951
  %3955 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3893, double* elementtype(double) %3937, i64 %3949)
  %3956 = load double, double* %3955, align 1
  %3957 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3893, double* elementtype(double) %3938, i64 %3949)
  %3958 = load double, double* %3957, align 1
  %3959 = fadd fast double %3958, %3956
  %3960 = fmul fast double %3959, %3954
  %3961 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3896, i64 8, double* elementtype(double) nonnull %3939, i64 %3949)
  %3962 = load double, double* %3961, align 1
  %3963 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3896, i64 8, double* elementtype(double) nonnull %3940, i64 %3949)
  %3964 = load double, double* %3963, align 1
  %3965 = fadd fast double %3964, %3962
  %3966 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3903, double* elementtype(double) %3941, i64 %3949)
  %3967 = load double, double* %3966, align 1
  %3968 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3896, i64 8, double* elementtype(double) nonnull %3942, i64 %3949)
  %3969 = load double, double* %3968, align 1
  %3970 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3896, i64 8, double* elementtype(double) nonnull %3943, i64 %3949)
  %3971 = load double, double* %3970, align 1
  %3972 = fadd fast double %3971, %3969
  %3973 = fmul fast double %3972, %3967
  %3974 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3896, i64 8, double* elementtype(double) nonnull %3944, i64 %3949)
  %3975 = load double, double* %3974, align 1
  %3976 = fmul fast double %3975, %3997
  %3977 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3915, double* elementtype(double) %3945, i64 %3949)
  %3978 = load double, double* %3977, align 1
  %3979 = fmul fast double %3978, %3998
  %3980 = fadd fast double %3979, %3976
  %3981 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3915, double* elementtype(double) %3946, i64 %3949)
  %3982 = load double, double* %3981, align 1
  %3983 = fmul fast double %3982, %3999
  %3984 = fsub fast double %3980, %3983
  %3985 = fmul fast double %3960, %3984
  %3986 = fadd fast double %3985, %3973
  %3987 = fdiv fast double %3986, %3965
  %3988 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %3884, i64 %3903, double* elementtype(double) %3947, i64 %3949)
  store double %3987, double* %3988, align 1
  %3989 = add nsw i64 %3949, 1
  %3990 = icmp eq i64 %3989, %3930
  br i1 %3990, label %3991, label %3948

3991:                                             ; preds = %3948, %3931
  %3992 = add nsw i64 %3932, 1
  %3993 = icmp eq i64 %3992, %3929
  br i1 %3993, label %4150, label %3931

3994:                                             ; preds = %4098
  %3995 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %3996 = load double, double* %3995, align 1
  %3997 = fmul fast double %3996, 0x3FCAAAAAAAAAAAAA
  %3998 = fmul fast double %3996, 0x3FD5555555555555
  %3999 = fmul fast double %3996, 0x3FA5555555555555
  br i1 %1320, label %3876, label %4000

4000:                                             ; preds = %3994
  %4001 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %4002 = load double*, double** %4001, align 1
  %4003 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %4004 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4003, i64 0, i32 1
  %4005 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4004, i32 0)
  %4006 = load i64, i64* %4005, align 1
  %4007 = sext i32 %55 to i64
  %4008 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4004, i32 1)
  %4009 = load i64, i64* %4008, align 1
  %4010 = sext i32 %56 to i64
  %4011 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %4012 = load double*, double** %4011, align 1
  %4013 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %4014 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4013, i64 0, i32 1
  %4015 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4014, i32 0)
  %4016 = load i64, i64* %4015, align 1
  %4017 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4014, i32 1)
  %4018 = load i64, i64* %4017, align 1
  %4019 = sext i32 %57 to i64
  %4020 = sext i32 %59 to i64
  %4021 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %4022 = load double*, double** %4021, align 1
  %4023 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %4024 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4023, i64 0, i32 1
  %4025 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4024, i32 0)
  %4026 = load i64, i64* %4025, align 1
  %4027 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4024, i32 1)
  %4028 = load i64, i64* %4027, align 1
  %4029 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4024, i32 2)
  %4030 = load i64, i64* %4029, align 1
  %4031 = sext i32 %223 to i64
  %4032 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4030, double* elementtype(double) %4022, i64 %4031)
  %4033 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %48, i64 0, i32 0
  %4034 = load double*, double** %4033, align 1
  %4035 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %48, i64 0, i32 6, i64 0
  %4036 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4035, i64 0, i32 1
  %4037 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4036, i32 0)
  %4038 = load i64, i64* %4037, align 1
  %4039 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4036, i32 1)
  %4040 = load i64, i64* %4039, align 1
  %4041 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4036, i32 2)
  %4042 = load i64, i64* %4041, align 1
  %4043 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4042, double* elementtype(double) %4034, i64 %4031)
  %4044 = sext i32 %224 to i64
  %4045 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4042, double* elementtype(double) %4034, i64 %4044)
  %4046 = load i32, i32* %13, align 1
  %4047 = sext i32 %4046 to i64
  %4048 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4030, double* elementtype(double) %4022, i64 %4047)
  %4049 = sext i32 %138 to i64
  %4050 = add nsw i32 %150, 1
  %4051 = sext i32 %174 to i64
  %4052 = sext i32 %232 to i64
  %4053 = sext i32 %4050 to i64
  br label %3817

4054:                                             ; preds = %4146
  %4055 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %4056 = load double, double* %4055, align 1
  br i1 %1320, label %3722, label %4057

4057:                                             ; preds = %4054
  %4058 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %4059 = load double*, double** %4058, align 1
  %4060 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %4061 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4060, i64 0, i32 1
  %4062 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4061, i32 0)
  %4063 = load i64, i64* %4062, align 1
  %4064 = sext i32 %55 to i64
  %4065 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4061, i32 1)
  %4066 = load i64, i64* %4065, align 1
  %4067 = sext i32 %56 to i64
  %4068 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %4069 = load double*, double** %4068, align 1
  %4070 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %4071 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4070, i64 0, i32 1
  %4072 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4071, i32 0)
  %4073 = load i64, i64* %4072, align 1
  %4074 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4071, i32 1)
  %4075 = load i64, i64* %4074, align 1
  %4076 = sext i32 %57 to i64
  %4077 = sext i32 %59 to i64
  %4078 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %4079 = load double*, double** %4078, align 1
  %4080 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %4081 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4080, i64 0, i32 1
  %4082 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4081, i32 0)
  %4083 = load i64, i64* %4082, align 1
  %4084 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4081, i32 1)
  %4085 = load i64, i64* %4084, align 1
  %4086 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4081, i32 2)
  %4087 = load i64, i64* %4086, align 1
  %4088 = sext i32 %223 to i64
  %4089 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4087, double* elementtype(double) %4079, i64 %4088)
  %4090 = load i32, i32* %13, align 1
  %4091 = sext i32 %4090 to i64
  %4092 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4087, double* elementtype(double) %4079, i64 %4091)
  %4093 = sext i32 %138 to i64
  %4094 = add nsw i32 %150, 1
  %4095 = sext i32 %174 to i64
  %4096 = sext i32 %232 to i64
  %4097 = sext i32 %4094 to i64
  br label %3673

4098:                                             ; preds = %4146
  %4099 = and i32 %227, 1
  %4100 = icmp eq i32 %4099, 0
  br i1 %4100, label %4150, label %3994

4101:                                             ; preds = %3526
  %4102 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([1 x double], [1 x double]* @mod_scalars_mp_dtfast_, i64 0, i64 0), i64 %99)
  %4103 = load double, double* %4102, align 1
  %4104 = fmul fast double %4103, 5.000000e-01
  br i1 %1320, label %3578, label %4105

4105:                                             ; preds = %4101
  %4106 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 0
  %4107 = load double*, double** %4106, align 1
  %4108 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %23, i64 0, i32 6, i64 0
  %4109 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4108, i64 0, i32 1
  %4110 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4109, i32 0)
  %4111 = load i64, i64* %4110, align 1
  %4112 = sext i32 %55 to i64
  %4113 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4109, i32 1)
  %4114 = load i64, i64* %4113, align 1
  %4115 = sext i32 %56 to i64
  %4116 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %4117 = load double*, double** %4116, align 1
  %4118 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0
  %4119 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4118, i64 0, i32 1
  %4120 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4119, i32 0)
  %4121 = load i64, i64* %4120, align 1
  %4122 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4119, i32 1)
  %4123 = load i64, i64* %4122, align 1
  %4124 = sext i32 %57 to i64
  %4125 = sext i32 %59 to i64
  %4126 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 0
  %4127 = load double*, double** %4126, align 1
  %4128 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %51, i64 0, i32 6, i64 0
  %4129 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4128, i64 0, i32 1
  %4130 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4129, i32 0)
  %4131 = load i64, i64* %4130, align 1
  %4132 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4129, i32 1)
  %4133 = load i64, i64* %4132, align 1
  %4134 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4129, i32 2)
  %4135 = load i64, i64* %4134, align 1
  %4136 = sext i32 %223 to i64
  %4137 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4135, double* elementtype(double) %4127, i64 %4136)
  %4138 = load i32, i32* %13, align 1
  %4139 = sext i32 %4138 to i64
  %4140 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4135, double* elementtype(double) %4127, i64 %4139)
  %4141 = sext i32 %138 to i64
  %4142 = add nsw i32 %150, 1
  %4143 = sext i32 %174 to i64
  %4144 = sext i32 %232 to i64
  %4145 = sext i32 %4142 to i64
  br label %3529

4146:                                             ; preds = %3526
  %4147 = load i32, i32* %225, align 1
  %4148 = and i32 %4147, 1
  %4149 = icmp eq i32 %4148, 0
  br i1 %4149, label %4098, label %4054

4150:                                             ; preds = %4098, %3991, %3876, %3814, %3722, %3670, %3578
  %4151 = load i32, i32* %225, align 1
  %4152 = and i32 %4151, 1
  %4153 = icmp eq i32 %4152, 0
  br i1 %4153, label %4232, label %4208

4154:                                             ; preds = %4209, %4166
  %4155 = phi i64 [ %4229, %4209 ], [ %4167, %4166 ]
  br i1 %1724, label %4166, label %4156

4156:                                             ; preds = %4154
  %4157 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4211, i64 %94, double* elementtype(double) nonnull %78, i64 %4155)
  %4158 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4221, i64 %4220, double* elementtype(double) %4226, i64 %4155)
  br label %4159

4159:                                             ; preds = %4159, %4156
  %4160 = phi i64 [ %4227, %4156 ], [ %4164, %4159 ]
  %4161 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4210, i64 8, double* elementtype(double) nonnull %4157, i64 %4160)
  %4162 = load double, double* %4161, align 1
  %4163 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4218, i64 %4217, double* elementtype(double) %4158, i64 %4160)
  store double %4162, double* %4163, align 1
  %4164 = add nsw i64 %4160, 1
  %4165 = icmp eq i64 %4164, %4231
  br i1 %4165, label %4166, label %4159

4166:                                             ; preds = %4159, %4154
  %4167 = add nsw i64 %4155, 1
  %4168 = icmp eq i64 %4167, %4230
  br i1 %4168, label %4169, label %4154

4169:                                             ; preds = %4208, %4166
  br i1 %1864, label %4232, label %4170

4170:                                             ; preds = %4169
  %4171 = sext i32 %57 to i64
  %4172 = sext i32 %59 to i64
  %4173 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %49, i64 0, i32 0
  %4174 = load double*, double** %4173, align 1
  %4175 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %49, i64 0, i32 6, i64 0
  %4176 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4175, i64 0, i32 1
  %4177 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4176, i32 0)
  %4178 = load i64, i64* %4177, align 1
  %4179 = sext i32 %55 to i64
  %4180 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4176, i32 1)
  %4181 = load i64, i64* %4180, align 1
  %4182 = sext i32 %56 to i64
  %4183 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4176, i32 2)
  %4184 = load i64, i64* %4183, align 1
  %4185 = load i32, i32* %11, align 1
  %4186 = sext i32 %4185 to i64
  %4187 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4184, double* elementtype(double) %4174, i64 %4186)
  %4188 = sext i32 %114 to i64
  %4189 = add nsw i32 %150, 1
  %4190 = sext i32 %198 to i64
  %4191 = sext i32 %232 to i64
  %4192 = sext i32 %4189 to i64
  br label %4193

4193:                                             ; preds = %4205, %4170
  %4194 = phi i64 [ %4190, %4170 ], [ %4206, %4205 ]
  br i1 %2096, label %4205, label %4195

4195:                                             ; preds = %4193
  %4196 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4172, i64 %94, double* elementtype(double) nonnull %77, i64 %4194)
  %4197 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %4182, i64 %4181, double* elementtype(double) %4187, i64 %4194)
  br label %4198

4198:                                             ; preds = %4198, %4195
  %4199 = phi i64 [ %4188, %4195 ], [ %4203, %4198 ]
  %4200 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4171, i64 8, double* elementtype(double) nonnull %4196, i64 %4199)
  %4201 = load double, double* %4200, align 1
  %4202 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %4179, i64 %4178, double* elementtype(double) %4197, i64 %4199)
  store double %4201, double* %4202, align 1
  %4203 = add nsw i64 %4199, 1
  %4204 = icmp eq i64 %4203, %4192
  br i1 %4204, label %4205, label %4198

4205:                                             ; preds = %4198, %4193
  %4206 = add nsw i64 %4194, 1
  %4207 = icmp eq i64 %4206, %4191
  br i1 %4207, label %4232, label %4193

4208:                                             ; preds = %4150
  br i1 %1320, label %4169, label %4209

4209:                                             ; preds = %4208
  %4210 = sext i32 %57 to i64
  %4211 = sext i32 %59 to i64
  %4212 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %48, i64 0, i32 0
  %4213 = load double*, double** %4212, align 1
  %4214 = getelementptr inbounds %"QNCA_a0$double*$rank3$", %"QNCA_a0$double*$rank3$"* %48, i64 0, i32 6, i64 0
  %4215 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4214, i64 0, i32 1
  %4216 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4215, i32 0)
  %4217 = load i64, i64* %4216, align 1
  %4218 = sext i32 %55 to i64
  %4219 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4215, i32 1)
  %4220 = load i64, i64* %4219, align 1
  %4221 = sext i32 %56 to i64
  %4222 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %4215, i32 2)
  %4223 = load i64, i64* %4222, align 1
  %4224 = load i32, i32* %11, align 1
  %4225 = sext i32 %4224 to i64
  %4226 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %4223, double* elementtype(double) %4213, i64 %4225)
  %4227 = sext i32 %138 to i64
  %4228 = add nsw i32 %150, 1
  %4229 = sext i32 %174 to i64
  %4230 = sext i32 %232 to i64
  %4231 = sext i32 %4228 to i64
  br label %4154

4232:                                             ; preds = %4205, %4169, %4150, %847
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare %"MOD_PARAM$.btT_BOUNDS"* @"llvm.intel.subscript.p0s_MOD_PARAM$.btT_BOUNDSs.i64.i64.p0s_MOD_PARAM$.btT_BOUNDSs.i64"(i8, i64, i64, %"MOD_PARAM$.btT_BOUNDS"*, i64) #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
