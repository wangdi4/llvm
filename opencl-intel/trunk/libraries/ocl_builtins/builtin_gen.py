import sys
from optparse import OptionParser

class Target:
	def __init__(self,cpu,svml_id,is64Bit):
		self.cpu=cpu
		self.svml_id=svml_id
		self.is64Bit=is64Bit

def main():
	parser = OptionParser()
	parser.add_option("-o", action='store', dest="out_file", default ='conversions_svml.inc',
                  help="Specify output file")
	(options, args ) = parser.parse_args()
    
	out = open(options.out_file, "w")
    
	targets = [Target('Nehalem', 'n8', False),
		Target('Nehalem', 'h8', True),
		Target('Penryn',  'p8', False),
		Target('Penryn',  'y8', True),
		Target('core2',  'v8', False),
		Target('core2',  'u8', True),
		Target('Prescot',  't7', False),
		Target('Prescot',  'e7', True),
		Target('Sandybridge',  'g9', False),
		Target('Sandybridge',  'e9', True)]

	out.write( '///------------ Beginning of auto-generated section ------------------///\n')
	out.write( '///------------ This file is built by builtin_gen.py ------------------///')
	for target in targets:
		        
		out.write( """

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprtn1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprtn2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprtn4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprtn8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprtn16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprtz1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprtz2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprtz4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprtz8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprtz16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofpup1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofpup2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofpup4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofpup8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofpup16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofpdown1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofpdown2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofpdown4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofpdown8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofpdown16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtnf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtnf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float3 __ocl_svml_{arch}_cvtu64tofprtnf3(_3u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtnf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtnf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtnf16(_16u64);
	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtef1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtef2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float3 __ocl_svml_{arch}_cvtu64tofprtef3(_3u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtef4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtef8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtef16(_16u64);
	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtzf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtzf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtzf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtzf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtzf16(_16u64);
	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofpupf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofpupf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofpupf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofpupf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofpupf16(_16u64);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofpdownf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofpdownf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofpdownf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofpdownf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofpdownf16(_16u64);
	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtnf1(_1i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtnf2(_2i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtnf4(_4i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtnf8(_8i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtnf16(_16i64 x);
	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtef1(_1i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtef2(_2i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtef4(_4i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtef8(_8i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtef16(_16i64 x);

	
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtzf1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtzf2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtzf4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtzf8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtzf16(_16i64);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofpupf1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofpupf2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofpupf4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofpupf8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofpupf16(_16i64);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofpdownf1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofpdownf2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofpdownf4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofpdownf8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofpdownf16(_16i64);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf16(float16);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64upnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64upnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64upnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64upnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64upnosatf16(float16);
	
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64downnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64downnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64downnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64downnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64downnosatf16(float16);
	
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf16(float16);

    /// This function appears to use the standard calling convention.
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtznosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtznosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtznosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtznosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtznosatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64upnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64upnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64upnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64upnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64upnosatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64downnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64downnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64downnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64downnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64downnosatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64upsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64upsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64upsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64upsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64upsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64downsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64downsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64downsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64downsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64downsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtzsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtzsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtzsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtzsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtzsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64upsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64upsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64upsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64upsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64upsatf16(float16);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64downsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64downsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64downsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64downsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64downsatf16(float16);
    
	__attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtnnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtnnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtnnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtnnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtnnosat16(double16);
    
	__attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtznosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtznosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtznosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtznosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtznosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32upnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32upnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32upnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32upnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32upnosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32downnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32downnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32downnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32downnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32downnosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtnsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtzsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32upsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32upsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32upsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32upsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32upsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32downsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32downsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32downsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32downsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32downsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprtn1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprtn2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprtn4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprtn8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprtn16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprtz1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprtz2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprtz4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprtz8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprtz16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofpup1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofpup2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofpup4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofpup8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofpup16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofpdown1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofpdown2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofpdown4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofpdown8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofpdown16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprtn1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprtn2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprtn4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprtn8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprtn16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprtz1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprtz2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprtz4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprtz8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprtz16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofpup1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofpup2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofpup4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofpup8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofpup16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofpdown1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofpdown2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofpdown4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofpdown8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofpdown16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprtn1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprtn2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprtn4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprtn8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprtn16(_16i64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprtz1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprtz2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprtz4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprtz8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprtz16(_16i64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofpup1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofpup2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofpup4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofpup8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofpup16(_16i64);
   
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofpdown1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofpdown2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofpdown4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofpdown8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofpdown16(_16i64);
    
	/*__attribute__((svmlcc)) __attribute__((const))*/ ulong __ocl_svml_{arch}_cvtfptou64rtznosat1 (double a);
    __attribute__((svmlcc)) __attribute__((const)) ulong2 __ocl_svml_{arch}_cvtfptou62rtznosat2 (double2 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong4 __ocl_svml_{arch}_cvtfptou64rtznosat4 (double4 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong8 __ocl_svml_{arch}_cvtfptou64rtznosat8 (double8 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong16 __ocl_svml_{arch}_cvtfptou64rtznosat16 (double16 a);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprtn1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprtn2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprtn3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprtn4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprtn8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprtn16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprtn1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprtn2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprtn3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprtn4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprtn8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprtn16 (_16i64);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtznosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtznosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtznosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtznosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtznosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtznosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnnosat1 (double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnnosat2 (double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtnnosat3 (double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnnosat4 (double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnnosat8 (double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnnosat16 (double16);
    
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofpup1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofpup2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofpup3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofpup4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofpup8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofpup16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofpdown1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofpdown2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofpdown3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofpdown4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofpdown8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofpdown16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprtz1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprtz2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprtz3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprtz4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprtz8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprtz16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofpup1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofpup2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofpup3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofpup4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofpup8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofpup16 (_16i64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofpdown1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofpdown2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofpdown3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofpdown4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofpdown8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofpdown16 (_16i64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprtz1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprtz2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprtz3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprtz4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprtz8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprtz16 (_16i64);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64upnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64upnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64upnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64upnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64upnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64upnosat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64downnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64downnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64downnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64downnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64downnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64downnosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtzsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtzsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtnsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnsat16(double16);    
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64downsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64downsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64downsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64downsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64downsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64downsat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64upsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64upsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64upsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64upsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64upsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64upsat16(double16);     
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64upnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64upnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64upnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64upnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64upnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64upnosat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64downnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64downnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64downnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64downnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64downnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64downnosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtzsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtzsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtnsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnsat16(double16);    
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64downsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64downsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64downsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64downsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64downsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64downsat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64upsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64upsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64upsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64upsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64upsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64upsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat16(double16);
	""".format(arch=target.svml_id))

	out.write( """
    ////------------ End of auto-generated section ------------------///
	""")

if __name__ == "__main__":
	ret = main()
	sys.exit(ret)
