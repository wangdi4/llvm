// INTEL_FEATURE_SW_ADVANCED
// 21914: Make sure that simplifycfg is run after DTrans+multiversioning,
// to clean up duplicate code.

// REQUIRES: asserts, intel_feature_sw_advanced
// Windows LTO requires -fuse-ld=lld and the lld linker to be built.
// UNSUPPORTED: win32, windows-gnu, windows-msvc

// RUN: %clang -qopt-mem-layout-trans=4 -xCORE-AVX2 -flto -mllvm -debug-pass=Arguments -flegacy-pass-manager %s 2>&1 | FileCheck %s

// If we didn't build the LTO plugin, there will be an error message with the
// word "plugin" or "linker" in it, which differs slightly for
// different platforms.
// CHECK: {{linker|plugin|-multiversioning -simplifycfg}}

int main(void) {
  return 0;
}
// end INTEL_FEATURE_SW_ADVANCED
