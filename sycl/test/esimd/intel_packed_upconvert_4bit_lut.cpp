// INTEL_FEATURE_ESIMD_EMBARGO
// The test verifies 4-bit LUT upconversion on Xe3+.
// REQUIRES: intel_feature_esimd_embargo

// RUN: %clangxx -fsycl -fsycl-device-only -S %s -o %t.ll
// RUN: sycl-post-link -lower-esimd -split-esimd -S %t.ll -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <sycl/ext/intel/esimd.hpp>

using namespace sycl;
using namespace sycl::ext::intel;
using namespace sycl::ext::intel::esimd;

SYCL_EXTERNAL void fourtosixteen() SYCL_ESIMD_FUNCTION {
  // CHECK: [[RD0:%.*]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> zeroinitializer, i32 4, i32 1, i32 0, i16 0, i32 0)
  // CHECK: = call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i8(<16 x i32> zeroinitializer, <16 x i8> [[RD0]])
  // CHECK: [[RD1:%.*]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> zeroinitializer, i32 4, i32 1, i32 0, i16 1, i32 0)
  // CHECK: = call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i8(<16 x i32> zeroinitializer, <16 x i8> [[RD1]])
  simd<uint32_t, 16> lut = 0;

  simd<uint32_t, 16> src(0, 0);

  simd<uint8_t, 16 * 4> src_byte = src.bit_cast_view<uint8_t>();

  simd<uint32_t, 16> res =
      experimental::esimd::packed_4bit_upconvert_lut<0>(lut, src_byte);

  res = experimental::esimd::packed_4bit_upconvert_lut<1>(lut, src_byte);
}

SYCL_EXTERNAL void fourtoeight() SYCL_ESIMD_FUNCTION {
  // CHECK: [[RD2:%.*]] = call <16 x i16> @llvm.genx.rdregioni.v16i16.v32i16.i16(<32 x i16> zeroinitializer, i32 2, i32 1, i32 0, i16 0, i32 0)
  // CHECK: = call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i16(<16 x i32> zeroinitializer, <16 x i16> [[RD2]])
  // CHECK: [[RD3:%.*]] = call <16 x i16> @llvm.genx.rdregioni.v16i16.v32i16.i16(<32 x i16> zeroinitializer, i32 2, i32 1, i32 0, i16 2, i32 0)
  // CHECK: = call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i16(<16 x i32> zeroinitializer, <16 x i16> [[RD3]])
  simd<uint32_t, 16> lut = 0;

  simd<uint32_t, 16> src(0, 0);

  simd<uint16_t, 16 * 2> src_byte = src.bit_cast_view<uint16_t>();

  simd<uint32_t, 16> res =
      experimental::esimd::packed_4bit_upconvert_lut<0>(lut, src_byte);
  res = experimental::esimd::packed_4bit_upconvert_lut<1>(lut, src_byte);
}

// end INTEL_FEATURE_ESIMD_EMBARGO
