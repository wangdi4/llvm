// Copyright (c) 2006-2012 Intel Corporation
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
//
// vector_comparator.h

#ifndef VECTOR_COMPARATOR_GTEST_
#define VECTOR_COMPARATOR_GTEST_

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <gtest/gtest.h>
#include <CL/cl.h>

/**
 * VectorComparator - responsible for comparing values
 **/
template <typename T>
class VectorComparator{
public:
	bool compare(T rhs, T lhs){
		if(rhs != lhs){
			return false;
		}
		return true;
	}
};

template <>
bool VectorComparator<HalfWrapper>::compare(HalfWrapper rhs, HalfWrapper lhs){
	if(rhs.half != lhs.half){
		return false;
	}
	return true;
}

template <>
bool VectorComparator<cl_char2>::compare(cl_char2 rhs, cl_char2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_char4>::compare(cl_char4 rhs, cl_char4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_char8>::compare(cl_char8 rhs, cl_char8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_char16>::compare(cl_char16 rhs, cl_char16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uchar2>::compare(cl_uchar2 rhs, cl_uchar2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uchar4>::compare(cl_uchar4 rhs, cl_uchar4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uchar8>::compare(cl_uchar8 rhs, cl_uchar8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uchar16>::compare(cl_uchar16 rhs, cl_uchar16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_short2>::compare(cl_short2 rhs, cl_short2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_short4>::compare(cl_short4 rhs, cl_short4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_short8>::compare(cl_short8 rhs, cl_short8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_short16>::compare(cl_short16 rhs, cl_short16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ushort2>::compare(cl_ushort2 rhs, cl_ushort2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ushort4>::compare(cl_ushort4 rhs, cl_ushort4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ushort8>::compare(cl_ushort8 rhs, cl_ushort8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ushort16>::compare(cl_ushort16 rhs, cl_ushort16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_int2>::compare(cl_int2 rhs,cl_int2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_int4>::compare(cl_int4 rhs, cl_int4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_int8>::compare(cl_int8 rhs, cl_int8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_int16>::compare(cl_int16 rhs, cl_int16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uint2>::compare(cl_uint2 rhs, cl_uint2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uint4>::compare(cl_uint4 rhs, cl_uint4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uint8>::compare(cl_uint8 rhs, cl_uint8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_uint16>::compare(cl_uint16 rhs, cl_uint16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_long2>::compare(cl_long2 rhs,cl_long2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_long4>::compare(cl_long4 rhs, cl_long4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_long8>::compare(cl_long8 rhs, cl_long8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_long16>::compare(cl_long16 rhs, cl_long16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ulong2>::compare(cl_ulong2 rhs, cl_ulong2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ulong4>::compare(cl_ulong4 rhs, cl_ulong4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ulong8>::compare(cl_ulong8 rhs, cl_ulong8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_ulong16>::compare(cl_ulong16 rhs, cl_ulong16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_float2>::compare(cl_float2 rhs, cl_float2 lhs){
	for(int i=0;i<2;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_float4>::compare(cl_float4 rhs, cl_float4 lhs){
	for(int i=0;i<4;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_float8>::compare(cl_float8 rhs, cl_float8 lhs){
	for(int i=0;i<8;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<cl_float16>::compare(cl_float16 rhs, cl_float16 lhs){
	for(int i=0;i<16;++i){
		if(rhs.s[i] != lhs.s[i]){
			return false;
		}
	}
	return true;
}

template <>
bool VectorComparator<UserDefinedStructure>::compare(UserDefinedStructure rhs, UserDefinedStructure lhs){
	if(rhs.x != lhs.x || fabs(fabs(rhs.y)-fabs(lhs.y))>1.0f || rhs.z != lhs.z){
		return false;
	}
	return true;
}


#endif /* VECTOR_COMPARATOR_GTEST_ */