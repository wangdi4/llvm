/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ALUTest.h

\*****************************************************************************/
#ifndef __ALUTEST_H__
#define __ALUTEST_H__

/// Common class for ALU tests. Each NEAT ALU test can use it as a base class
/// It prints seed if the test fails

#include <gtest/gtest.h>            // Test framework

#include "DGHelper.h"

// seed taken from command line
extern uint64_t seedForValidation; 

class ALUTest : public ::testing::Test {
public:
    uint64_t seed;

    ALUTest()
    {
        // take external seed from TestMain.cpp, if 0, produce seed inside generator
        seed = Validation::SetSeed(seedForValidation);
    }

    ~ALUTest()
    {
            // if some test fails, print seed
            if (HasFailure())
            {
                std::cout << "Seed: " << seed << std::endl;
            }
    }

};
#endif // #ifndef __ALUTEST_H__
