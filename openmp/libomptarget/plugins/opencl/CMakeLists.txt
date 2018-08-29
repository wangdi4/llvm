if(LIBOMPTARGET_DEP_OPENCL_FOUND)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)$" AND CMAKE_SYSTEM_NAME MATCHES "Linux")

        libomptarget_say("Building OpenCL offloading plugin.")

        # Define the suffix for the runtime messaging dumps.
        add_definitions(-DTARGET_NAME=OPENCL)

        if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
            add_definitions(-DSPIR_ERROR_REPORT)
        endif()

        include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
        include_directories(${LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS})

        add_library(omptarget.rtl.opencl SHARED src/rtl.cpp)

        # Install plugin under the lib destination folder.
        install(TARGETS omptarget.rtl.opencl LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
        target_link_libraries(omptarget.rtl.opencl
                ${LIBOMPTARGET_DEP_OPENCL_LIBRARIES}
                "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

        # Report to the parent scope that we are building a plugin for OpenCL.
        set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} spir64-unknown-unknown" PARENT_SCOPE)
    else()
        libomptarget_say("Not building OpenCL offloading plugin: only support OpenCL in Linux x86_64 hosts.")
    endif()
else()
    libomptarget_say("Not building OpenCL offloading plugin: OpenCL not found in system.")
endif()
