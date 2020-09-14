// 21914: Make sure that simplifycfg is run after DTrans+multiversioning,
// to clean up duplicate code.

// REQUIRES: asserts
// Multiversioning doesn't turn on for Windows with below options.
// XFAIL: win32, windows-gnu, windows-msvc

// RUN: %clang -qopt-mem-layout-trans=4 -xCORE-AVX2 -flto -mllvm -debug-pass=Arguments -fno-experimental-new-pass-manager %s 2>&1 | FileCheck %s

// If we didn't build the LTO plugin, there will be an error message with the
// word "plugin" in it, which differs slightly for different platforms.
// CHECK: {{plugin|-multiversioning -simplifycfg}}

int main(void) {
  return 0;
}
