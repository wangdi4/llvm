// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "OCLBuiltinParser.h"
#include "Exception.h"
#include "gtest_wrapper.h"

using namespace llvm;
using namespace Validation;

TEST(OCLBuiltinParser, BuiltinDetection) {
  OCLBuiltinParser::ArgVector args;
  std::string BIStr;
  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z12native_rsqrtDv4_f", BIStr, args);

  EXPECT_EQ("native_rsqrt", BIStr);
  EXPECT_EQ(1u, args.size());
  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[0].genType);
  EXPECT_EQ(4u, args[0].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[0].vecType.elType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z11read_imagefP10_image2d_tjDv2_i", BIStr,
                                    args);
  EXPECT_EQ("read_imagef", BIStr);
  EXPECT_EQ(3U, args.size());

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[0].genType);
  EXPECT_EQ("_image2d_t *", args[0].ptrType.ptrToStr);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::UINT, args[1].basicType);

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[2].genType);
  EXPECT_EQ(2U, args[2].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::INT, args[2].vecType.elType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z3minss", BIStr, args);
  EXPECT_EQ("min", BIStr);
  EXPECT_EQ(2U, args.size());

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[0].genType);
  EXPECT_EQ(OCLBuiltinParser::SHORT, args[0].basicType);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::SHORT, args[1].basicType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z6vload4jPU3AS1Kf", BIStr, args);
  EXPECT_EQ("vload4", BIStr);
  EXPECT_EQ(2U, args.size());

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[0].genType);
  EXPECT_EQ(OCLBuiltinParser::UINT, args[0].basicType);

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[1].genType);
  EXPECT_EQ("const __global float *", args[1].ptrType.ptrToStr);
  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[1].ptrType.ptrType[0].basicType);
  EXPECT_EQ(true, args[1].ptrType.isAddrSpace);
  EXPECT_EQ(OCLBuiltinParser::GLOBAL, args[1].ptrType.AddrSpace);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z7vstore8Dv8_tjPt", BIStr, args);
  EXPECT_EQ("vstore8", BIStr);
  EXPECT_EQ(3U, args.size());

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[0].genType);
  EXPECT_EQ(8U, args[0].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::USHORT, args[0].vecType.elType);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::UINT, args[1].basicType);

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[2].genType);
  EXPECT_EQ(OCLBuiltinParser::BASIC, args[2].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::USHORT, args[2].ptrType.ptrType[0].basicType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z5clampDv4_fff", BIStr, args);
  EXPECT_EQ("clamp", BIStr);
  EXPECT_EQ(3U, args.size());

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[0].genType);
  EXPECT_EQ(4U, args[0].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[0].vecType.elType);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[1].basicType);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[2].genType);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[2].basicType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z3mixDv4_fS_S_", BIStr, args);
  EXPECT_EQ("mix", BIStr);
  EXPECT_EQ(3U, args.size());

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[0].genType);
  EXPECT_EQ(4U, args[0].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[0].vecType.elType);

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[1].genType);
  EXPECT_EQ(4U, args[1].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[1].vecType.elType);

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[2].genType);
  EXPECT_EQ(4U, args[2].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[2].vecType.elType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z10atomic_addPU3AS1Vii", BIStr, args);
  EXPECT_EQ("atomic_add", BIStr);
  EXPECT_EQ(2U, args.size());

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[0].genType);
  EXPECT_EQ(OCLBuiltinParser::BASIC, args[0].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::INT, args[0].ptrType.ptrType[0].basicType);
  EXPECT_EQ(true, args[0].ptrType.isAddrSpace);
  EXPECT_EQ(OCLBuiltinParser::GLOBAL, args[0].ptrType.AddrSpace);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::INT, args[1].basicType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin(
      "_Z21async_work_group_copyPU3AS3cPU3AS1Kcjj", BIStr, args);
  EXPECT_EQ("async_work_group_copy", BIStr);
  EXPECT_EQ(4U, args.size());

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[0].genType);
  EXPECT_EQ(OCLBuiltinParser::BASIC, args[0].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::CHAR, args[0].ptrType.ptrType[0].basicType);
  EXPECT_EQ(true, args[0].ptrType.isAddrSpace);
  EXPECT_EQ(OCLBuiltinParser::LOCAL, args[0].ptrType.AddrSpace);

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::BASIC, args[1].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::CHAR, args[1].ptrType.ptrType[0].basicType);
  EXPECT_EQ(true, args[1].ptrType.isAddrSpace);
  EXPECT_EQ(OCLBuiltinParser::GLOBAL, args[1].ptrType.AddrSpace);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[2].genType);
  EXPECT_EQ(OCLBuiltinParser::UINT, args[2].basicType);

  EXPECT_EQ(OCLBuiltinParser::BASIC, args[3].genType);
  EXPECT_EQ(OCLBuiltinParser::UINT, args[3].basicType);

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z15get_image_widthP10_image3d_t", BIStr,
                                    args);
  EXPECT_EQ("get_image_width", BIStr);
  EXPECT_EQ(1U, args.size());

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[0].genType);
  EXPECT_EQ("_image3d_t *", args[0].ptrType.ptrToStr);
  EXPECT_EQ("_image3d_t", args[0].ptrType.ptrType[0].imgType.imgStr);
}

