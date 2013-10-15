use_rtti( FALSE )
set( TARGET_NAME LLVMVectorizer)

include_directories(BEFORE "Driver/OpenCL")
include_directories(BEFORE "Driver")
include_directories(BEFORE "Heuristic")
include_directories(BEFORE "PostDominanceFrontier")
include_directories(BEFORE "SoaAllocaAnalysis")
include_directories(BEFORE "Utils")
include_directories(BEFORE "WIAnalysis")
include_directories(BEFORE "OCLBranchProbability")

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/arch_headers
  ${CMAKE_SOURCE_DIR}/backend/reflection_module
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/passes
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/metadata_api/gen
  ${CMAKE_SOURCE_DIR}/cl_api
)

if(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  AppleWrapper/AppleWIDepPrePacketizationPass.cpp
  Driver/Apple/AppleOCLRuntime.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES 
  AppleWrapper/AppleWIDepPrePacketizationPass.h
  Driver/Apple/AppleOCLRuntime.h
)
else(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  Driver/Volcano/VolcanoOpenclRuntime.cpp
  Resolver/MICResolver.cpp
  Utils/IRInjection.cpp
  Utils/Namer.cpp
  Utils/PrintfAdder.cpp
#These files are needed only for building opt
  AppleWrapper/AppleWIDepPrePacketizationPass.cpp
  Driver/Apple/AppleOCLRuntime.cpp
#These files are needed only for building opt (to run dx LIT tests)
  Driver/DX/DXData.cpp
  Driver/DX/DXRuntime.cpp
  Driver/DX/DXWrapperParamsCheck.cpp
  Driver/Functions.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES 
  Driver/Volcano/VolcanoOpenclRuntime.h
  Resolver/MICResolver.h
  Utils/Namer.h
  Utils/PrintfAdder.h
#These files are needed only for building opt
  AppleWrapper/AppleWIDepPrePacketizationPass.h
  Driver/Apple/AppleOCLRuntime.h
#These files are needed only for building opt (to run dx LIT tests)
  Driver/DX/DXRuntime.h
  Driver/DX/DXWrapperParamsCheck.h
  Driver/Functions.h
)
endif(APPLE)

set(TARGET_INCLUDE_FILES
  Driver/OpenCL/OpenclRuntime.h
  Heuristic/InstCounter.h
  IRPrinter/IRPrinter.h
  LoopHandler/CLBuiltinLICM.h
  LoopHandler/CLStreamSampler.h
  LoopHandler/CLWGBoundDecoder.h
  LoopHandler/CLWGLoopBoundaries.h
  LoopHandler/CLWGLoopCreator.h
  LoopHandler/KernelAnalysis.h
  LoopHandler/LoopStridedCodeMotion.h
  LoopHandler/LoopWIAnalysis.h
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
  Utils/Logger.h
  Utils/Mangler.h
  Utils/RuntimeServices.h
  Utils/SimplifyGEP.h
  Utils/SpecialCaseFuncs.h
  Utils/VecConfig.h
  Utils/VectorizerCommon.h
  Utils/VectorizerCore.h
  Utils/VectorizerFunction.h
  VolcanoWrapper/Main.h
  WIAnalysis/WIAnalysis.h
  OCLBranchProbability/OCLBranchProbability.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set(TARGET_SOURCE_FILES
  Driver/OpenCL/OpenclRuntime.cpp
  Heuristic/InstCounter.cpp
  IRPrinter/IRPrinter.cpp
  LoopHandler/CLBuiltinLICM.cpp
  LoopHandler/CLStreamSampler.cpp
  LoopHandler/CLWGBoundDecoder.cpp
  LoopHandler/CLWGLoopBoundaries.cpp
  LoopHandler/CLWGLoopCreator.cpp
  LoopHandler/KernelAnalysis.cpp
  LoopHandler/LoopStridedCodeMotion.cpp
  LoopHandler/LoopWIAnalysis.cpp
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
  Utils/Mangler.cpp
  Utils/RuntimeServices.cpp
  Utils/SimplifyGEP.cpp
  Utils/SpecialCaseFuncs.cpp
  Utils/VectorizerCore.cpp
  VolcanoWrapper/Main.cpp
  WIAnalysis/WIAnalysis.cpp
  OCLBranchProbability/OCLBranchProbability.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi LoopUtils)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${TARGET_NAME} )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_SOURCE_FILES} ${TARGET_INCLUDE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
