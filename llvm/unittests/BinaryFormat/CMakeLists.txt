set(LLVM_LINK_COMPONENTS
  BinaryFormat
  TargetParser
  )

add_llvm_unittest(BinaryFormatTests
  DwarfTest.cpp
# INTEL_CUSTOMIZATION
  Intel_TraceTest.cpp
# end INTEL_CUSTOMIZATION
  MachOTest.cpp
  MsgPackDocumentTest.cpp
  MsgPackReaderTest.cpp
  MsgPackWriterTest.cpp
  TestFileMagic.cpp
  )

