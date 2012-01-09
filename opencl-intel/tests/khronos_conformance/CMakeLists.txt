
# Conformance will be build separately, because CMake does not allow using different compilers
# in the same build configuration.

project( ConformanceWrapper )

# Helper function.
function( set_if_empty name value )
    if( "${${name}}" STREQUAL "" )
        set( "${name}" "${value}" )
    endif()
endfunction()

if( WIN32 )
    set( os   "win"   )
    if( BUILD_X64 )
        set( arch "x64" )
    else()
        set( arch "x86" )
    endif()
    set( comp "intel" )
    set( ocl  "OpenCL" )    # On Windows, `OpenCL' should work.
elseif( UNIX )
    set( os   "lin"   )
    set( arch "x64"   )
    # On Linux, `OpenCL' does not work due to improperly configure build machines.
    # When machines are configured properly so `OpenCL' can find `intelocl',
    # this variable and `-l' option should be eliminated.
    set( ocl  "intelocl" )
else()
    message( FATAL_ERROR "Unsupported OS kind, neither Windows, nor Unix." )
endif()
set( comp "intel" )

set_if_empty( CMAKE_BUILD_TYPE "Release" )
string( TOLOWER "${CMAKE_BUILD_TYPE}" bld_type )

set( glext_inc_dir "" )
if( WIN32 )
    set( glext_inc_dir "${OCL_GLOBAL_INCLUDE}" )
endif()

find_package( PythonInterp REQUIRED )

add_custom_target(
    ConformanceWrapper
    ALL
    COMMAND
        "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/build_conform.py"
            # Pass-through some configuration variables:
            -D "CMAKE_COLOR_MAKEFILE=${CMAKE_COLOR_MAKEFILE}"
            -D "CMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}"
            -v  # Generate verbose makefile. Uncomment in case of troubles.
            # Specify compiler, architecture and build type:
            -c "${comp}" -a "${arch}" -t "${bld_type}"
            # Specify OpenCL library:
            -l "${ocl}"
            # Specify directories:
            -I "${OCL_GLOBAL_INCLUDE}"  # `CL/' with OpenCL headers are there.
#            -I "${glext_inc_dir}"    # `glext.h' header is there.
            -L "$<TARGET_FILE_DIR:OpenCL>"  # OpenCL libraries (e. g. `libOpenCL.so') are there.
            -L "$<TARGET_FILE_DIR:intelocl>" 
            -L "$<TARGET_FILE_DIR:cl_logger>" 
            -L "$<TARGET_FILE_DIR:task_executor>" 
            -L ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
            -B "${PROJECT_BINARY_DIR}/src/${os}.${arch}-${bld_type}-${comp}"
            # Output directory:
            -O "${CMAKE_INSTALL_PREFIX}/bin/validation/conformance"
    VERBATIM
)

add_dependencies(
    ConformanceWrapper        # Depends on:
    #external_headers_copy     # OpenCL headers (target from `cl_api/' directory).
    ${ocl}                    # OpenCL library.
)
set_target_properties( ConformanceWrapper PROPERTIES FOLDER "validation" )
# end of file #
