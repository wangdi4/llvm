##===----------------------------------------------------------------------===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
# 
##===----------------------------------------------------------------------===##
#
# Build a plugin for a csa machine if available.
#
##===----------------------------------------------------------------------===##

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  
  set(tmachine "x86_64")
  set(tmachine_name "csa")
  set(tmachine_libname "csa")
  set(tmachine_triple "csa")
  set(elf_machine_id "62")
#  build_generic_elf64("x86_64" "csa" "csa" "csa" "62")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "${tmachine}$")
    if(LIBOMPTARGET_DEP_LIBELF_FOUND)
#      if(LIBOMPTARGET_DEP_LIBFFI_FOUND)

        find_library(LibCSASIM csasim PATHS $ENV{CSASIM_TOP} NO_DEFAULT_PATH)
        if(LibCSASIM)

          libomptarget_say("Building ${tmachine_name} offloading plugin.")
    
#          include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
          include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR})
          include_directories($ENV{CSASIM_TOP})

          # Define macro to be used as prefix of the runtime messages for this target.
          add_definitions("-DTARGET_NAME=${tmachine_name}")
      
          # Define macro with the ELF ID for this target.
          add_definitions("-DTARGET_ELF_ID=${elf_machine_id}")
    
          # Defining OMPTARGET_DEBUG defines DP so it displays debugging info
#          add_definitions("-DOMPTARGET_DEBUG")

          add_library("omptarget.rtl.${tmachine_libname}" SHARED 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rtl.cpp)
        
          # Install plugin under the lib destination folder.
          install(TARGETS "omptarget.rtl.${tmachine_libname}" 
            LIBRARY DESTINATION "lib")
        
          target_link_libraries(
            "omptarget.rtl.${tmachine_libname}"
#            ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES} 
            ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
            ${LibCSASIM}
            dl
            rt
            "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
    
          # Report to the parent scope that we are building a plugin.
          set(LIBOMPTARGET_SYSTEM_TARGETS 
            "${LIBOMPTARGET_SYSTEM_TARGETS} ${tmachine_triple}" PARENT_SCOPE)
        else(LibCSASIM)
          libomptarget_say("Not building ${tmachine_name} offloading plugin: libcsasim.so dependency not found.")
        endif(LibCSASIM)
#      else(LIBOMPTARGET_DEP_LIBFFI_FOUND)
#        libomptarget_say("Not building ${tmachine_name} offloading plugin: libffi dependency not found.")
#      endif(LIBOMPTARGET_DEP_LIBFFI_FOUND)
    else(LIBOMPTARGET_DEP_LIBELF_FOUND)
      libomptarget_say("Not building ${tmachine_name} offloading plugin: libelf dependency not found.")
    endif(LIBOMPTARGET_DEP_LIBELF_FOUND)
  else()
    libomptarget_say("Not building ${tmachine_name} offloading plugin: machine not found in the system.")
  endif()
else()
 libomptarget_say("Not building csa offloading plugin: machine not found in the system.")
endif()
