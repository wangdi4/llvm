# binary output directory suffix
if (BUILD_X64)    
    set(BIN_OUTPUT_DIR_SUFFIX win64)
else (BUILD_X64)    
    set(BIN_OUTPUT_DIR_SUFFIX win32)
endif (BUILD_X64)

# TAL libraries directory suffix
if (BUILD_X64)    
    set(TAL_LIB_DIR_SUFFIX x64)
else (BUILD_X64)    
    set(TAL_LIB_DIR_SUFFIX x86)
endif (BUILD_X64)

# C Compiler
if (DEFINED INTEL_COMPILER)
    set( CMAKE_C_COMPILER "icl" )
else ()
    set( CMAKE_C_COMPILER "cl" )
endif (DEFINED INTEL_COMPILER)

# Microsoft Assembler setup - use private rules
if (BUILD_X64)
    set( CMAKE_ASM_COMPILER            ml64 ) #OLD changed due to issues in TFW (didn't find asm compiler on win 64	
else (BUILD_X64)
    set( CMAKE_ASM_COMPILER            ml )
endif (BUILD_X64)

if (BUILD_X64)
    set( CMAKE_ASM_FLAGS               /nologo /c /Zi) # do not quote this!!!!
else (BUILD_X64)
    set( CMAKE_ASM_FLAGS               /nologo /c /coff /Zi) # do not quote this!!!!
endif (BUILD_X64)

set( CMAKE_ASM_INCLUDE_DIR_FLAG    /I )

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_C_FLAGS         "/Oi -D WINDOWS_ENABLE_CPLUSPLUS")
set (ADD_C_FLAGS_DEBUG   "-D _DEBUG /Gm /RTC1 /MDd")  #MTd
set (ADD_C_FLAGS_RELEASE "/Zi /Gy -D NDEBUG /MD /Ob0") #/GL") #MT

# Compiler switches that CAN be modified during makefile generation and configuration-independent
add_definitions( -DWIN32 )

# Linker switches
if (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X64 /OPT:REF /INCREMENTAL:NO")
else (BUILD_X64)
    set (INIT_LINKER_FLAGS        "/MACHINE:X86 /OPT:REF /INCREMENTAL:NO")
endif (BUILD_X64)

set (ADD_LINKER_FLAGS_DEBUG "/DEBUG")
set (ADD_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF ") #/LTCG")


# setup
set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER} )
enable_language( C )
enable_language( CXX )

# remove CRT DLL option from default C/C++ switches
string( REPLACE /MDd "" CMAKE_C_FLAGS_DEBUG       ${CMAKE_C_FLAGS_DEBUG} )
string( REPLACE /MD  "" CMAKE_C_FLAGS_RELEASE     ${CMAKE_C_FLAGS_RELEASE} )
string( REPLACE /MDd "" CMAKE_CXX_FLAGS_DEBUG     ${CMAKE_CXX_FLAGS_DEBUG} )
string( REPLACE /MD  "" CMAKE_CXX_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )

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

