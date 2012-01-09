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

#include <stdlib.h>
#include "stdafx.h"
#include "Test_6_11_1.h"

Test_6_11_1::Test_6_11_1(tDiscrete* pDiscrete):
    CBuildInFunc(pDiscrete)
{
}

Test_6_11_1::~Test_6_11_1()
{
}

int Test_6_11_1::Run(int rc)
{
    rc |= Test_6_11_1_get_work_dim();

    return rc;
}

int Test_6_11_1::Test_6_11_1_get_work_dim()
{
	// constants
	const char		FUNCTION[] = "get_work_dim";
	const char		TAIL[] = "0";
	const char      TEST[] = "6_11_1";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST, FUNCTION, TAIL))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	Arrangement     ar;
	if (PROCESSED_OK != ar.Init(TEST, FUNCTION, TAIL))
		return Finish(&status, TEST, FUNCTION, TAIL, PROCESSED_FAIL);
	cl_uint			*dim = new cl_uint[ar.GetItems()];
	memset(dim, 0, sizeof(cl_uint) * ar.GetItems());
	if (PROCESSED_OK != ar.SetParameter(dim, sizeof(cl_uint) * ar.GetItems()))
		return Finish(&status, TEST, FUNCTION, TAIL, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.InvokeKernel())
		return Finish(&status, TEST, FUNCTION, TAIL, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.GetResults(dim, sizeof(cl_uint) * ar.GetItems()))
		return Finish(&status, TEST, FUNCTION, TAIL, PROCESSED_FAIL);
	unsigned		i;
	for (i = 0; i < ar.GetItems(); i++)
	{
		if (dim[i] != 1)
		{
			rc = PROCESSED_FAIL;
			break;
		}
	}

	return Finish(&status, TEST, FUNCTION, TAIL, rc);
}
