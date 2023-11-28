// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

//OpenCL 1.2 rev 15, Table 6.15
let Types = [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v2i16, v3i16, v4i16, v8i16, v16i16,
             v2u16, v3u16, v4u16, v8u16, v16u16,
             v2i32, v3i32, v4i32, v8i32, v16i32,
             v2u32, v3u32, v4u32, v8u32, v16u32,
             v2i64, v3i64, v4i64, v8i64, v16i64,
             v2u64, v3u64, v4u64, v8u64, v16u64,
             v2f32, v3f32, v4f32, v8f32, v16f32,
             v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [OVERLOADABLE] in {
def vload_gas   : REF_OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type1, nvec1]> >, CONST_GLOBAL;
def vload_cas   : REF_OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type1, nvec1]> >, CONSTANT;
def vload_las   : REF_OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type1, nvec1]> >, CONST_LOCAL;
def vload_pas   : REF_OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type1, nvec1]> >, CONST_PRIVATE;
def vload_gnas  : REF_OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type1, nvec1]> >, CONST_GENERIC;
def vstore_pas  : REF_OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], MathTemplate>, PRIVATE;
def vstore_las  : REF_OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], MathTemplate>, LOCAL;
def vstore_gas  : REF_OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], MathTemplate>, GLOBAL;
def vstore_gnas : REF_OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], MathTemplate>, GENERIC;
}

let Types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE] in {
def vload_half_gas        : REF_OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_false]>>, CONST_GLOBAL;
def vload_half_cas        : REF_OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_false]>>, CONSTANT;
def vload_half_las        : REF_OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_false]>>, CONST_LOCAL;
def vload_half_pas        : REF_OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_false]>>, CONST_PRIVATE;
def vload_half_gnas       : REF_OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_false]>>, CONST_GENERIC;
def vstoref_half_pas      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, PRIVATE;
def vstoref_half_las      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, LOCAL;
def vstoref_half_gas      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, GLOBAL;
def vstoref_half_gnas     : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, GENERIC;

def vstoref_half_rte_pas  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstoref_half_rte_las  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstoref_half_rte_gas  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstoref_half_rte_gnas : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstoref_half_rtz_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstoref_half_rtz_las  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstoref_half_rtz_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstoref_half_rtz_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstoref_half_rtp_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstoref_half_rtp_las  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstoref_half_rtp_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstoref_half_rtp_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstoref_half_rtn_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstoref_half_rtn_las  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstoref_half_rtn_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstoref_half_rtn_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstored_half_pas      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, PRIVATE;
def vstored_half_las      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, LOCAL;
def vstored_half_gas      : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, GLOBAL;
def vstored_half_gnas     : REF_OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_false]>>, GENERIC;
def vstored_half_rte_pas  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstored_half_rte_las  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstored_half_rte_gas  : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstored_half_rte_gnas : REF_OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstored_half_rtz_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstored_half_rtz_las  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstored_half_rtz_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstored_half_rtz_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstored_half_rtp_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstored_half_rtp_las  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstored_half_rtp_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstored_half_rtp_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vstored_half_rtn_pas  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, PRIVATE;
def vstored_half_rtn_las  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, LOCAL;
def vstored_half_rtn_gas  : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GLOBAL;
def vstored_half_rtn_gnas : REF_OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_false]>>, GENERIC;
def vloada_half_gas       : REF_OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_true]>>, CONST_GLOBAL;
def vloada_half_cas       : REF_OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_true]>>, CONSTANT;
def vloada_half_las       : REF_OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_true]>>, CONST_LOCAL;
def vloada_half_pas       : REF_OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_true]>>, CONST_PRIVATE;
def vloada_half_gnas      : REF_OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], TemplateAttr< [nvec1, arg_true]>>, CONST_GENERIC;
def vstoreaf_half_pas     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, PRIVATE;
def vstoreaf_half_las     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, LOCAL;
def vstoreaf_half_gas     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, GLOBAL;
def vstoreaf_half_gnas    : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, GENERIC;
}
let Types = [v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE] in {
def vstoreaf_half_rte_pas : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoreaf_half_rte_las : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoreaf_half_rte_gas : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoreaf_half_rte_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoreaf_half_rtz_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoreaf_half_rtz_las : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoreaf_half_rtz_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoreaf_half_rtz_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoreaf_half_rtp_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoreaf_half_rtp_las : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoreaf_half_rtp_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoreaf_half_rtp_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoreaf_half_rtn_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoreaf_half_rtn_las : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoreaf_half_rtn_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoreaf_half_rtn_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
}
let Types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE] in {
def vstoread_half_pas     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, PRIVATE;
def vstoread_half_las     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, LOCAL;
def vstoread_half_gas     : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, GLOBAL;
def vstoread_half_gnas    : REF_OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [type0, vec0, arg_true]>>, GENERIC;
}
let Types = [v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE] in {
def vstoread_half_rte_pas : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoread_half_rte_las : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoread_half_rte_gas : REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoread_half_rte_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoread_half_rtz_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoread_half_rtz_las : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoread_half_rtz_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoread_half_rtz_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoread_half_rtp_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoread_half_rtp_las : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoread_half_rtp_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoread_half_rtp_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
def vstoread_half_rtn_pas : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, PRIVATE;
def vstoread_half_rtn_las : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, LOCAL;
def vstoread_half_rtn_gas : REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GLOBAL;
def vstoread_half_rtn_gnas: REF_OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], TemplateAttr< [vec0, arg_true]>>, GENERIC;
}

