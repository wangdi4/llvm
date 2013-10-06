# Create the passes seperate projects
use_rtti( FALSE )

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )

add_subdirectory(AddImplicitArgs)
add_subdirectory(BuiltInFuncImport)
add_subdirectory(CloneBlockInvokeFuncToKernel)
add_subdirectory(DetectFunctionPtrCalls)
add_subdirectory(InstToFuncCall)
add_subdirectory(LocalBuffAnalysis)
add_subdirectory(LocalBuffers)
add_subdirectory(ModuleCleanup)
add_subdirectory(PrepareKernelArgs)
add_subdirectory(PreventDivCrashes)
add_subdirectory(RelaxedPass)
add_subdirectory(ResolveWICall)
add_subdirectory(ShiftZeroUpperBits)
add_subdirectory(ShuffleCallToInst)
add_subdirectory(UndefExternalFuncs)
add_subdirectory(KernelInfo)
add_subdirectory(GenericAddressResolution)
add_subdirectory(LinearIdResolver)
add_subdirectory(ResolveBlockToStaticCall)
add_subdirectory(obfuscation)

if(APPLE)
  add_subdirectory(ClangCompatFixer)
else(APPLE)
  add_subdirectory(spir)
  add_subdirectory(ProfilingInfoPass)
  add_subdirectory(OpenCLAliasAnalysisSupport)
  add_subdirectory(DebugInfoPass)
  add_subdirectory(Prefetch)
  add_subdirectory(BackendPassesWrappers)
  add_subdirectory(PrintIRPass)
  add_subdirectory(ReduceAlignment)
endif(APPLE)

if(APPLE)
  set(TARGET_INCLUDE_FILES
      InitializePasses.h
      OCLPassSupport.h
  )

  set(TARGET_SOURCE_FILES
  )

  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
