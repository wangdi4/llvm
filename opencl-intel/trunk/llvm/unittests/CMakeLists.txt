#  LLVM Unittests files

# Settings to link google test library
add_definitions(-I${CMAKE_SOURCE_DIR}/utils/unittest/googletest/include -DGTEST_HAS_RTTI=0 -DGTEST_HAS_TR1_TUPLE=0)
set(LLVM_LIBS_FOR_UNIT_TESTS LLVMSupport LLVMSelectionDAG LLVMTarget)
set(UNIT_TEST_LIBS GoogleTest UnitTestMain ${LLVM_LIBS_FOR_UNIT_TESTS})

if( LLVM_BUILD_VERIFICATION_LIB )
  add_subdirectory(Validation)
endif( LLVM_BUILD_VERIFICATION_LIB )

add_llvm_executable(LLVMUnitTests
   VMCore/ConstantsTest.cpp
   VMCore/DerivedTypesTest.cpp
   VMCore/MetadataTest.cpp
   VMCore/PassManagerTest.cpp
   VMCore/VerifierTest.cpp
   Transforms/Utils/Cloning.cpp
   Support/AllocatorTest.cpp
   Support/CommandLineTest.cpp
   Support/ConstantRangeTest.cpp
   Support/LeakDetectorTest.cpp
   Support/MathExtrasTest.cpp
   Support/raw_ostream_test.cpp
   Support/RegexTest.cpp
   Support/System.cpp
   Support/TypeBuilderTest.cpp
   Support/ValueHandleTest.cpp
   ExecutionEngine/ExecutionEngineTest.cpp
   ExecutionEngine/JIT/JITEventListenerTest.cpp
   ExecutionEngine/JIT/JITMemoryManagerTest.cpp
   ExecutionEngine/JIT/JITTest.cpp
   ExecutionEngine/JIT/MultiJITTest.cpp
   ADT/APFloatTest.cpp
   ADT/APIntTest.cpp
   ADT/BitVectorTest.cpp
   ADT/DeltaAlgorithmTest.cpp
   ADT/DenseMapTest.cpp
   ADT/DenseSetTest.cpp
   ADT/ImmutableSetTest.cpp
   ADT/SmallBitVectorTest.cpp
   ADT/SmallStringTest.cpp
   ADT/SmallVectorTest.cpp
   ADT/SparseBitVectorTest.cpp
   ADT/StringMapTest.cpp
   ADT/StringRefTest.cpp
   ADT/TripleTest.cpp
   ADT/TwineTest.cpp
   ADT/ValueMapTest.cpp
  )
  
  include_directories(${CMAKE_SOURCE_DIR}/../llvm/lib/VMCore)
  include_directories(${CMAKE_SOURCE_DIR}/../build/Win32/include)
  
  set(LLVM_UT_LIBS LLVMCore LLVMJIT LLVMTarget LLVMAnalysis LLVMipa LLVMExecutionEngine LLVMInterpreter LLVMX86CodeGen LLVMAsmPrinter LLVMCodeGen LLVMAsmParser LLVMBitReader LLVMMC LLVMX86Info LLVMBitWriter)
  
  target_link_libraries(LLVMUnitTests ${LLVM_UT_LIBS} ${UNIT_TEST_LIBS})
  add_dependencies(LLVMUnitTests ${LLVM_UT_LIBS})


