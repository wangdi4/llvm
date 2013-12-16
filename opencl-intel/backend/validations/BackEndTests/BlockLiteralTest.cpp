/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BlockLiteralTest.cpp

\*****************************************************************************/


#include "BlockLiteral.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

using namespace Intel::OpenCL::DeviceBackend;


struct MyBlockLiteral
{
  /// initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
    void *isa; 
    int flags;
    int reserved;
    /// invoke address of Block function
    void *invoke; 
    struct Block_descriptor {
      int64_t reserved;
      /// size of BlockLiteral structure
      int64_t size;
    };
    Block_descriptor *desc;
    int A;
    float b[16];
};

static BlockLiteral * InitTestBlockLiteral(MyBlockLiteral *in, MyBlockLiteral::Block_descriptor *bd)
{
  assert(in && bd && "input args NULL");
  in->invoke = (void*) 0xDEADDEAD;
  in->desc = (MyBlockLiteral::Block_descriptor*)bd;
  bd->size = sizeof(MyBlockLiteral);
  in->A = 0xDEAD;
  in->b[0] = 2.7f;
  in->b[15] = 38.7f;
  return reinterpret_cast<BlockLiteral*>(in);
}


/// test to copy BlockLiteral to memory and extract it from here
TEST(BlockLiteralTest, SerializeDeserialize)
{
  std::auto_ptr<MyBlockLiteral> MyBL(new MyBlockLiteral);
  std::auto_ptr<MyBlockLiteral::Block_descriptor> MyBD(new MyBlockLiteral::Block_descriptor);

  BlockLiteral * pTestBL = InitTestBlockLiteral(MyBL.get(), MyBD.get());

  // get needed space for buffer
  const size_t memneeded = pTestBL->GetBufferSizeForSerialization();
  // alloc space for dest buffer
  std::vector<char> DestBuf(memneeded);
  // copy to buffer
  pTestBL->Serialize(&DestBuf[0]);

  // alloc space for another buffer to copy initial
  std::vector<char> NewDestBuf(DestBuf);

  // init BL from copied buffer
  BlockLiteral * pNewBL = BlockLiteral::DeserializeInBuffer(&NewDestBuf[0]);

  // check invoke addr
  EXPECT_EQ(MyBL->invoke, pNewBL->GetInvoke());

  // extract and test int field
  int* pImportedInt = (int*)((char*)pNewBL + sizeof(BlockLiteral));
  EXPECT_EQ(MyBL->A, *pImportedInt);

  // extract and test float
  float* pImportedFloat = (float*)((char*)pNewBL + sizeof(BlockLiteral) + sizeof(int));
  EXPECT_EQ(MyBL->b[0], pImportedFloat[0]);
  EXPECT_EQ(MyBL->b[15], pImportedFloat[15]);
}


