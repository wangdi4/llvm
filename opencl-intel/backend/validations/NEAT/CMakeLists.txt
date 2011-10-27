include_directories(
	${BACKEND_ROOT_DIR}/../cl_api
        ../REFALU
	)

add_llvm_library(NEAT STATIC
NEATALU.cpp
NEATValue.cpp
NEATVector.cpp
)

add_dependencies(NEAT REFALU)
target_link_libraries(NEAT REFALU)
