if (WIN32)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	  create_target ( corei7 n8 __SSE4_2__ /QxSSE4.1)	# sse4.2 is not recognized by the compiler
	else ()
	  create_target ( corei7 h8 __SSE4_2__ /QxSSE4.1) # sse4.2 is not recognized by the compiler
	endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
else (WIN32)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	  create_target ( corei7 n8 __SSE4_2__ "-msse4.1 -Dclbltfnn8_EXPORTS")	# sse4.2 is not recognized by the compiler
	else ()
	  create_target ( corei7 h8 __SSE4_2__ "-msse4.1 -Dclbltfnh8_EXPORTS") # sse4.2 is not recognized by the compiler
	endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
endif (WIN32)

