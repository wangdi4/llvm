/**
***  Copyright  (C) 1985-2014 Intel Corporation. All rights reserved.
***
***  The information and source code contained herein is the exclusive
***  property of Intel Corporation and may not be disclosed, examined
***  or reproduced in whole or in part without explicit written authorization
***  from the company.
**/

/**
static char cvs_id[] = "$Id: STI.h 325 2015-02-22 16:17:54Z aaboud $"
**/

/* 
This file contains definitions  for the Microsoft Symbol and Type information
or CV4 using the TIS Formats specification for Windows.  Both CV4 and CV5
formats are supported.
*/

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_STI_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_STI_H

//===----------------------------------------------------------------------===//
// STIMachineID
//===----------------------------------------------------------------------===//

#define STI_MACHINE_KINDS                               \
    X(STI_MACHINE_INTEL_8080,           0x0000)         \
    X(STI_MACHINE_INTEL_8086,           0x0001)         \
    X(STI_MACHINE_INTEL_80286,          0x0002)         \
    X(STI_MACHINE_INTEL_80386,          0x0003)         \
    X(STI_MACHINE_INTEL_80486,          0x0004)         \
    X(STI_MACHINE_INTEL_PENTIUM,        0x0005)         \
    X(STI_MACHINE_INTEL_PENTIUM_PRO,    0x0006)         \
    X(STI_MACHINE_INTEL_PENTIUM_III,    0x0007)         \
    X(STI_MACHINE_INTEL_IPF,            0x0080)         \
    X(STI_MACHINE_INTEL_IPF2,           0x0081)         \
    X(STI_MACHINE_INTEL64,              0x00d0)

enum STIMachineIDEnum {
#define X(NAME,VALUE) NAME = VALUE,
    STI_MACHINE_KINDS
#undef  X
};
typedef enum STIMachineIDEnum STIMachineID;

//===----------------------------------------------------------------------===//
// STISubsectionID
//===----------------------------------------------------------------------===//

#define STI_SUBSECTION_KINDS                            \
    X(STI_SUBSECTION_SYMBOLS,           0x000000f1)     \
    X(STI_SUBSECTION_LINES,             0x000000f2)     \
    X(STI_SUBSECTION_STRINGTABLE,       0x000000f3)     \
    X(STI_SUBSECTION_FILECHKSMS,        0x000000f4)     \
    X(STI_SUBSECTION_FRAMEDATA,         0x000000f5)

enum STISubsectionIDEnum {
#define X(KIND,VALUE) KIND = VALUE,
    STI_SUBSECTION_KINDS
#undef  X
};
typedef enum STISubsectionIDEnum STISubsectionID;

//===----------------------------------------------------------------------===//
// STISignatureID
//===----------------------------------------------------------------------===//

#define STI_SIGNATURE_KINDS                                     \
    X(STI_SIGNATURE_VS2012,             0x0004)                 \
    X(STI_SIGNATURE_LATEST,             STI_SIGNATURE_VS2012)   \

enum STISignatureIDEnum {
#define X(KIND,VALUE) KIND = VALUE,
    STI_SIGNATURE_KINDS
#undef  X
};
typedef enum STISignatureIDEnum STISignatureID;

//===----------------------------------------------------------------------===//
// STISectionSignatureID
//===----------------------------------------------------------------------===//

#define STI_SECTION_SIGNATURE_KINDS                     \
    X(STI_SECTION_SIGNATURE_CV4,        0x0001)         \
    X(STI_SECTION_SIGNATURE_CV5,        0x0002)         \
    X(STI_SECTION_SIGNATURE_CV7,        0x0004)

enum STISectionSignatureIDEnum {
#define X(KIND,VALUE) KIND = VALUE,
    STI_SECTION_SIGNATURE_KINDS
#undef  X
};
typedef enum STISectionSignatureIDEnum STISectionSignatureID;

//===----------------------------------------------------------------------===//
// STIRegID
//===----------------------------------------------------------------------===//

#define STI_REGISTER_KINDS                              \
    X(STI_REGISTER_NONE,                0x0000)         \
    X(STI_REGISTER_EAX,                 0x0011)         \
    X(STI_REGISTER_ECX,                 0x0012)         \
    X(STI_REGISTER_EDX,                 0x0013)         \
    X(STI_REGISTER_EBX,                 0x0014)         \
    X(STI_REGISTER_ESP,                 0x0015)         \
    X(STI_REGISTER_EBP,                 0x0016)         \
    X(STI_REGISTER_ESI,                 0x0017)         \
    X(STI_REGISTER_EDI,                 0x0018)         \
    X(STI_REGISTER_RAX,                 0x0148)         \
    X(STI_REGISTER_RCX,                 0x0149)         \
    X(STI_REGISTER_RDX,                 0x014a)         \
    X(STI_REGISTER_RBX,                 0x014b)         \
    X(STI_REGISTER_RSI,                 0x014c)         \
    X(STI_REGISTER_RDI,                 0x014d)         \
    X(STI_REGISTER_RBP,                 0x014e)         \
    X(STI_REGISTER_RSP,                 0x014f) 


enum STIRegIDEnum {
#define X(KIND, VALUE) KIND = VALUE,
    STI_REGISTER_KINDS
#undef  X
};
typedef enum STIRegIDEnum STIRegID;


// All higher ones have been replaced with a better arrangement
// of variables controlling uses of features

#define IS_CV5 false

/**************************************************************************/
/* Symbol indices */
/* V7 spec 2.1.1 Symbol Record Types */

/***** CV4 specific enums .. define only */
#define         S_REGISTER_CV4  0x0002          /* Register variable */
#define         S_CONSTANT_CV4  0x0003          /* Constant symbol */
#define         S_UDT_CV4       0x0004          /* User defined type */
#define         S_MANYREG_CV4   0x000c          /* Many registe symbol */
#define         S_BPREL32_CV4   0x0200          /* BP relative 16:32 addresses */
#define         S_LDATA32_CV4   0x0201          /* Local data 16:32 */
#define         S_GDATA32_CV4   0x0202          /* Global data 16:32 */
#define         S_PUB32_CV4     0x0203          /* Public symbol 16:32 */
#define         S_LPROC32_CV4   0x0204          /* Local procedure start 16:32 */
#define         S_GPROC32_CV4   0x0205          /* Global procedure start 16:32 */
#define         S_REGREL32_CV4  0x020c          /* Offset relative to register */
#define         S_LTHREAD32_CV4         0x020d          /* Local thread storage area */
#define         S_GTHREAD32_CV4         0x020e          /* Global thread storage area */

/***** CV5 specific enums */
#define S_COMPILE_CV5   0x0001
#define S_OBJNAME_CV5   0x0009
#define         S_ENTRYTHIS_CV5         0x000e          /* Desc of 'this' pointer at entry*/
#define         S_THUNK32_CV5   0x0206          /* Thunk start 16:32 */
#define         S_BLOCK32_CV5   0x0207          /* Block start 16:32 */
#define         S_WITH32_CV5    0x0208          /* With start 16:32 */
#define         S_LABEL32_CV5   0x0209          /* Code Label 16:32 */
#define         S_VFTPATH32_CV5         0x020b          /* Virtual function table path desc */
#define S_PROCREF_CV5   0x0400          /* Reference to a procedure */
#define S_DATAREF_CV5   0x0401          /* Reference to data */
#define S_ALIGN_CV5     0x0402          /* Page align symbols */
#define S_REGISTER_CV5  0x1001          /* Register variable */
#define S_CONSTANT_CV5  0x1002          /* Constant symbol */
#define S_UDT_CV5       0x1003          /* User-defined type */
#define S_MANYREG_CV5   0x1005          /* Many registers */
#define S_BPREL32_CV5   0x1006          /* BP relative 16:32 */
#define S_LDATA32_CV5   0x1007          /* Local data 16:32 */
#define S_GDATA32_CV5   0x1008          /* Global data 16:32 */
#define S_PUB32_CV5     0x1009          /* Public symbol 16:32 */
#define S_LPROC32_CV5   0x100a          /* Local procedure start 16:32 */
#define S_GPROC32_CV5   0x100b          /* Global procedure start 16:32 */
#define S_REGREL32_CV5  0x100d          /* 16:32 offset relative to arbitrary register */
#define S_LTHREAD32_CV5         0x100e          /* Local Thread Storage data */
#define S_GTHREAD32_CV5         0x100f          /* Global Thread Storage data */

