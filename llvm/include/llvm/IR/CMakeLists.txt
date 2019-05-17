set(LLVM_TARGET_DEFINITIONS Attributes.td)
tablegen(LLVM Attributes.inc -gen-attrs)

set(LLVM_TARGET_DEFINITIONS Intrinsics.td)
tablegen(LLVM IntrinsicEnums.inc -gen-intrinsic-enums)
tablegen(LLVM IntrinsicImpl.inc -gen-intrinsic-impl)
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
