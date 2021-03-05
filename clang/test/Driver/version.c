// RUN: %clang --version 2>&1 | FileCheck %s
#if INTEL_CUSTOMIZATION
CHECK: Intel(R) oneAPI DPC++/C++ Compiler
#endif // INTEL_CUSTOMIZATION

// Don't add llvm::TargetRegistry::printRegisteredTargetsForVersion()
// to --version output, see D79210 and D33900.
CHECK-NOT: Registered Targets:
