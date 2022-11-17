; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -passes='lto<O3>' -loopopt -debug-pass-manager -enable-npm-dtrans < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-LTO-O3"

; Verify that loopopt passes guarded under dtrans, are properly skipped/run for functions
; NoLoopOptFunc() and LightLoopOptFunc() but run for FullLoopOptFunc()

; CHECK-LTO-O3: Skipping pass HIRRowWiseMVPass on NoLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-LTO-O3: Skipping pass HIRRowWiseMVPass on LightLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-LTO-O3: Running pass: HIRRowWiseMVPass on FullLoopOptFunc


define void @NoLoopOptFunc() {
  ret void
}

define void @LightLoopOptFunc() "loopopt-pipeline"="light" {
  ret void
}

define void @FullLoopOptFunc() "loopopt-pipeline"="full" {
  ret void
}

; end INTEL_FEATURE_SW_DTRANS

