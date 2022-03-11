// INTEL_FEATURE_ISA_PREFETCHI
// REQUIRES: intel_feature_isa_prefetchi
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: prefetchit0 485498096
// CHECK: encoding: [0x0f,0x18,0x3c,0x25,0xf0,0x1c,0xf0,0x1c]
prefetchit0 485498096

// CHECK: prefetchit0 64(%rdx)
// CHECK: encoding: [0x0f,0x18,0x7a,0x40]
prefetchit0 64(%rdx)

// CHECK: prefetchit0 64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x7c,0x82,0x40]
prefetchit0 64(%rdx,%rax,4)

// CHECK: prefetchit0 -64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x7c,0x82,0xc0]
prefetchit0 -64(%rdx,%rax,4)

// CHECK: prefetchit0 64(%rdx,%rax)
// CHECK: encoding: [0x0f,0x18,0x7c,0x02,0x40]
prefetchit0 64(%rdx,%rax)

// CHECK: prefetchit0 (%rdx)
// CHECK: encoding: [0x0f,0x18,0x3a]
prefetchit0 (%rdx)

// CHECK: prefetchit1 485498096
// CHECK: encoding: [0x0f,0x18,0x34,0x25,0xf0,0x1c,0xf0,0x1c]
prefetchit1 485498096

// CHECK: prefetchit1 64(%rdx)
// CHECK: encoding: [0x0f,0x18,0x72,0x40]
prefetchit1 64(%rdx)

// CHECK: prefetchit1 64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x74,0x82,0x40]
prefetchit1 64(%rdx,%rax,4)

// CHECK: prefetchit1 -64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x74,0x82,0xc0]
prefetchit1 -64(%rdx,%rax,4)

// CHECK: prefetchit1 64(%rdx,%rax)
// CHECK: encoding: [0x0f,0x18,0x74,0x02,0x40]
prefetchit1 64(%rdx,%rax)

// CHECK: prefetchit1 (%rdx)
// CHECK: encoding: [0x0f,0x18,0x32]
prefetchit1 (%rdx)
// end INTEL_FEATURE_ISA_PREFETCHI
