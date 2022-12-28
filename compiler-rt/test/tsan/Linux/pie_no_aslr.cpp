// RUN: %clang_tsan %s -pie -fPIE -o %t && %run setarch x86_64 -R %t
// REQUIRES: x86_64-target-arch
//
// INTEL_CUSTOMIZATION
// We disabled this lit since tsan does not support linux with 5-level paging
// and will fail on icelake platform.
// UNSUPPORTED: true
// end INTEL_CUSTOMIZATION

int main() {
  return 0;
}