TEST(OCLBuiltinParser, BuiltinDetectionNegative) {
  OCLBuiltinParser::ArgVector args;
  std::string BIStr;
  bool res;

  ///////////////////////////////////////////////////////////////////////////////////////
  // no _Z{num} prefix in function
  res = OCLBuiltinParser::ParseOCLBuiltin("Z3mixU8__vector4fS_S_", BIStr, args);
  EXPECT_FALSE(res);

  ///////////////////////////////////////////////////////////////////////////////////////

  // unknown U78__vector subtype of arguments
  EXPECT_THROW(
      OCLBuiltinParser::ParseOCLBuiltin("_Z3mixU78__vector4fS_S_", BIStr, args),
      Exception::InvalidArgument);
  // too much length in builtin name
  EXPECT_THROW(OCLBuiltinParser::ParseOCLBuiltin("_Z20sxsdaa", BIStr, args),
               Exception::InvalidArgument);
  // too much length in pointer type name
  EXPECT_THROW(
      OCLBuiltinParser::ParseOCLBuiltin("_Z3sxsP10struct", BIStr, args),
      Exception::InvalidArgument);
  // unknown data type
  EXPECT_THROW(OCLBuiltinParser::ParseOCLBuiltin("_Z3sxsG", BIStr, args),
               Exception::InvalidArgument);
  // unknown data type
  EXPECT_THROW(OCLBuiltinParser::ParseOCLBuiltin("_Z3sxsiG", BIStr, args),
               Exception::InvalidArgument);
  // array with unknown element type
  EXPECT_THROW(OCLBuiltinParser::ParseOCLBuiltin("_Z3sxsA32_G", BIStr, args),
               Exception::InvalidArgument);
  // substitution cannot be first argument
  // EXPECT_THROW(OCLBuiltinParser::ParseOCLBuiltin("_Z3sxsS_", BIStr, args),
  // Exception::InvalidArgument);
}

// OCLBuiltinParser fails parsing builtin
TEST(OCLBuiltinParser, BuiltinDetectionBug13Mar11) {

  OCLBuiltinParser::ArgVector args;
  std::string BIStr;

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z6sincosDv2_fPS_", BIStr, args);
  EXPECT_EQ("sincos", BIStr);
  EXPECT_EQ(2U, args.size());

  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[0].genType);
  EXPECT_EQ(2U, args[0].vecType.elNum);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[0].vecType.elType);

  EXPECT_EQ(OCLBuiltinParser::POINTER, args[1].genType);
  EXPECT_EQ(OCLBuiltinParser::VECTOR, args[1].ptrType.ptrType[0].genType);
  EXPECT_EQ(OCLBuiltinParser::FLOAT, args[1].ptrType.ptrType[0].vecType.elType);
  EXPECT_EQ(2U, args[1].ptrType.ptrType[0].vecType.elNum);
}

// OCLBuiltinParser fails parsing builtin
TEST(OCLBuiltinParser, BuiltinDetectionBug03Oct12) {

  OCLBuiltinParser::ArgVector args;
  std::string BIStr;

  ///////////////////////////////////////////////////////////////////////////////////////
  OCLBuiltinParser::ParseOCLBuiltin("_Z15get_image_width9image2d_t", BIStr,
                                    args);
  EXPECT_EQ("get_image_width", BIStr);

  EXPECT_EQ(OCLBuiltinParser::IMAGE, args[0].genType);
  EXPECT_EQ("image2d_t", args[0].imgType.imgStr);
}

TEST(OCLBuiltinParser, BuiltinMangling) {
  // char2 vload2(size_t offset, const __global char *p);
  std::string bltName("vload2");
  OCLBuiltinParser::ArgVector args;

  // first argument offset - unsigned int.
  OCLBuiltinParser::ARG offset;
  offset.genType = OCLBuiltinParser::BASIC;
  offset.basicType = OCLBuiltinParser::UINT;
  args.push_back(offset);

  // second argument pointer - const __global char*.
  OCLBuiltinParser::ARG p;
  p.genType = OCLBuiltinParser::POINTER;
  p.ptrType.isAddrSpace = true;
  p.ptrType.AddrSpace = OCLBuiltinParser::GLOBAL;
  p.ptrType.isPointsToConst = true;
  OCLBuiltinParser::ARG ptrT;
  ptrT.genType = OCLBuiltinParser::BASIC;
  ptrT.basicType = OCLBuiltinParser::CHAR;
  p.ptrType.ptrType.push_back(ptrT);
  args.push_back(p);

  std::string mangledName;
  OCLBuiltinParser::GetOCLMangledName(bltName, args, mangledName);
  EXPECT_EQ("_Z6vload2jPU3AS1Kc", mangledName);
}
