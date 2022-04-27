// INTEL_FEATURE_ISA_PREFETCHI
// REQUIRES: intel_feature_isa_prefetchi
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: prefetchit0 -485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xbc,0x82,0x10,0xe3,0x0f,0xe3]
prefetchit0 -485498096(%edx,%eax,4)

// CHECK: prefetchit0 485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xbc,0x82,0xf0,0x1c,0xf0,0x1c]
prefetchit0 485498096(%edx,%eax,4)

// CHECK: prefetchit0 485498096(%edx)
// CHECK: encoding: [0x0f,0x18,0xba,0xf0,0x1c,0xf0,0x1c]
prefetchit0 485498096(%edx)

// CHECK: prefetchit0 485498096
// CHECK: encoding: [0x0f,0x18,0x3d,0xf0,0x1c,0xf0,0x1c]
prefetchit0 485498096

// CHECK: prefetchit0 64(%edx,%eax)
// CHECK: encoding: [0x0f,0x18,0x7c,0x02,0x40]
prefetchit0 64(%edx,%eax)

// CHECK: prefetchit0 (%edx)
// CHECK: encoding: [0x0f,0x18,0x3a]
prefetchit0 (%edx)

// CHECK: prefetchit1 -485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xb4,0x82,0x10,0xe3,0x0f,0xe3]
prefetchit1 -485498096(%edx,%eax,4)

// CHECK: prefetchit1 485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xb4,0x82,0xf0,0x1c,0xf0,0x1c]
prefetchit1 485498096(%edx,%eax,4)

// CHECK: prefetchit1 485498096(%edx)
// CHECK: encoding: [0x0f,0x18,0xb2,0xf0,0x1c,0xf0,0x1c]
prefetchit1 485498096(%edx)

// CHECK: prefetchit1 485498096
// CHECK: encoding: [0x0f,0x18,0x35,0xf0,0x1c,0xf0,0x1c]
prefetchit1 485498096

// CHECK: prefetchit1 64(%edx,%eax)
// CHECK: encoding: [0x0f,0x18,0x74,0x02,0x40]
prefetchit1 64(%edx,%eax)

// CHECK: prefetchit1 (%edx)
// CHECK: encoding: [0x0f,0x18,0x32]
prefetchit1 (%edx)
// end INTEL_FEATURE_ISA_PREFETCHI