/***** CV7 specific enums */
#define S_CEXMODEL32_CV7 0x020a

#define S_FRAMEPROC_CV7 0x1012
#define S_ANNOTATION_CV7 0x1019
#define S_OBJNAME_CV7   0x1101
#define S_THUNK32_CV7   0x1102
#define S_BLOCK32_CV7   0x1103
#define         S_WITH32_CV7    0x1104
#define S_LABEL32_CV7   0x1105
#define S_COBOLUDT_CV7  0x1109
#define S_REGISTER_CV7  0x1106
#define S_CONSTANT_CV7  0x1107
#define S_UDT_CV7       0x1108

#define S_MANYREG_V7    0x110a
#define S_BPREL32_CV7   0x110b
#define S_LDATA32_CV7   0x110c
#define S_GDATA32_CV7   0x110d
#define S_PUB32_CV7     0x110e
#define S_LPROC32_CV7   0x110f
#define S_GPROC32_CV7   0x1110
#define S_REGREL32_CV7  0x1111
#define S_LTHREAD32_CV7 0x1112
#define S_GTHREAD32_CV7 0x1113
#define S_LPROCMIPS_CV7 0x1114
#define S_GPROCMIPS_CV7 0x1115
#define S_COMPILE2_CV7  0x1116
#define S_MANYREG2_CV7  0x1117
#define S_LMANDATA_CV7 0x111c
#define S_GMANDATA_CV7 0x111d
#define S_MANSLOT_CV7  0x1120
#define S_UNAMESPACE_CV7 0x1124
#define S_GMANPROC_CV7 0x112a
#define S_LMANPROC_CV7 0x112b
#define S_TRAMPOLINE_CV7 0x112c
#define S_MANCONSTANT_CV7 0x112d
#define S_ATTR_FRAMEREL_CV7 0x112e
#define S_ATTR_REGISTER_CV7 0x112f
#define S_ATTR_REGREL_CV7  0x1130
#define S_ATTR_MANYREG_CV7 0x1131
#define S_SEPCODE_CV7  0x1132
#define S_LOCAL_CV7    0x1133
#define S_DEFRANGE_CV7 0x1134
#define S_DEFRANGE2_CV7 0x1135

#define S_COMPILE3              0x113c  /* Replacement for S_COMPILE2        */
#define S_ENVBLOCK              0x113d  /* Environment block symbol          */

#define S_GPROC32_ID    0x1147
#define S_LPROC32_ID    0x1146
#define S_PROC_ID_END   0x114f

/* Common enums or macros (in CV7 enum order) */
#define         S_END           0x0006          /* End block, procedure, thunk */
#define         S_SKIP          0x0007          /* Skip - Reserve symbol space */
#define         S_ENDARG        0x000a          /* End of arguments */
#define         S_RETURN        0x000d          /* Function return desription */

#define S_CEXMODEL32    (IS_CV5 ? INVALID : S_CEXMODEL32_CV7)

#define S_VFTABLE32     0x100c          /* Virtual function table path descriptor 16:32 */
#define S_FRAMEPROC     (IS_CV5 ? INVALID : S_FRAMEPROC_CV7) 
#define S_ANNOTATION    (IS_CV5 ? INVALID : S_ANNOTATION_CV7)
#define S_OBJNAME       (IS_CV5 ? S_OBJNAME_CV5         : S_OBJNAME_CV7)
#define S_THUNK32       (IS_CV5 ? S_THUNK32_CV5                 : S_THUNK32_CV7)
#define S_BLOCK32       (IS_CV5 ? S_BLOCK32_CV5                 : S_BLOCK32_CV7)
#define S_LABEL32       (IS_CV5 ? S_LABEL32_CV5                 : S_LABEL32_CV7)
#define S_REGISTER      (IS_CV5 ? S_REGISTER_CV5        : S_REGISTER_CV7)
#define S_CONSTANT      (IS_CV5 ? S_CONSTANT_CV5        : S_CONSTANT_CV7)
#define S_UDT           (IS_CV5 ? S_UDT_CV5             : S_UDT_CV7)
#define S_COBOLUDT      (IS_CV5 ? INVALID : S_COBOLUDT_CV7)
#define S_MANYREG       (IS_CV5 ? S_MANYREG_CV5                 : S_MANYREG_CV7)
#define S_BPREL32       (IS_CV5 ? S_BPREL32_CV5         : S_BPREL32_CV7)
#define S_LDATA32       (IS_CV5 ? S_LDATA32_CV5         : S_LDATA32_CV7)
#define S_GDATA32       (IS_CV5 ? S_GDATA32_CV5                 : S_GDATA32_CV7)
#define S_PUB32                 (IS_CV5 ? S_PUB32_CV5           : S_PUB32_CV7)
#define S_LPROC32       (IS_CV5 ? S_LPROC32_CV5         : S_LPROC32_CV7)
#define S_GPROC32       (IS_CV5 ? S_GPROC32_CV5         : S_GPROC32_CV7)
#define S_REGREL32      (IS_CV5 ? S_REGREL32_CV5        : S_REGREL32_CV7)
#define S_LTHREAD32     (IS_CV5 ? S_LTHREAD32_CV5       : S_LTHREAD32_CV7)
#define S_GTHREAD32     (IS_CV5 ? S_GTHREAD32_CV5       : S_GTHREAD32_CV7)
#define S_LPROCMIPS     (IS_CV5 ? INVALID : S_LPROCMIPS_CV7)
#define S_GPROCMIPS     (IS_CV5 ? INVALID : S_GPROCMIPS_CV7)
#define S_COMPILE2      (IS_CV5 ? INVALID : S_COMPILE2_CV7)
#define S_MANYREG2      (IS_CV5 ? INVALID : S_MANYREG2_CV7)

#define S_LMANDATA      (IS_CV5 ? INVALID : S_LMANDATA_CV7)
#define S_GMANDATA      (IS_CV5 ? INVALID : S_GMANDATA_CV7)
#define S_MANSLOT       (IS_CV5 ? INVALID : S_MANSLOT_CV7)

#define S_UNAMESPACE    (IS_CV5 ? INVALID : S_UNAMESPACE_CV7)

#define S_GMANPROC      (IS_CV5 ? INVALID : S_GMANPROC_CV7)
#define S_LMANPROC      (IS_CV5 ? INVALID : S_LMANPROC_CV7)
#define S_TRAMPOLINE    (IS_CV5 ? INVALID : S_TRAMPOLINE_CV7)
#define S_MANCONSTANT   (IS_CV5 ? INVALID : S_MANCONSTANT_CV7)
#define S_ATTR_FRAMEREL (IS_CV5 ? INVALID : S_ATTR_FRAMEREL_CV7)
#define S_ATTR_REGISTER (IS_CV5 ? INVALID : S_ATTR_REGISTESR_CV7)
#define S_ATTR_REGREL   (IS_CV5 ? INVALID : S_ATTR_REGREL_CV7)
#define S_ATTR_MANYREG  (IS_CV5 ? INVALID : S_ATTR_MANYREG_CV7)
#define S_SEPCODE       (IS_CV5 ? INVALID : S_SEPCODE_CV7)
#define S_LOCAL         (IS_CV5 ? INVALID : S_LOCAL_CV7)
#define S_DEFRANGE      (IS_CV5 ? INVALID : S_DEFRANGE_CV7)
#define S_DEFRANGE2     (IS_CV5 ? INVALID : S_DEFRANGE2_CV7)

/* CV5-only */
#define S_COMPILE       (IS_CV5 ? S_COMPILE_CV5       : INVALID)
#define S_ENTRYTHIS     (IS_CV5 ? S_ENTRYTHIS_CV5     : INVALID)
#define         S_VFTPATH32     (IS_CV5 ? S_VFTPATH32_CV5     : INVALID)
#define S_PROCREF       (IS_CV5 ? S_PROCREF_CV5       : INVALID)
#define S_DATAREF       (IS_CV5 ? S_DATAREF_CV5       : INVALID)
#define S_ALIGN         (IS_CV5 ? S_ALIGN_CV5         : INVALID)

/**************************************************************************/
#define S_REGTAHOE      0x0500          /* Tahoe register variable */
#define S_MANYREGTAHOE  0x0501          /* Tahoe many register symbol */

#define S_REGRELTAHOE   0x0600          /* Offset relative to arbitrary register */
#define S_MANYLOCTAHOE  0x0601          /* Multiple locations for a Tahoe symbol */
#define S_LPROCTAHOE    0x0602          /* Local procedure start Tahoe */
#define S_GPROCTAHOE    0x0603          /* Global Procedure start Tahoe */

/* Symbols for IA64 Architectures */
#ifdef MSVC60DEBUG
#define S_LPROCIA64     0x1015          /* Local procedure start */
#define S_GPROCIA64     0x1016          /* Global procedure start */
#endif

/* S_COMPILE & S_COMPILE2 fields */


/* S_COMPILE:flags && S_COMPILE2:flags*/
/* bits0:7 - language*//* common to S_COMPILE and S_COMPILE2 */
#define STI_C           0x00
#define STI_C_PLUS_PLUS 0x01
#define STI_Fortran     0x02

#define STI_FLAGS       0x1000          /* PcodePresent - False
                                           Float Precision - False
                                           Float Package - hardware
                                           Ambient Code and Data - Near
                                           Mode32 - True */
                             
/* S_COMPILE:flags:second byte */
#define         STI_Float_Precision 0x0002      /* FP precision */
#define         STI_Mode32          0x0800      /* Compiled for 32 bit address */

/* S_COMPILE2:flags:second byte */
#define STI_fEC             0x0001
#define STI_fNoDbgInfo      0x0002
#define STI_fLTCG           0x0004
#define STI_fNoDataAlign    0x0010
#define STI_fManagedPresent 0x0020
#define STI_fSecurityChecks 0x0040

/* Properties of user-defined type (S_UDT) */
#define STI_UDT_IS_TAG  0x8000
#define STI_UDT_IS_NEST         0x4000

/* Function  call and returns */
#define         STI_CSTYLE      0x0001
#define         STI_RSCLEAN     0x0002
#define STI_VOID_RET    0x00            /* Void return */
#define STI_REG_RET     0x01            /* Return value in registers */
#define STI_CA_NEAR     0x02            /* Caller allocated near */
#define STI_CA_FAR      0x03            /* Caller allocated far */
#define STI_RA_NEAR     0x04            /* Returnee allocated near */
#define STI_RA_FAR      0x05            /* Returnee allocated far */

/* Procedure and code label flags */
#define STI_FPO                 0x01            /* Frame pointer omitted */
#define         STI_INTERRUPT   0x02            /* Interrupt routine */
#define STI_NEVER_RET   0x08            /* Never returns */

/* Thunk Ordinal */
#define STI_THUNK_NOTYPE        0
#define STI_THUNK_ADJUSTOR      1
#define STI_THUNK_VCALL         2
#define STI_THUNK_PCODE         3

/* Execution models */

#define         STI_EXEC_NO             0x00            /* Not executable code */
#define STI_EXEC_JUMP_TBL       0x01            /* Compiler generated jump table */
#define STI_EXEC_PAD_DATA       0x02            /* PAdding for data */
#define STI_EXEC_NATIV_MODEL    0x20    /* Native model (no processor specified)*/
#define STI_EXEC_PAD_ALIGN      0x22            /* Code padding for alignment */
#define STI_EXEC_CODE           0x23            /* code */
#define STI_EXEC_PCODE          0x40            /* Pcode */

/**************************************************************************/
/* Type indices */
/* V7 spec 3.2 Type String */

/* Leaf indices for type records that can be referenced from symbols or are standalone
   records */
/***** CV4 specific enums -- define only */
#define         LF_MODIFIER_CV4         0x0001          /* Properties of a type */
#define         LF_POINTER_CV4  0x0002          /* Generic pointer type */
#define         LF_ARRAY_CV4    0x0003          /* Simple array */
#define         LF_CLASS_CV4    0x0004          /* Class */
#define         LF_STRUCTURE_CV4        0x0005          /* Structure */
#define         LF_UNION_CV4    0x0006          /* Union */
#define         LF_ENUM_CV4     0x0007          /* Enumerated type */
#define         LF_PROCEDURE_CV4        0x0008          /* Procedure type */ 
#define         LF_MFUNCTION_CV4        0x0009          /* Procedure type */ 
#define LF_BARRAY_CV4   0x000d          /* Basic Array */
#define         LF_DIMARRAY_CV4         0x0011          /* Multiply dimensioned array */
#define         LF_VFTPATH_CV4  0x0012          /*  */
#define         LF_PRECOMP_CV4  0x0013          /* Reference precompiled types */
#define LF_OEM_CV4      0x0015          /* OEM Generic Type */

/***** CV5 specific enums */
#define         LF_LABEL_CV5    0x000e          /* Code label */
#define         LF_NULL_CV5     0x000f          /* Symbol requires type rec but null */
#define LF_NOTTRANS_CV5         0x0010
#define         LF_ENDPRECOMP_CV5       0x0014          /* End of precompiled types */
#define LF_TYPESERVER_CV5       0x0016
#define         LF_ARRAY_CV5    0x1003          /* Simple array */
#define         LF_CLASS_CV5    0x1004          /* Class */
#define         LF_STRUCTURE_CV5        0x1005          /* Structure */
#define         LF_UNION_CV5    0x1006          /* Union */
#define         LF_ENUM_CV5     0x1007          /* Enumerated type */
#define LF_BARRAY_CV5   0x100d          /* Basic Array */
#define         LF_DIMARRAY_CV5         0x100c          /* Multiply dimensioned array */
#define         LF_VFTPATH_CV5  0x1012          /*  */
#define         LF_PRECOMP_CV5  0x1013          /* Reference precompiled types */
#define LF_OEM_CV5      0x100f          /* OEM Generic Type */

/***** CV7 specific enums */
#define         LF_ARRAY_CV7    0x1503
#define         LF_CLASS_CV7    0x1504
#define         LF_STRUCTURE_CV7        0x1505
#define         LF_UNION_CV7    0x1506
#define         LF_ENUM_CV7     0x1507
#define LF_COBOL0_CV7   0x100a
#define LF_COBOL1_CV7   0x100c
#define LF_BARRAY_CV7   0x100b
#define LF_LABEL_CV7    0x000e
#define LF_NULL_CV7     0x000f
#define LF_NOTTRAN_CV7  0x0010
#define LF_DIMARRAY_CV7 0x1508
#define LF_VFTPATH_CV7  0x100d
#define LF_PRECOMP_CV7  0x1509
#define LF_ENDPRECOMP_CV7 0x0014
#define LF_OEM_CV7      0x100f
#define LF_OEM2_CV7     0x1011
#define LF_TYPESERVER_CV7 0x1501
#define LF_ALIAS_CV7     0x150a
#define LF_MANAGED_CV7 0x1514
#define LF_TYPESERVER2_CV7 0x1515

/* Common enums or macros (in CV7 enum order) */
#define         LF_MODIFIER     0x1001          /* Properties of a type */
#define         LF_POINTER      0x1002          /* Generic pointer type */
#define LF_ARRAY        (IS_CV5 ? LF_ARRAY_CV5       : LF_ARRAY_CV7)
#define LF_CLASS        (IS_CV5 ? LF_CLASS_CV5       : LF_CLASS_CV7)
#define LF_STRUCTURE    (IS_CV5 ? LF_STRUCTURE_CV5   : LF_STRUCTURE_CV7)
#define LF_UNION        (IS_CV5 ? LF_UNION_CV5       : LF_UNION_CV7)
#define LF_ENUM         (IS_CV5 ? LF_ENUM_CV5        : LF_ENUM_CV7)
#define         LF_PROCEDURE    0x1008          /* Procedure type */
#define         LF_MFUNCTION    0x1009          /* Procedure type */
#define         LF_VTSHAPE      0x000a          /* Virtual shape */
#define LF_COBOL0       (IS_CV5 ? INVALID            : LF_COBOL0_CV7)
#define LF_COBOL1       (IS_CV5 ? INVALID            : LF_COBOL1_CV7)
#define LF_BARRAY       (IS_CV5 ? LF_BARRAY_CV5      : LF_BARRAY_CV7)
#define LF_LABEL        (IS_CV5 ? LF_LABEL_CV5               : LF_LABEL_CV7)
#define LF_NULL         (IS_CV5 ? LF_NULL_CV5        : LF_NULL_CV7)
#define LF_NOTTRAN      (IS_CV5 ? LF_NOTTRANS_CV5    : LF_NOTTRAN_CV7)
#define LF_DIMARRAY     (IS_CV5 ? LF_DIMARRAY_CV5    : LF_DIMARRAY_CV7)
#define LF_VFTPATH      (IS_CV5 ? LF_VFTPATH_CV5     : LF_VFTPATH_CV7)
#define LF_PRECOMP      (IS_CV5 ? LF_PRECOMP_CV5     : LF_PRECOMP_CV7)
#define LF_ENDPRECOMP   (IS_CV5 ? LF_ENDPRECOMP_CV5  : LF_ENDPRECOMP_CV7)
#define LF_OEM          (IS_CV5 ? LF_OEM_CV5         : LF_OEM_CV7)
#define LF_OEM2                 (IS_CV5 ? INVALID            : LF_OEM2_CV7)
#define LF_TYPESERVER   (IS_CV5 ? INVALID            : LF_TYPESERVER_CV7)
#define LF_ALIAS        (IS_CV5 ? INVALID            : LF_ALIAS_CV7)
#define LF_MANAGED      (IS_CV5 ? INVALID            : LF_MANAGED_CV7)
#define LF_TYPESERVER2  (IS_CV5 ? INVALID            : LF_TYPESERVER2_CV7)

/***** Intel OEM records. Supported by EDB and IDB v7. No longer supported
 in 9.0. Need to be deleted. */
#define LF_OEM_IDENT_INTEL 0xf001       /* Intel OEM identifier */
#define LF_recOEM_ASSMD_SHP_ARR         0x0001  /* Intel Assumed shape array */
#define LF_recOEM_OFFSET_POINTER 0x0002         /* Intel Offset pointer */
#define LF_recOEM_PARENT_FUNCTION 0x0003
/* Contains name of functions parent function when nested */
#define LF_recOEM_OFFSET 0x0004
/* Like OFFSET_POINTER but do not dereference */

/***** Microsoft OEM records. Supported by MS .NET debugger and IDB v8+ */
#define LF_OEM_IDENT_MSF90 0xf090       /* Microsoft F90 OEM identifier */
#define LF_recOEM_MSF90_DESCR_ARR 0x0000 /* Microsoft F90 Described Array */
#define LF_recOEM_MSF90_MODULE_DESCR 0x0002 
/* Microsoft F90 Module Descriptor */
#define LF_recOEM_MSF90_HOST_REF 0x0003         /* Microsoft F90 Host Reference */
#define LF_recOEM_F090_HOST_PDATA 0x0004 /* Intel extension for post indexed data */
#define LF_recOEM_MSF90_DESCRIPTOR 0x0005 // Intel's new generic descriptor type


#define PROP_PACKED     0x0001          /* Packed structure or union */
#define PROP_CTOR       0x0002          /* Has constructuctors and/or desctructors */
#define PROP_OVEROPS    0x0004          /* Has overloaded operators */
#define PROP_ISNESTED   0x0008          /* Nested structure or union */
#define PROP_CNESTED    0x0010          /* Contains nested classes */
#define PROP_OPASSIGN   0x0020          /* Has overloaded assignment */
#define PROP_OPCAST     0x0040          /* Has casting methods */
#define PROP_FWDREF     0x0080          /* Is a forward (incomplete) reference */
#define PROP_SCOPED     0x0100          /* This is a scoped definition */
#define PROP_REALNAME   0x0200          /* has extra 'real' name field */
#define PROP_SEALED     0x0400          /* object cannot be derived from */


/*
  Leaf indices for type records that can be referenced from other type records
*/
/***** CV4 specific enums -- define only */
#define LF_SKIP_CV4     0x0200
#define         LF_ARGLIST_CV4  0x0201          /* Formal parameters argument list */
#define         LF_DEFARG_CV4   0x0202          /* Supplied default argument */
#define         LF_FIELDLIST_CV4        0x0204          /* Fields of structures */
#define         LF_DERIVED_CV4  0x0205          /* Derived classes */
#define         LF_BITFIELD_CV4         0x0206          /* Bit field type definition */
#define         LF_MLIST_CV4    0x0207          /* Method list */
#define LF_DIMCONU_CV4  0x0208          /* Array with const upper bound */
#define LF_DIMCONLU_CV4         0x0209          /* Array - const lower & upper bound */
#define LF_DIMVARU_CV4  0x020a          /* Variable lower & upper bound */
#define LF_DIMVARLU_CV4         0x020b          /* Variable lower & upper bound */

/* CV5 specific */
#define         LF_LIST_CV5     0x0203          /* List */
#define         LF_DEFARG_CV5   0x1202          /* Supplied default argument */

/* CV7 specific */
#define         LF_DEFARG_CV7   0x150b          /* Supplied default argument */

