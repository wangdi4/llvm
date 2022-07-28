set(LLVM_LINK_COMPONENTS
  Analysis
  AsmParser
  BitWriter
  Core
  Support
  TransformUtils
  Passes
  Vectorize
  )

add_llvm_unittest(UtilsTests
  ASanStackFrameLayoutTest.cpp
  BasicBlockUtilsTest.cpp
  CallPromotionUtilsTest.cpp
  CloningTest.cpp
  CodeExtractorTest.cpp
  CodeMoverUtilsTest.cpp
  DebugifyTest.cpp
  FunctionComparatorTest.cpp
  IntegerDivisionTest.cpp
# INTEL_CUSTOMIZATION
  Intel_IMLUtilsTest.cpp
# end INTEL_CUSTOMIZATION
  LocalTest.cpp
  LoopRotationUtilsTest.cpp
  LoopUtilsTest.cpp
  MemTransferLowering.cpp
  ModuleUtilsTest.cpp
  ScalarEvolutionExpanderTest.cpp
  SizeOptsTest.cpp
  SSAUpdaterBulkTest.cpp
  UnrollLoopTest.cpp
  ValueMapperTest.cpp
  VFABIUtils.cpp
<<<<<<< HEAD
# INTEL_COLLAB
  VPOUtilsTest.cpp
# end INTEL_COLLAB
=======
  ProfDataUtilTest.cpp
>>>>>>> 6047deb7c2aa94d9bc2b70b49799d22cce778bd4
  )

set_property(TARGET UtilsTests PROPERTY FOLDER "Tests/UnitTests/TransformsTests")
