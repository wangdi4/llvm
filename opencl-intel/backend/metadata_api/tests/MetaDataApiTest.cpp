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

  File Name: TestBuiltinKeeper.cpp

\****************************************************************************/

#include <gtest/gtest.h>
#include <utility>
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#include "MetaDataApi.h"


using namespace Intel;
//
//
TEST(MetaDataTest, basic)
{
    //
    //parse the test ir file into a module
    //
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> pModule;
    llvm::SMDiagnostic errDiagnostic;
    pModule = llvm::parseIRFile("metadatatest.ll", errDiagnostic, context);
    //
    // read the kernels and print their names
    MetaDataUtils moduleMD(pModule.release());

    MetaDataUtils::KernelsList::iterator i = moduleMD.begin_Kernels();
    MetaDataUtils::KernelsList::iterator e = moduleMD.end_Kernels();

    for(; i != e; ++i )
    {
        KernelMetaDataHandle kernel = *i;

        std::cout <<  "kernel name:" << kernel->getFunction()->getName().str() << std::endl;

        if( kernel->isVecTypeHintHasValue() )
        {
            std::cout <<  "VecTypeHist:" << kernel->getVecTypeHint() << std::endl;
        }


       KernelMetaData::ArgNameList::iterator ai = kernel->begin_ArgName();
       KernelMetaData::ArgNameList::iterator ae = kernel->end_ArgName();

       for(; ai != ae; ++ai)
       {
           std::cout << "arg:" << *ai << std::endl;
       }


    }
    SUCCEED();
}

int main(int argc, char** argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