/* Common enums or macros (in CV7 enum order) */
#define LF_SKIP         0x1200
#define         LF_ARGLIST      0x1201          /* Formal parameters argument list */
#define LF_DEFARG       (IS_CV5 ? LF_DEFARG_CV5 : LF_DEFARG_CV7)
#define         LF_FIELDLIST    0x1203          /* Fields of structures */
#define         LF_DERIVED      0x1204          /* Derived classes */
#define         LF_BITFIELD     0x1205          /* Bit field type definition */
#define         LF_MLIST        0x1206          /* Method list */
#define LF_DIMCONU      0x1207          /* Array with const upper bound */
#define LF_DIMCONLU     0x1208          /* Array - const lower & upper bound */
#define LF_DIMVARU      0x1209          /* Variable lower & upper bound */
#define LF_DIMVARLU     0x120a                  /* Variable lower & upper bound */
#define LF_REFSYM       0x020c          /* Referenced symbol */
/* CV5 only */
#define         LF_LIST         (IS_CV5 ? LF_LIST_CV5   : INVALID)

/*
  Leaf indices for fields of complex lists
*/
/***** CV4 specific enums -- define only */
#define         LF_BCLASS_CV4   0x0400          /* Leaf specifies a real base class*/
#define         LF_VBCLASS_CV4  0x0401          /* Direct inherited virtual base class*/
#define         LF_IVBCLASS_CV4         0x0402          /*Indirect inheritd virtual base class*/
#define LF_FRIENDFCN_CV4        0x0404  /* Friend function */
#define LF_INDEX_CV4    0x0405          /* Index to another type record */
#define LF_MEMBER_CV4   0x0406          /* Nonstatic data member of a struct */
#define LF_STMEMBER_CV4         0x0407          /*  */
#define LF_METHOD_CV4   0x0408          /*  */
#define LF_NESTEDTYPE_CV4       0x0409  /* Nested type definition */
#define LF_VFUNCTAB_CV4         0x040a          /*  */
#define LF_FRIENDCLS_CV4        0x040b  /*  */
#define LF_ONEMETHOD_CV4        0x040c
#define LF_VFUNCOFF_CV4         0x040d

/* CV5 specific */
#define LF_ENUMERATE_CV5  0x0403 /* Name and value of an enumerate */
#define LF_FRIENDFCN_CV5  0x1403 /* Friend function */
#define LF_INDEX_CV5      0x1404 /* Index to another type record */
#define LF_MEMBER_CV5     0x1405 /* Nonstatic data member of a struct */
#define LF_STMEMBER_CV5           0x1406
#define LF_METHOD_CV5     0x1407
#define LF_NESTEDTYPE_CV5 0x1408 /* Nested type definition */
#define LF_ONEMETHOD_CV5  0x140b

/* CV7 specific */
#define LF_ENUMERATE_CV7        0x1502          /* Name and value of an enumerate */
#define LF_FRIENDFCN_CV7        0x150c
#define LF_INDEX_CV7            0x1404
#define LF_MEMBER_CV7   0x150d
#define LF_STMEMBER_CV7         0x150e
#define LF_METHOD_CV7   0x150f
#define LF_NESTEDTYPE_CV7       0x1510
#define LF_ONEMETHOD_CV7        0x1511
#define LF_NESTTYPEEX_CV7       0x1512
#define LF_MEMBERMODIFY_CV7     0x1513

/* Common enums or macros (in CV7 enum order) */
#define         LF_BCLASS       0x1400          /* Leaf specifies a real base class*/
#define         LF_VBCLASS      0x1401          /* Direct inherited virtual base class*/
#define         LF_IVBCLASS     0x1402          /* Indirect inherited virtual base class*/
#define LF_ENUMERATE    (IS_CV5 ? LF_ENUMERATE_CV5  : LF_ENUMERATE_CV7)
#define LF_FRIENDFCN    (IS_CV5 ? LF_FRIENDFCN_CV5  : LF_FRIENDFCN_CV7)
#define LF_INDEX        (IS_CV5 ? LF_INDEX_CV5      : LF_INDEX_CV7)
#define LF_MEMBER       (IS_CV5 ? LF_MEMBER_CV5     : LF_MEMBER_CV7)
#define LF_STMEMBER     (IS_CV5 ? LF_STMEMBER_CV5   : LF_STMEMBER_CV7)
#define LF_METHOD       (IS_CV5 ? LF_METHOD_CV5     : LF_METHOD_CV7)
#define LF_NESTEDTYPE   (IS_CV5 ? LF_NESTEDTYPE_CV5 : LF_NESTEDTYPE_CV7)
#define LF_VFUNCTAB     0x1409          /*  */
#define LF_FRIENDCLS    0x140a  /*  */
#define LF_ONEMETHOD    (IS_CV5 ? LF_ONEMETHOD_CV5  : LF_ONEMETHOD_CV7)
#define LF_VFUNCOFF     0x140c
#define LF_NESTTYPEEX   (IS_CV5 ? INVALID           : LF_NESTTYPEEX_CV7)
#define LF_MEMBERMODIFY (IS_CV5 ? INVALID           : LF_MEMBERMODIFY_CV7)

/* Access attribute bit field values */
        /* access are bit 1 and 2 */
#define         STI_ACCESS_NO           0x0000  /* No access protection */
#define STI_ACCESS_PRIVATE      0x0001  /* Private */
#define STI_ACCESS_PROTECT      0x0002  /* Protected */
#define         STI_ACCESS_PUBLIC       0x0003  /* Public */
        /* mprob is bit 3,4,5 */
#define STI_MPROP_VIRTUAL       0x0004  /* Virtual method */
#define STI_MPROP_STATIC        0x0008  /* Static method */
#define STI_MPROP_FRIEND        0x000c  /* Friend method */
#define STI_MPROP_INTR_VRT      0x0010  /* Introducing Virtual method */
#define STI_MPROP_PURE_VRT      0x0014  /* Pure Virtual method */
#define STI_MPROP_PURE_INTR_VRT         0x0018  /* Pure Introducing Virtual method */

/* Type modifier values */

#define         STI_MOD_CONST           0x01    /* Const modifier property */
#define         STI_MOD_VOLATILE        0x02    /* Volatile modifier property */
#define         STI_MOD_UNALIGN                 0x04    /* Unaligned modifier property */

/* Member attribute field types */
#define STI_MEMB_ATTR_ACC_PRIVATE 1             /* Private access */
#define STI_MEMB_ATTR_ACC_PROTECTED 2   /* Protected access */
#define STI_MEMB_ATTR_ACC_PUBLIC 3              /* Public access */

/* Pointer attributes for LF_POINTER */
//===----------------------------------------------------------------------===//
// STIPointerType
//===----------------------------------------------------------------------===//
#define STI_POINTER_TYPES                                                   \
    X(ATTR_PTRTYPE_SEG,     0x0003)     /* based on segment             */  \
    X(ATTR_PTRTYPE_VALUE,   0x0004)     /* based on value-based pointers*/  \
    X(ATTR_PTRTYPE_NEAR32,  0x000a)     /* near 32-bit pointer          */  \
    X(ATTR_PTRTYPE_64,      0x000c)     /* 64-bit pointer               */

enum STIPointerTypeEnum {
#define X(NAME,VALUE) NAME = VALUE,
    STI_POINTER_TYPES
#undef  X
};
typedef enum STIPointerTypeEnum STIPointerType;

#define ATTR_PTRMODE_REFERENCE  0x0020  /* (1) Reference */
#define ATTR_PTRMODE_DATAMB     0x0040  /* (2) Pointer to data member */
#define ATTR_PTRMODE_METHOD     0x0060  /* (3) Pointer to method */
#define ATTR_PTRMODE_RVALUE     0x0080; /* rvalue reference (VS2010 only) */
#define ATTR_ISFLAT32           0x0100  /* TRUE if 16:32 pointer */
#define ATTR_VOLATILE           0x0200  /* TRUE if pointer is volatile */
#define ATTR_CONST              0x0400  /* TRUE if pointer is const */
#define ATTR_UNALIGNED          0x0800  /* TRUE if pointer is unaligne */

