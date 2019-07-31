set(PRE_RELEASE_HEADERS_LIB pre_release_header)
set(LINUX_RESOURCE_LINKER_COMMAND header_linker)

function(copy_file SRC DST)
add_custom_command(
    OUTPUT  ${DST}
    MAIN_DEPENDENCY ${SRC}
    COMMAND ${CMAKE_COMMAND} -E copy "${SRC}" "${DST}"
    COMMENT "Copying '${SRC}' to ${DST}"
)
endfunction(copy_file)

set (PRE_RELEASE_HEADER_DIR "${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins")

copy_file(${PRE_RELEASE_HEADER_DIR}/opencl_cth_pre_release.h opencl_cth_pre_release.h)

add_custom_target (
  opencl.cth.pre.release.target
  DEPENDS
  opencl_cth_pre_release.h
)

function(pack_to_obj SRC DST TAG)
  add_custom_command (
    OUTPUT ${DST}
    DEPENDS ${SRC} ${LINUX_RESOURCE_LINKER_COMMAND}
    COMMAND ${LINUX_RESOURCE_LINKER_COMMAND} "${SRC}" "${DST}" "${TAG}"
    COMMENT "Packing ${SRC}"
    )
endfunction(pack_to_obj)

pack_to_obj(opencl_cth_pre_release.h opencl_cth_pre_release.h.cpp "OPENCL_CTH_PRE_RELEASE_H")
list(APPEND CL_HEADERS_SRC
  opencl_cth_pre_release.h.cpp
)

add_library(${PRE_RELEASE_HEADERS_LIB} OBJECT
  ${CL_HEADERS_SRC}
)

add_dependencies(${PRE_RELEASE_HEADERS_LIB} opencl.cth.pre.release.target)
