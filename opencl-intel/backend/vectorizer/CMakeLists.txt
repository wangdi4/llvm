use_rtti( FALSE )
set( TARGET_NAME LLVMVectorizer)

add_custom_command(
  OUTPUT VectInfo.gen
  COMMAND ocl-tblgen -gen-vectorization-info
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_BINARY_DIR}/VectInfo.gen
  DEPENDS ocl-tblgen
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v1.2.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0WG.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/sub_group_builtin.td
  COMMENT "Generating built-in vectorization info ..."
  VERBATIM
)

set(VECTORIZATION_INFO_GEN VectInfo.gen)

include_directories(BEFORE "Heuristic")
include_directories(BEFORE "PostDominanceFrontier")
include_directories(BEFORE "SoaAllocaAnalysis")
include_directories(BEFORE "Utils")
include_directories(BEFORE "WIAnalysis")
include_directories(BEFORE "Predication")
include_directories(BEFORE "OCLBranchProbability")
include_directories(BEFORE "ChooseVectorizationDimension")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/utils/LoopUtils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
  ${BACKEND_ROOT_DIR}/metadata_api
  ${CMAKE_BINARY_DIR}/include
)

set(TARGET_EXTRA_SOURCE_FILES
  Resolver/AVX512Resolver.cpp
  Utils/IRInjection.cpp
  Utils/Namer.cpp
  Utils/PrintfAdder.cpp
  Renderscript/RenderscriptDriver.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES
  Resolver/AVX512Resolver.h
  Utils/Namer.h
  Utils/PrintfAdder.h
  Renderscript/RenderscriptDriver.h
)

set(TARGET_INCLUDE_FILES
  Heuristic/InstCounter.h
  IRPrinter/IRPrinter.h
  LoopHandler/CLBuiltinLICM.h
  LoopHandler/CLStreamSampler.h
  LoopHandler/CLWGBoundDecoder.h
  LoopHandler/CLWGLoopBoundaries.h
  LoopHandler/CLWGLoopCreator.h
  LoopHandler/KernelAnalysis.h
  LoopHandler/LoopPeeling.h
  LoopHandler/LoopStridedCodeMotion.h
  LoopHandler/LoopWIAnalysis.h
  OCLVecClone/OCLPrepareKernelForVecClone.h
  OCLVecClone/OCLVecClone.h
  OCLVecClone/OCLPostVect.h
  OCLVPOCheckVF/OCLVPOCheckVF.h
  Packetizer/Packetizer.h
  PostDominanceFrontier/PostDominanceFrontier.h
  Predication/Linearizer.h
  Predication/PhiCanon.h
  Predication/Predicator.h
  Predication/Specializer.h
  PreVectorization/OCLBuiltinPreVectorizationPass.h
  Resolver/Resolver.h
  Resolver/SpecialCaseBuiltinResolver.h
  Scalarizer/Scalarize.h
  SoaAllocaAnalysis/SoaAllocaAnalysis.h
  Utils/FakeExtractInsert.h
  Utils/SimplifyGEP.h
  Utils/VecConfig.h
  Utils/VectorizerCommon.h
  Utils/VectorizerCore.h
  VolcanoWrapper/Main.h
  WIAnalysis/WIAnalysis.h
  OCLBranchProbability/OCLBranchProbability.h
  ChooseVectorizationDimension/ChooseVectorizationDimension.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set(TARGET_SOURCE_FILES
  Heuristic/InstCounter.cpp
  IRPrinter/IRPrinter.cpp
  LoopHandler/CLBuiltinLICM.cpp
  LoopHandler/CLStreamSampler.cpp
  LoopHandler/CLWGBoundDecoder.cpp
  LoopHandler/CLWGLoopBoundaries.cpp
  LoopHandler/CLWGLoopCreator.cpp
  LoopHandler/KernelAnalysis.cpp
  LoopHandler/LoopPeeling.cpp
  LoopHandler/LoopStridedCodeMotion.cpp
  LoopHandler/LoopWIAnalysis.cpp
  OCLVecClone/OCLPrepareKernelForVecClone.cpp
  OCLVecClone/OCLVecClone.cpp
  OCLVecClone/OCLPostVect.cpp
  OCLVPOCheckVF/OCLVPOCheckVF.cpp
  Packetizer/Packetizer.cpp
  Packetizer/PacketizerInstData.cpp
  PostDominanceFrontier/PostDominanceFrontier.cpp
  Predication/Linearizer.cpp
  Predication/PhiCanon.cpp
  Predication/Predicator.cpp
  Predication/Specializer.cpp
  PreVectorization/OCLBuiltinPreVectorizationPass.cpp
  Resolver/Resolver.cpp
  Resolver/SpecialCaseBuiltinResolver.cpp
  Scalarizer/PreScalarizeScan.cpp
  Scalarizer/Scalarize.cpp
  SoaAllocaAnalysis/SoaAllocaAnalysis.cpp
  Utils/SimplifyGEP.cpp
  Utils/VectorizerCore.cpp
  VolcanoWrapper/Main.cpp
  WIAnalysis/WIAnalysis.cpp
  OCLBranchProbability/OCLBranchProbability.cpp
  ChooseVectorizationDimension/ChooseVectorizationDimension.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
  ${VECTORIZATION_INFO_GEN}
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} LoopUtils OclBackendUtils)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

if (NOT OPENCL_INTREE_BUILD)
  set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${TARGET_NAME} )
endif()
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
