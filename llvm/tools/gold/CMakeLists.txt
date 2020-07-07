set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/gold.exports)

if( LLVM_ENABLE_PIC AND LLVM_BINUTILS_INCDIR )
  include_directories( ${LLVM_BINUTILS_INCDIR} )

  set(LLVM_LINK_COMPONENTS
     ${LLVM_TARGETS_TO_BUILD}
     Linker
     LTO
     BitWriter
     IPO
     )

  add_llvm_library(LLVMgold MODULE
    gold-plugin.cpp
    )

  if(INTEL_CUSTOMIZATION)
    # CMPLRLLVM-20054: icx-lto is a release name for LLVMgold plugin.
    #
    # LLVMgold is only used for LIT testing. We have to keep LLVMgold,
    # since we do not want to modify LIT tests that hard-code LLVMgold
    # plugin name, and deal with conflicts and test fails after llorg
    # pulldowns.
    add_llvm_library(icx-lto MODULE
      gold-plugin.cpp
      )
  endif(INTEL_CUSTOMIZATION)
endif()