class Generic {
  code Prolog = [{
/*****************************************************************************\

Copyright (C) 2013 Intel Corporation

This software and the related documents are Intel copyrighted materials, and
your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise, you may
not use, modify, copy, publish, distribute, disclose or transmit this
software or the related documents without Intel's prior written permission.

This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

File Name:  VLoadStore.cpp

\*****************************************************************************/

#include "VLoadStore.h"
#include "Conformance/reference_convert.h"
#include "llvm/Support/DynamicLibrary.h"

using namespace llvm;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif

namespace Validation {
namespace OCLBuiltins {

template<> llvm::GenericValue lle_X_vload_half<3, true>(
    llvm::FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args)
{
    llvm::GenericValue R;
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    uint32_t offset  = arg0.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg1.PointerVal);
    p += offset;
    R.AggregateVal.resize(3);
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        getRef<float, 3> (R, i) = float(CFloat16(*p));
    }
    return R;
}

template<>
uint16_t convert2half<float>(float f)
{
    return Conformance::float2half_rte(f);
}

template<>
uint16_t convert2half<double>(double f)
{
    return Conformance::double2half_rte(f);
}


template<> llvm::GenericValue lle_X_vstore_half<float, 3, true>(
    llvm::FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    uint32_t offset  = arg1.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);
    p += offset;
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        *p = CFloat16(getVal<float, 3>(arg0, i)).GetBits();
    }
    return llvm::GenericValue();
}

template<> llvm::GenericValue lle_X_vstore_half<double, 3, true>(
    llvm::FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    llvm::GenericValue arg2 = Args[2];
    uint32_t offset  = arg1.IntVal.getZExtValue();
    offset *= 4;
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);
    p += offset;
    for (uint32_t i = 0; i < 3; ++i, ++p)
    {
        *p = Conformance::double2half_rte(getVal<double, 3>(arg0, i));
    }
    return llvm::GenericValue();
}

#define VSTOREF_HALF_CONVERT(RMODE)                                                 \
template<int n, bool aligned>                                                       \
llvm::GenericValue lle_X_vstoref_half_ ## RMODE (llvm::FunctionType *FT,      \
                                     llvm::ArrayRef<llvm::GenericValue> Args)   \
{                                                                                   \
    llvm::GenericValue arg0 = Args[0];                                              \
    llvm::GenericValue arg1 = Args[1];                                              \
    llvm::GenericValue arg2 = Args[2];                                              \
    uint32_t offset  = arg1.IntVal.getZExtValue();                                  \
    offset *= n;                                                                    \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                          \
    p += offset;                                                                    \
    for (uint32_t i = 0; i < n; ++i, ++p)                                           \
    {                                                                               \
        *p = Conformance::float2half_ ## RMODE(getVal<float, n>(arg0, i));          \
    }                                                                               \
    return llvm::GenericValue();                                                    \
}

