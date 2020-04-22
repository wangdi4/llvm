set(LLVM_TARGET_DEFINITIONS Attributes.td)
tablegen(LLVM Attributes.inc -gen-attrs)

set(LLVM_TARGET_DEFINITIONS Intrinsics.td)
tablegen(LLVM IntrinsicImpl.inc -gen-intrinsic-impl)
tablegen(LLVM IntrinsicEnums.inc -gen-intrinsic-enums)
tablegen(LLVM IntrinsicsAArch64.h -gen-intrinsic-enums -intrinsic-prefix=aarch64)
tablegen(LLVM IntrinsicsAMDGPU.h -gen-intrinsic-enums -intrinsic-prefix=amdgcn)
tablegen(LLVM IntrinsicsARM.h -gen-intrinsic-enums -intrinsic-prefix=arm)
tablegen(LLVM IntrinsicsBPF.h -gen-intrinsic-enums -intrinsic-prefix=bpf)
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_CSA
tablegen(LLVM IntrinsicsCSA.h -gen-intrinsic-enums -intrinsic-prefix=csa)
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
tablegen(LLVM IntrinsicsHexagon.h -gen-intrinsic-enums -intrinsic-prefix=hexagon)
tablegen(LLVM IntrinsicsMips.h -gen-intrinsic-enums -intrinsic-prefix=mips)
tablegen(LLVM IntrinsicsNVPTX.h -gen-intrinsic-enums -intrinsic-prefix=nvvm)
tablegen(LLVM IntrinsicsPowerPC.h -gen-intrinsic-enums -intrinsic-prefix=ppc)
tablegen(LLVM IntrinsicsR600.h -gen-intrinsic-enums -intrinsic-prefix=r600)
tablegen(LLVM IntrinsicsRISCV.h -gen-intrinsic-enums -intrinsic-prefix=riscv)
tablegen(LLVM IntrinsicsS390.h -gen-intrinsic-enums -intrinsic-prefix=s390)
tablegen(LLVM IntrinsicsWebAssembly.h -gen-intrinsic-enums -intrinsic-prefix=wasm)
tablegen(LLVM IntrinsicsX86.h -gen-intrinsic-enums -intrinsic-prefix=x86)
tablegen(LLVM IntrinsicsXCore.h -gen-intrinsic-enums -intrinsic-prefix=xcore)
add_public_tablegen_target(intrinsics_gen)

if (INTEL_COLLAB)
set(LLVM_TARGET_DEFINITIONS Directives.td)
tablegen(LLVM Directives.gen -gen-directives)
add_public_tablegen_target(directives_gen)
endif(INTEL_COLLAB)

if (INTEL_CUSTOMIZATION)
set(LLVM_TARGET_DEFINITIONS Intel_SVML.td) # TODO: VEC to COLLAB
tablegen(LLVM Intel_SVML.gen -gen-svml)
add_public_tablegen_target(intel_svml_gen)

set(LLVM_TARGET_DEFINITIONS Intel_Libmvec.td)
tablegen(LLVM Intel_Libmvec.gen -gen-libmvec)
add_public_tablegen_target(intel_libmvec_gen)
endif(INTEL_CUSTOMIZATION)
