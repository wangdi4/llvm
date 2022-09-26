// Repeat a given statement 200 times.
#define REPEAT(stmt) \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; \
  stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt; stmt;

__attribute__((noinline))
void foo(__global int *x, int s) {
  int gid = get_global_id(0);
  s += x[gid];
  barrier(CLK_LOCAL_MEM_FENCE);
  x[gid] = s;
  barrier(CLK_LOCAL_MEM_FENCE);

  int sglid = get_sub_group_local_id();
  // Insert many subgroup builtin calls that would require barrier buffer memory.
  REPEAT(x[gid] += sub_group_reduce_add(x[sglid]))
  REPEAT(x[gid] += sub_group_reduce_add(x[sglid]))
}

#define KERNEL(id) \
__attribute__((intel_reqd_sub_group_size(64))) \
__kernel void k##id (__global int *x) { \
  REPEAT(foo(x, 0)) \
  REPEAT(foo(x, 0)) \
}

// Define multiple kernels that all invoke foo()
// All call edges on `foo` contributes to the total barrier buffer size
// due to the limitation of barrier design [CMPLRLLVM-29148].
KERNEL(1)
KERNEL(2)
KERNEL(3)
KERNEL(4)
KERNEL(5)
KERNEL(6)
KERNEL(7)
KERNEL(8)
KERNEL(9)
KERNEL(10)
KERNEL(11)
KERNEL(12)
KERNEL(13)
KERNEL(14)
KERNEL(15)
KERNEL(16)
KERNEL(17)
KERNEL(18)
KERNEL(19)
KERNEL(20)
