# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.
if(WIN32)
    if( INCLUDE_MIC_DEVICE AND ( CMAKE_SIZEOF_VOID_P EQUAL 8 ) )
        list(APPEND STATIC_LLVM_MODULE_LIBS
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            libpcg
            svml_dispmt
            libirc
            libmmt
            libdecimal
            )
    endif()
	list(APPEND STATIC_LLVM_MODULE_LIBS
		LLVMAsmParser 
		LLVMTableGen 
		LLVMOpenCLAliasAnalysisSupport
		LLVMMICModuleJIT
		LLVMDebugInfo 
		LLVMX86AsmParser 
		LLVMX86Disassembler 
		LLVMX86CodeGen 
		LLVMSelectionDAG 
		LLVMAsmPrinter 
		LLVMX86Desc 
		LLVMX86Info 
		LLVMX86AsmPrinter 
		LLVMX86Utils 
		LLVMJIT 
		LLVMMCDisassembler 
		LLVMMCParser 
		LLVMInstrumentation 
		LLVMInterpreter 
		LLVMCodeGen 
		LLVMipo 
		LLVMVectorize 
		LLVMScalarOpts 
		LLVMInstCombine 
		LLVMLinker 
		LLVMTransformUtils 
		LLVMipa 
		LLVMAnalysis 
		LLVMArchive 
		LLVMBitReader 
		LLVMBitWriter 
		LLVMMCJIT 
		LLVMRuntimeDyld 
		LLVMExecutionEngine 
		LLVMTarget 
		LLVMMC 
		LLVMObject 
		LLVMObjCARCOpts 
		LLVMCore 
		LLVMIRReader 
		LLVMSupport
	)
    if(NOT CMAKE_CROSSCOMPILING)
        list(APPEND STATIC_LLVM_MODULE_LIBS
            LLVMIntelJITEvents 
        )
    endif()

else()
	list(APPEND STATIC_LLVM_MODULE_LIBS
		 LLVMCppBackendCodeGen
		 LLVMCppBackendInfo
	)
    if( INCLUDE_MIC_DEVICE)
        list(APPEND STATIC_LLVM_MODULE_LIBS
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            pcg
            svml
            irc
            )
    endif()
	list(APPEND STATIC_LLVM_MODULE_LIBS
		LLVMAsmParser 
		LLVMTableGen 
		LLVMOpenCLAliasAnalysisSupport
		LLVMMICModuleJIT
		LLVMDebugInfo 
		LLVMX86AsmParser 
		LLVMX86Disassembler 
		LLVMX86CodeGen 
		LLVMSelectionDAG 
		LLVMAsmPrinter 
		LLVMX86Desc 
		LLVMX86Info 
		LLVMX86AsmPrinter 
		LLVMX86Utils 
		LLVMJIT 
		LLVMMCDisassembler 
		LLVMMCParser 
		LLVMInstrumentation 
		LLVMInterpreter 
		LLVMCodeGen 
		LLVMipo 
		LLVMVectorize 
		LLVMScalarOpts 
		LLVMInstCombine 
		LLVMLinker 
		LLVMTransformUtils 
		LLVMipa 
		LLVMAnalysis 
		LLVMArchive 
		LLVMBitReader 
		LLVMBitWriter 
		LLVMMCJIT 
		LLVMRuntimeDyld 
		LLVMExecutionEngine 
		LLVMTarget 
		LLVMMC 
		LLVMObject 
		LLVMObjCARCOpts 
		LLVMCore 
		LLVMIRReader 
		LLVMSupport
		)
    if(NOT CMAKE_CROSSCOMPILING)
        list(APPEND STATIC_LLVM_MODULE_LIBS
            LLVMIntelJITEvents 
        )
    endif()
endif()
option(BUILD_LLVM_FROM_SOURCE "Build the llvm package from source instead of using pre-installed binaries" OFF)
if(BUILD_LLVM_FROM_SOURCE )
	set (LLVM_INCLUDE_DIR ${LLVM_SRC_ROOT}/include ${LLVM_SRC_ROOT}/tools/clang/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/tools/clang/include)
	set (LLVM_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm/lib/${CMAKE_CFG_INTDIR})
	set (LLVM_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/llvm/bin/${CMAKE_CFG_INTDIR})
	set (LLVM_MODULE_LIBS ${STATIC_LLVM_MODULE_LIBS})
else(BUILD_LLVM_FROM_SOURCE )
	if (APPLE)
		set(LLVM_INCLUDE_DIR   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders/llvm/include")
		set(CVMS_INCLUDE_DIR   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders")
	else (APPLE)
		if(NOT ANDROID)
			find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config HINTS ~/usr/local/bin /usr/local/bin ${LLVM_PATH} ENV LLVM_PATH PATH_SUFFIXES bin DOC "llvm-config executable")
		endif()

		if (LLVM_CONFIG_EXECUTABLE)
			message(STATUS "Using llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}")
			
			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
			  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
			  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --bindir
			  OUTPUT_VARIABLE LLVM_BINARY_DIR
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			
			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
			  OUTPUT_VARIABLE LLVM_CFLAGS
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
			  OUTPUT_VARIABLE LLVM_LFLAGS
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			execute_process(
			  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
			  OUTPUT_VARIABLE LLVM_MODULE_LIBS
			  OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			if( INCLUDE_MIC_DEVICE)
				list(APPEND LLVM_MODULE_LIBS
					pcg
					svml
					irc
					)
			endif( INCLUDE_MIC_DEVICE)
						
		else (LLVM_CONFIG_EXECUTABLE)

			if( NOT WIN32 AND NOT ANDROID)
				message( FATAL_ERROR "Can't find llvm-config. LLVM installation is corrupted or missing" )		
			endif()
			
			if( DEFINED LLVM_PATH )
				set(LLVM_INSTALL_PATH ${LLVM_PATH})
				message (STATUS "Using LLVM root location specified in LLVM_PATH variable: " ${LLVM_PATH})
			elseif( EXISTS $ENV{LLVM_PATH} )
				set(LLVM_INSTALL_PATH $ENV{LLVM_PATH})
				message (STATUS "Using LLVM location specified in LLVM_PATH environment variable: " $ENV{LLVM_PATH} )
			else()
				message( FATAL_ERROR "Can't find LLVM library. Please specify LLVM library location using either LLVM_PATH environment variable or defining LLVM_PATH parameter to CMAKE" )		
			endif()
			  
			set(LLVM_INCLUDE_DIR ${LLVM_INSTALL_PATH}/include)
			set(LLVM_LIBRARY_DIR ${LLVM_INSTALL_PATH}/lib)
			string( CONFIGURE ${LLVM_LIBRARY_DIR} LLVM_LIBRARY_DIR)
			set(LLVM_BINARY_DIR ${LLVM_INSTALL_PATH}/bin)
			string( CONFIGURE ${LLVM_BINARY_DIR} LLVM_BINARY_DIR)
			set(LLVM_CFLAGS)
			set(LLVM_LFLAGS)
			set(LLVM_MODULE_LIBS ${STATIC_LLVM_MODULE_LIBS})

		endif (LLVM_CONFIG_EXECUTABLE)

	endif (APPLE)

endif(BUILD_LLVM_FROM_SOURCE )

message( STATUS "LLVM libs: ${LLVM_MODULE_LIBS}")
