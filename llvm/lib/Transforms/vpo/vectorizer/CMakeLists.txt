# ------------------------ DEFINITIONS -------------------------

# Set OCL Tuning to ON. ON by default. can be turned off for release.
set( OCLT ON)
if ( OCLT )
  add_definitions( -DOCLT )
endif (OCLT)

#remove_definitions( -D_UNICODE -DUNICODE )

# Define a macro which can be used to hide MIC symbols when we do not want to
# expose IP
if (INCLUDE_MIC_DEVICE)
  add_definitions( -DINCLUDE_MIC_DEVICE )
endif (INCLUDE_MIC_DEVICE)

# -------------------------- SERVICES ---------------------------

set(SERVICES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Services)

set(SERVICES_TARGET_SOURCE_FILES 
  Services/passes/BuiltinLibInfo/BuiltinLibInfo.cpp
  Services/passes/BuiltinLibInfo/TestRuntimeServices.cpp
  Services/reflection_module/Mangler.cpp
  Services/name_mangling/Mangler.cpp
  Services/name_mangling/FunctionDescriptor.cpp
  Services/name_mangling/ParameterType.cpp
  Services/name_mangling/Demangler.cpp
  Services/name_mangling/Utils.cpp
  Services/name_mangling/DemangleParser.cpp
  Services/utils/OclTune.cpp
  Services/utils/LoopUtils/LoopUtils.cpp
  Services/utils/CompilationUtils.cpp
  Services/utils/VectorizerUtils.cpp
  Services/utils/TypeConversion.cpp
#  Services/metadata_api/MetaDataApi.cpp
)

# -------------------------- VECTORIZER ---------------------------

set(TARGET_EXTRA_SOURCE_FILES 
  Resolver/MICResolver.cpp
  Utils/Namer.cpp
  Utils/PrintfAdder.cpp
)

include_directories(BEFORE
  ${SERVICES_SOURCE_DIR}/arch_headers
  ${SERVICES_SOURCE_DIR}/reflection_module
  ${SERVICES_SOURCE_DIR}/name_mangling
  ${SERVICES_SOURCE_DIR}/utils
  ${SERVICES_SOURCE_DIR}/utils/LoopUtils
  ${SERVICES_SOURCE_DIR}/passes
  ${SERVICES_SOURCE_DIR}/passes/BuiltinLibInfo
  ${SERVICES_SOURCE_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
  ${SERVICES_SOURCE_DIR}/metadata_api
)

#include_directories(BEFORE "Services")
include_directories(BEFORE "Heuristic")
include_directories(BEFORE "PostDominanceFrontier")
include_directories(BEFORE "SoaAllocaAnalysis")
include_directories(BEFORE "Utils")
include_directories(BEFORE "WIAnalysis")
include_directories(BEFORE "Predication")
include_directories(BEFORE "OCLBranchProbability")
include_directories(BEFORE "ChooseVectorizationDimension")

add_llvm_library(LLVMVPOVectorizer
  Heuristic/InstCounter.cpp
  IRPrinter/IRPrinter.cpp
  LoopHandler/LoopStridedCodeMotion.cpp
  LoopHandler/LoopWIAnalysis.cpp
  Packetizer/Packetizer.cpp
  Packetizer/PacketizerInstData.cpp
  PostDominanceFrontier/PostDominanceFrontier.cpp
  Predication/Linearizer.cpp
  Predication/PhiCanon.cpp
  Predication/Predicator.cpp
  Predication/Specializer.cpp
  Resolver/Resolver.cpp
  Scalarizer/PreScalarizeScan.cpp
  Scalarizer/Scalarize.cpp
  SoaAllocaAnalysis/SoaAllocaAnalysis.cpp
  Utils/SimplifyGEP.cpp
  Utils/VectorizerCore.cpp
  Utils/VectorVariant.cpp
  VolcanoWrapper/Main.cpp
  WIAnalysis/WIAnalysis.cpp
  OCLBranchProbability/OCLBranchProbability.cpp
  ChooseVectorizationDimension/ChooseVectorizationDimension.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
  ${SERVICES_TARGET_SOURCE_FILES}
)

add_dependencies(LLVMVPOVectorizer intrinsics_gen)
