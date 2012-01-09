// Copyright (c) 1997-2004 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTO_S_/_"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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

#pragma once
#include "DiscreteClass.h"

class Chapter_2_14: 
    public Helper
{
private:
    static int      context_2_14_1(const char* test);
    static int      context_2_14_2(const char* test);
    static int      context_2_14_3(const char* test);
    static int      context_2_14_4(const char* test);
    static int      context_2_14_5(const char* test);
    static int      context_2_14_6(const char* test);
    static int      context_2_14_7(const char* test);
    static int      context_2_14_8(const char* test);
    static int      context_2_14_9(const char* test);
    static int      context_2_14_10(const char* test);
    static int      context_2_14_11(const char* test);
    static int      context_2_14_12(const char* test);
    static int      context_2_14_13(const char* test);
    static int      context_2_14_14(const char* test);
    static int      context_2_14_15(const char* test);
    static int      context_2_14_16(const char* test);
    static int      context_2_14_17(const char* test);
    static int      empty_2_14_18(const char* test);

public:
                    Chapter_2_14(tDiscrete* pDiscr);
                    ~Chapter_2_14(void);

    static int      Run(const char* test, int rc);
};