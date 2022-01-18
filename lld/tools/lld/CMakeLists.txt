set(LLVM_LINK_COMPONENTS
  Support
  )

add_lld_tool(lld
  lld.cpp

  SUPPORT_PLUGINS
  )
export_executable_symbols_for_plugins(lld)

target_link_libraries(lld
  PRIVATE
  lldCommon
  lldCOFF
  lldELF
# INTEL_CUSTOMIZATION
# Disable unused functionality to speed up builds.
#  lldMachO
#  lldMinGW
#  lldWasm
# end INTEL_CUSTOMIZATION
  )

install(TARGETS lld
  RUNTIME DESTINATION bin)

if(NOT LLD_SYMLINKS_TO_CREATE)
  set(LLD_SYMLINKS_TO_CREATE
      lld-link ld.lld ld64.lld wasm-ld)
endif()

if(INTEL_CUSTOMIZATION)
  # All LLD symlinks are installed as part of LLD component.
  # We do not need to deploy darwin and wasm symlinks.
  set(INTEL_LLD_SYMLINKS_TO_DELETE ld64.lld ld64.lld.darwinnew wasm-ld)
  foreach(link ${INTEL_LLD_SYMLINKS_TO_DELETE})
    list(REMOVE_ITEM LLD_SYMLINKS_TO_CREATE ${link})
  endforeach()
endif(INTEL_CUSTOMIZATION)

foreach(link ${LLD_SYMLINKS_TO_CREATE})
  add_lld_symlink(${link} lld)
endforeach()
