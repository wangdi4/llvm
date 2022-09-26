# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
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
if (";${LLVM_TARGETS_TO_BUILD};" MATCHES ";CSA;")
tablegen(LLVM IntrinsicsCSA.h -gen-intrinsic-enums -intrinsic-prefix=csa)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
tablegen(LLVM IntrinsicsDirectX.h -gen-intrinsic-enums -intrinsic-prefix=dx)
tablegen(LLVM IntrinsicsHexagon.h -gen-intrinsic-enums -intrinsic-prefix=hexagon)
tablegen(LLVM IntrinsicsLoongArch.h -gen-intrinsic-enums -intrinsic-prefix=loongarch)
tablegen(LLVM IntrinsicsMips.h -gen-intrinsic-enums -intrinsic-prefix=mips)
tablegen(LLVM IntrinsicsNVPTX.h -gen-intrinsic-enums -intrinsic-prefix=nvvm)
tablegen(LLVM IntrinsicsPowerPC.h -gen-intrinsic-enums -intrinsic-prefix=ppc)
tablegen(LLVM IntrinsicsR600.h -gen-intrinsic-enums -intrinsic-prefix=r600)
tablegen(LLVM IntrinsicsRISCV.h -gen-intrinsic-enums -intrinsic-prefix=riscv)
tablegen(LLVM IntrinsicsSPIRV.h -gen-intrinsic-enums -intrinsic-prefix=spv)
tablegen(LLVM IntrinsicsS390.h -gen-intrinsic-enums -intrinsic-prefix=s390)
tablegen(LLVM IntrinsicsWebAssembly.h -gen-intrinsic-enums -intrinsic-prefix=wasm)
tablegen(LLVM IntrinsicsX86.h -gen-intrinsic-enums -intrinsic-prefix=x86)
tablegen(LLVM IntrinsicsXCore.h -gen-intrinsic-enums -intrinsic-prefix=xcore)
tablegen(LLVM IntrinsicsVE.h -gen-intrinsic-enums -intrinsic-prefix=ve)
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
