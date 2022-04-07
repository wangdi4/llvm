use_rtti( FALSE )
set( TARGET_NAME LLVMVectorizer)

set(VECTORIZATION_INFO_GEN ${OCL_BINARY_DIR}/include/VectInfo.gen)
file(MAKE_DIRECTORY ${OCL_BINARY_DIR}/include)
add_custom_command(
  OUTPUT ${VECTORIZATION_INFO_GEN}
  COMMAND ocl-tblgen -gen-vectorization-info
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${OCL_BINARY_DIR}/include/VectInfo.gen
  DEPENDS ocl-tblgen
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v1.2.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0WG.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/sub_group_builtin.td
  COMMENT "Generating built-in vectorization info ..."
  VERBATIM
)

add_custom_target(VectorizationInfoGen ALL
  DEPENDS ${VECTORIZATION_INFO_GEN}
  VERBATIM
)

include_directories(BEFORE "Heuristic")
include_directories(BEFORE "PostDominanceFrontier")
include_directories(BEFORE "Utils")
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
  ${OCL_BINARY_DIR}/include
)

set(TARGET_EXTRA_SOURCE_FILES
  Resolver/AVX512Resolver.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES
  Resolver/AVX512Resolver.h
)

set(TARGET_INCLUDE_FILES
  ChooseVectorizationDimension/ChooseVectorizationDimension.h
  Heuristic/InstCounter.h
  IRPrinter/IRPrinter.h
  LoopHandler/CLBuiltinLICM.h
  LoopHandler/CLStreamSampler.h
  OCLBranchProbability/OCLBranchProbability.h
  OCLVecClone/OCLVecClone.h
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
  Utils/FakeExtractInsert.h
  Utils/SimplifyGEP.h
  Utils/VecConfig.h
  Utils/VectorizerCommon.h
  Utils/VectorizerCore.h
  VectorKernelDiscard/VectorKernelDiscard.h
  VolcanoWrapper/Main.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set(TARGET_SOURCE_FILES
  ChooseVectorizationDimension/ChooseVectorizationDimension.cpp
  Heuristic/InstCounter.cpp
  IRPrinter/IRPrinter.cpp
  LoopHandler/CLBuiltinLICM.cpp
  LoopHandler/CLStreamSampler.cpp
  OCLBranchProbability/OCLBranchProbability.cpp
  OCLVecClone/OCLVecClone.cpp
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
  Utils/SimplifyGEP.cpp
  Utils/VectorizerCore.cpp
  Utils/VectorizerCommon.cpp
  VectorKernelDiscard/VectorKernelDiscard.cpp
  VolcanoWrapper/Main.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} LoopUtils OclBackendUtils VectorizationInfoGen)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

if (NOT OPENCL_INTREE_BUILD)
  set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${TARGET_NAME} )
endif()
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
