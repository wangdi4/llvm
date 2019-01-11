// REQUIRES: intel_feature_isa_amx
// RUN: llvm-mc -triple x86_64-unknown-unknown -show-encoding %s > %t 2> %t.err
// RUN: FileCheck < %t %s
// some AMX instruction must use SIB.

// CHECK: sttilecfg       (%rsi)
// CHECK: encoding: [0xc4,0xe2,0x79,0x49,0x06]
sttilecfg       (%rsi)

// CHECK: sttilecfg       (%rcx,%rdx,2)
// CHECK: encoding: [0xc4,0xe2,0x79,0x49,0x04,0x51]
sttilecfg       (%rcx,%rdx,2)

// CHECK: ldtilecfg       (%rdi)
// CHECK: encoding: [0xc4,0xe2,0x78,0x49,0x07]
ldtilecfg       (%rdi)

// CHECK: ldtilecfg       (%rcx,%rdx,2)
// CHECK: encoding: [0xc4,0xe2,0x78,0x49,0x04,0x51]
ldtilecfg       (%rcx,%rdx,2)

// CHECK: tileloadd     (%rdi), %tmm1
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0x0c,0x27]
tileloadd       (%rdi), %tmm1

// CHECK: tileloadd     (%rsi), %tmm7
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0x3c,0x26]
tileloadd       (%rsi), %tmm7

// CHECK: tileloadd       foo, %tmm5
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0x2c,0x25,A,A,A,A]
tileloadd       foo, %tmm5

// CHECK: tileloadd       (%rcx,%riz), %tmm5
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0x2c,0x21]
tileloadd       (%rcx,%riz,1), %tmm5

// CHECK: tileloadd       (%ecx,%eiz), %tmm5
// CHECK: encoding: [0x67,0xc4,0xe2,0x7b,0x4b,0x2c,0x21]
tileloadd       (%ecx,%eiz,1), %tmm5

// CHECK: tileloadd       (%rcx,%rdx), %tmm5
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0x2c,0x11]
tileloadd       (%rcx,%rdx,1), %tmm5

// CHECK: tileloadd        (%ecx,%edx,2), %tmm1
// CHECK: encoding: [0x67,0xc4,0xe2,0x7b,0x4b,0x0c,0x51]
tileloadd       (%ecx,%edx,2), %tmm1

// CHECK: tileloaddt1   (%rsi), %tmm7
// CHECK: encoding: [0xc4,0xe2,0x79,0x4b,0x3c,0x26]
tileloaddt1     (%rsi), %tmm7

// CHECK: tileloaddt1     foo, %tmm5
// CHECK: encoding: [0xc4,0xe2,0x79,0x4b,0x2c,0x25,A,A,A,A]
tileloaddt1     foo, %tmm5

// CHECK: tileloaddt1     (%rcx,%riz), %tmm5
// CHECK: encoding: [0xc4,0xe2,0x79,0x4b,0x2c,0x21]
tileloaddt1     (%rcx,%riz,1), %tmm5

// CHECK: tileloaddt1     (%ecx,%eiz), %tmm5
// CHECK: encoding: [0x67,0xc4,0xe2,0x79,0x4b,0x2c,0x21]
tileloaddt1     (%ecx,%eiz,1), %tmm5

// CHECK: tileloaddt1     (%rcx,%rdx), %tmm5
// CHECK: encoding: [0xc4,0xe2,0x79,0x4b,0x2c,0x11]
tileloaddt1     (%rcx,%rdx,1), %tmm5

// CHECK: tileloaddt1     (%ecx,%edx,2), %tmm1
// CHECK: encoding: [0x67,0xc4,0xe2,0x79,0x4b,0x0c,0x51]
tileloaddt1     (%ecx,%edx,2), %tmm1

// CHECK: tileloaddt1     (%rcx,%riz,2), %tmm1
// CHECK: encoding: [0xc4,0xe2,0x79,0x4b,0x0c,0x61]
tileloaddt1     (%rcx,%riz,2), %tmm1

// CHECK: tilerelease
// CHECK: encoding: [0xc4,0xe2,0x78,0x49,0xc0]
tilerelease

// CHECK: tilestored    %tmm4, (%rsi)
// CHECK: encoding: [0xc4,0xe2,0x7a,0x4b,0x24,0x26]
tilestored      %tmm4, (%rsi)

// CHECK: tilestored      %tmm5, (%rcx,%riz)
// CHECK: encoding: [0xc4,0xe2,0x7a,0x4b,0x2c,0x21]
tilestored      %tmm5, (%rcx,%riz,1)

// CHECK: tilestored      %tmm5, (%ecx,%eiz)
// CHECK: encoding: [0x67,0xc4,0xe2,0x7a,0x4b,0x2c,0x21]
tilestored      %tmm5, (%ecx,%eiz,1)

// CHECK: tilestored      %tmm5, (%rcx,%rdx)
// CHECK: encoding: [0xc4,0xe2,0x7a,0x4b,0x2c,0x11]
tilestored      %tmm5, (%rcx,%rdx,1)

// CHECK: tilestored      %tmm1, (%ecx,%edx,2)
// CHECK: encoding: [0x67,0xc4,0xe2,0x7a,0x4b,0x0c,0x51]
tilestored      %tmm1, (%ecx,%edx,2)

// CHECK: tilezero      %tmm0
// CHECK: encoding: [0xc4,0xe2,0x7b,0x49,0xc0]
tilezero        %tmm0

// CHECK: tilezero      %tmm5
// CHECK: encoding: [0xc4,0xe2,0x7b,0x49,0xe8]
tilezero        %tmm5

// CHECK: tilezero      %tmm7
// CHECK: encoding: [0xc4,0xe2,0x7b,0x49,0xf8]
tilezero        %tmm7

// CHECK: tdpbf16ps     %tmm5, %tmm4, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x52,0x5c,0xdc]
tdpbf16ps       %tmm5, %tmm4, %tmm3

// CHECK: tdpbssd       %tmm3, %tmm2, %tmm1
// CHECK: encoding: [0xc4,0xe2,0x63,0x5e,0xca]
tdpbssd %tmm3, %tmm2, %tmm1

// CHECK: tdpbsud       %tmm3, %tmm2, %tmm1
// CHECK: encoding: [0xc4,0xe2,0x62,0x5e,0xca]
tdpbsud %tmm3, %tmm2, %tmm1

// CHECK: tdpbusd       %tmm3, %tmm2, %tmm1
// CHECK: encoding: [0xc4,0xe2,0x61,0x5e,0xca]
tdpbusd %tmm3, %tmm2, %tmm1

// CHECK: tdpbuud       %tmm3, %tmm2, %tmm1
// CHECK: encoding: [0xc4,0xe2,0x60,0x5e,0xca]
tdpbuud %tmm3, %tmm2, %tmm1
