// INTEL_COLLAB
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
