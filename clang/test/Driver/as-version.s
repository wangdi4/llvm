// Test version information.

// UNSUPPORTED: target={{.*}}-zos{{.*}}, target={{.*}}-aix{{.*}}
// RUN: %clang -Wa,--version -c -fintegrated-as %s -o /dev/null \
// RUN:   | FileCheck --check-prefix=IAS %s
// INTEL_CUSTOMIZATION
// IAS: Intel(R) oneAPI DPC++/C++ Compiler
// end INTEL_CUSTOMIZATION
