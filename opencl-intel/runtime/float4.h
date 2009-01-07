#pragma once


/*****************************************************************************************************************************
*		Base class defenition
*****************************************************************************************************************************/

// float4 structure defenition
struct __declspec(align(16)) float4
{
	float4(const __m128& val)
	{
		xyzw = val;
	}
	friend float4 operator+(float4& origVal, float4& addVal);
	friend float dot(const float4 &a, const float4 &b);

	__forceinline float4& operator+(float4& addVal)
	{
		xyzw = _mm_add_ps(xyzw, addVal.xyzw);
		return *this;
	}
	
	union
	{
		struct
		{
			float	x;
			float	y;
			float	z;
			float	w;
		};
		__m128	xyzw;
	};
};

// float4 operator overloading
__forceinline float4 operator+(float4& origVal, float4& addVal)
{
	return _mm_add_ps(origVal.xyzw, addVal.xyzw);
}

/*****************************************************************************************************************************
*		Geometry function implementation
*****************************************************************************************************************************/
__forceinline float dot(const float4 &a, const float4 &b)
{
	__m128 tmp = _mm_mul_ps(a.xyzw, b.xyzw);
	tmp = _mm_hadd_ps(tmp, tmp);
	tmp = _mm_hadd_ps(tmp, tmp);
	return  _mm_cvtss_f32(tmp);
}
