// RUN: %clangxx %s -pie -fPIE -o %t && %run setarch x86_64 -R %t
// REQUIRES: x86_64-target-arch

// INTEL_CUSTOMIZATION
// JIRA: CMPLRLLVM-47138
// Tsan not support 5-level paging
// UNSUPPORTED: tsan && la57
// end INTEL_CUSTOMIZATION

int main() { return 0; }
