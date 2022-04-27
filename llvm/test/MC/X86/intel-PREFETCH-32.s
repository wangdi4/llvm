// INTEL_FEATURE_ISA_PREFETCHST2
// REQUIRES: intel_feature_isa_prefetchst2
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: prefetchst2 -485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xa4,0x82,0x10,0xe3,0x0f,0xe3]
prefetchst2 -485498096(%edx,%eax,4)

// CHECK: prefetchst2 485498096(%edx,%eax,4)
// CHECK: encoding: [0x0f,0x18,0xa4,0x82,0xf0,0x1c,0xf0,0x1c]
prefetchst2 485498096(%edx,%eax,4)

// CHECK: prefetchst2 485498096(%edx)
// CHECK: encoding: [0x0f,0x18,0xa2,0xf0,0x1c,0xf0,0x1c]
prefetchst2 485498096(%edx)

// CHECK: prefetchst2 485498096
// CHECK: encoding: [0x0f,0x18,0x25,0xf0,0x1c,0xf0,0x1c]
prefetchst2 485498096

// CHECK: prefetchst2 64(%edx,%eax)
// CHECK: encoding: [0x0f,0x18,0x64,0x02,0x40]
prefetchst2 64(%edx,%eax)

// CHECK: prefetchst2 (%edx)
// CHECK: encoding: [0x0f,0x18,0x22]
prefetchst2 (%edx)
// end INTEL_FEATURE_ISA_PREFETCHST2
