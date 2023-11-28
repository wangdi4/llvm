// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "TypeConversion.h"
#include "gtest_wrapper.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

using namespace llvm;

TEST(ReflectionToLLVM, i64) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I64_TY = llvm::IntegerType::get(CTX, 64);
  reflection::RefParamType longTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_LONG));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, longTy);
  ASSERT_EQ(llvmITy, LLVM_I64_TY);
}

TEST(ReflectionToLLVM, i32) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  reflection::RefParamType intTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, intTy);
  ASSERT_EQ(llvmITy, LLVM_I32_TY);
}

TEST(ReflectionToLLVM, i16) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I16_TY = llvm::IntegerType::get(CTX, 16);
  reflection::RefParamType shortTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_SHORT));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, shortTy);
  ASSERT_EQ(llvmITy, LLVM_I16_TY);
}

TEST(ReflectionToLLVM, i8) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I8_TY = llvm::IntegerType::get(CTX, 8);
  reflection::RefParamType charTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_UCHAR));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, charTy);
  ASSERT_EQ(llvmITy, LLVM_I8_TY);
}

TEST(ReflectionToLLVM, i1) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I1_TY = llvm::IntegerType::get(CTX, 1);
  reflection::RefParamType bTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_BOOL));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, bTy);
  ASSERT_EQ(llvmITy, LLVM_I1_TY);
}

TEST(ReflectionToLLVM, VectorOfInt) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_V4i32_TY = llvm::FixedVectorType::get(LLVM_I32_TY, 4U);
  reflection::RefParamType iTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::RefParamType vTy(new reflection::VectorType(iTy, 4));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, vTy);
  ASSERT_EQ(llvmITy, LLVM_V4i32_TY);
}

TEST(ReflectionToLLVM, PointerToInt) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_pi32_TY = llvm::PointerType::get(LLVM_I32_TY, 0U);
  reflection::RefParamType iTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::RefParamType pTy(new reflection::PointerType(iTy));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, pTy);
  ASSERT_EQ(llvmITy, LLVM_pi32_TY);
}

TEST(ReflectionToLLVM, PointerAS1ToInt) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  llvm::Type *LLVM_pi32AS1_TY = llvm::PointerType::get(LLVM_I32_TY, 1U);
  reflection::RefParamType iTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
  reflection::PointerType *pointerTy = new reflection::PointerType(iTy);
  pointerTy->addAttribute(reflection::ATTR_GLOBAL);
  reflection::RefParamType pTy(pointerTy);
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, pTy);
  ASSERT_EQ(llvmITy, LLVM_pi32AS1_TY);
}

TEST(ReflectionToLLVM, Struct) {
  llvm::LLVMContext CTX;
  const char MY_STRUCT[] = "myStruct";
  std::string str(MY_STRUCT);
  reflection::RefParamType sTy(new reflection::UserDefinedType(str));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ(MY_STRUCT, Name.data());
}

TEST(ReflectionToLLVM, Float) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_F_Ty = llvm::Type::getFloatTy(CTX);
  reflection::RefParamType FTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_FLOAT));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, FTy);
  ASSERT_EQ(LLVM_F_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Double) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_D_Ty = llvm::Type::getDoubleTy(CTX);
  reflection::RefParamType DTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_DOUBLE));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, DTy);
  ASSERT_EQ(LLVM_D_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Half) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_H_Ty = llvm::Type::getHalfTy(CTX);
  reflection::RefParamType HTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_HALF));
  llvm::Type *llvmTy = intel::reflectionToLLVM(CTX, HTy);
  ASSERT_EQ(LLVM_H_Ty, llvmTy);
}

TEST(ReflectionToLLVM, Image1d) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_t", Name.data());
}

TEST(ReflectionToLLVM, Image2d) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_2D_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_t", Name.data());
}

TEST(ReflectionToLLVM, Image2ddepth) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_2D_DEPTH_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_depth_t", Name.data());
}

TEST(ReflectionToLLVM, Image3d) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_3D_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image3d_t", Name.data());
}

TEST(ReflectionToLLVM, Image1dbuffer) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_BUFFER_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_buffer_t", Name.data());
}

TEST(ReflectionToLLVM, Image1darray) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_1D_ARRAY_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image1d_array_t", Name.data());
}

TEST(ReflectionToLLVM, Image2darray) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_IMAGE_2D_ARRAY_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);

  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_array_t", Name.data());
}

TEST(ReflectionToLLVM, Image2darraydepth) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(new reflection::PrimitiveType(
      reflection::PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.image2d_array_depth_t", Name.data());
}

TEST(ReflectionToLLVM, Event) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_EVENT_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.event_t", Name.data());
}

TEST(ReflectionToLLVM, ClkEvent) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_CLK_EVENT_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.clk_event_t", Name.data());
}

TEST(ReflectionToLLVM, Queue) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_QUEUE_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.queue_t", Name.data());
}

TEST(ReflectionToLLVM, Pipe) {
  llvm::LLVMContext CTX;
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_PIPE_RO_T));
  llvm::Type *llvmSTy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_TRUE(llvmSTy->isStructTy());
  llvm::StructType *STy = llvm::dyn_cast<llvm::StructType>(llvmSTy);
  llvm::StringRef Name = STy->getName();
  ASSERT_STREQ("opencl.pipe_ro_t", Name.data());

  reflection::RefParamType sTy2(
      new reflection::PrimitiveType(reflection::PRIMITIVE_PIPE_WO_T));
  llvm::Type *llvmSTy2 = intel::reflectionToLLVM(CTX, sTy2);
  ASSERT_TRUE(llvmSTy2->isStructTy());
  llvm::StructType *STy2 = llvm::dyn_cast<llvm::StructType>(llvmSTy2);
  llvm::StringRef Name2 = STy2->getName();
  ASSERT_STREQ("opencl.pipe_wo_t", Name2.data());
}

TEST(ReflectionToLLVM, Sampler) {
  llvm::LLVMContext CTX;
  llvm::Type *LLVM_I32_TY = llvm::IntegerType::get(CTX, 32);
  reflection::RefParamType sTy(
      new reflection::PrimitiveType(reflection::PRIMITIVE_SAMPLER_T));
  llvm::Type *llvmITy = intel::reflectionToLLVM(CTX, sTy);
  ASSERT_EQ(llvmITy, LLVM_I32_TY);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
