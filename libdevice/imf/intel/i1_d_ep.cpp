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
namespace __imf_impl_i1_d_ep {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall9 = {0x3c3d84d70562f7deUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall8 = {0x3cc2aefd0b413632UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall7 = {0x3d4523b6341517a3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall6 = {0x3dc27e4912cc9e6bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall5 = {0x3e3845c8d3823cd1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall4 = {0x3ea6c16c15d78f60UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall3 = {0x3f0c71c71c769399UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall2 = {0x3f655555555548abUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall1 = {0x3fb000000000001aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___csmall0 = {0x3fe0000000000000UL};
// |x|<=14.0
// minimax(BesselI(1, sqrt(x)), x = 0. .. 14.0^2, [18, 0], 1/BesselI(1,
// sqrt(x)), 'me')
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_18 = {0x36e36040900df0c8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_18l = {0x338b547893773fb1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_17 = {0xb74f2a1a19cd0c3cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_17l = {0xb3ed9d4386b78114UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_16 = {0x381fa2771fe67802UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_16l = {0x34bb5f5e204862ecUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_15 = {0x38b3657badd422e9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_15l = {0xb55317a194c712c3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_14 = {0x3955ead74c1e7eecUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_14l = {0x35f0dd5bea55e092UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_13 = {0x39f148d219e82254UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_13l = {0xb69a004243007bc8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_12 = {0x3a88c3daf0136464UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_12l = {0x37006e326d5246b8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_11 = {0x3b1e24f6dd9202b8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_11l = {0xb7a1b9a7f6eb10b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_10 = {0x3baf179511b4fd75UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_10l = {0xb84d4b885ad53516UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_9 = {0x3c3ab81a36deebffUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_9l = {0xb8ce79b9f11d630fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_8 = {0x3cc2c975e37cc6d4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_8l = {0x39560812cf7b72ccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_7 = {0x3d4522a435fa3738UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_7l = {0xb9cb500a6fc2a1b7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_6 = {0x3dc27e4fb8339f3dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_6l = {0xba6dda65e746d938UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_5 = {0x3e3845c8a0baa473UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_5l = {0x3acdd0ee6639626cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_4 = {0x3ea6c16c16c2bec0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_4l = {0x3b1bce2f58dd332dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_3 = {0x3f0c71c71c71ac87UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_3l = {0xbb807bc955443eccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_2 = {0x3f65555555555662UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_2l = {0xbc057938975c4a96UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_1 = {0x3faffffffffffff0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_1l = {0x3c34a01cd3e05440UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_0 = {0x3fe0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c0_0l = {0x3c7297045a67b0e0UL};
// coefficients for i1(x)/exp(x)*2^64
// 14.0 .. 45.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_28 = {0xbb0b3f122e87d92cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_28l = {0xb791323b10f84d54UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_27 = {0x3ba6825a910cbc11UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_27l = {0x38481b6a441a95bdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_26 = {0xbc31da6f8b990cbbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_26l = {0x38c077cab60eec5dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_25 = {0x3cb219ce67c95047UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_25l = {0xb95c08fca64ab334UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_24 = {0xbd2a59a1d27e0e72UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_24l = {0xb9cdd2e7aa2364c9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_23 = {0x3d9d53557a63080aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_23l = {0xba3a400681cf8cafUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_22 = {0xbe09f18b3109ee7eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_22l = {0xba90684851e6561aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_21 = {0x3e72ba9084788874UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_21l = {0xbae85dcf97649a84UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_20 = {0xbed67a03ddf34a53UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_20l = {0xbb733fea797d6114UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_19 = {0x3f36b91f315b9dafUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_19l = {0xbbdca4eb4c96730dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_18 = {0xbf938aba63132b42UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_18l = {0xbc3cd7513217b3c2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_17 = {0x3feccbe1be63d73aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_17l = {0xbc881f14fd419879UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_16 = {0xc042447cf5c247caUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_16l = {0xbceee413a45668b4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_15 = {0x409404b80b3d5d9fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_15l = {0xbd338bd792fd4443UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_14 = {0xc0e2fa368e9fc141UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_14l = {0x3d84f038f3e2f2adUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_13 = {0x412f2050d085ec85UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_13l = {0xbd920dfdb41f8329UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_12 = {0xc1760ba5c826bdbfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_12l = {0x3e143059d4c1630fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_11 = {0x41badfafca06d2b1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_11l = {0x3e5be8362b88fd7aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_10 = {0xc1fc05fd7582afa7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_10l = {0xbe62e96825c7c826UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_9 = {0x4238c3465fa8a1e3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_9l = {0x3eb9025c067b2ef1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_8 = {0xc2724465ac1d4322UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_8l = {0x3f12a49b5b90590fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_7 = {0x42a5f010e2f56424UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_7l = {0x3f2ab988d56dada4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_6 = {0xc2d46ae6d417398fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_6l = {0xbf6ad1f8d9803a5aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_5 = {0x42f9eeedf38b2bf7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_5l = {0x3f8125cb8f180b59UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_4 = {0xc304d124b4863fb4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_4l = {0x3fa557196cf22c7aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_3 = {0xc34662bf9817764eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_3l = {0xbfb39c3c50df1a96UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_2 = {0x4380e506fc54ad1fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_2l = {0xc0118921d374d2a1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_1 = {0xc3abaa0d12704274UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_1l = {0x4040e1ea860e14a4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_0 = {0x43d3780d7db6f09cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c1_0l = {0xc07274bdf5aa8970UL};
// coefficients for i1(x)/exp(x)*2^64
// 45.0..150.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_28 = {0x3810e1ad37f30f78UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_28l = {0x34bcb60639a21a54UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_27 = {0xb8c74518d0b08af7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_27l = {0x35588cce55bdffc9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_26 = {0x396ecf94d71a4e0eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_26l = {0xb5f04e5379cbc1dbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_25 = {0xba0a16dfca01f9a1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_25l = {0xb691d2c2472900b7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_24 = {0x3a9fbddac0629482UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_24l = {0xb716fdd3c4ccdbdbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_23 = {0xbb2d8c48897580a9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_23l = {0xb7c99102341d5f48UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_22 = {0x3bb5e2cc0c1e81b3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_22l = {0xb84be817af064111UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_21 = {0xbc3a7c883cf7073fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_21l = {0xb8d678d79cf1d17fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_20 = {0x3cbaae09af33ade5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_20l = {0xb9547287f845037eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_19 = {0xbd36acff51403c2cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_19l = {0xb9c1b22eafd88202UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_18 = {0x3db06d3cf067fa96UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_18l = {0x3a5eab015e3296b0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_17 = {0xbe24702966636629UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_17l = {0xbabb9f22fec82345UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_16 = {0x3e95f53ecbaa3c44UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_16l = {0xbb34fc557f8707a2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_15 = {0xbf047316b3414543UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_15l = {0x3baf14683314167aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_14 = {0x3f708cbb0ff90b31UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_14l = {0x3c07404a70564f77UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_13 = {0xbfd74e7a47360c87UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_13l = {0x3c65fe6379babc6aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_12 = {0x403c8f195016d1eeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_12l = {0x3cd46f079b12463fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_11 = {0xc09e6aa01a204ccdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_11l = {0xbd35f78a492e7fb6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_10 = {0x40fc177e37363d3bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_10l = {0x3d9e4aa15394ac7cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_9 = {0xc1566accb3ffd383UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_9l = {0x3df61087cb3fcea9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_8 = {0x41aec15a444dcd01UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_8l = {0x3e477b07f48c03a9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_7 = {0xc20202b7dd1a7e99UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_7l = {0x3ea3f331e6210c9fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_6 = {0x4251d817eefb3041UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_6l = {0xbeeb5ffc607640d2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_5 = {0xc29d8dc9c81f70a2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_5l = {0x3f1429bf2ea8177eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_4 = {0x42e42372a8c48e09UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_4l = {0x3f753c22d6046bcaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_3 = {0xc326215dafc90f52UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_3l = {0x3fa5e226a4894264UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_2 = {0x4363217f8284436eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_2l = {0x40075daa6f770b6bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_1 = {0xc3998e22962e36daUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_1l = {0x402967afba9a67c0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_0 = {0x43cede8f8b10b42aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c2_0l = {0xc055c42e9f5d45e2UL};
// coefficients for i1(x)/exp(x)*2^64
// 150.0 .. 325.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_21 = {0xc39fdd11e6439860UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_21l = {0xc036eebc608fce05UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_20 = {0x43e4510b04ed3edfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_20l = {0xc06e5fcfe071f1a6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_19 = {0xc418a74a2d887fb2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_19l = {0xc0bfac4407b40156UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_18 = {0x4442ef5cc63a4124UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_18l = {0x40d5e206e9937d7eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_17 = {0xc464a8a99c845af8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_17l = {0x41008f657f27c1fdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_16 = {0x448107798c42b51dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_16l = {0xc117df67aa616cb6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_15 = {0xc49606dce8d07cbeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_15l = {0xc131b09437e46690UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_14 = {0x44a6e9cbe2673cc1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_14l = {0x414d08abaacd8779UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_13 = {0xc4b37e228746c52bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_13l = {0xc1506dee768fecd4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_12 = {0x44bb6ec48c5114f2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_12l = {0x415a367ff4d6631dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_11 = {0xc4c015d256f358fbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_11l = {0x4161f5029f3467f8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_10 = {0x44bf921cc3359d8bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_10l = {0x415d9260f5fd1c1aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_9 = {0xc4b9f76cd811289aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_9l = {0xc15c1c1aa786a5afUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_8 = {0x44b1e18ff9a0b93bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_8l = {0x415a3c533b2ce436UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_7 = {0xc4a48b129c3e0f8dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_7l = {0xc12ae0ac3f214791UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_6 = {0x44938f352799178eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_6l = {0xc13652c92922b2ebUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_5 = {0xc47e8edc86dc0dd2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_5l = {0x410434a35d02ec3fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_4 = {0x44634e1ac8cf6a7bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_4l = {0xc0fa072af78d9670UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_3 = {0xc4435667bcc15fcfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_3l = {0xc0e594fa8a1edfa0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_2 = {0x441dec0cf595c834UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_2l = {0x40b046a5a9f5694dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_1 = {0xc3f176cbf3e53770UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_1l = {0x409d1aacbcf03f77UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_0 = {0x43c19fb2f51445e3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c3_0l = {0xc041f0e9fd2eeea9UL};
// coefficients for i1(x)/exp(x)*2^64
// 325..714.0
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_21 = {0xc21c34a59d29e1f0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_21l = {0xbead29c4051a8073UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_20 = {0x4273aa83f8ceb8b8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_20l = {0x3f07eade2d70b9aaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_19 = {0xc2ba17bee5fff90eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_19l = {0x3f535275bb8e1eeeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_18 = {0x42f5e8cb89c6a0c7UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_18l = {0xbf5393a0d3f5f51cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_17 = {0xc32a21432539f887UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_17l = {0x3fc8545f9ed045e8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_16 = {0x43578ab12029af89UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_16l = {0x3ff7f66301cf5e55UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_15 = {0xc380a385c196a28dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_15l = {0xc024c93e170bf71bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_14 = {0x43a2e99ea93e7015UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_14l = {0x4044473cc594c918UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_13 = {0xc3c193f23b7eb41cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_13l = {0xc069d222707accb2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_12 = {0x43db05cea507426dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_12l = {0x40760e771738560eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_11 = {0xc3f14e3113ced304UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_11l = {0xc08e20fb48d5361fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_10 = {0x44028b8c2a3b78e6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_10l = {0x4098de36f9663500UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_9 = {0xc410a731dc52bd9dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_9l = {0x40bf8b4d51871dd9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_8 = {0x44190905df0507eaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_8l = {0xc0b63484ee108cbfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_7 = {0xc41f63681584b121UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_7l = {0xc0958617c9741770UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_6 = {0x44204d4fb5064f49UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_6l = {0xc0cf4806f6f0e093UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_5 = {0xc41bc6adb230d91cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_5l = {0x407ac6145d78f9b2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_4 = {0x441320be5cdbf71cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_4l = {0x40ae49a148588971UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_3 = {0xc404dee196a0f6f2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_3l = {0x4065a499d8c23cbcUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_2 = {0x43f1914fc50b7beeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_2l = {0x40907cbd87d56a30UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_1 = {0xc3d641934ac95e67UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_1l = {0xc07b461154fabb65UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_0 = {0x43b835a66bbbda62UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___c4_0l = {0xc02b05976409e5ebUL};
// exp(R) coefficients
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce11 = {0x3e5ae6449e62ecf6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce10 = {0x3e928a27e303b465UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce9 = {0x3ec71de8e64711a9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce8 = {0x3efa019a6b2470acUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce7 = {0x3f2a01a01710652fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce6 = {0x3f56c16c17f29c89UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce5 = {0x3f8111111111a24eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce4 = {0x3fa555555555211dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce3 = {0x3fc5555555555530UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce2 = {0x3fe0000000000005UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___ce1 = {0x3ff0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___Shifter = {0x43380000000003bfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___L2E = {0x3ff71547652B82FEUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___L2H = {0x3fe62E42FEFA39EFUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___L2L = {0x3c7ABC9E3B39803FUL};
// 2^(-8)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __di1_ep___scx = {0x3F70000000000000UL};
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
inline int __devicelib_imf_internal_di1(const double *pa, double *pres) {
  int nRet = 0;
  double xin = *pa;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, S, N, epoly, epoly_l, Te, RTe, H, L, LH, epoly2, xs, sgn_x;
  double x2, poly, x2l, poly_l, R, R0, R1, R1h, Rl;
  double bc_h[32], bc_l[32];
  x.f = sgn_x.f = xin;
  // |xin|
  x.w = x.w & 0x7fffffffffffffffUL;
  sgn_x.w = sgn_x.w ^ x.w;
  if (x.f <= 14.0) {
    x2 = __fma(xin, xin, 0.0);
    if (x.f <= 3.0) {
      poly = __fma(x2, __di1_ep___csmall9.f, __di1_ep___csmall8.f);
      poly = __fma(poly, x2, __di1_ep___csmall7.f);
      poly = __fma(poly, x2, __di1_ep___csmall6.f);
      poly = __fma(poly, x2, __di1_ep___csmall5.f);
      poly = __fma(poly, x2, __di1_ep___csmall4.f);
      poly = __fma(poly, x2, __di1_ep___csmall3.f);
      poly = __fma(poly, x2, __di1_ep___csmall2.f);
      poly = __fma(poly, x2, __di1_ep___csmall1.f);
      poly = __fma(poly, x2, __di1_ep___csmall0.f);
      *pres = poly * xin;
      return nRet;
    } else // 2.0 .. 14.0
    {
      x2l = __fma(xin, xin, -x2);
      // poly_l = 0.0;
      // 0.525 ulp for all multi-precision evaluation
      {
        poly = __fma(x2, __di1_ep___c0_18.f, 0.0);
        poly_l = __fma(x2, __di1_ep___c0_18.f, -poly);
        poly_l = __fma(x2l, __di1_ep___c0_18.f, poly_l);
        poly_l = __fma(x2, __di1_ep___c0_18l.f, poly_l);
      };
      {
        double __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0, __di1_ep___c0_17.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_17.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_17l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_16.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_16.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_16l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_15.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_15.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_15l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_14.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_14.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_14l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_13.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_13.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_13l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_12.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_12.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_12l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_11.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_11.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_11l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_10.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_10.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_10l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_9.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_9.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_9l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_8.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_8.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_8l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_7.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_7.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_7l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_6.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_6.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_6l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_5.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_5.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_5l.f) + __ahl;
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
        __bh = (__fabs(poly) <= __fabs(__di1_ep___c0_4.f)) ? (__di1_ep___c0_4.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di1_ep___c0_4.f))
                   ? (poly)
                   : (__di1_ep___c0_4.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_4l.f) + __ahl;
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
        __bh = (__fabs(poly) <= __fabs(__di1_ep___c0_3.f)) ? (__di1_ep___c0_3.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di1_ep___c0_3.f))
                   ? (poly)
                   : (__di1_ep___c0_3.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_3l.f) + __ahl;
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
        __bh = (__fabs(poly) <= __fabs(__di1_ep___c0_2.f)) ? (__di1_ep___c0_2.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di1_ep___c0_2.f))
                   ? (poly)
                   : (__di1_ep___c0_2.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_2l.f) + __ahl;
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
        __bh = (__fabs(poly) <= __fabs(__di1_ep___c0_1.f)) ? (__di1_ep___c0_1.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__di1_ep___c0_1.f))
                   ? (poly)
                   : (__di1_ep___c0_1.f);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_1l.f) + __ahl;
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
        __ph = __fma(poly, 1.0, __di1_ep___c0_0.f);
        __ahh = __fma(__ph, 1.0, -__di1_ep___c0_0.f);
        __ahl = __fma(poly, 1.0, -__ahh);
        poly_l = (poly_l + __di1_ep___c0_0l.f) + __ahl;
        poly = __ph;
      };
      *pres = __fma(xin, poly, __fma(xin, poly_l, 0.0));
      return nRet;
    }
  } else if (x.f <= 714.0f) {
    if (x.f <= 150.0) {
      bc_h[28] = (x.f <= 45.0) ? __di1_ep___c1_28.f : __di1_ep___c2_28.f;
      bc_h[27] = (x.f <= 45.0) ? __di1_ep___c1_27.f : __di1_ep___c2_27.f;
      bc_h[26] = (x.f <= 45.0) ? __di1_ep___c1_26.f : __di1_ep___c2_26.f;
      bc_h[25] = (x.f <= 45.0) ? __di1_ep___c1_25.f : __di1_ep___c2_25.f;
      bc_h[24] = (x.f <= 45.0) ? __di1_ep___c1_24.f : __di1_ep___c2_24.f;
      bc_h[23] = (x.f <= 45.0) ? __di1_ep___c1_23.f : __di1_ep___c2_23.f;
      bc_h[22] = (x.f <= 45.0) ? __di1_ep___c1_22.f : __di1_ep___c2_22.f;
      bc_h[21] = (x.f <= 45.0) ? __di1_ep___c1_21.f : __di1_ep___c2_21.f;
      bc_h[20] = (x.f <= 45.0) ? __di1_ep___c1_20.f : __di1_ep___c2_20.f;
      bc_h[19] = (x.f <= 45.0) ? __di1_ep___c1_19.f : __di1_ep___c2_19.f;
      bc_h[18] = (x.f <= 45.0) ? __di1_ep___c1_18.f : __di1_ep___c2_18.f;
      bc_h[17] = (x.f <= 45.0) ? __di1_ep___c1_17.f : __di1_ep___c2_17.f;
      bc_h[16] = (x.f <= 45.0) ? __di1_ep___c1_16.f : __di1_ep___c2_16.f;
      bc_h[15] = (x.f <= 45.0) ? __di1_ep___c1_15.f : __di1_ep___c2_15.f;
      bc_h[14] = (x.f <= 45.0) ? __di1_ep___c1_14.f : __di1_ep___c2_14.f;
      bc_h[13] = (x.f <= 45.0) ? __di1_ep___c1_13.f : __di1_ep___c2_13.f;
      bc_h[12] = (x.f <= 45.0) ? __di1_ep___c1_12.f : __di1_ep___c2_12.f;
      bc_h[11] = (x.f <= 45.0) ? __di1_ep___c1_11.f : __di1_ep___c2_11.f;
      bc_h[10] = (x.f <= 45.0) ? __di1_ep___c1_10.f : __di1_ep___c2_10.f;
      bc_h[9] = (x.f <= 45.0) ? __di1_ep___c1_9.f : __di1_ep___c2_9.f;
      bc_h[8] = (x.f <= 45.0) ? __di1_ep___c1_8.f : __di1_ep___c2_8.f;
      bc_h[7] = (x.f <= 45.0) ? __di1_ep___c1_7.f : __di1_ep___c2_7.f;
      bc_h[6] = (x.f <= 45.0) ? __di1_ep___c1_6.f : __di1_ep___c2_6.f;
      bc_h[5] = (x.f <= 45.0) ? __di1_ep___c1_5.f : __di1_ep___c2_5.f;
      bc_h[4] = (x.f <= 45.0) ? __di1_ep___c1_4.f : __di1_ep___c2_4.f;
      bc_h[3] = (x.f <= 45.0) ? __di1_ep___c1_3.f : __di1_ep___c2_3.f;
      bc_h[2] = (x.f <= 45.0) ? __di1_ep___c1_2.f : __di1_ep___c2_2.f;
      bc_h[1] = (x.f <= 45.0) ? __di1_ep___c1_1.f : __di1_ep___c2_1.f;
      bc_h[0] = (x.f <= 45.0) ? __di1_ep___c1_0.f : __di1_ep___c2_0.f;
      bc_l[28] = (x.f <= 45.0) ? __di1_ep___c1_28l.f : __di1_ep___c2_28l.f;
      bc_l[27] = (x.f <= 45.0) ? __di1_ep___c1_27l.f : __di1_ep___c2_27l.f;
      bc_l[26] = (x.f <= 45.0) ? __di1_ep___c1_26l.f : __di1_ep___c2_26l.f;
      bc_l[25] = (x.f <= 45.0) ? __di1_ep___c1_25l.f : __di1_ep___c2_25l.f;
      bc_l[24] = (x.f <= 45.0) ? __di1_ep___c1_24l.f : __di1_ep___c2_24l.f;
      bc_l[23] = (x.f <= 45.0) ? __di1_ep___c1_23l.f : __di1_ep___c2_23l.f;
      bc_l[22] = (x.f <= 45.0) ? __di1_ep___c1_22l.f : __di1_ep___c2_22l.f;
      bc_l[21] = (x.f <= 45.0) ? __di1_ep___c1_21l.f : __di1_ep___c2_21l.f;
      bc_l[20] = (x.f <= 45.0) ? __di1_ep___c1_20l.f : __di1_ep___c2_20l.f;
      bc_l[19] = (x.f <= 45.0) ? __di1_ep___c1_19l.f : __di1_ep___c2_19l.f;
      bc_l[18] = (x.f <= 45.0) ? __di1_ep___c1_18l.f : __di1_ep___c2_18l.f;
      bc_l[17] = (x.f <= 45.0) ? __di1_ep___c1_17l.f : __di1_ep___c2_17l.f;
      bc_l[16] = (x.f <= 45.0) ? __di1_ep___c1_16l.f : __di1_ep___c2_16l.f;
      bc_l[15] = (x.f <= 45.0) ? __di1_ep___c1_15l.f : __di1_ep___c2_15l.f;
      bc_l[14] = (x.f <= 45.0) ? __di1_ep___c1_14l.f : __di1_ep___c2_14l.f;
      bc_l[13] = (x.f <= 45.0) ? __di1_ep___c1_13l.f : __di1_ep___c2_13l.f;
      bc_l[12] = (x.f <= 45.0) ? __di1_ep___c1_12l.f : __di1_ep___c2_12l.f;
      bc_l[11] = (x.f <= 45.0) ? __di1_ep___c1_11l.f : __di1_ep___c2_11l.f;
      bc_l[10] = (x.f <= 45.0) ? __di1_ep___c1_10l.f : __di1_ep___c2_10l.f;
      bc_l[9] = (x.f <= 45.0) ? __di1_ep___c1_9l.f : __di1_ep___c2_9l.f;
      bc_l[8] = (x.f <= 45.0) ? __di1_ep___c1_8l.f : __di1_ep___c2_8l.f;
      bc_l[7] = (x.f <= 45.0) ? __di1_ep___c1_7l.f : __di1_ep___c2_7l.f;
      bc_l[6] = (x.f <= 45.0) ? __di1_ep___c1_6l.f : __di1_ep___c2_6l.f;
      bc_l[5] = (x.f <= 45.0) ? __di1_ep___c1_5l.f : __di1_ep___c2_5l.f;
      bc_l[4] = (x.f <= 45.0) ? __di1_ep___c1_4l.f : __di1_ep___c2_4l.f;
      bc_l[3] = (x.f <= 45.0) ? __di1_ep___c1_3l.f : __di1_ep___c2_3l.f;
      bc_l[2] = (x.f <= 45.0) ? __di1_ep___c1_2l.f : __di1_ep___c2_2l.f;
      bc_l[1] = (x.f <= 45.0) ? __di1_ep___c1_1l.f : __di1_ep___c2_1l.f;
      bc_l[0] = (x.f <= 45.0) ? __di1_ep___c1_0l.f : __di1_ep___c2_0l.f;
      // poly ~ 2^64*BesselI(1, x.f)/exp(x.f)
      poly = bc_h[28];
      poly_l = bc_l[28];
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
      bc_h[21] = (x.f <= 325.0) ? __di1_ep___c3_21.f : __di1_ep___c4_21.f;
      bc_h[20] = (x.f <= 325.0) ? __di1_ep___c3_20.f : __di1_ep___c4_20.f;
      bc_h[19] = (x.f <= 325.0) ? __di1_ep___c3_19.f : __di1_ep___c4_19.f;
      bc_h[18] = (x.f <= 325.0) ? __di1_ep___c3_18.f : __di1_ep___c4_18.f;
      bc_h[17] = (x.f <= 325.0) ? __di1_ep___c3_17.f : __di1_ep___c4_17.f;
      bc_h[16] = (x.f <= 325.0) ? __di1_ep___c3_16.f : __di1_ep___c4_16.f;
      bc_h[15] = (x.f <= 325.0) ? __di1_ep___c3_15.f : __di1_ep___c4_15.f;
      bc_h[14] = (x.f <= 325.0) ? __di1_ep___c3_14.f : __di1_ep___c4_14.f;
      bc_h[13] = (x.f <= 325.0) ? __di1_ep___c3_13.f : __di1_ep___c4_13.f;
      bc_h[12] = (x.f <= 325.0) ? __di1_ep___c3_12.f : __di1_ep___c4_12.f;
      bc_h[11] = (x.f <= 325.0) ? __di1_ep___c3_11.f : __di1_ep___c4_11.f;
      bc_h[10] = (x.f <= 325.0) ? __di1_ep___c3_10.f : __di1_ep___c4_10.f;
      bc_h[9] = (x.f <= 325.0) ? __di1_ep___c3_9.f : __di1_ep___c4_9.f;
      bc_h[8] = (x.f <= 325.0) ? __di1_ep___c3_8.f : __di1_ep___c4_8.f;
      bc_h[7] = (x.f <= 325.0) ? __di1_ep___c3_7.f : __di1_ep___c4_7.f;
      bc_h[6] = (x.f <= 325.0) ? __di1_ep___c3_6.f : __di1_ep___c4_6.f;
      bc_h[5] = (x.f <= 325.0) ? __di1_ep___c3_5.f : __di1_ep___c4_5.f;
      bc_h[4] = (x.f <= 325.0) ? __di1_ep___c3_4.f : __di1_ep___c4_4.f;
      bc_h[3] = (x.f <= 325.0) ? __di1_ep___c3_3.f : __di1_ep___c4_3.f;
      bc_h[2] = (x.f <= 325.0) ? __di1_ep___c3_2.f : __di1_ep___c4_2.f;
      bc_h[1] = (x.f <= 325.0) ? __di1_ep___c3_1.f : __di1_ep___c4_1.f;
      bc_h[0] = (x.f <= 325.0) ? __di1_ep___c3_0.f : __di1_ep___c4_0.f;
      bc_l[21] = (x.f <= 325.0) ? __di1_ep___c3_21l.f : __di1_ep___c4_21l.f;
      bc_l[20] = (x.f <= 325.0) ? __di1_ep___c3_20l.f : __di1_ep___c4_20l.f;
      bc_l[19] = (x.f <= 325.0) ? __di1_ep___c3_19l.f : __di1_ep___c4_19l.f;
      bc_l[18] = (x.f <= 325.0) ? __di1_ep___c3_18l.f : __di1_ep___c4_18l.f;
      bc_l[17] = (x.f <= 325.0) ? __di1_ep___c3_17l.f : __di1_ep___c4_17l.f;
      bc_l[16] = (x.f <= 325.0) ? __di1_ep___c3_16l.f : __di1_ep___c4_16l.f;
      bc_l[15] = (x.f <= 325.0) ? __di1_ep___c3_15l.f : __di1_ep___c4_15l.f;
      bc_l[14] = (x.f <= 325.0) ? __di1_ep___c3_14l.f : __di1_ep___c4_14l.f;
      bc_l[13] = (x.f <= 325.0) ? __di1_ep___c3_13l.f : __di1_ep___c4_13l.f;
      bc_l[12] = (x.f <= 325.0) ? __di1_ep___c3_12l.f : __di1_ep___c4_12l.f;
      bc_l[11] = (x.f <= 325.0) ? __di1_ep___c3_11l.f : __di1_ep___c4_11l.f;
      bc_l[10] = (x.f <= 325.0) ? __di1_ep___c3_10l.f : __di1_ep___c4_10l.f;
      bc_l[9] = (x.f <= 325.0) ? __di1_ep___c3_9l.f : __di1_ep___c4_9l.f;
      bc_l[8] = (x.f <= 325.0) ? __di1_ep___c3_8l.f : __di1_ep___c4_8l.f;
      bc_l[7] = (x.f <= 325.0) ? __di1_ep___c3_7l.f : __di1_ep___c4_7l.f;
      bc_l[6] = (x.f <= 325.0) ? __di1_ep___c3_6l.f : __di1_ep___c4_6l.f;
      bc_l[5] = (x.f <= 325.0) ? __di1_ep___c3_5l.f : __di1_ep___c4_5l.f;
      bc_l[4] = (x.f <= 325.0) ? __di1_ep___c3_4l.f : __di1_ep___c4_4l.f;
      bc_l[3] = (x.f <= 325.0) ? __di1_ep___c3_3l.f : __di1_ep___c4_3l.f;
      bc_l[2] = (x.f <= 325.0) ? __di1_ep___c3_2l.f : __di1_ep___c4_2l.f;
      bc_l[1] = (x.f <= 325.0) ? __di1_ep___c3_1l.f : __di1_ep___c4_1l.f;
      bc_l[0] = (x.f <= 325.0) ? __di1_ep___c3_0l.f : __di1_ep___c4_0l.f;
      // x*2^(-8)
      xs.f = __fma(x.f, __di1_ep___scx.f, 0.0);
      // poly ~ 2^64*BesselI(1, x.f)/exp(x.f)
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
        double __ph, __ahl, __ahh, __alh;
        double __ah, __bh, __pl;
        __bh = (__fabs(poly) <= __fabs(bc_h[8])) ? (bc_h[8]) : (poly);
        __ah = (__fabs(poly) <= __fabs(bc_h[8])) ? (poly) : (bc_h[8]);
        __ph = __fma(__ah, 1.0, __bh);
        __ahh = __fma(__ph, 1.0, -__bh);
        __ahl = __fma(__ah, 1.0, -__ahh);
        __pl = (poly_l + bc_l[8]) + __ahl;
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
    S.f = __fma(x.f, __di1_ep___L2E.f, __di1_ep___Shifter.f);
    // (int)(x2h*L2E)
    N.f = S.f - __di1_ep___Shifter.f;
    // x^2 - N*log(2)
    R0 = __fma(-N.f, __di1_ep___L2H.f, x.f);
    R1 = __fma(-N.f, __di1_ep___L2L.f, 0.0);
    R = R0 + R1;
    R1h = R - R0;
    Rl = R1 - R1h;
    // 2^(N)
    Te.w = S.w << 52;
    // exp(R)-1
    epoly.f = __fma(__di1_ep___ce11.f, R, __di1_ep___ce10.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce9.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce8.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce7.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce6.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce5.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce4.f);
    epoly.f = __fma(epoly.f, R, __di1_ep___ce3.f);
    epoly2.f = __fma(epoly.f, R, __di1_ep___ce2.f);
    epoly.f = __fma(epoly2.f, R, __di1_ep___ce1.f);
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
    nRet = (H.w == 0x7ff0000000000000UL) ? 3 : nRet;
    H.w ^= sgn_x.w;
    *pres = H.f;
    return nRet;
  }
  H.w = 0x7ff0000000000000UL;
  H.f += x.f; // large inputs overflow; quietize NaNs
              // large inputs overflow
  nRet = (x.w < 0x7ff0000000000000UL) ? 3 : nRet;
  H.w ^= sgn_x.w;
  *pres = H.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_i1_d_ep */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_i1(double x) {
  using namespace __imf_impl_i1_d_ep;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    VUINT64 lRangeMask;
    lRangeMask = 0xFFFFuLL;
    vm = 0;
    vm = lRangeMask;
    vr1 = va1;
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_di1(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
