foreach(t ${LLVM_TARGETS_TO_BUILD})
  if(INTEL_CUSTOMIZATION)
# INTEL_FEATURE_CSA
    if("${t}" STREQUAL "CSA")
      is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
      if(p)
        add_subdirectory(Intel_${t})
        continue()
      else()
        message(FATAL_ERROR
          "Trying to build CSA target without CSA enabled in "
          "LLVM_INTEL_FEATURES.")
      endif()
    endif()
# end INTEL_FEATURE_CSA
  endif(INTEL_CUSTOMIZATION)
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${t})
    add_subdirectory(${t})
  endif()
endforeach()

set(LLVM_LINK_COMPONENTS Core Target AllTargetsCodeGens AllTargetsDescs AllTargetsInfos)

add_llvm_unittest(TargetMachineCTests
  TargetMachineOptionsTest.cpp
)

set_property(TARGET TargetMachineCTests
  PROPERTY FOLDER "Tests/UnitTests/TargetTests")
