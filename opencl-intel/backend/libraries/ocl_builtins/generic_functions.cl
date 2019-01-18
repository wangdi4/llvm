__attribute__((overloadable))
cl_mem_fence_flags get_fence(void* genptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(const void* genptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__global void* globptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__global const void* globptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__local void* localptr) {
  return CLK_LOCAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__local const void* localptr) {
  return CLK_LOCAL_MEM_FENCE;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__private void* privptr) {
  return 0;
}

__attribute__((overloadable))
cl_mem_fence_flags get_fence(__private const void* privptr) {
  return 0;
}
