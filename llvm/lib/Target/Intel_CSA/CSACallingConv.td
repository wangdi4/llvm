//===- CSACallingConv.td - Calling Conventions CSA ---------*- tablegen -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the CSA architecture.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// For register assignments generally, see CSARegisterInfo.td

// CSA 64-bit C return-value convention using registers
def RetCC_Reg_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v2f32], CCAssignToReg<[R0, R1]>>,
  CCAssignToStack<8, 8>
]>;

// CSA 64-bit C Calling convention using registers
def CC_Reg_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v2f32],
        CCAssignToReg<[R2, R3, R4, R5, R6, R7, R8, R9,
                       R10, R11, R12, R13, R14, R15, R16, R17]>>,
  CCAssignToStack<8, 8>
]>;

// CSA 64-bit C Calling convention for varargs - all memory
def CC_Reg_VarArg_CSA : CallingConv<[
  CCAssignToStack<8, 8>
]>;

// LICs used for parameters and return
// P64_0 to P64_129 dedicated for this

// CSA 64-bit C return-value convention using LICs
def RetCC_LIC_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v2f32], CCAssignToReg<[P64_0, P64_1]>>
]>;

// CSA 64-bit C Calling convention using LICs
def CC_LIC_CSA : CallingConv<[
  CCIfType<[i1,i8,i16,i32,i64,f16,f32,f64,v2f32],
        CCAssignToReg<[P64_2, P64_3, P64_4, P64_5, P64_6, P64_7, P64_8, P64_9,
		         P64_10, P64_11, P64_12, P64_13, P64_14, P64_15, P64_16, P64_17, P64_18, P64_19, 
					   P64_20, P64_21, P64_22, P64_23, P64_24, P64_25, P64_26, P64_27, P64_28, P64_29,
					   P64_30, P64_31, P64_32, P64_33, P64_34, P64_35, P64_36, P64_37, P64_38, P64_39,
					   P64_40, P64_41, P64_42, P64_43, P64_44, P64_45, P64_46, P64_47, P64_48, P64_49,
					   P64_50, P64_51, P64_52, P64_53, P64_54, P64_55, P64_56, P64_57, P64_58, P64_59,
					   P64_60, P64_61, P64_62, P64_63, P64_64, P64_65, P64_66, P64_67, P64_68, P64_69,
					   P64_70, P64_71, P64_72, P64_73, P64_74, P64_75, P64_76, P64_77, P64_78, P64_79,
					   P64_80, P64_81, P64_82, P64_83, P64_84, P64_85, P64_86, P64_87, P64_88, P64_89,
					   P64_90, P64_91, P64_92, P64_93, P64_94, P64_95, P64_96, P64_97, P64_98, P64_99,
					   P64_100, P64_101, P64_102, P64_103, P64_104, P64_105, P64_106, P64_107, P64_108, P64_109,
					   P64_110, P64_111, P64_112, P64_113, P64_114, P64_115, P64_116, P64_117, P64_118, P64_119,
					   P64_120, P64_121, P64_122, P64_123, P64_124, P64_125, P64_126, P64_127, P64_128, P64_129
					   ]>>
]>;

