/*******************************************************************************
 * INTEL CONFIDENTIAL
 * Copyright 1996 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted  materials, and
 * your use of  them is  governed by the  express license  under which  they
 *were provided to you (License).  Unless the License provides otherwise, you
 *may not use, modify, copy, publish, distribute,  disclose or transmit this
 *software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents  are provided as  is,  with no
 *express or implied  warranties,  other  than those  that are  expressly stated
 *in the License.
 *******************************************************************************/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_i0_d_ep {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall8 = {0x3d06502454112921UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall7 = {0x3d8519f65c1be795UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall6 = {0x3e002ea7f6336590UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall5 = {0x3e723455dc769179UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall4 = {0x3edc71c71fb137afUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall3 = {0x3f3c71c71c6824ccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall2 = {0x3f90000000000e2aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall1 = {0x3fcfffffffffffc1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___csmall0 = {0x3ff0000000000000UL};
// |x|<=12.0
// minimax(BesselI(0, sqrt(x)), x = 0. .. 12.0^2, [17, 0], 1/BesselI(0,
// sqrt(x)), 'me')
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_17 = {0x37d7d83369436cb8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_17l = {0xb473b8d777bd007aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_16 = {0x384b36771a8456c1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_16l = {0xb4e94dcd8bc23f4bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_15 = {0x390bb580c64b2ecbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_15l = {0xb5a4a45222bafedeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_14 = {0x39a2d0cfa8bcd30dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_14l = {0x3637141f1f9f9d39UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_13 = {0x3a3ec28f4ec3f929UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_13l = {0x36ca6e35b0801cf4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_12 = {0x3ad410f53517c4c2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_12l = {0x377ab91d8ca05d29UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_11 = {0x3b669dff2b936bf7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_11l = {0xb80fa2eef367e6dbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_10 = {0x3bf55ff0e9095d9aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_10l = {0x3887d697710cd6d9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_9 = {0x3c80b3168f520a3fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_9l = {0x38ff8077a8704c2eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_8 = {0x3d0522a3d1dd9c75UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_8l = {0x39a4c8fd950c9e01UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_7 = {0x3d8522a4494e58e7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_7l = {0xba27499b3e3da80dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_6 = {0x3e002e85bfe90145UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_6l = {0xba8f416fad33a992UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_5 = {0x3e72345678a85f56UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_5l = {0xbb1952cf1cef4754UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_4 = {0x3edc71c71c7050a5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_4l = {0xbb6be7a2af6a8856UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_3 = {0x3f3c71c71c71de3dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_3l = {0x3bdab29c3767a465UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_2 = {0x3f8ffffffffffe97UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_2l = {0xbc14cedec48a206aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_1 = {0x3fd0000000000008UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_1l = {0x3c50ca92e834d05aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_0 = {0x3ff0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c0_0l = {0xbc8bd4452ea5c4d2UL};
// coefficients for i0(x)/exp(x)*2^64
// 12.0 .. 42.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_30 = {0x3ae32ccdcda0db3dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_30l = {0x374b921edc20d9e8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_29 = {0xbb7f3ccc7b98f7bdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_29l = {0xb81b2ee04e08058eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_28 = {0x3c087e9b8b8642e6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_28l = {0xb896d9a1e207f9e3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_27 = {0xbc889fd6ed970861UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_27l = {0xb8fd1cedc3e48612UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_26 = {0x3d01d445abb726d6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_26l = {0x397b8db52c18b1d0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_25 = {0xbd73cecb3e9a5b42UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_25l = {0xba04ee65e11033e9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_24 = {0x3de18f8f06934ad9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_24l = {0xba64f226d35c63e9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_23 = {0xbe49847d109b17acUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_23l = {0xbadbbff335c9e685UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_22 = {0x3eaef7ccad85e621UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_22l = {0x3b4a04a98e72d6d1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_21 = {0xbf0fd407e25b1b41UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_21l = {0x3b91a418ef33fcd4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_20 = {0x3f6bfe9d214c2facUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_20l = {0x3c0cc9b837857b95UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_19 = {0xbfc53d129d5710d5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_19l = {0x3c23a637d3b6e185UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_18 = {0x401bf66670c41bceUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_18l = {0x3caa67fd588314eaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_17 = {0xc0700ad5c97fecd4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_17l = {0x3d1e0a726f62faabUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_16 = {0x40c0176379ec1452UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_16l = {0xbd6edd5d3c629bd9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_15 = {0xc10c4638faff0e9fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_15l = {0xbd6a6f1e8b1bc105UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_14 = {0x4155c6db6b4d9aaaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_14l = {0xbdfe2fdaad76a84eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_13 = {0xc19d679bd748b61cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_13l = {0x3e349aee6edd7a47UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_12 = {0x41e1621b282c8012UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_12l = {0x3e7142dec9383301UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_11 = {0xc221f562c2b29a8bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_11l = {0xbec685c61ebbca9dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_10 = {0x4260287a47789379UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_10l = {0x3ed1cba74e44c7d6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_9 = {0xc299352cbeb3e92cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_9l = {0x3f275d566349855fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_8 = {0x42d0f13f06ace3b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_8l = {0xbf6813d6427de3bfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_7 = {0xc30377233a478adbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_7l = {0xbfab07ccf31c6f75UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_6 = {0x4332e9cfc947f9daUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_6l = {0x3fb81076e6cae19aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_5 = {0xc35ea6beab10939aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_5l = {0x3ffa763b5788bfdfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_4 = {0x4384546b2d6b004cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_4l = {0xc02df8aa00eacfb6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_3 = {0xc3a584f465b7516dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_3l = {0xc01918701d04a363UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_2 = {0x43c18dd4d4378c66UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_2l = {0xc0412ece78fcda19UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_1 = {0xc3d52853b1289936UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_1l = {0xc0757cf48c88a8c1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_0 = {0x43e44fb89e1c5100UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c1_0l = {0x4086d1dc8a2221a2UL};
// coefficients for i0(x)/exp(x)*2^64
// 42.0..150.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_30 = {0x375b0d7bb5b6b682UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_30l = {0xb3f8e88c035bde3bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_29 = {0xb8139bddebe3fae3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_29l = {0xb496a9b128f4a4f6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_28 = {0x38bb5d41969faed9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_28l = {0x34f8556214334f69UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_27 = {0xb9587a3a97b5e6caUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_27l = {0x35f23d7b8062be3bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_26 = {0x39ef89a54e369adaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_26l = {0xb684424003ffc5e2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_25 = {0xba7f2c639c96ba28UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_25l = {0x37106d19be975cafUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_24 = {0x3b08969839f26cf2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_24l = {0x37abd0b796255b0dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_23 = {0xbb8fc974cbe95f30UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_23l = {0xb80679ed02e5e9aeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_22 = {0x3c1128eb81c17485UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_22l = {0x38b74fe959c3f946UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_21 = {0xbc8f617e63cf3528UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_21l = {0x3911af60379e5f60UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_20 = {0x3d088e4b43e5cba0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_20l = {0xb9ae0906b6d1cb02UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_19 = {0xbd80932d7d8a43b2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_19l = {0xba19dc7fc78994b2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_18 = {0x3df36aaadcd30a09UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_18l = {0x3a9586fa743b8f76UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_17 = {0xbe63d3273e21566fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_17l = {0xbaf3d4c2f0629d37UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_16 = {0x3ed1b27c2a2f19e3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_16l = {0x3b6a041b8c6894e9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_15 = {0xbf3badd60a90264cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_15l = {0x3bdc97b919261698UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_14 = {0x3fa2faf418ca43b1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_14l = {0xbc3baa64151206d5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_13 = {0xc006d332750a2c8aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_13l = {0x3c713f98bb99b4e9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_12 = {0x40680b5daa06cebdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_12l = {0xbd01ab1dbe739215UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_11 = {0xc0c624b0c463b381UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_11l = {0xbd53632fbcc9d916UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_10 = {0x4121c5c1c2c0552eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_10l = {0xbdc4b2e1a53b7083UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_9 = {0xc178c154ce789240UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_9l = {0xbe126e744115e9a6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_8 = {0x41cdbef951759d5dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_8l = {0x3e48846504dbd172UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_7 = {0xc21e993e5bbbc953UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_7l = {0xbebf5686235fe271UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_6 = {0x426aad63101f2ab7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_6l = {0xbefc58192a28af71UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_5 = {0xc2b374ef62b81807UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_5l = {0x3f5f95951a21edb2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_4 = {0x42f75623d9ad4452UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_4l = {0xbf9d03e1aaa2bc9fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_3 = {0xc3367e6b7d39feafUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_3l = {0xbfcba770b7f49827UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_2 = {0x4370e978556d3419UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_2l = {0x401fec8a885f60ddUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_1 = {0xc3a33e6642351938UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_1l = {0xc02a64e692215bd2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_0 = {0x43d29bdf34b36a85UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c2_0l = {0xc07a7f2818ce87a2UL};
// coefficients for i0(x)/exp(x)*2^64
// 150.0 .. 325.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_21 = {0xc3a1a80b9e17e5c8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_21l = {0x4034935d1bb2cf61UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_20 = {0x43e682c36b12751dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_20l = {0xc07ada54379eb750UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_19 = {0xc41b4f2d72eed093UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_19l = {0xc0a08bf811c38911UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_18 = {0x4444f8120329b0c2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_18l = {0xc0c06791f1b16d1dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_17 = {0xc466defc5dc014b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_17l = {0xc10c7969dcebe222UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_16 = {0x4482d8a6d0cf5655UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_16l = {0x4126d4382a7b15aaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_15 = {0xc4985e2a27194f27UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_15l = {0xc13a676e4017443eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_14 = {0x44a9566754738e2bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_14l = {0xc144cb45163fdf06UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_13 = {0xc4b58b5eaa907975UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_13l = {0xc1496a33e5bb3bb2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_12 = {0x44be4d899885f28fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_12l = {0xc150f2062428104cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_11 = {0xc4c1c1a7c80de049UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_11l = {0x415b2f213cecf17aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_10 = {0x44c16974dcef28d0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_10l = {0x414f740e3777b85eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_9 = {0xc4bc9d9f5b6ffce7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_9l = {0xc141906df37a4aceUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_8 = {0x44b3aef1e4da1d1cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_8l = {0x414f04d715622f0dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_7 = {0xc4a69527743fddaeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_7l = {0x4147a46b1acc1a8bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_6 = {0x449576ab1f46edf4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_6l = {0x41363b5763705267UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_5 = {0xc480ba6c6d4e61eeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_5l = {0x411310b817c43c19UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_4 = {0x44651210a8564d89UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_4l = {0xc10dc915dcbadacfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_3 = {0xc44503015e30f68fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_3l = {0x40dcfb1cc7ac301aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_2 = {0x4420249fc0d24e2fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_2l = {0xc0b6e861295c5b3eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_1 = {0xc3f29c3701d3a565UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_1l = {0x4091f37da75f0245UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_0 = {0x43c2402f5e527c0bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c3_0l = {0xc033a4d2328b15c2UL};
// coefficients for i0(x)/exp(x)*2^64
// 325..714.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_21 = {0xc21d86cf07155b2aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_21l = {0x3ead7f7bdec3e24eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_20 = {0x427495c7c82ba86dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_20l = {0x3f0d29d8d7e9bb5fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_19 = {0xc2bb4f242967835bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_19l = {0xbf2d4375981c8975UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_18 = {0x42f6ed91469aaf51UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_18l = {0xbf94d4afae2081edUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_17 = {0xc32b57584c7107afUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_17l = {0xbfcbc9feb0be6a04UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_16 = {0x4358a121699740f0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_16l = {0xbfce675b1fc80449UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_15 = {0xc3816793ca2f6e11UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_15l = {0xc00fb0360c2e5c66UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_14 = {0x43a3c784429b5678UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_14l = {0x40373a8616ab81bfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_13 = {0xc3c2612d3ae58ef6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_13l = {0xc050549ac97f1fe4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_12 = {0x43dc3f876d8fa511UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_12l = {0xc0593728e5d9c288UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_11 = {0xc3f215c685084a5eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_11l = {0x409a271f75b7bc10UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_10 = {0x44035fc2694d7b57UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_10l = {0x40a9a143577c234cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_9 = {0xc41163f86be9d088UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_9l = {0xc0a1cbaf03501956UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_8 = {0x441a219056d58755UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_8l = {0x40ac7499a4180ff4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_7 = {0xc4205f0cb7aadcc1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_7l = {0xc0a5860417acb1bbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_6 = {0x4420fe0e7ba37b73UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_6l = {0xc0b04b6633220f66UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_5 = {0xc41cec71a2f759f9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_5l = {0xc0982953f5909319UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_4 = {0x4413e41f6baa3631UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_4l = {0x40bc712ffeef973cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_3 = {0xc405a92e4e156d45UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_3l = {0xc0aa7475366eab36UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_2 = {0x43f22d44514cbfccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_2l = {0x4087d4d3ee736c19UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_1 = {0xc3d6e6bc330f2af2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_1l = {0x407223ec109516e1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_0 = {0x43b8985f31561c22UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___c4_0l = {0x405db11d07165ef2UL};
// exp(R) coefficients
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce11 = {0x3e5ae6449e62ecf6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce10 = {0x3e928a27e303b465UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce9 = {0x3ec71de8e64711a9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce8 = {0x3efa019a6b2470acUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce7 = {0x3f2a01a01710652fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce6 = {0x3f56c16c17f29c89UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce5 = {0x3f8111111111a24eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce4 = {0x3fa555555555211dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce3 = {0x3fc5555555555530UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce2 = {0x3fe0000000000005UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___ce1 = {0x3ff0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___Shifter = {0x43380000000003bfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___L2E = {0x3ff71547652B82FEUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___L2H = {0x3fe62E42FEFA39EFUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___L2L = {0x3c7ABC9E3B39803FUL};
// 2^(-8)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di0_ep___scx = {0x3F70000000000000UL};
/////////////////////////////////////////////////////////////////////
//
//  Multi-precision macros (double precision)
//
/////////////////////////////////////////////////////////////////////
// #define DP_FABS(x)  (((x)>=0.0)? (x): -(x))
//   (Ah + Al) *= B
//   (Ah + Al) * B
//   (Ah + Al) *= (Bh + Bl)
//   (Ah + Al) += (Bh, Bl)
//  |Bh| >= |Ah|
//   (Ah + Al) += Bh
//  |Bh| >= |Ah|
//   (Ah + Al) += (Bh, Bl)
//  no restrictions on A, B ordering
//   Rh + Rl = (Ah + Al) * (Bh + Bl)
//   (Ah + Al) += (Bh, Bl)
//  no restrictions on A, B ordering
inline int __devicelib_imf_internal_di0(const double *pa, double *pres) {
  int nRet = 0;
  double xin = *pa;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, S, N, epoly, epoly_l, Te, RTe, H, L, LH, epoly2, xs;
  double x2, poly, x2l, poly_l, R, R0, R1, R1h, Rl;
  double bc_h[32], bc_l[32];
  x.f = xin;
  // |xin|
  x.w = x.w & 0x7fffffffffffffffUL;
  if (x.f <= 12.0) {
    x2 = __fma(xin, xin, 0.0);
    if (x.f <= 2.0) {
      poly = __fma(x2, __di0_ep___csmall8.f, __di0_ep___csmall7.f);
      poly = __fma(poly, x2, __di0_ep___csmall6.f);
      poly = __fma(poly, x2, __di0_ep___csmall5.f);
      poly = __fma(poly, x2, __di0_ep___csmall4.f);
      poly = __fma(poly, x2, __di0_ep___csmall3.f);
      poly = __fma(poly, x2, __di0_ep___csmall2.f);
      poly = __fma(poly, x2, __di0_ep___csmall1.f);
      poly = __fma(poly, x2, __di0_ep___csmall0.f);
      *pres = poly;
      return nRet;
    } else // 2.0 .. 12.0
    {
      x2l = __fma(xin, xin, -x2);
      // poly_l = 0.0;
      // 0.525 ulp for all multi-precision evaluation
      {
        poly = __fma(x2, __di0_ep___c0_17.f, 0.0);
        poly_l = __fma(x2, __di0_ep___c0_17.f, -poly);
        poly_l = __fma(x2l, __di0_ep___c0_17.f, poly_l);
        poly_l = __fma(x2, __di0_ep___c0_17l.f, poly_l);
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_16.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_16.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_16l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_15.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_15.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_15l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_14.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_14.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_14l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_13.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_13.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_13l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_12.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_12.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_12l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_11.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_11.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_11l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_10.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_10.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_10l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_9.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_9.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_9l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_8.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_8.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_8l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_7.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_7.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_7l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_6.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_6.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_6l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_5.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_5.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_5l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        double __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__di0_ep___c0_4.f)) ? (__di0_ep___c0_4.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di0_ep___c0_4.f))
                   ? (poly)
                   : (__di0_ep___c0_4.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_4l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        double __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__di0_ep___c0_3.f)) ? (__di0_ep___c0_3.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di0_ep___c0_3.f))
                   ? (poly)
                   : (__di0_ep___c0_3.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_3l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        double __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__di0_ep___c0_2.f)) ? (__di0_ep___c0_2.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di0_ep___c0_2.f))
                   ? (poly)
                   : (__di0_ep___c0_2.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_2l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        double __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__di0_ep___c0_1.f)) ? (__di0_ep___c0_1.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di0_ep___c0_1.f))
                   ? (poly)
                   : (__di0_ep___c0_1.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_1l.f) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x2, 0.0);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di0_ep___c0_0.f);
        __ahh = __fma(__ph, 1.0, -__di0_ep___c0_0.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di0_ep___c0_0l.f) + __ahl;
        poly = __ph;
      };
      *pres = poly + poly_l;
      return nRet;
    }
  } else if (x.f <= 714.0f) {
    if (x.f <= 150.0) {
      bc_h[30] = (x.f <= 42.0) ? __di0_ep___c1_30.f : __di0_ep___c2_30.f;
      bc_h[29] = (x.f <= 42.0) ? __di0_ep___c1_29.f : __di0_ep___c2_29.f;
      bc_h[28] = (x.f <= 42.0) ? __di0_ep___c1_28.f : __di0_ep___c2_28.f;
      bc_h[27] = (x.f <= 42.0) ? __di0_ep___c1_27.f : __di0_ep___c2_27.f;
      bc_h[26] = (x.f <= 42.0) ? __di0_ep___c1_26.f : __di0_ep___c2_26.f;
      bc_h[25] = (x.f <= 42.0) ? __di0_ep___c1_25.f : __di0_ep___c2_25.f;
      bc_h[24] = (x.f <= 42.0) ? __di0_ep___c1_24.f : __di0_ep___c2_24.f;
      bc_h[23] = (x.f <= 42.0) ? __di0_ep___c1_23.f : __di0_ep___c2_23.f;
      bc_h[22] = (x.f <= 42.0) ? __di0_ep___c1_22.f : __di0_ep___c2_22.f;
      bc_h[21] = (x.f <= 42.0) ? __di0_ep___c1_21.f : __di0_ep___c2_21.f;
      bc_h[20] = (x.f <= 42.0) ? __di0_ep___c1_20.f : __di0_ep___c2_20.f;
      bc_h[19] = (x.f <= 42.0) ? __di0_ep___c1_19.f : __di0_ep___c2_19.f;
      bc_h[18] = (x.f <= 42.0) ? __di0_ep___c1_18.f : __di0_ep___c2_18.f;
      bc_h[17] = (x.f <= 42.0) ? __di0_ep___c1_17.f : __di0_ep___c2_17.f;
      bc_h[16] = (x.f <= 42.0) ? __di0_ep___c1_16.f : __di0_ep___c2_16.f;
      bc_h[15] = (x.f <= 42.0) ? __di0_ep___c1_15.f : __di0_ep___c2_15.f;
      bc_h[14] = (x.f <= 42.0) ? __di0_ep___c1_14.f : __di0_ep___c2_14.f;
      bc_h[13] = (x.f <= 42.0) ? __di0_ep___c1_13.f : __di0_ep___c2_13.f;
      bc_h[12] = (x.f <= 42.0) ? __di0_ep___c1_12.f : __di0_ep___c2_12.f;
      bc_h[11] = (x.f <= 42.0) ? __di0_ep___c1_11.f : __di0_ep___c2_11.f;
      bc_h[10] = (x.f <= 42.0) ? __di0_ep___c1_10.f : __di0_ep___c2_10.f;
      bc_h[9] = (x.f <= 42.0) ? __di0_ep___c1_9.f : __di0_ep___c2_9.f;
      bc_h[8] = (x.f <= 42.0) ? __di0_ep___c1_8.f : __di0_ep___c2_8.f;
      bc_h[7] = (x.f <= 42.0) ? __di0_ep___c1_7.f : __di0_ep___c2_7.f;
      bc_h[6] = (x.f <= 42.0) ? __di0_ep___c1_6.f : __di0_ep___c2_6.f;
      bc_h[5] = (x.f <= 42.0) ? __di0_ep___c1_5.f : __di0_ep___c2_5.f;
      bc_h[4] = (x.f <= 42.0) ? __di0_ep___c1_4.f : __di0_ep___c2_4.f;
      bc_h[3] = (x.f <= 42.0) ? __di0_ep___c1_3.f : __di0_ep___c2_3.f;
      bc_h[2] = (x.f <= 42.0) ? __di0_ep___c1_2.f : __di0_ep___c2_2.f;
      bc_h[1] = (x.f <= 42.0) ? __di0_ep___c1_1.f : __di0_ep___c2_1.f;
      bc_h[0] = (x.f <= 42.0) ? __di0_ep___c1_0.f : __di0_ep___c2_0.f;
      bc_l[30] = (x.f <= 42.0) ? __di0_ep___c1_30l.f : __di0_ep___c2_30l.f;
      bc_l[29] = (x.f <= 42.0) ? __di0_ep___c1_29l.f : __di0_ep___c2_29l.f;
      bc_l[28] = (x.f <= 42.0) ? __di0_ep___c1_28l.f : __di0_ep___c2_28l.f;
      bc_l[27] = (x.f <= 42.0) ? __di0_ep___c1_27l.f : __di0_ep___c2_27l.f;
      bc_l[26] = (x.f <= 42.0) ? __di0_ep___c1_26l.f : __di0_ep___c2_26l.f;
      bc_l[25] = (x.f <= 42.0) ? __di0_ep___c1_25l.f : __di0_ep___c2_25l.f;
      bc_l[24] = (x.f <= 42.0) ? __di0_ep___c1_24l.f : __di0_ep___c2_24l.f;
      bc_l[23] = (x.f <= 42.0) ? __di0_ep___c1_23l.f : __di0_ep___c2_23l.f;
      bc_l[22] = (x.f <= 42.0) ? __di0_ep___c1_22l.f : __di0_ep___c2_22l.f;
      bc_l[21] = (x.f <= 42.0) ? __di0_ep___c1_21l.f : __di0_ep___c2_21l.f;
      bc_l[20] = (x.f <= 42.0) ? __di0_ep___c1_20l.f : __di0_ep___c2_20l.f;
      bc_l[19] = (x.f <= 42.0) ? __di0_ep___c1_19l.f : __di0_ep___c2_19l.f;
      bc_l[18] = (x.f <= 42.0) ? __di0_ep___c1_18l.f : __di0_ep___c2_18l.f;
      bc_l[17] = (x.f <= 42.0) ? __di0_ep___c1_17l.f : __di0_ep___c2_17l.f;
      bc_l[16] = (x.f <= 42.0) ? __di0_ep___c1_16l.f : __di0_ep___c2_16l.f;
      bc_l[15] = (x.f <= 42.0) ? __di0_ep___c1_15l.f : __di0_ep___c2_15l.f;
      bc_l[14] = (x.f <= 42.0) ? __di0_ep___c1_14l.f : __di0_ep___c2_14l.f;
      bc_l[13] = (x.f <= 42.0) ? __di0_ep___c1_13l.f : __di0_ep___c2_13l.f;
      bc_l[12] = (x.f <= 42.0) ? __di0_ep___c1_12l.f : __di0_ep___c2_12l.f;
      bc_l[11] = (x.f <= 42.0) ? __di0_ep___c1_11l.f : __di0_ep___c2_11l.f;
      bc_l[10] = (x.f <= 42.0) ? __di0_ep___c1_10l.f : __di0_ep___c2_10l.f;
      bc_l[9] = (x.f <= 42.0) ? __di0_ep___c1_9l.f : __di0_ep___c2_9l.f;
      bc_l[8] = (x.f <= 42.0) ? __di0_ep___c1_8l.f : __di0_ep___c2_8l.f;
      bc_l[7] = (x.f <= 42.0) ? __di0_ep___c1_7l.f : __di0_ep___c2_7l.f;
      bc_l[6] = (x.f <= 42.0) ? __di0_ep___c1_6l.f : __di0_ep___c2_6l.f;
      bc_l[5] = (x.f <= 42.0) ? __di0_ep___c1_5l.f : __di0_ep___c2_5l.f;
      bc_l[4] = (x.f <= 42.0) ? __di0_ep___c1_4l.f : __di0_ep___c2_4l.f;
      bc_l[3] = (x.f <= 42.0) ? __di0_ep___c1_3l.f : __di0_ep___c2_3l.f;
      bc_l[2] = (x.f <= 42.0) ? __di0_ep___c1_2l.f : __di0_ep___c2_2l.f;
      bc_l[1] = (x.f <= 42.0) ? __di0_ep___c1_1l.f : __di0_ep___c2_1l.f;
      bc_l[0] = (x.f <= 42.0) ? __di0_ep___c1_0l.f : __di0_ep___c2_0l.f;
      // poly ~ 2^64*BesselI(0, x.f)/exp(x.f)
      poly = bc_h[30];
      poly_l = bc_l[30];
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[29]);
        __ahh = __fma(__ph, 1.0, -bc_h[29]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[29]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[28]);
        __ahh = __fma(__ph, 1.0, -bc_h[28]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[28]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[27]);
        __ahh = __fma(__ph, 1.0, -bc_h[27]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[27]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[26]);
        __ahh = __fma(__ph, 1.0, -bc_h[26]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[26]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[25]);
        __ahh = __fma(__ph, 1.0, -bc_h[25]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[25]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[24]);
        __ahh = __fma(__ph, 1.0, -bc_h[24]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[24]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[23]);
        __ahh = __fma(__ph, 1.0, -bc_h[23]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[23]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[22]);
        __ahh = __fma(__ph, 1.0, -bc_h[22]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[22]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[21]);
        __ahh = __fma(__ph, 1.0, -bc_h[21]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[21]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[20]);
        __ahh = __fma(__ph, 1.0, -bc_h[20]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[20]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[19]);
        __ahh = __fma(__ph, 1.0, -bc_h[19]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[19]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[18]);
        __ahh = __fma(__ph, 1.0, -bc_h[18]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[18]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[17]);
        __ahh = __fma(__ph, 1.0, -bc_h[17]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[17]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[16]);
        __ahh = __fma(__ph, 1.0, -bc_h[16]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[16]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[15]);
        __ahh = __fma(__ph, 1.0, -bc_h[15]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[15]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[14]);
        __ahh = __fma(__ph, 1.0, -bc_h[14]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[14]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[13]);
        __ahh = __fma(__ph, 1.0, -bc_h[13]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[13]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[12]);
        __ahh = __fma(__ph, 1.0, -bc_h[12]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[12]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[11]);
        __ahh = __fma(__ph, 1.0, -bc_h[11]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[11]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[10]);
        __ahh = __fma(__ph, 1.0, -bc_h[10]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[10]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[9]);
        __ahh = __fma(__ph, 1.0, -bc_h[9]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[9]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[8]);
        __ahh = __fma(__ph, 1.0, -bc_h[8]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[8]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[7]);
        __ahh = __fma(__ph, 1.0, -bc_h[7]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[7]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[6]);
        __ahh = __fma(__ph, 1.0, -bc_h[6]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[6]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[5]);
        __ahh = __fma(__ph, 1.0, -bc_h[5]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[5]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[4])) ? (bc_h[4]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[4])) ? (poly) : (bc_h[4]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[4]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[3])) ? (bc_h[3]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[3])) ? (poly) : (bc_h[3]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[3]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[2])) ? (bc_h[2]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[2])) ? (poly) : (bc_h[2]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[2]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[1])) ? (bc_h[1]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[1])) ? (poly) : (bc_h[1]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[1]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, x.f, 0.0);
        __phl = __fma(poly, x.f, -__ph);
        poly_l = __fma(poly_l, x.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[0])) ? (bc_h[0]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[0])) ? (poly) : (bc_h[0]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[0]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
    } else // 150.0 .. 714.0
    {
      bc_h[21] = (x.f <= 325.0) ? __di0_ep___c3_21.f : __di0_ep___c4_21.f;
      bc_h[20] = (x.f <= 325.0) ? __di0_ep___c3_20.f : __di0_ep___c4_20.f;
      bc_h[19] = (x.f <= 325.0) ? __di0_ep___c3_19.f : __di0_ep___c4_19.f;
      bc_h[18] = (x.f <= 325.0) ? __di0_ep___c3_18.f : __di0_ep___c4_18.f;
      bc_h[17] = (x.f <= 325.0) ? __di0_ep___c3_17.f : __di0_ep___c4_17.f;
      bc_h[16] = (x.f <= 325.0) ? __di0_ep___c3_16.f : __di0_ep___c4_16.f;
      bc_h[15] = (x.f <= 325.0) ? __di0_ep___c3_15.f : __di0_ep___c4_15.f;
      bc_h[14] = (x.f <= 325.0) ? __di0_ep___c3_14.f : __di0_ep___c4_14.f;
      bc_h[13] = (x.f <= 325.0) ? __di0_ep___c3_13.f : __di0_ep___c4_13.f;
      bc_h[12] = (x.f <= 325.0) ? __di0_ep___c3_12.f : __di0_ep___c4_12.f;
      bc_h[11] = (x.f <= 325.0) ? __di0_ep___c3_11.f : __di0_ep___c4_11.f;
      bc_h[10] = (x.f <= 325.0) ? __di0_ep___c3_10.f : __di0_ep___c4_10.f;
      bc_h[9] = (x.f <= 325.0) ? __di0_ep___c3_9.f : __di0_ep___c4_9.f;
      bc_h[8] = (x.f <= 325.0) ? __di0_ep___c3_8.f : __di0_ep___c4_8.f;
      bc_h[7] = (x.f <= 325.0) ? __di0_ep___c3_7.f : __di0_ep___c4_7.f;
      bc_h[6] = (x.f <= 325.0) ? __di0_ep___c3_6.f : __di0_ep___c4_6.f;
      bc_h[5] = (x.f <= 325.0) ? __di0_ep___c3_5.f : __di0_ep___c4_5.f;
      bc_h[4] = (x.f <= 325.0) ? __di0_ep___c3_4.f : __di0_ep___c4_4.f;
      bc_h[3] = (x.f <= 325.0) ? __di0_ep___c3_3.f : __di0_ep___c4_3.f;
      bc_h[2] = (x.f <= 325.0) ? __di0_ep___c3_2.f : __di0_ep___c4_2.f;
      bc_h[1] = (x.f <= 325.0) ? __di0_ep___c3_1.f : __di0_ep___c4_1.f;
      bc_h[0] = (x.f <= 325.0) ? __di0_ep___c3_0.f : __di0_ep___c4_0.f;
      bc_l[21] = (x.f <= 325.0) ? __di0_ep___c3_21l.f : __di0_ep___c4_21l.f;
      bc_l[20] = (x.f <= 325.0) ? __di0_ep___c3_20l.f : __di0_ep___c4_20l.f;
      bc_l[19] = (x.f <= 325.0) ? __di0_ep___c3_19l.f : __di0_ep___c4_19l.f;
      bc_l[18] = (x.f <= 325.0) ? __di0_ep___c3_18l.f : __di0_ep___c4_18l.f;
      bc_l[17] = (x.f <= 325.0) ? __di0_ep___c3_17l.f : __di0_ep___c4_17l.f;
      bc_l[16] = (x.f <= 325.0) ? __di0_ep___c3_16l.f : __di0_ep___c4_16l.f;
      bc_l[15] = (x.f <= 325.0) ? __di0_ep___c3_15l.f : __di0_ep___c4_15l.f;
      bc_l[14] = (x.f <= 325.0) ? __di0_ep___c3_14l.f : __di0_ep___c4_14l.f;
      bc_l[13] = (x.f <= 325.0) ? __di0_ep___c3_13l.f : __di0_ep___c4_13l.f;
      bc_l[12] = (x.f <= 325.0) ? __di0_ep___c3_12l.f : __di0_ep___c4_12l.f;
      bc_l[11] = (x.f <= 325.0) ? __di0_ep___c3_11l.f : __di0_ep___c4_11l.f;
      bc_l[10] = (x.f <= 325.0) ? __di0_ep___c3_10l.f : __di0_ep___c4_10l.f;
      bc_l[9] = (x.f <= 325.0) ? __di0_ep___c3_9l.f : __di0_ep___c4_9l.f;
      bc_l[8] = (x.f <= 325.0) ? __di0_ep___c3_8l.f : __di0_ep___c4_8l.f;
      bc_l[7] = (x.f <= 325.0) ? __di0_ep___c3_7l.f : __di0_ep___c4_7l.f;
      bc_l[6] = (x.f <= 325.0) ? __di0_ep___c3_6l.f : __di0_ep___c4_6l.f;
      bc_l[5] = (x.f <= 325.0) ? __di0_ep___c3_5l.f : __di0_ep___c4_5l.f;
      bc_l[4] = (x.f <= 325.0) ? __di0_ep___c3_4l.f : __di0_ep___c4_4l.f;
      bc_l[3] = (x.f <= 325.0) ? __di0_ep___c3_3l.f : __di0_ep___c4_3l.f;
      bc_l[2] = (x.f <= 325.0) ? __di0_ep___c3_2l.f : __di0_ep___c4_2l.f;
      bc_l[1] = (x.f <= 325.0) ? __di0_ep___c3_1l.f : __di0_ep___c4_1l.f;
      bc_l[0] = (x.f <= 325.0) ? __di0_ep___c3_0l.f : __di0_ep___c4_0l.f;
      // x*2^(-8)
      xs.f = __fma(x.f, __di0_ep___scx.f, 0.0);
      // poly ~ 2^64*BesselI(0, x.f)/exp(x.f)
      poly = bc_h[21];
      poly_l = bc_l[21];
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[20]);
        __ahh = __fma(__ph, 1.0, -bc_h[20]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[20]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[19]);
        __ahh = __fma(__ph, 1.0, -bc_h[19]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[19]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[18]);
        __ahh = __fma(__ph, 1.0, -bc_h[18]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[18]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[17]);
        __ahh = __fma(__ph, 1.0, -bc_h[17]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[17]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[16]);
        __ahh = __fma(__ph, 1.0, -bc_h[16]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[16]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[15]);
        __ahh = __fma(__ph, 1.0, -bc_h[15]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[15]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[14]);
        __ahh = __fma(__ph, 1.0, -bc_h[14]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[14]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[13]);
        __ahh = __fma(__ph, 1.0, -bc_h[13]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[13]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[12]);
        __ahh = __fma(__ph, 1.0, -bc_h[12]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[12]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[11]);
        __ahh = __fma(__ph, 1.0, -bc_h[11]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[11]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[10]);
        __ahh = __fma(__ph, 1.0, -bc_h[10]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[10]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[9]);
        __ahh = __fma(__ph, 1.0, -bc_h[9]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[9]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[8]);
        __ahh = __fma(__ph, 1.0, -bc_h[8]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[8]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[7]);
        __ahh = __fma(__ph, 1.0, -bc_h[7]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[7]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[6]);
        __ahh = __fma(__ph, 1.0, -bc_h[6]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[6]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[5]);
        __ahh = __fma(__ph, 1.0, -bc_h[5]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[5]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[4]);
        __ahh = __fma(__ph, 1.0, -bc_h[4]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[4]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, bc_h[3]);
        __ahh = __fma(__ph, 1.0, -bc_h[3]);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + bc_l[3]) + __ahl;
        poly = __ph;
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[2])) ? (bc_h[2]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[2])) ? (poly) : (bc_h[2]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[2]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[1])) ? (bc_h[1]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[1])) ? (poly) : (bc_h[1]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[1]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
      {
        double __ph, __phl;
        __ph = __fma(poly, xs.f, 0.0);
        __phl = __fma(poly, xs.f, -__ph);
        poly_l = __fma(poly_l, xs.f, __phl);
        poly = __ph;
      };
      {
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[0])) ? (bc_h[0]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[0])) ? (poly) : (bc_h[0]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[0]) + __ahl;
        poly = __fma(__ph, 1.0, __pl);
        __alh = __fma(poly, 1.0, -__ph);
        poly_l = __fma(__pl, 1.0, -__alh);
      };
    }
    // H+L ~exp(x.f)*2^(-64)
    // x2h*L2E + Shifter
    S.f = __fma(x.f, __di0_ep___L2E.f, __di0_ep___Shifter.f);
    // (int)(x2h*L2E)
    N.f = S.f - __di0_ep___Shifter.f;
    // x^2 - N*log(2)
    R0 = __fma(-N.f, __di0_ep___L2H.f, x.f);
    R1 = __fma(-N.f, __di0_ep___L2L.f, 0.0);
    R = R0 + R1;
    R1h = R - R0;
    Rl = R1 - R1h;
    // 2^(N)
    Te.w = S.w << 52;
    // exp(R)-1
    epoly.f = __fma(__di0_ep___ce11.f, R, __di0_ep___ce10.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce9.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce8.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce7.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce6.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce5.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce4.f);
    epoly.f = __fma(epoly.f, R, __di0_ep___ce3.f);
    epoly2.f = __fma(epoly.f, R, __di0_ep___ce2.f);
    epoly.f = __fma(epoly2.f, R, __di0_ep___ce1.f);
    epoly_l.f = __fma(epoly.f, Rl, 0.0);
    // 2^(N)*exp(R)
    RTe.f = R * Te.f;
    H.f = __fma(epoly.f, RTe.f, Te.f);
    LH.f = H.f - Te.f;
    L.f = __fma(epoly.f, RTe.f, -LH.f);
    L.f = __fma(epoly_l.f, Te.f, L.f);
    // (H+L)*(poly+poly_l)
    L.f = __fma(L.f, poly, __fma(H.f, poly_l, 0.0));
    H.f = __fma(H.f, poly, L.f);
    // check for overflow
    nRet = (H.f == 0x7ff0000000000000UL) ? 3 : nRet;
    *pres = H.f;
    return nRet;
  }
  H.w = 0x7ff0000000000000UL;
  *pres = H.f + x.f;
  // large inputs overflow
  nRet = (x.w < 0x7ff0000000000000UL) ? 3 : nRet;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_i0_d_ep */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_i0(double x) {
  using namespace __imf_impl_i0_d_ep;
  double r;
  __devicelib_imf_internal_di0(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
