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
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"
#include "TypeConversion.h"

TEST(ReflectionToLLVM, i64){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I64_TY = llvm::IntegerType::get(CTX, 64);
  reflection::RefParamType longTy(new reflection::PrimitiveType(reflection::PRIMITIVE_LONG));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, longTy);

  ASSERT_EQ(llvmITy, LLVM_I64_TY);
}

TEST(ReflectionToLLVM, i32){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  reflection::RefParamType intTy(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, intTy);

  ASSERT_EQ(llvmITy, LLVM_I32_TY);
}

TEST(ReflectionToLLVM, i16){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I16_TY = llvm::IntegerType::get(CTX, 16);
  reflection::RefParamType shortTy(new reflection::PrimitiveType(reflection::PRIMITIVE_SHORT));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, shortTy);
  
  ASSERT_EQ(llvmITy, LLVM_I16_TY);
}

TEST(ReflectionToLLVM, i8){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I8_TY = llvm::IntegerType::get(CTX, 8);
  reflection::RefParamType charTy(new reflection::PrimitiveType(reflection::PRIMITIVE_UCHAR));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, charTy);
  
  ASSERT_EQ(llvmITy, LLVM_I8_TY);
}

TEST(ReflectionToLLVM, i1){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I1_TY = llvm::IntegerType::get(CTX, 1);
  reflection::RefParamType bTy(new reflection::PrimitiveType(reflection::PRIMITIVE_BOOL));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, bTy);
  
  ASSERT_EQ(llvmITy, LLVM_I1_TY);
}

TEST(ReflectionToLLVM, VectorOfInt){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_V4i32_TY = llvm::VectorType::get(LLVM_I32_TY, 4U);
  reflection::RefParamType iTy(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::RefParamType vTy(new reflection::VectorType(iTy, 4));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, vTy);
  
  ASSERT_EQ(llvmITy, LLVM_V4i32_TY);
}

TEST(ReflectionToLLVM, PointerToInt){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_pi32_TY  = llvm::PointerType::get(LLVM_I32_TY, 0U);
  reflection::RefParamType iTy(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::RefParamType pTy(new reflection::PointerType(iTy));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, pTy);
  
  ASSERT_EQ(llvmITy, LLVM_pi32_TY);
}

TEST(ReflectionToLLVM, PointerAS1ToInt){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_pi32AS1_TY  = llvm::PointerType::get(LLVM_I32_TY, 1U);
  reflection::RefParamType iTy(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::PointerType* pointerTy = new reflection::PointerType(iTy);
  pointerTy->addAttribute(reflection::ATTR_GLOBAL);
  reflection::RefParamType pTy(pointerTy);
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, pTy);
  
  ASSERT_EQ(llvmITy, LLVM_pi32AS1_TY);
}

TEST(ReflectionToLLVM, Struct){
  llvm::LLVMContext CTX;
  const char MY_STRUCT[] = "myStruct";
  std::string str(MY_STRUCT);
  reflection::RefParamType sTy(new reflection::UserDefinedType(str));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ(MY_STRUCT, Name.data());
}

TEST(ReflectionToLLVM, Float){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_F_Ty  = llvm::Type::getFloatTy(CTX);
  reflection::RefParamType FTy(new reflection::PrimitiveType(reflection::PRIMITIVE_FLOAT));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, FTy);
  ASSERT_EQ(LLVM_F_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Double){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_D_Ty  = llvm::Type::getDoubleTy(CTX);
  reflection::RefParamType DTy(new reflection::PrimitiveType(reflection::PRIMITIVE_DOUBLE));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, DTy);
  ASSERT_EQ(LLVM_D_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Half){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_H_Ty  = llvm::Type::getHalfTy(CTX);
  reflection::RefParamType HTy(new reflection::PrimitiveType(reflection::PRIMITIVE_HALF));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, HTy);
  ASSERT_EQ(LLVM_H_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Image1d){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_t", Name.data());
}

TEST(ReflectionToLLVM, Image2d){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_2D_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_t", Name.data());
}

TEST(ReflectionToLLVM, Image3d){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_3D_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image3d_t", Name.data());
}

TEST(ReflectionToLLVM, Image1dbuffer){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_BUFFER_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_buffer_t", Name.data());
}

TEST(ReflectionToLLVM, Image1darray){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_ARRAY_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_array_t", Name.data());
}

TEST(ReflectionToLLVM, Image2darray){
  llvm::LLVMContext CTX;

  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_2D_ARRAY_T));
  llvm::Type* llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_array_t", Name.data());
}

TEST(ReflectionToLLVM, Sampler){
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  reflection::RefParamType sTy(new reflection::PrimitiveType(reflection::PRIMITIVE_SAMPLER_T));
  llvm::Type* llvmITy = intel::reflectionToLLVM(CTX, sTy);
  
  ASSERT_EQ(llvmITy, LLVM_I32_TY);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
