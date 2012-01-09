// Copyright (c) 1997-2004 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <stdio.h>
#include <string.h>
#include "Chapter_2_3.h"

Chapter_2_3::Chapter_2_3(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_3::~Chapter_2_3()
{
}

int Chapter_2_3::Run(const char* test, int rc)
{
    rc |= CPU_2_3_1(test);
    rc |= CPU_2_3_2(test);
    rc |= CPU_2_3_3(test);
    rc |= CPU_2_3_4(test);
    rc |= CPU_2_3_5(test);
    rc |= CPU_2_3_6(test);
    rc |= CPU_2_3_7(test);
    rc |= CPU_2_3_8(test);
    rc |= CPU_2_3_9(test);
    rc |= CPU_2_3_10(test);
    rc |= CPU_2_3_11(test);
    rc |= CPU_2_3_12(test);
    rc |= CPU_2_3_13(test);
    rc |= CPU_2_3_14(test);
    rc |= CPU_2_3_15(test);
    rc |= CPU_2_3_16(test);
    rc |= CPU_2_3_17(test);
    rc |= CPU_2_3_18(test);
    rc |= CPU_2_3_19(test);
    rc |= CPU_2_3_20(test);
    rc |= CPU_2_3_21(test);
    rc |= CPU_2_3_22(test);
    rc |= CPU_2_3_23(test);
    rc |= CPU_2_3_24(test);
    rc |= CPU_2_3_25(test);
    rc |= CPU_2_3_26(test);
    rc |= CPU_2_3_27(test);
    rc |= CPU_2_3_28(test);
    rc |= CPU_2_3_29(test);
    rc |= CPU_2_3_30(test);
    rc |= CPU_2_3_31(test);
    rc |= CPU_2_3_32(test);
    rc |= CPU_2_3_33(test);
    rc |= CPU_2_3_34(test);
    rc |= CPU_2_3_35(test);
    rc |= CPU_2_3_36(test);
    rc |= CPU_2_3_37(test);
    rc |= CPU_2_3_37(test);
    rc |= CPU_2_3_38(test);
    rc |= CPU_2_3_39(test);
    rc |= CPU_2_3_40(test);
    rc |= CPU_2_3_41(test);
    rc |= CPU_2_3_43(test);
    rc |= CPU_2_3_43(test);
    rc |= CPU_2_3_45(test);
    rc |= CPU_2_3_46(test);
    rc |= CPU_2_3_47(test);
    rc |= CPU_2_3_48(test);
    rc |= CPU_2_3_49(test);
    rc |= CPU_2_3_50(test);
    rc |= CPU_2_3_51(test);
    rc |= CPU_2_3_52(test);
    rc |= CPU_2_3_53(test);

    rc |= GPU_2_3_1(test);
    rc |= GPU_2_3_2(test);
    rc |= GPU_2_3_3(test);
    rc |= GPU_2_3_4(test);
    rc |= GPU_2_3_5(test);
    rc |= GPU_2_3_6(test);
    rc |= GPU_2_3_7(test);
    rc |= GPU_2_3_8(test);
    rc |= GPU_2_3_9(test);
    rc |= GPU_2_3_10(test);
    rc |= GPU_2_3_11(test);
    rc |= GPU_2_3_12(test);
    rc |= GPU_2_3_13(test);
    rc |= GPU_2_3_14(test);
    rc |= GPU_2_3_15(test);
    rc |= GPU_2_3_16(test);
    rc |= GPU_2_3_17(test);
    rc |= GPU_2_3_18(test);
    rc |= GPU_2_3_19(test);
    rc |= GPU_2_3_20(test);
    rc |= GPU_2_3_21(test);
    rc |= GPU_2_3_22(test);
    rc |= GPU_2_3_23(test);
    rc |= GPU_2_3_24(test);
    rc |= GPU_2_3_25(test);
    rc |= GPU_2_3_26(test);
    rc |= GPU_2_3_27(test);
    rc |= GPU_2_3_28(test);
    rc |= GPU_2_3_29(test);
    rc |= GPU_2_3_30(test);
    rc |= GPU_2_3_31(test);
    rc |= GPU_2_3_32(test);
    rc |= GPU_2_3_33(test);
    rc |= GPU_2_3_34(test);
    rc |= GPU_2_3_35(test);
    rc |= GPU_2_3_36(test);
    rc |= GPU_2_3_37(test);
    rc |= GPU_2_3_37(test);
    rc |= GPU_2_3_38(test);
    rc |= GPU_2_3_39(test);
    rc |= GPU_2_3_40(test);
    rc |= GPU_2_3_41(test);
    rc |= GPU_2_3_43(test);
    rc |= GPU_2_3_43(test);
    rc |= GPU_2_3_45(test);
    rc |= GPU_2_3_46(test);
    rc |= GPU_2_3_47(test);
    rc |= GPU_2_3_48(test);
    rc |= GPU_2_3_49(test);
    rc |= GPU_2_3_50(test);
    rc |= GPU_2_3_51(test);
    rc |= GPU_2_3_52(test);
    rc |= GPU_2_3_53(test);

    rc |= ACCELERATOR_2_3_1(test);
    rc |= ACCELERATOR_2_3_2(test);
    rc |= ACCELERATOR_2_3_3(test);
    rc |= ACCELERATOR_2_3_4(test);
    rc |= ACCELERATOR_2_3_5(test);
    rc |= ACCELERATOR_2_3_6(test);
    rc |= ACCELERATOR_2_3_7(test);
    rc |= ACCELERATOR_2_3_8(test);
    rc |= ACCELERATOR_2_3_9(test);
    rc |= ACCELERATOR_2_3_10(test);
    rc |= ACCELERATOR_2_3_11(test);
    rc |= ACCELERATOR_2_3_12(test);
    rc |= ACCELERATOR_2_3_13(test);
    rc |= ACCELERATOR_2_3_14(test);
    rc |= ACCELERATOR_2_3_15(test);
    rc |= ACCELERATOR_2_3_16(test);
    rc |= ACCELERATOR_2_3_17(test);
    rc |= ACCELERATOR_2_3_18(test);
    rc |= ACCELERATOR_2_3_19(test);
    rc |= ACCELERATOR_2_3_20(test);
    rc |= ACCELERATOR_2_3_21(test);
    rc |= ACCELERATOR_2_3_22(test);
    rc |= ACCELERATOR_2_3_23(test);
    rc |= ACCELERATOR_2_3_24(test);
    rc |= ACCELERATOR_2_3_25(test);
    rc |= ACCELERATOR_2_3_26(test);
    rc |= ACCELERATOR_2_3_27(test);
    rc |= ACCELERATOR_2_3_28(test);
    rc |= ACCELERATOR_2_3_29(test);
    rc |= ACCELERATOR_2_3_30(test);
    rc |= ACCELERATOR_2_3_31(test);
    rc |= ACCELERATOR_2_3_32(test);
    rc |= ACCELERATOR_2_3_33(test);
    rc |= ACCELERATOR_2_3_34(test);
    rc |= ACCELERATOR_2_3_35(test);
    rc |= ACCELERATOR_2_3_36(test);
    rc |= ACCELERATOR_2_3_37(test);
    rc |= ACCELERATOR_2_3_37(test);
    rc |= ACCELERATOR_2_3_38(test);
    rc |= ACCELERATOR_2_3_39(test);
    rc |= ACCELERATOR_2_3_40(test);
    rc |= ACCELERATOR_2_3_41(test);
    rc |= ACCELERATOR_2_3_43(test);
    rc |= ACCELERATOR_2_3_43(test);
    rc |= ACCELERATOR_2_3_45(test);
    rc |= ACCELERATOR_2_3_46(test);
    rc |= ACCELERATOR_2_3_47(test);
    rc |= ACCELERATOR_2_3_48(test);
    rc |= ACCELERATOR_2_3_49(test);
    rc |= ACCELERATOR_2_3_50(test);
    rc |= ACCELERATOR_2_3_51(test);
    rc |= ACCELERATOR_2_3_52(test);
    rc |= ACCELERATOR_2_3_53(test);

    return rc;
}