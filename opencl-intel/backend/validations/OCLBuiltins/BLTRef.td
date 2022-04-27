class TemplateAttr< list<OclBuiltinAttr> tattrs >
{
    list<OclBuiltinAttr> tAttrs = tattrs;
}

class OclRoundingMode<string name>
{
    string RMode = name;
}

class REF_OclBuiltin<string name, dag outs, dag ins, list<OclType> types, list<OclBuiltinAttr> attrs, TemplateAttr template_format> :
OclBuiltin<name, outs, ins, types, attrs>
{
    TemplateAttr templateFormat = template_format;
}

class REF_Conversions<string name, dag outs, dag ins, list<OclType> types, list<OclBuiltinAttr> attrs, TemplateAttr template_format, bit sat> :
OclBuiltin<name, outs, ins, types, attrs>
{
    TemplateAttr templateFormat = template_format;
    list<OclRoundingMode> RModes = [];
    bit Saturate = sat;
}

def type0 : OclBuiltinAttr<"t0">;
def type1 : OclBuiltinAttr<"t1">;
def type2 : OclBuiltinAttr<"t2">;
def rtype : OclBuiltinAttr<"r">;

def vec0 : OclBuiltinAttr<"v0">;
def vec1 : OclBuiltinAttr<"v1">;
def vec2 : OclBuiltinAttr<"v2">;
def rvec : OclBuiltinAttr<"f">;

def nvec0 : OclBuiltinAttr<"n0">;//sizes of not-mapped vector types
def nvec1 : OclBuiltinAttr<"n1">;
def nvec2 : OclBuiltinAttr<"n2">;

def sat : OclBuiltinAttr<"s">;
def rmode : OclBuiltinAttr<"m">;

def arg_false : OclBuiltinAttr<"u_false">;
def arg_true : OclBuiltinAttr<"u_true">;


def MathTemplate : TemplateAttr< [type0, vec0]>;
def TypeTemplate : TemplateAttr< [type0]>;
def ConvTemplate : TemplateAttr< [rtype, type0, rvec, sat, rmode]>;

def rte : OclRoundingMode<"_rte">;
def rtz : OclRoundingMode<"_rtz">;
def rtp : OclRoundingMode<"_rtp">;
def rtn : OclRoundingMode<"_rtn">;
def defRM : OclRoundingMode<"">;
