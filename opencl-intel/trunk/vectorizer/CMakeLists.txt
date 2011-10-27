file(GLOB_RECURSE headers *.h)
include_directories(BEFORE "WIAnalysis")
include_directories(BEFORE "Utils")
include_directories(BEFORE "Driver")
include_directories(BEFORE "Driver/DX")
include_directories(BEFORE "Driver/Apple")
include_directories(BEFORE "Driver/OpenCL")
include_directories(BEFORE "Resolver")
include_directories(BEFORE "VolcanoWrapper")
include_directories(BEFORE "AppleWrapper")
include_directories(BEFORE "PreVectorization")
include_directories(../arch_headers)

add_llvm_library( LLVMVectorizer
  Utils/Namer.cpp
  Utils/SpecialCaseFuncs.cpp
  Utils/RuntimeServices.cpp
  Driver/OpenCL/OpenclRuntime.cpp
  Driver/OpenCL/OCLData.cpp
  Driver/DX/DXRuntime.cpp
  Driver/DX/DXData.cpp
  Driver/DX/DXWrapperParamsCheck.cpp
  Driver/Functions.cpp
  Driver/Apple/AppleOCLData.cpp
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
  Resolver/X86Lower.cpp
  Resolver/SelectLower.cpp
  Resolver/SpecialCaseBuiltinResolver.cpp
  VolcanoWrapper/Main.cpp
  VolcanoWrapper/VecHeuristics.cpp
  Utils/Mangler.cpp
  Utils/VectorizerUtils.cpp
  AppleWrapper/AppleWIDepPrePacketizationPass.cpp
  PreVectorization/OCLBuiltinPreVectorizationPass.cpp
  ${headers}
)
