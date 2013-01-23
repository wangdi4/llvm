/****************************************************************************
  Copyright (c) Intel Corporation (2012,2013).

  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
  LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
  including liability for infringement of any proprietary rights, relating to
  use of the code. No license, express or implied, by estoppels or otherwise,
  to any intellectual property rights is granted herein.

  File Name: TestTypeConversion.cpp
\****************************************************************************/

#include "gtest/gtest.h"
#include "llvm/LLVMContext.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"
#include "TypeConversion.h"

llvm::LLVMContext CTX;

const char *MY_STRUCT = "myStruct";

llvm::Type *LLVM_I64_TY = llvm::IntegerType::get(CTX, 64);
llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
llvm::Type *LLVM_I16_TY = llvm::IntegerType::get(CTX, 16);
llvm::Type *LLVM_I8_TY = llvm::IntegerType::get(CTX, 8);
llvm::Type *LLVM_I1_TY = llvm::IntegerType::get(CTX, 1);
llvm::Type *LLVM_D_Ty  = llvm::Type::getDoubleTy(CTX);
llvm::Type *LLVM_F_Ty  = llvm::Type::getFloatTy(CTX);
llvm::Type *LLVM_H_Ty  = llvm::Type::getHalfTy(CTX);
llvm::Type *LLVM_V_Ty  = llvm::Type::getVoidTy(CTX);
llvm::Type *LLVM_V4i32_TY = llvm::VectorType::get(LLVM_I32_TY, 4U);
llvm::Type *LLVM_pi32_TY  = llvm::PointerType::get(LLVM_I32_TY, 0U);
llvm::Type *LLVM_pi32AS1_TY  = llvm::PointerType::get(LLVM_I32_TY, 1U);

TEST(ReflectionToLLVM, i64){
  reflection::Type longTy(reflection::primitives::LONG);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &longTy);

  ASSERT_EQ(llvmITy, LLVM_I64_TY);
}

TEST(ReflectionToLLVM, i32){
  reflection::Type intTy(reflection::primitives::INT);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &intTy);

  ASSERT_EQ(llvmITy, LLVM_I32_TY);
}

TEST(ReflectionToLLVM, i16){
  reflection::Type shortTy(reflection::primitives::SHORT);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &shortTy);
  
  ASSERT_EQ(llvmITy, LLVM_I16_TY);
}

TEST(ReflectionToLLVM, i8){
  reflection::Type charTy(reflection::primitives::UCHAR);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &charTy);
  
  ASSERT_EQ(llvmITy, LLVM_I8_TY);
}

TEST(ReflectionToLLVM, i1){
  reflection::Type bTy(reflection::primitives::BOOL);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &bTy);
  
  ASSERT_EQ(llvmITy, LLVM_I1_TY);
}

TEST(ReflectionToLLVM, VectorOfInt){
  reflection::Type iTy(reflection::primitives::INT);
  reflection::Vector vTy(&iTy, 4);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &vTy);
  
  ASSERT_EQ(llvmITy, LLVM_V4i32_TY);
}

TEST(ReflectionToLLVM, PointerToInt){
  reflection::Type iTy(reflection::primitives::INT);
  reflection::Pointer pTy(&iTy);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &pTy);
  
  ASSERT_EQ(llvmITy, LLVM_pi32_TY);
}

TEST(ReflectionToLLVM, PointerAS1ToInt){
  reflection::Type iTy(reflection::primitives::INT);
  reflection::Pointer pTy(&iTy);
  pTy.addAttribute("__global");
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, &pTy);
  
  ASSERT_EQ(llvmITy, LLVM_pi32AS1_TY);
}

TEST(ReflectionToLLVM, Struct){
  reflection::UserDefinedTy sTy(MY_STRUCT);
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, &sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ(MY_STRUCT,Name.data());
}

TEST(ReflectionToLLVM, Float){
  reflection::Type FTy(reflection::primitives::FLOAT);
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, &FTy);
  ASSERT_EQ(LLVM_F_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Double){
  reflection::Type DTy(reflection::primitives::DOUBLE);
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, &DTy);
  ASSERT_EQ(LLVM_D_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Half){
  reflection::Type HTy(reflection::primitives::HALF);
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, &HTy);
  ASSERT_EQ(LLVM_H_Ty, llvmTy);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