VSTOREF_HALF_CONVERT(rte)
VSTOREF_HALF_CONVERT(rtz)
VSTOREF_HALF_CONVERT(rtp)
VSTOREF_HALF_CONVERT(rtn)

#define VSTORED_HALF_CONVERT(RMODE)                                                 \
template<int n, bool aligned>                                                       \
llvm::GenericValue lle_X_vstored_half_ ## RMODE (llvm::FunctionType *FT,      \
                                    llvm::ArrayRef<llvm::GenericValue> Args)    \
{                                                                                   \
    llvm::GenericValue arg0 = Args[0];                                              \
    llvm::GenericValue arg1 = Args[1];                                              \
    llvm::GenericValue arg2 = Args[2];                                              \
    uint32_t offset  = arg1.IntVal.getZExtValue();                                  \
    offset *= n;                                                                    \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                          \
    p += offset;                                                                    \
    for (uint32_t i = 0; i < n; ++i, ++p)                                           \
    {                                                                               \
        *p = Conformance::double2half_ ## RMODE(getVal<double, n>(arg0, i));        \
    }                                                                               \
    return llvm::GenericValue();                                                    \
}

VSTORED_HALF_CONVERT(rte)
VSTORED_HALF_CONVERT(rtz)
VSTORED_HALF_CONVERT(rtp)
VSTORED_HALF_CONVERT(rtn)

#define VSTOREAF_HALF_CONVERT(RMODE)                                        \
template<> llvm::GenericValue lle_X_vstoref_half_ ## RMODE <3, true>(       \
    llvm::FunctionType *FT,                                           \
    llvm::ArrayRef<llvm::GenericValue> Args)                            \
{                                                                           \
    llvm::GenericValue arg0 = Args[0];                                      \
    llvm::GenericValue arg1 = Args[1];                                      \
    llvm::GenericValue arg2 = Args[2];                                      \
    uint32_t offset  = arg1.IntVal.getZExtValue();                          \
    offset *= 4;                                                            \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                  \
    p += offset;                                                            \
    for (uint32_t i = 0; i < 3; ++i, ++p)                                   \
    {                                                                       \
        *p = Conformance::float2half_ ## RMODE(getVal<float, 3>(arg0, i));  \
    }                                                                       \
    return llvm::GenericValue();                                            \
}

VSTOREAF_HALF_CONVERT(rte)
VSTOREAF_HALF_CONVERT(rtz)
VSTOREAF_HALF_CONVERT(rtp)
VSTOREAF_HALF_CONVERT(rtn)

#define VSTOREAD_HALF_CONVERT(RMODE)                                            \
template<> llvm::GenericValue lle_X_vstored_half_ ## RMODE <3, true>(           \
    llvm::FunctionType *FT,                                               \
    llvm::ArrayRef<llvm::GenericValue> Args)                                \
{                                                                               \
    llvm::GenericValue arg0 = Args[0];                                          \
    llvm::GenericValue arg1 = Args[1];                                          \
    llvm::GenericValue arg2 = Args[2];                                          \
    uint32_t offset  = arg1.IntVal.getZExtValue();                              \
    offset *= 4;                                                                \
    uint16_t* p = static_cast<uint16_t*>(arg2.PointerVal);                      \
    p += offset;                                                                \
    for (uint32_t i = 0; i < 3; ++i, ++p)                                       \
    {                                                                           \
        *p = Conformance::double2half_ ## RMODE(getVal<double, 3>(arg0, i));    \
    }                                                                           \
    return llvm::GenericValue();                                                \
}

VSTOREAD_HALF_CONVERT(rte)
VSTOREAD_HALF_CONVERT(rtz)
VSTOREAD_HALF_CONVERT(rtp)
VSTOREAD_HALF_CONVERT(rtn)
}
}

extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsVLoadStore() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
