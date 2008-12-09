#pragma once

#include "cl.h"

// CL_SUCCEEDED
// Checks whether a return code is success
#define VET_SUCCEEDED(code)         (CL_SUCCESS == (code))

// CL_FAILED
// Checks whether a return code is failure
#define VET_FAILED(code)			(CL_SUCCESS > (code))

// custom error codes
#define		CL_NOT_IMPLEMENTED		-800