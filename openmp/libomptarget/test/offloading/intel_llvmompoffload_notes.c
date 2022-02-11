// REQUIRES: libomptarget-debug
// RUN: %libomptarget-compile-x86_64-pc-linux-gnu && env LIBOMPTARGET_DEBUG=1 LIBOMPTARGET_PLUGIN=x86_64 %libomptarget-run-x86_64-pc-linux-gnu 2>&1 | %fcheck-x86_64-pc-linux-gnu

// CHECK: TARGET ELF Common --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_VERSION with value: '1.0'
// CHECK: TARGET ELF Common --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_PRODUCER with value: 'Intel(R) oneAPI DPC++/C++ Compiler'
// CHECK: TARGET ELF Common --> LLVMOMPOFFLOAD ELF note NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION with value: '2022.{{[0-9]}}.0{{( [0-9a-fA-F]+)?}}'

int main() {
#pragma omp target
  ;

  return 0;
}
