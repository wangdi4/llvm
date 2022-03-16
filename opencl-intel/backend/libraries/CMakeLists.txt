project(libraries)

# In this project we use binaries from OpenCL project (e.g. oclopt), LLVM for
# front-end compiler (e.g. clang) and LLVM for back-end compiler (e.g. llc).
# oclopt utility is built with LLVM version for the back-end compiler, so when
# applies passes to the RTL it converts LLVM IR to newer version. This restrict
# the tools we use after oclopt to the tools from LLVM for back-end compiler.
# So we have to set path to tools directory for BE version of LLVM.
# We know that LLVM for the back-end compiler is already configured, so just
# save LLVM_BINARY_DIR for later use by LLVM tools running after oclopt.
# TODO: to make this file more self contained we should not rely on the
# assumption above and configure LLVM for the back-end right here.
set(BE_LLVM_BINARY_DIR ${LLVM_BINARY_DIR})

if (NOT OPENCL_INTREE_BUILD)
  if( NOT DEFINED LLVM_PATH_FE )
    message(FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM "
        "library location for fronted using LLVM_PATH_FE parameter to CMAKE")
  endif()

  set(LLVM_PATH ${LLVM_PATH_FE})
  find_package(LLVM REQUIRED)
endif()

set(CL_LIBRARY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TOOL_BINARY_DIR "${BE_LLVM_BINARY_DIR}")

# global headers
if(DEFINED OCL_CLANG_HEADERS)
  set (CLANG_HEADERS ${OCL_CLANG_HEADERS})
else()
  set (CLANG_HEADERS ${CL_LIBRARY_SOURCE_DIR}/../clang_headers)
endif()
if(NOT EXISTS ${CLANG_HEADERS})
  message (FATAL_ERROR "Cannot find Clang headers (${CLANG_HEADERS})!")
endif()

if(NOT DEFINED OCL_GLOBAL_INCLUDE)
  message (FATAL_ERROR "CL API directory path is not set!")
endif()
set(IMPORT_DIR ${OCL_GLOBAL_INCLUDE})

if(NOT OPENCL_INTREE_BUILD)
  # LLVM tools are assumed to be pre-built within the global project
  if(NOT DEFINED OCL_HOST_CLANG)
    set (CLANG "${LLVM_BINARY_DIR}/clang")
  else()
    set (CLANG ${OCL_HOST_CLANG})
  endif()
else()
  set(CLANG clang)
endif()

if(NOT DEFINED OCL_HOST_OPT)
  set(OPT ${OCL_OPT})
else()
  set(OPT OCL_HOST_OPT)
endif()
set(OCL_TBLGEN ocl-tblgen)
set(LLVM_TBLGEN llvm-tblgen)

if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED OCL_HOST_LLVM_LINK)
    set(LLVM_LINK "${TOOL_BINARY_DIR}/llvm-link")
  else()
    set(LLVM_LINK OCL_HOST_LLVM_LINK)
  endif()
else()
  set(LLVM_LINK llvm-link)
endif()

if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED OCL_HOST_LLC)
    set(LLC "${TOOL_BINARY_DIR}/llc")
  else()
    set (LLC OCL_HOST_LLC)
  endif()
else()
  set(LLC llc)
endif()

# LLVM triple is per OS and Arch (keep consistent with Materializer),
# also, note that -elf postfix is added to windows triples later in the file.
# adding "-elf" postfix to the triple for Win32 to codegen ELF with llc later
# (to be consistent with Backend Materializer)
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(DEVICE_TRIPLE i686-pc-win32-gnu-elf)
    set(HOST_TRIPLE   i686-pc-win32-gnu)
  else()
    set(DEVICE_TRIPLE x86_64-pc-win32-gnu-elf)
    set(HOST_TRIPLE   x86_64-pc-win32-gnu)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
else(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(DEVICE_TRIPLE i686-pc-linux)
    set(HOST_TRIPLE   i686-pc-linux)
  else()
    set(DEVICE_TRIPLE x86_64-pc-linux)
    set(HOST_TRIPLE   x86_64-pc-linux)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
endif(WIN32)

# ARCH
if (BUILD_X64)
  set(PrefixAMX    z1)
  set(PrefixAVX512 z0)
  set(PrefixAVX2   l9)
  set(PrefixAVX    e9)
  set(PrefixSSE    h8)
else()
  set(PrefixAMX    x1)
  set(PrefixAVX512 x0)
  set(PrefixAVX2   s9)
  set(PrefixAVX    g9)
  set(PrefixSSE    n8)
endif()

# clang
set(CLANG_LLVM_INCLUDE_DIRS)
string(REPLACE \${CMAKE_CFG_INTDIR} ${CMAKE_CFG_INTDIR} LLVM_INCLUDE_DIRS
       "${LLVM_INCLUDE_DIRS}")
foreach(LLVM_INCLUDE_DIR_ITEM ${LLVM_INCLUDE_DIRS} )
  set(CLANG_LLVM_INCLUDE_DIRS ${CLANG_LLVM_INCLUDE_DIRS}
      -I ${LLVM_INCLUDE_DIR_ITEM})
endforeach( LLVM_INCLUDE_DIR_ITEM)

# llc parameters
set (LLC_PARAMS -filetype=obj)
if (BUILD_X64)
  set (LLC_PARAMS ${LLC_PARAMS} -code-model=large -relocation-model=pic)
else()
  set (LLC_PARAMS ${LLC_PARAMS} -relocation-model=static)
endif()

# options for clang to compile OpenCL 2.0 code
set(CLANG_CL_COMMON_FLAGS
  -cc1 -triple ${DEVICE_TRIPLE} -emit-llvm-bc -fwrapv -O3
  -disable-intel-proprietary-opts
  -x cl -I ${IMPORT_DIR} -I ${CLANG_HEADERS} -I ${CLANG_OPENCL_HEADERS_DIR}
  ${CLANG_LLVM_INCLUDE_DIRS} -include opencl-c.h
  -cl-std=CL2.0 -D __OPENCL_C_VERSION__=200 -ffake-address-space-map
)

# llvm-link parameters
set(LLVM_LINK_PARAMS -suppress-warnings)

add_subdirectory(kernel       "${CMAKE_CURRENT_BINARY_DIR}/kernel")
add_subdirectory(ocl_builtins "${CMAKE_CURRENT_BINARY_DIR}/ocl_builtins")
