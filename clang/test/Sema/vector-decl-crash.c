<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// INTEL_FEATURE_ISA_BF16_BASE
// UNSUPPORTED: intel_feature_isa_bf16_base
// end INTEL_FEATURE_ISA_BF16_BASE
// end INTEL_CUSTOMIZATION
// RUN: %clang_cc1 %s -fsyntax-only -verify -triple x86_64-unknown-unknown
=======
// RUN: %clang_cc1 %s -fsyntax-only -verify -triple riscv64-unknown-unknown
>>>>>>> e4888a37d36780872d685c68ef8b26b2e14d6d39

// GH50171
// This would previously crash when __bf16 was not a supported type.
__bf16 v64bf __attribute__((vector_size(128))); // expected-error {{__bf16 is not supported on this target}} \
                                                   expected-error {{vector size not an integral multiple of component size}}

