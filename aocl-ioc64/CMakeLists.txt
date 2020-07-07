cmake_minimum_required(VERSION 3.2)

# Supported configurations
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(OpenCL_INCLUDE_DIR AND OpenCL_LIBRARY)
    find_package(OpenCL)
endif()

include(ExternalProject)

if (NOT OpenCL_INCLUDE_DIRS)
    message("OpenCL_INCLUDE_DIRS is missing. Will try to download OpenCL headers from github.com")
    set(OpenCL_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/OpenCL/inc")
    ExternalProject_Add(opencl-headers-repo
            GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
            GIT_TAG origin/master
            SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/OpenCL/inc"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E copy_directory ${OpenCL_INCLUDE_DIRS}/CL ${CMAKE_CURRENT_BINARY_DIR}/inc/CL
            INSTALL_COMMAND ""
            STEP_TARGETS build
            COMMENT "Downloading OpenCL headers."
            )
    add_definitions(-DCL_TARGET_OPENCL_VERSION=220)
else ()
    add_custom_target(opencl-headers-repo ALL
            DEPENDS ${OpenCL_INCLUDE_DIRS}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${OpenCL_INCLUDE_DIRS}/CL ${CMAKE_CURRENT_BINARY_DIR}/inc/CL
            COMMENT "Copying OpenCL headers ..."
            )
endif ()

if (NOT OpenCL_LIBRARIES)
    message("OpenCL_LIBRARIES is missing. Will try to download OpenCL ICD Loader from github.com")
    if (MSVC)
        set(OpenCL_LIBRARIES
                "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}OpenCL${CMAKE_STATIC_LIBRARY_SUFFIX}")
        set(OPENCL_ICD_LOADER_WDK "-DOPENCL_ICD_LOADER_REQUIRE_WDK=OFF")
    else ()
        set(OpenCL_LIBRARIES
                "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}OpenCL${CMAKE_SHARED_LIBRARY_SUFFIX}")
        set(OPENCL_ICD_LOADER_WDK "")
    endif ()
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/icd_build)
    ExternalProject_Add(opencl-icd-repo
            GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
            GIT_TAG origin/master
            SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/OpenCL/icd"
            BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/icd_build"
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
            -DOPENCL_ICD_LOADER_HEADERS_DIR=${OpenCL_INCLUDE_DIRS}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
            -DCMAKE_INSTALL_LIBDIR:PATH=lib${LLVM_LIBDIR_SUFFIX}
            ${OPENCL_ICD_LOADER_WDK}
            STEP_TARGETS configure,build,install
            DEPENDS opencl-headers-repo
            BUILD_BYPRODUCTS ${OpenCL_LIBRARIES}
            )
else ()
    file(GLOB ICD_LOADER_SRC "${OpenCL_LIBRARIES}*")
    file(COPY ${ICD_LOADER_SRC} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_target(opencl-icd-repo DEPENDS ${OpenCL_LIBRARIES} COMMENT "Copying OpenCL ICD Loader ...")
endif ()

set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )
include_directories(${CMAKE_CURRENT_BINARY_DIR}/inc
                    ${LLVM_MAIN_SRC_DIR}/../opencl/utils/cl_sys_utils/export
                    ${LLVM_MAIN_SRC_DIR}/../opencl/cl_api
		    ${LLVM_MAIN_SRC_DIR}/../opencl/framework/icd/inc
        )
link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib${LLVM_LIBDIR_SUFFIX}
                 ${CMAKE_CURRENT_BINARY_DIR}
        )
file(GLOB TARGET_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set(AOCL_IOC64_PROJECT_NAME aocl-ioc64)

add_llvm_tool(${AOCL_IOC64_PROJECT_NAME} ${TARGET_SOURCES} CUSTOM_WIN_VER)

add_dependencies(${AOCL_IOC64_PROJECT_NAME} opencl-headers-repo opencl-icd-repo)

target_link_libraries(${AOCL_IOC64_PROJECT_NAME} PRIVATE OpenCL)

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(${AOCL_IOC64_PROJECT_NAME}
    "${AOCL_IOC64_PROJECT_NAME}.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "OpenCL ahead-of-time (AOT) compilation tool for Intel(R) FPGA Emulation Platform.")
endif()

# Use it as fake dependency in order to force another command(s) to execute.
add_custom_command(OUTPUT __force_it
        COMMAND "${CMAKE_COMMAND}" -E echo
        )
#Serialize installation to avoid missing components due to build race conditions
set(__chain_dep __force_it)

set(manifest_list)

message( STATUS "Adding component ${AOCL_IOC64_PROJECT_NAME} to deploy")

set (manifest_file ${CMAKE_CURRENT_BINARY_DIR}/install_manifest_${AOCL_IOC64_PROJECT_NAME}.txt)
add_custom_command(OUTPUT ${manifest_file}
        COMMAND "${CMAKE_COMMAND}"
        "-DCMAKE_INSTALL_COMPONENT=${AOCL_IOC64_PROJECT_NAME}"
        -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        DEPENDS  ${__chain_dep}
        COMMENT "Deploying component ${AOCL_IOC64_PROJECT_NAME}"
        USES_TERMINAL
        )
list(APPEND manifest_list ${manifest_file})
set(__chain_dep ${manifest_file})

add_custom_target(deploy-${AOCL_IOC64_PROJECT_NAME}
        DEPENDS ${AOCL_IOC64_PROJECT_NAME} ${manifest_list}
        )