/* Calling conventions for procedure */

#define NEAR_C          0x00            /* Near C */
#define         NEAR_PASCAL     0x03            /* Near Pascal */
#define NEAR_FASTCALL   0x04            /* Near Fastcall */
#define         NEAR_STDCALL    0x07            /* Near stdcall */
#define THISCALL        0x0b            /* This call calling convention */

/* Addressing modes of labels */

#define         LBL_NEAR        0               /* Near label */
#define         LBL_FAR                 4               /* Far label */

/* 5.2 Primitive Type Listing */
/* 5.2.1 Special types */
#define T_NOTYPE        0x0000 /* Unxharacterized type */
#define T_ABS           0x0001 /* Absolute symbol */
#define T_SEGMENT       0x0002 /* Segment type */
#define T_VOID          0x0003 /* Void */
#define         T_PVOID                 0x0103 /* Near pointer to void */
#define         T_PFVOID        0x0203 /* Far pointer to void */
#define         T_PHVOID        0x0303 /* Huge pointer to void */
#define         T_32PVOID       0x0403 /* 32-bit near pointer to void */
#define T_32PFVOID      0x0503 /* 64-bit far pointer to void */
#define T_64PVOID       0x0603 /* 64-bit Pointer to void */

/* The rest not used */

/* 5.2.2 Character types */
#define         T_CHAR          0x0010 /* 8-bit signed */
#define         T_UCHAR                 0x0020 /* 8-bit unsigned */
#define         T_PCHAR                 0x0110 /* Near pointer to 8-bit signed */
#define         T_PUCHAR        0x0120 /* Near pointer to 8-bit unsigned */
#define         T_32PCHAR       0x0410 /* 16:32 near pointer to 8-bit signed */
#define         T_32PUCHAR      0x0420 /* 16:32 near pointer to 8-bit unsigned */
#define T_32PFCHAR      0x0510 /* 16:32 far pointer to 8-bit signed */
#define T_32PFUCHAR     0x0520 /* 16:32 far pointer to 8-bit unsigned */
#define T_64PCHAR       0x0610 /* 64-bit pointer to 8-bit signed */
#define T_64PUCHAR      0x0620 /* 64-bit pointer to 8-bit unsigned */
/* The rest not used */

/* 5.2.3 Really a character types */
#define         T_RCHAR                 0x0070 /* 8-bit real char */
#define         T_PRCHAR        0x0170 /* Near pointer */
#define         T_PFRCHAR       0x0270 /* Far pointer */
#define         T_PHRCHAR       0x0370 /* Huge pointer */
#define         T_32PRCHAR      0x0470 /* 16:32 near pointer */
#define T_32PFRCHAR     0x0570 /* 16:32 far pointer */
#define T_64PRCHAR      0x0670 /* 64-bit pointer */

/* 5.2.4 Wide character types */
#define T_32PWCHAR      0x0471  /* 16:32 near pointer */
#define T_32PFWCHAR     0x0571  /* 16:32 far pointer */
#define T_64PWCHAR      0x0671  /* 64-bit pointer */
/* The rest not used */

/* 5.2.5 Really 16 bit integer types */
#define         T_INT2          0x0072 /* really 16-bit signed int */
#define         T_UINT2                 0x0073 /* really 16-bit unsigned int */
#define         T_PINT2                 0x0172 /* Near pointer to 16-bit signed int */
#define         T_PUINT2        0x0173 /* Near pointer to 16-bit unsigned int */
#define         T_PFINT2        0x0272 /* Far pointer to 16-bit signed int */
#define         T_PFUINT2       0x0273 /* Far pointer to 16-bit unsigned int */
#define         T_32PINT2       0x0472 /* Near pointer to 16-bit signed int */
#define         T_32PUINT2      0x0473 /* Near pointer to 16-bit unsigned int */
#define T_32PFINT2      0x0572 /* Far pointer to 16-bit signed int */
#define T_32PFUINT2     0x0573 /* Far pointer to 16-bit unsigned int */
#define T_64PINT2       0x0672 /* 64-bit pointer to 16-bit signed int */
#define T_64PUINT2      0x0673 /* 64-bit pointer to 16-bit unsigned int */
/* The rest not used */

/* 5.2.6 16-bit short types */
#define         T_SHORT                 0x0011 /* 16-bit signed */
#define         T_USHORT        0x0021 /* 16-bit unsigned */
#define         T_PSHORT        0x0111 /* Near pointer to 16-bit signed */
#define         T_PUSHORT       0x0121 /* Near pointer to 16-bit unsigned */
#define         T_PFSHORT       0x0211 /* Far pointer to 16-bit signed */
#define         T_FUSHORT       0x0221 /* Far pointer to 16-bit unsigned */
#define         T_PHSHORT       0x0311 /* Huge pointer to 16-bit signed */
#define         T_PHUSHORT      0x0321 /* Huge pointer to 16-bit unsigned */
#define         T_32PSHORT      0x0411 /* 16:32 near pointer to 16-bit signed */
#define         T_32PUSHORT     0x0421 /* 16:32 near pointer to 16-bit unsigned */
#define T_32PFSHORT     0x0511 /* 16:32 far pointer to 16-bit signed */
#define T_32PFUSHORT    0x0521 /* 16:32 far pointer to 16-bit unsigned */
#define T_64PSHORT      0x0611 /* 64-bit pointer to 16-bit signed */
#define T_64PUSHORT     0x0621 /* 64-bit pointer to 16-bit unsigned */

/* 5.2.7 Really 32 bit integer types */
#define         T_INT4          0x0074 /* really 32 bit signed int */
#define         T_UINT4                 0x0075 /* really 32 bit unsigned int */
#define         T_PINT4                 0x0174 /* Near pointer to 32 bit signed int */
#define         T_PUINT4        0x0175 /* Near pointer to 32 bit unsigned int */
#define T_PFINT4        0x0274 /* Far pointer to 32 bit signed int */
#define T_PFUINT4       0x0275 /* Far pointer to 32 bit unsigned int */
#define         T_32PINT4       0x0474 /* Near pointer to 32 bit signed int */
#define         T_32PUINT4      0x0475 /* Near pointer to 32 bit unsigned int */
#define T_32PFINT4      0x0574 /* Near pointer to 32 bit signed int */
#define T_32PFUINT4     0x0575 /* Near pointer to 32 bit unsigned int */
#define T_64PINT4       0x0674 /* 64 bit pointer to 32 bit signed int */
#define T_64PUINT4      0x0675 /* 64 bit pointer to 32 bit unsigned int */
/* The rest not used */

/* 5.2.8 32-bit long types */
#define         T_LONG          0x0012          /* 32 bit integer */
#define         T_ULONG                 0x0022          /* 32 bit integer */
#define         T_PLONG                 0x0112          /* Pointer to long */
#define         T_PULONG        0x0122          /* Pointer to unsigned long */
#define         T_PFLONG        0x0212          /* Far pointer to long */
#define         T_PFULONG       0x0222          /* Far pointer to unsigned long */
#define         T_32PLONG       0x0412          /* Pointer to long */
#define         T_32PULONG      0x0422          /* Pointer to unsigned long */
#define T_32PFLONG      0x0512  /* Pointer to long */
#define T_32PFULONG     0x0522  /* Pointer to unsigned long */
#define T_64PLONG       0x0612  /* 64 bit pointer to 32 bit signed */
#define T_64PULONG      0x0622  /* 64 bit pointer to 32 bit unsigned */
/* The rest not used */

/* 5.2.9 Really 64-bit integer types */
#define T_INT8          0x0076 /* signed int */
#define T_UINT8                 0x0077 /* unsigned int */
#define T_PINT8                 0x0176 /* Near pointer to signed int */
#define T_PUINT8        0x0177 /* Near pointer to unsigned int */
#define T_PFINT8        0x0276 /* Far pointer to signed int */
#define T_PFUINT8       0x0277 /* Far pointer to unsigned int */
#define T_PHINT8        0x0376 /* Huge pointer to signed int */
#define T_PHUINT8       0x0377 /* Huge pointer to unsigned int */
#define T_32PINT8       0x0476  /* 16:32 near pointer to 64 bit signed int */
#define T_32PUINT8      0x0477  /* 16:32 near pointer to 64 bit unsigned int */
#define T_32PFINT8      0x0576  /* 16:32 near pointer to 64 bit signed int */
#define T_32PFUINT8     0x0577  /* 16:32 near pointer to 64 bit unsigned int */
#define T_64PINT8       0x0676  /* 64 bit pointer to 64 bit signed int */
#define T_64PUINT8      0x0677  /* 64 bit pointer to 64 bit unsigned int */
/* The rest not used */

/* 5.2.10 64-bit integer types */
#define         T_QUAD          0x0013          /* 64 bit integer */
#define         T_UQUAD                 0x0023          /* 64 bit integer */
#define         T_PQUAD                 0x0113          /* Pointer to quad */
#define         T_PUQUAD        0x0123          /* Pointer to unsigned quad */
#define         T_32PQUAD       0x0413          /* Pointer to quad */
#define         T_32PUQUAD      0x0423          /* Pointer to unsigned quad */
#define T_32PFQUAD      0x0513  /* Pointer to quad */
#define T_32PFUQUAD     0x0523  /* Pointer to unsigned quad */
#define T_64PQUAD       0x613   /* 64 bit pointer to 64 bit signed */
#define T_64PUQUAD      0x623   /* 64 bit pointer to 64 bit unsigned */
/* The rest not used */

/* 128 bit integer types */
//      128 bit int types

#define T_INT16         0x0078   /* 128 bit signed int */
#define T_PINT16        0x0178   /* 16 bit pointer to 128 bit signed int */
#define T_PFINT16       0x0278   /* 16:16 far pointer to 128 bit signed int */
#define T_PHINT16       0x0378   /* 16:16 huge pointer to 128 bit signed int */
#define T_32PINT16      0x0478   /* 32 bit pointer to 128 bit signed int */
#define T_32PFINT16     0x0578   /* 16:32 pointer to 128 bit signed int */
#define T_64PINT16      0x0678   /* 64 bit pointer to 128 bit signed int */
#define T_UINT16        0x0079   /* 128 bit unsigned int */
#define T_PUINT16       0x0179   /* 16 bit pointer to 128 bit unsigned int */
#define T_PFUINT16      0x0279   /* 16:16 far pointer to 128 bit unsigned int */
#define T_PHUINT16      0x0379   /* 16:16 huge pointer to 128 bit unsigned int */
#define T_32PUINT16     0x0479   /* 32 bit pointer to 128 bit unsigned int */
#define T_32PFUINT16    0x0579   /* 16:32 pointer to 128 bit unsigned int */
#define T_64PUINT16     0x0679   /* 64 bit pointer to 128 bit unsigned int */

/* 5.2.11 32-bit real types */
#define         T_REAL32        0x0040          /* 32 bit real */
#define         T_PREAL32       0x0140          /* Pointer to 32 bit real */
#define         T_PFREAL32      0x0240          /* Far pointer to 32 bit real */
#define         T_32PREAL32     0x0440          /* Pointer to 32 bit real */
#define T_32PFREAL32    0x0540  /* Pointer to 32 bit real */
#define T_64PREAL32     0x640   /* 64 bit pointer to 32 bit real */
/* The rest not used */

/* 5.2.12 48-bit real types */
#define         T_REAL48        0x0044          /* 48 bit real */
#define         T_PREAL48       0x0144          /* Pointer to 48 bit real */
#define         T_PFREAL48      0x0244          /* Far pointer to 48 bit real */
#define         T_32PREAL48     0x0444          /* Pointer to 48 bit real */
#define T_32PFREAL48    0x0544  /* Pointer to 48 bit real */
#define T_64PREAL48     0x644   /* 64 bit pointer to 48 bit real */
/* The rest not used */

/* 5.2.13 64-bit real types */
#define         T_REAL64        0x0041          /* 64 bit real */
#define         T_PREAL64       0x0141          /* Pointer to 64 bit real */
#define         T_PFREAL64      0x0241          /* Far pointer to 64 bit real */
#define         T_32PREAL64     0x0441          /* Pointer to 64 bit real */
#define T_32PFREAL64    0x0541  /* Pointer to 64 bit real */
#define T_64PREAL64     0x641   /* 64 bit pointer to 64 bit real */
/* The rest not used */

/* 5.2.14 80-bit real types */
#define         T_REAL80        0x0042          /* 80 bit real */
#define         T_PREAL80       0x0142          /* Pointer to 80 bit real */
#define         T_PFREAL80      0x0242          /* Far pointer to 80 bit real */
#define         T_32PREAL80     0x0442          /* Pointer to 80 bit real */
#define T_32PFREAL80    0x0542  /* Pointer to 80 bit real */
#define T_64PREAL80     0x642   /* 64 bit pointer to 80 bit real */
/* The rest not used */

/* 5.2.15 128-bit real types */
#define         T_REAL128       0x0043          /* 128 bit real */
#define         T_PREAL128      0x0143          /* Pointer to 128 bit real */
#define         T_PFREAL128     0x0243          /* Far pointer to 128 bit real */
#define         T_32PREAL128    0x0443          /* Pointer to 128 bit real */
#define T_32PFREAL128   0x0543  /* Pointer to 128 bit real */
#define T_64PREAL128    0x643   /* 64 bit pointer to 128 bit real */
/* The rest not used */

/* 5.2.16 32-bit complex types */
#define         T_CPLX32        0x0050          /* 32 bit complex type */
#define         T_PCPLX32       0x0150  /* Pointer to 32 bit complex type */
#define         T_32PCPLX32     0x0450  /* Pointer to 32 bit complex type */
#define T_32PFCPLX32    0x0550  /* Pointer to 32 bit complex type */
#define T_64PCPLX32     0x0650  /* 64 bit pointer to 32 bit complex */
/* The rest not used */

/* 5.2.17 64-bit complex types */
#define         T_CPLX64        0x0051          /* 64 bit complex type */
#define         T_PCPLX64       0x0151  /* Pointer to 64 bit complex type */
#define         T_32PCPLX64     0x0451  /* Pointer to 64 bit complex type */
#define T_32PFCPLX64    0x0551  /* Pointer to 64 bit complex type */
#define T_64PCPLX64     0x0651  /* 64 bit pointer to 64 bit complex */
/* The rest not used */

/* 5.2.18 80-bit complex types */
#define         T_CPLX80        0x0052          /* 80 bit complex type */
#define         T_PCPLX80       0x0152  /* Pointer to 80 bit complex type */
#define         T_32PCPLX80     0x0452  /* Pointer to 80 bit complex type */
#define T_32PFCPLX80    0x0552  /* Pointer to 80 bit complex type */
#define T_64PCPLX80     0x0652  /* 64 bit pointer to 80 bit complex */
/* The rest not used */

