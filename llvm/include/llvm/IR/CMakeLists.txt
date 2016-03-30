set(LLVM_TARGET_DEFINITIONS Attributes.td)
tablegen(LLVM Attributes.inc -gen-attrs)

set(LLVM_TARGET_DEFINITIONS Intrinsics.td)
tablegen(LLVM Intrinsics.gen -gen-intrinsic)
add_public_tablegen_target(intrinsics_gen)

if (INTEL_CUSTOMIZATION)
set(LLVM_TARGET_DEFINITIONS Intel_Directives.td)
tablegen(LLVM Intel_Directives.gen -gen-directives)
add_public_tablegen_target(intel_directives_gen)
endif() # INTEL_CUSTOMIZATION
