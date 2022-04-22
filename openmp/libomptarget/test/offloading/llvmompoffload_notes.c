// INTEL_COLLAB
// INTEL_CUSTOMIZATION
// xmain runs intel_llvmompoffload_notes.c instead.
// XFAIL: *
// We have to mark it as unsupported on Windows, since the below command
// is a no-op on Windows and produces 'pass', which conflicts with the 'xfail'.
// Introduction of "*-newDriver" target makes all commands ignore the actual
// commands if they are not "generic" target commands, so we mark "*-newDriver"
// target unsupported.
// UNSUPPORTED: system-windows, x86_64-pc-linux-gnu-newDriver, x86_64-pc-linux-gnu-oldDriver
// end INTEL_CUSTOMIZATION
// RUN: %libomptarget-compile-x86_64-pc-linux-gnu && env LIBOMPTARGET_DEBUG=1 %libomptarget-run-x86_64-pc-linux-gnu 2>&1 | %fcheck-x86_64-pc-linux-gnu

// CHECK: TARGET Common ELF --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_VERSION with value: '1.0'
// CHECK: TARGET Common ELF --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_PRODUCER with value: 'LLVM'
// CHECK: TARGET Common ELF --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION with value:

int main() {
#pragma omp target
  ;

  return 0;
}
// end INTEL_COLLAB
