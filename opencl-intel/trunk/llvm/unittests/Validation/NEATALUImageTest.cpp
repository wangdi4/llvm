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

File Name:  NEATALUImageTest.cpp

\*****************************************************************************/

#include <gtest/gtest.h>            // Test framework
#include <memory>

#include "DataGenerator.h"
#include "DGHelper.h"

#include "NEATVector.h"
#include "RefALU.h"
#include "NEATALU.h"
#include "NEATValue.h"

#include "NEATALUUtils.h"
#include "ALUTest.h"
#include "llvm/System/DataTypes.h"

using namespace Validation;
class ImageALUNEAT : public ALUTest
{
protected:
    virtual void SetUp() {};
};

TEST_F(ImageALUNEAT, read_imagef_src_noneat_test)
{
    const int WIDTH = 2;
    const int HEIGHT = 2;
    const int NUMPIXELS = WIDTH * HEIGHT;

    struct XYZ 
    {
        float x,y,z;
    };

    // Fill up argument values with random data
    //GenerateRandomVectorsAutoSeed(F32, spData->pData, V1, NUMPIXELS);

    float pixData[NUMPIXELS] = 
    {
        0.0f, 1.0f, 
        0.0f, 1.0f
    };

    const XYZ coords [NUMPIXELS] = {
        { 0.0f, 0.0f, 0.0f},
        { 1.0f, 0.0f, 0.0f},
        { 0.0f, 1.0f, 0.0f},
        { 1.0f, 1.0f, 0.0f}
    };

    cl_image_format img_fmt;
    Conformance::image_descriptor imageInfo;
    Conformance::image_sampler_data imageSampler;

    // test for CL_R, CL_FLOAT image
    img_fmt.image_channel_data_type = CL_FLOAT;
    img_fmt.image_channel_order = CL_R;

    imageInfo.depth = 2;
    imageInfo.width = WIDTH;
    imageInfo.height = HEIGHT;
    imageInfo.rowPitch = WIDTH * sizeof(float);
    imageInfo.format = &img_fmt;

    imageSampler.addressing_mode = CL_ADDRESS_NONE;
    imageSampler.normalized_coords = false;

    imageSampler.filter_mode = CL_FILTER_NEAREST;
    for(int32_t cntPix = 0; cntPix < NUMPIXELS; ++cntPix)
    {
        const float pixRef[4] = { pixData[cntPix], 0.0f, 0.0f, 1.0f };

        NEATVector vecNeat = NEATALU::read_imagef_src_noneat(pixData, 
            &imageInfo,
            coords[cntPix].x, coords[cntPix].y, coords[cntPix].z, 
            &imageSampler);
        
        EXPECT_TRUE(vecNeat[0].Includes(pixRef[0]));
        EXPECT_TRUE(vecNeat[1].IsAcc() && (vecNeat[1].Includes(pixRef[1])));
        EXPECT_TRUE(vecNeat[2].IsAcc() && (vecNeat[2].Includes(pixRef[2])));
        EXPECT_TRUE(vecNeat[3].IsAcc() && (vecNeat[3].Includes(pixRef[3])));

    }


}

