; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-VPO1"
; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-VPO2"
; RUN: opt -passes='default<O3>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-O3"
; RUN: opt -passes='lto<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-LTO"
; RUN: opt -passes='lto<O3>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-LTO-O3"


; Verify that full loopopt passes, enabled under -O2, are properly skipped for functions
; NoLoopOptFunc() and LightLoopOptFunc() but run for FullLoopOptFunc()

; CHECK: Skipping pass HIRAosToSoaPass on NoLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK: Skipping pass HIRAosToSoaPass on LightLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK: Running pass: HIRAosToSoaPass on FullLoopOptFunc


; Verify that Light LoopOpt limited vpo::VPlanDriverHIRPass(true) pass, is properly skipped for functions
; NoLoopOptFunc() and FullLoopOptFunc() but run on LightLoopOptFunc()

; CHECK-VPO1: Skipping pass vpo::VPlanDriverHIRPass on NoLoopOptFunc due to incompatible Light LoopOpt limiter on pass
; CHECK-VPO1: Running pass: vpo::VPlanDriverHIRPass on LightLoopOptFunc
; CHECK-VPO1: Skipping pass vpo::VPlanDriverHIRPass on FullLoopOptFunc due to incompatible Light LoopOpt limiter on pass


; Verify that Full LoopOpt limited vpo::VPlanDriverHIRPass(false) pass, is properly skipped for functions
; NoLoopOptFunc() and LightLoopOptFunc() but run on FullLoopOptFunc()

; CHECK-VPO2: Skipping pass vpo::VPlanDriverHIRPass on NoLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-VPO2: Skipping pass vpo::VPlanDriverHIRPass on LightLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-VPO2: Running pass: vpo::VPlanDriverHIRPass on FullLoopOptFunc


; Verify that full loopopt passes, enabled only under -O3, are properly skipped for functions
; NoLoopOptFunc() and LightLoopOptFunc() but run for FullLoopOptFunc()

; CHECK-O3: Skipping pass HIRLoopConcatenationPass on NoLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-O3: Skipping pass HIRLoopConcatenationPass on LightLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-O3: Running pass: HIRLoopConcatenationPass on FullLoopOptFunc


; Verify that full loopopt passes, enabled only under lto<O2>, are properly skipped for functions
; NoLoopOptFunc() and LightLoopOptFunc() but run for FullLoopOptFunc()

; CHECK-LTO: Skipping pass ADCEPass on NoLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-LTO: Skipping pass ADCEPass on LightLoopOptFunc due to incompatible Full LoopOpt limiter on pass
; CHECK-LTO: Running pass: ADCEPass on FullLoopOptFunc


; Verify that full loopopt passes, enabled only under lto<O3>, are properly skipped for functions
; NoLoopOptFunc() LightLoopOptFunc() but run for FullLoopOptFunc()

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
