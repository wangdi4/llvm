// RUN: clang-tidy %s -checks='-*,llvm-namespace-comment' -- | FileCheck %s

// INTEL_CUSTOMIZATION
// CMPLRLLVM-42770
// XFAIL: *
// end INTEL_CUSTOMIZATION

namespace i {
}
// CHECK: warning: namespace 'i' not terminated with a closing comment [llvm-namespace-comment]
