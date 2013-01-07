use_rtti( FALSE )
set( TARGET_NAME LLVMVectorizer)

include_directories(BEFORE "WIAnalysis")
include_directories(BEFORE "Utils")
include_directories(BEFORE "Driver")
if(NOT APPLE)
    include_directories(BEFORE "Driver/DX")
endif(NOT APPLE)
include_directories(BEFORE "Driver/Apple")
include_directories(BEFORE "Driver/OpenCL")
include_directories(BEFORE "Resolver")
include_directories(BEFORE "Packetizer")
include_directories(BEFORE "VolcanoWrapper")
include_directories(BEFORE "AppleWrapper")
include_directories(BEFORE "PreVectorization")
include_directories(BEFORE "LoopHandler")
include_directories(BEFORE "SoaAllocaAnalysis")
include_directories(BEFORE "IRPrinter")
include_directories(BEFORE "Heuristic")
include_directories(BEFORE "PostDominanceFrontier")
include_directories(../arch_headers)
include_directories(${CMAKE_SOURCE_DIR}/backend/reflection_module)
include_directories(${CMAKE_SOURCE_DIR}/backend/name_mangling)

file(GLOB_RECURSE TARGET_VOLCANO_WRAPPER_INCLUDE_FILES VolcanoWrapper/*.h)
file(GLOB_RECURSE TARGET_INCLUDE_FILES *.h )

if(NOT APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  Resolver/MICResolver.cpp
  Driver/DX/DXRuntime.cpp
  Driver/DX/DXData.cpp
  Driver/DX/DXWrapperParamsCheck.cpp
)
else(NOT APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
)
endif(NOT APPLE)

set(TARGET_SOURCE_FILES
  Utils/Namer.cpp
  Utils/SpecialCaseFuncs.cpp
  Utils/RuntimeServices.cpp
  Utils/SimplifyGEP.cpp
  Driver/OpenCL/OpenclRuntime.cpp
  Driver/Functions.cpp
  Driver/Apple/AppleOCLRuntime.cpp
  Predication/Predicator.cpp
  Predication/Specializer.cpp
  Predication/PhiCanon.cpp
  Predication/Linearizer.cpp
  WIAnalysis/WIAnalysis.cpp
  Scalarizer/Scalarize.cpp
  Scalarizer/PreScalarizeScan.cpp
  Packetizer/Packetizer.cpp
  Packetizer/PacketizerInstData.cpp
  Resolver/Resolver.cpp
  Resolver/SelectLower.cpp
  Resolver/SpecialCaseBuiltinResolver.cpp
  VolcanoWrapper/Main.cpp
  Utils/Mangler.cpp
  Utils/VectorizerCore.cpp
  Utils/VectorizerUtils.cpp
  Utils/IRInjection.cpp
  Utils/PrintfAdder.cpp
  AppleWrapper/AppleWIDepPrePacketizationPass.cpp
  PreVectorization/OCLBuiltinPreVectorizationPass.cpp
  LoopHandler/KernelAnalysis.cpp
  LoopHandler/LoopUtils.cpp
  LoopHandler/CLWGLoopCreator.cpp
  LoopHandler/CLWGLoopBoundaries.cpp
  LoopHandler/CLWGBoundDecoder.cpp
  LoopHandler/LoopStridedCodeMotion.cpp
  LoopHandler/CLStreamSampler.cpp
  LoopHandler/LoopWIAnalysis.cpp
  LoopHandler/CLBuiltinLICM.cpp
  SoaAllocaAnalysis/SoaAllocaAnalysis.cpp
  IRPrinter/IRPrinter.cpp
  Heuristic/InstCounter.cpp
  PostDominanceFrontier/PostDominanceFrontier.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

target_link_libraries(
  LLVMVectorizer
  reflection_module
  name_mangle
)

set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${TARGET_NAME} )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_SOURCE_FILES} ${TARGET_INCLUDE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
  install( FILES ${TARGET_VOLCANO_WRAPPER_INCLUDE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}/VolcanoWrapper
         )
endif(APPLE)
