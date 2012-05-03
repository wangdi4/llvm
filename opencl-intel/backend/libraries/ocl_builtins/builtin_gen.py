import sys
from optparse import OptionParser

class Target:
    def __init__(self,cpu,svml_id,is64Bit):
        self.cpu=cpu
        self.svml_id=svml_id
        self.is64Bit=is64Bit


''' 
    Generator of SVML function names for floating point to integer conversions
    variables: 
        input floating point type,
        output integer type, 
        vector width,
        saturation on/off,
        cpu architecutres
'''
class SVMLNameGeneratorSVMLFPToInt:
    def __init__(self, in_out):
        self.TemplateStr = \
"""     __attribute__((svmlcc)) __attribute__((const)) {rettype}{widthocl} __ocl_svml_{arch}_cvt{fp}to{int}{rmode}{sat}{isfloat}{width}({inptype}{widthocl});
"""
        self.out = in_out
        
    def run(self, in_arch, in_rettype, in_inptype, in_fpfrom, in_intto, in_rmode, in_sat, in_isfloat, in_width, in_widthocl):
        self.out.write(self.TemplateStr.format(arch=in_arch, rettype=in_rettype, inptype=in_inptype, 
            fp=in_fpfrom, int=in_intto, rmode=in_rmode, sat=in_sat, isfloat=in_isfloat, width=in_width, widthocl=in_widthocl))
        return
    def runOnWidth(self, in_arch, in_rettype, in_inptype, in_fpfrom, in_intto, in_rmode, in_sat, in_isfloat):
        Widths = [1, 2, 3, 4, 8, 16]
        for width in Widths:
            widthocl = width
            if width == 1:
                widthocl = ''
            self.run(in_arch, in_rettype, in_inptype, in_fpfrom, in_intto, in_rmode, in_sat, in_isfloat, str(width), widthocl)
    def runOnRmode(self, in_arch, in_rettype, in_inptype, in_fpfrom, in_intto, in_sat, in_isfloat):
        Rmodes = ['rte', 'rtp', 'rtn', 'rtz']
        for rmode in Rmodes:
            self.runOnWidth(in_arch, in_rettype, in_inptype, in_fpfrom, in_intto, rmode, in_sat, in_isfloat)

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
        Target('Sandybridge',  'e9', True),
		Target('Haswell',  's9', False),
        Target('Haswell',  'l9', True)]

    out.write( '///------------ Beginning of auto-generated section ------------------///\n')
    out.write( '///------------ This file is built by builtin_gen.py ------------------///')
    for target in targets:
                
        out.write( """

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprte1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprte2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprte4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprte8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprte16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprtz1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprtz2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprtz4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprtz8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprtz16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprtp1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprtp2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprtp4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprtp8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprtp16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu32tofprtn1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu32tofprtn2(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu32tofprtn4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu32tofprtn8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu32tofprtn16(_16u32);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtef1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtef2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float3 __ocl_svml_{arch}_cvtu64tofprtef3(_3u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtef4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtef8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtef16(_16u64);
    
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
    
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtpf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtpf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtpf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtpf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtpf16(_16u64);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvtu64tofprtnf1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvtu64tofprtnf2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvtu64tofprtnf4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvtu64tofprtnf8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtu64tofprtnf16(_16u64);
    
    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtef1(_1i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtef2(_2i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtef4(_4i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtef8(_8i64 x);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtef16(_16i64 x);
    
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

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtpf1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtpf2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtpf4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtpf8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtpf16(_16i64);

    __attribute__((svmlcc)) __attribute__((const)) float __ocl_svml_{arch}_cvti64tofprtnf1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) float2 __ocl_svml_{arch}_cvti64tofprtnf2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) float4 __ocl_svml_{arch}_cvti64tofprtnf4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) float8 __ocl_svml_{arch}_cvti64tofprtnf8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvti64tofprtnf16(_16i64);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtenosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtenosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtenosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtenosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtenosatf16(float16);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtznosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtpnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtpnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtpnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtpnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtpnosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnnosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtenosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtenosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtenosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtenosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtenosatf16(float16);

    /// This function appears to use the standard calling convention.
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtznosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtznosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtznosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtznosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtznosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtpnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtpnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtpnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtpnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtpnosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnnosatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtesatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtesatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtesatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtesatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtesatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtzsatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtpsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtpsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtpsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtpsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtpsatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnsatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtesatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtesatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtesatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtesatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtesatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtzsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtzsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtzsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtzsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtzsatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtpsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtpsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtpsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtpsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtpsatf16(float16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnsatf1(float);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnsatf2(float2);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnsatf4(float4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnsatf8(float8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnsatf16(float16);
    
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtenosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtenosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtenosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtenosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtenosat16(double16);
    
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtznosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtznosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtznosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtznosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtznosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtpnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtpnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtpnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtpnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtpnosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtnnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtnnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtnnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtnnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtnnosat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtesat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtesat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtesat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtesat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtesat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtzsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtpsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtpsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtpsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtpsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtpsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) _1u32 __ocl_svml_{arch}_cvtfptou32rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u32 __ocl_svml_{arch}_cvtfptou32rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _4u32 __ocl_svml_{arch}_cvtfptou32rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u32 __ocl_svml_{arch}_cvtfptou32rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u32 __ocl_svml_{arch}_cvtfptou32rtnsat16(double16);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprte1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprte2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprte4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprte8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprte16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprtz1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprtz2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprtz4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprtz8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprtz16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprtp1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprtp2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprtp4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprtp8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprtp16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu32tofprtn1(_1u32);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu32tofprtn2(_2u32);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu32tofprtn4(_4u32);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu32tofprtn8(_8u32);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu32tofprtn16(_16u32);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprte1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprte2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprte4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprte8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprte16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprtz1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprtz2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprtz4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprtz8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprtz16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprtp1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprtp2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprtp4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprtp8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprtp16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvtu64tofprtn1(_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvtu64tofprtn2(_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvtu64tofprtn4(_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvtu64tofprtn8(_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvtu64tofprtn16(_16u64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprte1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprte2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprte4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprte8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprte16(_16i64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprtz1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprtz2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprtz4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprtz8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprtz16(_16i64);
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprtp1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprtp2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprtp4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprtp8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprtp16(_16i64);
   
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml__{arch}_cvti64tofprtn1(_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml__{arch}_cvti64tofprtn2(_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml__{arch}_cvti64tofprtn4(_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml__{arch}_cvti64tofprtn8(_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml__{arch}_cvti64tofprtn16(_16i64);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ ulong __ocl_svml_{arch}_cvtfptou64rtznosat1 (double a);
    __attribute__((svmlcc)) __attribute__((const)) ulong2 __ocl_svml_{arch}_cvtfptou62rtznosat2 (double2 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong4 __ocl_svml_{arch}_cvtfptou64rtznosat4 (double4 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong8 __ocl_svml_{arch}_cvtfptou64rtznosat8 (double8 a);
    __attribute__((svmlcc)) __attribute__((const)) ulong16 __ocl_svml_{arch}_cvtfptou64rtznosat16 (double16 a);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprte1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprte2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprte3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprte4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprte8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprte16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprte1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprte2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprte3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprte4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprte8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprte16 (_16i64);

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtznosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtznosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtznosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtznosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtznosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtznosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtenosat1 (double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtenosat2 (double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtenosat3 (double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtenosat4 (double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtenosat8 (double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtenosat16 (double16);
    
    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprtp1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprtp2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprtp3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprtp4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprtp8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprtp16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprtn1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprtn2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprtn3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprtn4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprtn8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprtn16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvtu64tofprtz1 (_1u64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvtu64tofprtz2 (_2u64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvtu64tofprtz3 (_3u64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvtu64tofprtz4 (_4u64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvtu64tofprtz8 (_8u64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvtu64tofprtz16 (_16u64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprtp1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprtp2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprtp3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprtp4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprtp8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprtp16 (_16i64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprtn1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprtn2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprtn3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprtn4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprtn8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprtn16 (_16i64);

    __attribute__((svmlcc)) __attribute__((const)) double __ocl_svml_{arch}_cvti64tofprtz1 (_1i64);
    __attribute__((svmlcc)) __attribute__((const)) double2 __ocl_svml_{arch}_cvti64tofprtz2 (_2i64);
    __attribute__((svmlcc)) __attribute__((const)) double3 __ocl_svml_{arch}_cvti64tofprtz3 (_3i64);
    __attribute__((svmlcc)) __attribute__((const)) double4 __ocl_svml_{arch}_cvti64tofprtz4 (_4i64);
    __attribute__((svmlcc)) __attribute__((const)) double8 __ocl_svml_{arch}_cvti64tofprtz8 (_8i64);
    __attribute__((svmlcc)) __attribute__((const)) double16 __ocl_svml_{arch}_cvti64tofprtz16 (_16i64);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtpnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtpnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtpnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtpnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtpnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtpnosat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtnnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnnosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtzsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtzsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtesat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtesat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtesat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtesat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtesat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtesat16(double16);    
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtnsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtnsat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1u64 __ocl_svml_{arch}_cvtfptou64rtpsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2u64 __ocl_svml_{arch}_cvtfptou64rtpsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3u64 __ocl_svml_{arch}_cvtfptou64rtpsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4u64 __ocl_svml_{arch}_cvtfptou64rtpsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8u64 __ocl_svml_{arch}_cvtfptou64rtpsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16u64 __ocl_svml_{arch}_cvtfptou64rtpsat16(double16);     
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtpnosat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnnosat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtzsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtzsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtzsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtzsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtzsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtzsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtesat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtesat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtesat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtesat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtesat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtesat16(double16);    
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtnsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtnsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtnsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtnsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtnsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtnsat16(double16);    

    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtpsat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtpsat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtpsat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtpsat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtpsat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtpsat16(double16);
    
    /*__attribute__((svmlcc)) __attribute__((const))*/ _1i64 __ocl_svml_{arch}_cvtfptoi64rtenosat1(double);
    __attribute__((svmlcc)) __attribute__((const)) _2i64 __ocl_svml_{arch}_cvtfptoi64rtenosat2(double2);
    __attribute__((svmlcc)) __attribute__((const)) _3i64 __ocl_svml_{arch}_cvtfptoi64rtenosat3(double3);
    __attribute__((svmlcc)) __attribute__((const)) _4i64 __ocl_svml_{arch}_cvtfptoi64rtenosat4(double4);
    __attribute__((svmlcc)) __attribute__((const)) _8i64 __ocl_svml_{arch}_cvtfptoi64rtenosat8(double8);
    __attribute__((svmlcc)) __attribute__((const)) _16i64 __ocl_svml_{arch}_cvtfptoi64rtenosat16(double16);
    
    /* Double to float conversions */
    __attribute__((svmlcc)) __attribute__((const)) float   __ocl_svml_{arch}_cvtfp64tofp32rte1(double);
    __attribute__((svmlcc)) __attribute__((const)) float2  __ocl_svml_{arch}_cvtfp64tofp32rte2(double2);
    __attribute__((svmlcc)) __attribute__((const)) float3  __ocl_svml_{arch}_cvtfp64tofp32rte3(double3);
    __attribute__((svmlcc)) __attribute__((const)) float4  __ocl_svml_{arch}_cvtfp64tofp32rte4(double4);
    __attribute__((svmlcc)) __attribute__((const)) float8  __ocl_svml_{arch}_cvtfp64tofp32rte8(double8);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtfp64tofp32rte16(double16);
    
    __attribute__((svmlcc)) __attribute__((const)) float   __ocl_svml_{arch}_cvtfp64tofp32rtp1(double);
    __attribute__((svmlcc)) __attribute__((const)) float2  __ocl_svml_{arch}_cvtfp64tofp32rtp2(double2);
    __attribute__((svmlcc)) __attribute__((const)) float3  __ocl_svml_{arch}_cvtfp64tofp32rtp3(double3);
    __attribute__((svmlcc)) __attribute__((const)) float4  __ocl_svml_{arch}_cvtfp64tofp32rtp4(double4);
    __attribute__((svmlcc)) __attribute__((const)) float8  __ocl_svml_{arch}_cvtfp64tofp32rtp8(double8);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtfp64tofp32rtp16(double16);

    __attribute__((svmlcc)) __attribute__((const)) float   __ocl_svml_{arch}_cvtfp64tofp32rtn1(double);
    __attribute__((svmlcc)) __attribute__((const)) float2  __ocl_svml_{arch}_cvtfp64tofp32rtn2(double2);
    __attribute__((svmlcc)) __attribute__((const)) float3  __ocl_svml_{arch}_cvtfp64tofp32rtn3(double3);
    __attribute__((svmlcc)) __attribute__((const)) float4  __ocl_svml_{arch}_cvtfp64tofp32rtn4(double4);
    __attribute__((svmlcc)) __attribute__((const)) float8  __ocl_svml_{arch}_cvtfp64tofp32rtn8(double8);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtfp64tofp32rtn16(double16);
    
    __attribute__((svmlcc)) __attribute__((const)) float   __ocl_svml_{arch}_cvtfp64tofp32rtz1(double);
    __attribute__((svmlcc)) __attribute__((const)) float2  __ocl_svml_{arch}_cvtfp64tofp32rtz2(double2);
    __attribute__((svmlcc)) __attribute__((const)) float3  __ocl_svml_{arch}_cvtfp64tofp32rtz3(double3);
    __attribute__((svmlcc)) __attribute__((const)) float4  __ocl_svml_{arch}_cvtfp64tofp32rtz4(double4);
    __attribute__((svmlcc)) __attribute__((const)) float8  __ocl_svml_{arch}_cvtfp64tofp32rtz8(double8);
    __attribute__((svmlcc)) __attribute__((const)) float16 __ocl_svml_{arch}_cvtfp64tofp32rtz16(double16);
    
    """.format(arch=target.svml_id))
        
        # float to int conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtenosatf1(float);
        out.write('/* float to int conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'int', 'float', 'fp', 'i32', 'nosat', 'f')

        # float to int with saturation conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtesatf1(float);
        out.write('/* float to int with saturation conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'int', 'float', 'fp', 'i32', 'sat', 'f')
    
        # double to int conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtenosat1(double);
        out.write('/* double to int conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'int', 'double', 'fp', 'i32', 'nosat', '')

        # double to int with saturation conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtesat1(double);
        out.write('/* double to int with saturation conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'int', 'double', 'fp', 'i32', 'sat', '')

        # int to float conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvti32tofprtzf1(float);
        out.write('/* int to float conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'float', 'int', 'i32', 'fp', '', 'f')

        # uint to float conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtu32tofprtzf1(float);
        out.write('/* uint to float conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'float', 'uint', 'u32', 'fp', '', 'f')

        # float to uint conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtenosatf1(float);
        out.write('/* float to uint conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'uint', 'float', 'fp', 'u32', 'nosat', 'f')

        # float to uint with saturation conversions
        # _attribute__((svmlcc)) __attribute__((const)) int    __ocl_svml_{arch}_cvtfptoi32rtesatf1(float);
        out.write('/* float to uint with saturation conversions */ \n')
        SVMLNameGeneratorSVMLFPToInt(out).runOnRmode(target.svml_id, 'uint', 'float', 'fp', 'u32', 'sat', 'f')

        out.write( """
    ////------------ End of auto-generated section ------------------///
    """)

if __name__ == "__main__":
    ret = main()
    sys.exit(ret)
