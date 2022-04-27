// INTEL_FEATURE_ISA_PREFETCHST2
// REQUIRES: intel_feature_isa_prefetchst2
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: prefetchst2 485498096
// CHECK: encoding: [0x0f,0x18,0x24,0x25,0xf0,0x1c,0xf0,0x1c]
prefetchst2 485498096

// CHECK: prefetchst2 64(%rdx)
// CHECK: encoding: [0x0f,0x18,0x62,0x40]
prefetchst2 64(%rdx)

// CHECK: prefetchst2 64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x64,0x82,0x40]
prefetchst2 64(%rdx,%rax,4)

// CHECK: prefetchst2 -64(%rdx,%rax,4)
// CHECK: encoding: [0x0f,0x18,0x64,0x82,0xc0]
prefetchst2 -64(%rdx,%rax,4)

// CHECK: prefetchst2 64(%rdx,%rax)
// CHECK: encoding: [0x0f,0x18,0x64,0x02,0x40]
prefetchst2 64(%rdx,%rax)

// CHECK: prefetchst2 (%rdx)
// CHECK: encoding: [0x0f,0x18,0x22]
prefetchst2 (%rdx)
// end INTEL_FEATURE_ISA_PREFETCHST2
