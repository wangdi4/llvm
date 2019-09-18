# binary output directory suffix
if (BUILD_X64)
    set(BIN_OUTPUT_DIR_SUFFIX win64)
else (BUILD_X64)
    set(BIN_OUTPUT_DIR_SUFFIX win32)
endif (BUILD_X64)

# Microsoft Assembler setup - use private rules
if (BUILD_X64)
    set( CMAKE_ASM_COMPILER            ml64 ) #OLD changed due to issues in TFW (didn't find asm compiler on win 64
    set( CMAKE_ASM_FLAGS               /nologo /c /Zi) # do not quote this!!!!
else (BUILD_X64)
    set( CMAKE_ASM_COMPILER            ml )
    set( CMAKE_ASM_FLAGS               /nologo /safeseh /c /coff /Zi) # do not quote this!!!!
endif (BUILD_X64)

set( CMAKE_ASM_INCLUDE_DIR_FLAG    /I )

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_C_FLAGS         "/Oi -D WINDOWS_ENABLE_CPLUSPLUS /GS")
set (ADD_C_FLAGS_DEBUG   "-D _DEBUG /RTC1")
set (ADD_C_FLAGS_RELEASE "/Zi /Gy")

if (NOT DEFINED INTEL_COMPILER)
    set (ADD_C_FLAGS_RELEASE "${ADD_C_FLAGS_RELEASE} /GS")
endif()

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( -DWIN32 )

# ITT/GPA/VTUNE integration
if (USE_GPA)
    add_definitions( -DUSE_GPA )
    include_directories( ${CMAKE_SOURCE_DIR}/externals/gpa/include )
endif (USE_GPA)

# Linker switches
if (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X64 /OPT:REF /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT")
else (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X86 /OPT:REF /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT /SAFESEH")
endif (BUILD_X64)

set (ADD_LINKER_FLAGS_DEBUG "/DEBUG /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCPMT")
set (ADD_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCPMTD")

# setup
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER} )

# remove /INCREMENTAL:YES option from DEBUG Linker switches
string( REPLACE /INCREMENTAL:YES "" CMAKE_EXE_LINKER_FLAGS_DEBUG    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} )
string( REPLACE /INCREMENTAL:YES "" CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} )

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# Linker switches - EXE
set( CMAKE_EXE_LINKER_FLAGS           ${INIT_LINKER_FLAGS})
set( CMAKE_EXE_LINKER_FLAGS_DEBUG     "${CMAKE_EXE_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_EXE_LINKER_FLAGS_RELEASE   "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

# Linker switches - DLL
set( CMAKE_SHARED_LINKER_FLAGS          ${INIT_LINKER_FLAGS})
set( CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_LINKER_FLAGS_DEBUG}")
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_LINKER_FLAGS_RELEASE}")

if (UWD_BUILD)
    set( CMAKE_SYSTEM_VERSION 10.0 )
    ocl_replace_compiler_option(CMAKE_C_FLAGS_RELEASE "/MD" "/MT")
    ocl_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "/MD" "/MT")
    ocl_replace_compiler_option(CMAKE_C_FLAGS_DEBUG "/MDd" "/MTd")
    ocl_replace_compiler_option(CMAKE_CXX_FLAGS_DEBUG "/MDd" "/MTd")
endif()