/* 5.2.19 128-bit complex types */
#define         T_CPLX128       0x0053          /* 128 bit complex type */
#define         T_PCPLX128      0x0153  /* Pointer to 128 bit complex type */
#define         T_32PCPLX128    0x0453  /* Pointer to 128 bit complex type */
#define T_32PFCPLX128   0x0553  /* Pointer to 128 bit complex type */
#define T_64PCPLX128    0x653   /* 64 bit pointer to 128 bit complex */
/* The rest not used */

/* 5.2.20 Boolean types */
#define         T_BOOL08        0x0030          /* 8 bit boolean */
#define         T_BOOL16        0x0031          /* 16 bit boolean */
#define         T_BOOL32        0x0032          /* 32 bit boolean */
#define         T_BOOL64        0x0033          /* 64 bit boolean */
#define         T_PBOOL08       0x0130          /* Pointer to eight bit boolean */
#define         T_PBOOL16       0x0131          /* Pointer to 16 bit boolean */
#define         T_PBOOL32       0x0132          /* Pointer to 32 bit boolean */
#define         T_PBOOL64       0x0133          /* Pointer to 64 bit boolean */
#define         T_32PBOOL08     0x0430          /* Pointer to eight bit boolean */
#define         T_32PBOOL16     0x0431          /* Pointer to 16 bit boolean */
#define         T_32PBOOL32     0x0432          /* Pointer to 32 bit boolean */
#define         T_32PBOOL64     0x0433          /* Pointer to 64 bit boolean */
#define T_32PFBOOL08    0x0530  /* Pointer to eight bit boolean */
#define T_32PFBOOL16    0x0531  /* Pointer to 16 bit boolean */
#define T_32PFBOOL32    0x0532  /* Pointer to 32 bit boolean */
#define T_32PFBOOL64    0x0533  /* Pointer to 64 bit boolean */
#define T_64PBOOL08     0x0630  /* 64 bit pointer to 8 bit boolean */
#define T_64PBOOL16     0x0631  /* 64 bit pointer to 16 bit boolean */
#define T_64PBOOL32     0x0632  /* 64 bit pointer to 32 bit boolean */
#define T_64PBOOL64     0x0633  /* 64 bit pointer to 64 bit boolean */
/* The rest not used */

/* Leaf indices for numeric fields of symbols and type records */

#define         LF_NUMERIC      0x8000          /* Decides if actual numeric value */
#define LF_CHAR                 0x8000          /* Signed char */
#define LF_SHORT        0x8001          /* Signed short */
#define LF_USHORT       0x8002          /* Unsigned short */
#define LF_LONG                 0x8003          /* Signed long */
#define LF_ULONG        0x8004          /* Unsigned long */
#define LF_REAL32       0x8005          /* 32 bit real */
#define LF_REAL64       0x8006          /* 64 bit real */
#define LF_REAL80       0x8007          /* 80 bit real */
#define LF_REAL128      0x8008          /* 128 bit real */
#define LF_QUADWORD     0x8009          /* Signed quad */
#define LF_UQUADWORD    0x800a          /* Unsigned quad */
#define LF_REAL48       0x800b          /* 48 bit real */
#define LF_COMPLEX32    0x800c          /* 32 bit complex */
#define LF_COMPLEX64    0x800d          /* 64 bit complex */
#define LF_COMPLEX80    0x800e          /* 80 bit complex */
#define LF_COMPLEX128   0x800f          /* 128 bit complex */
#define LF_VARSTRING    0x8010          /* Variable length string */
#define LF_OCTWORD      0x8017 /* cv7*/
#define LF_UOCTWORD     0x8018/* cv7*/
#define LF_DECIMAL      0x8019/* cv7*/
#define LF_UTFSTRING    0x801a/* cv7*/

#define LF_PAD0                 0xf0
#define LF_PAD1                 0xf1
#define LF_PAD2                 0xf2
#define LF_PAD3                 0xf3
#define LF_PAD4                 0xf4
#define LF_PAD5                 0xf5
#define LF_PAD6                 0xf6
#define LF_PAD7                 0xf7
#define LF_PAD8                 0xf8
#define LF_PAD9                 0xf9
#define LF_PAD10        0xfa
#define LF_PAD11        0xfb
#define LF_PAD12        0xfc
#define LF_PAD13        0xfd
#define LF_PAD14        0xfe
#define LF_PAD15        0xff


/*
** Macros to help hide the difference between CV5 and CV7, in which
** a number of the symbol and type information enumerations changed.
*/
#define S_WITH32        (IS_CV5 ? S_WITH32_CV5          : S_WITH32_CV7)

#define LF_ARRAY        (IS_CV5 ? LF_ARRAY_CV5          : LF_ARRAY_CV7)
#define LF_CLASS        (IS_CV5 ? LF_CLASS_CV5          : LF_CLASS_CV7)
#define LF_STRUCTURE    (IS_CV5 ? LF_STRUCTURE_CV5      : LF_STRUCTURE_CV7)
#define LF_UNION        (IS_CV5 ? LF_UNION_CV5                  : LF_UNION_CV7)
#define LF_ENUM                 (IS_CV5 ? LF_ENUM_CV5           : LF_ENUM_CV7)

#define LF_PRECOMP      (IS_CV5 ? LF_PRECOMP_CV5        : LF_PRECOMP_CV7)
#define LF_OEM          (IS_CV5 ? LF_OEM_CV5            : LF_OEM_CV7)
#define LF_DEFARG       (IS_CV5 ? LF_DEFARG_CV5         : LF_DEFARG_CV7)
#define LF_INDEX        (IS_CV5 ? LF_INDEX_CV5                  : LF_INDEX_CV7)

#define LF_FUNC_ID 0x1601
/* 
 * Used in DGI for MS debug info generation.
 * The MS CV debug document has not given enumeration for Xscale registers
 * yet.  The values used here are temporarily based on ARM enumeration until
 * consistence is achieved between the XScale C ABI and MS debugger.
 */
#define CV_XSCALE_R7       17  /* r11 - using ARM enum CV_ARM_R7 */
#define CV_XSCALE_R11      21  /* r11 - using ARM enum CV_ARM_R11 */
#define CV_XSCALE_R13      23  /* r13 - using ARM enum CV_ARM_SP */

/* Attributes in methodlists */
#define    CV_MTvanilla         0x00
#define    CV_MTvirtual         0x01
#define    CV_MTstatic          0x02
#define    CV_MTfriend          0x03
#define    CV_MTintro           0x04
#define    CV_MTpurevirt        0x05
#define    CV_MTpureintro       0x06

/* Attributes in methodlists */
#define    CV_MTvanilla         0x00
#define    CV_MTvirtual         0x01
#define    CV_MTstatic          0x02
#define    CV_MTfriend          0x03
#define    CV_MTintro           0x04
#define    CV_MTpurevirt        0x05
#define    CV_MTpureintro       0x06

// 3.3.10 (0x000A) Virtual Function Table Shape
//
// Descriptor values for LF_VTSHAPE.
//
#define CV_VFTS_NEAR            0x0
#define CV_VFTS_FAR             0x1
#define CV_VFTS_THIN            0x2
#define CV_VFTS_ADDR_DISPLACE   0x3
#define CV_VFTS_FAR_META        0x4
#define CV_VFTS_NEAR32          0x5
#define CV_VFTS_FAR32           0x6


#endif

