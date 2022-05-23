# Create the passes seperate projects
use_rtti( FALSE )

include_directories(${LLVM_INCLUDE_DIRS}
                    ${CL_API_HEADERS}
                    ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(PipeSupport)
add_subdirectory(BuiltinLibInfo)
add_subdirectory(KernelInfo)
add_subdirectory(SubGroupAdaptation)
add_subdirectory(DebugInfoPass)
add_subdirectory(Prefetch)
add_subdirectory(PrintIRPass)
add_subdirectory(SmartGVN)
add_subdirectory(StripIntelIP)
