//===--- IntelAttr.td - Intel attribute definitions -----------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

include "clang/Basic/intel/IntelAttrDocs.td"

class SourceLocArgument<string name, bit opt = 0> : Argument<name, opt>;
class CheckedExprArgument<string name> : Argument<name, 1>;
class VariadicIntArgument<string name> : Argument<name, 1>;

def OMPDirectiveFlag : StmtAttr {
  let Spellings = []; // No spellings, implicit only.
  let Args = [StringArgument<"Value">];
  let Documentation = [InternalOnly];
}

def AvoidFalseShare : InheritableAttr {
  let Spellings = [Declspec<"avoid_false_share">];
  let Args = [StringArgument<"ExceptionId">];
  let HasCustomParsing = 1;
  let Documentation = [Undocumented];
  int Requires = 4;
}

def Allocate : InheritableAttr {
  let Spellings = [GNU<"allocate">];
  let Args = [StringArgument<"Name">];
  let HasCustomParsing = 1;
  let Documentation = [Undocumented];
  int Requires = 4;
}

def GCCStruct : InheritableAttr {
  let Spellings = [GNU<"gcc_struct">];
  let Subjects = SubjectList<[Record]>;
  let HasCustomParsing = 1;
  let Documentation = [Undocumented];
  int Requires = 4;
}

def BNDLegacy : InheritableAttr {
  let Spellings = [GNU<"bnd_legacy">];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  int Requires = 4;
}

def BNDVarSize : InheritableAttr {
  let Spellings = [GNU<"bnd_variable_size">];
  let Subjects = SubjectList<[Field], WarnDiag,
                             "ExpectedFieldOrGlobalVar">;
  let Documentation = [Undocumented];
  int Requires = 4;
}

def IntelInline : Attr {
  let Spellings = [Pragma<"", "inline">, Pragma<"", "forceinline">,
                   Pragma<"", "noinline">];
  let Args =  [EnumArgument<"Option", "OptionType",
                           ["recursive", "<unspecified>"],
                           ["Recursive", "NotRecursive"]>];
  let AdditionalMembers = [{
  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    OS << getDiagnosticName(Policy);
  }
  std::string getDiagnosticName(const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    if (SpellingIndex == Pragma_inline)
      return "#pragma inline";
    else if (SpellingIndex == Pragma_forceinline)
      return "#pragma forceinline";
    else
      return "#pragma noinline";
  }
  }];
  let Documentation = [Undocumented];
}

// Pragma block_loop and noblock_loop
def IntelBlockLoop : Attr {
  let Spellings = [Pragma<"", "block_loop">,
                   Pragma<"", "noblock_loop">];
  let Args = [
    VariadicExprArgument<"Factors">, VariadicIntArgument<"Levels">,
    VariadicExprArgument<"Privates">
  ];
  let AdditionalMembers = [{
  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    OS << getDiagnosticName(Policy);
  }
  std::string getDiagnosticName(const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    if (SpellingIndex == Pragma_block_loop)
      return "#pragma block_loop";
    else
      return "#pragma noblock_loop";
  }
  }];
  let Documentation = [Undocumented];
}

// Pragma prefetch and noprefetch
def IntelPrefetch : Attr {
  let Spellings = [Pragma<"", "prefetch">,
                   Pragma<"", "noprefetch">];
  let Args = [
    VariadicExprArgument<"PrefetchExprs">
  ];
  let AdditionalMembers = [{
  void printPrettyPragma(raw_ostream &OS, const PrintingPolicy &Policy) const {
    OS << getDiagnosticName(Policy);
  }
  std::string getDiagnosticName(const PrintingPolicy &Policy) const {
    unsigned SpellingIndex = getSpellingListIndex();
    if (SpellingIndex == Pragma_prefetch)
      return "#pragma prefetch";
    else
      return "#pragma noprefetch";
  }
  }];
  let Documentation = [Undocumented];
}


// FPGA-specific attributes

def OpenCLBlocking : Attr {
  let Spellings = [GNU<"blocking">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [OpenCLBlockingDocs];
  let LangOpts = [OpenCL];
}

def OpenCLDepth : Attr {
  let Spellings = [GNU<"depth">];
  let Args = [UnsignedArgument<"Depth">];
  let Subjects = SubjectList<[ParmVar, GlobalVar]>;
  let Documentation = [OpenCLDepthDocs];
  let LangOpts = [OpenCL];
}

def OpenCLIO : Attr {
  let Spellings = [GNU<"io">];
  let Args = [StringArgument<"IOName">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [OpenCLIODocs];
  let LangOpts = [OpenCL];
}

def OpenCLLocalMemSize : Attr {
  let Spellings = [GNU<"local_mem_size">];
  let Args = [UnsignedArgument<"LocalMemSize">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
// FIXME: Add documentation
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let LangOpts = [OpenCL, HLS];
  let AdditionalMembers = [{
    static int getMinValue() {
      return 2;
    }
    static int getMaxValue() {
      return 128*1024;
    }
  }];

}

def OpenCLBufferLocation : Attr {
  let Spellings = [GNU<"buffer_location">];
  let Args = [StringArgument<"BufferLocation">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let Documentation = [OpenCLBufferLocationDocs];
  let LangOpts = [OpenCL];
}

def NumComputeUnits : InheritableAttr {
  let Spellings = [GNU<"num_compute_units">];
  let LangOpts = [OpenCL];
  let Args = [IntArgument<"XDim">, DefaultIntArgument<"YDim", 1>,
              DefaultIntArgument<"ZDim", 1>];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [NumComputeUnitsDocs];
}

def Autorun : InheritableAttr {
  let Spellings = [GNU<"autorun">];
  let LangOpts = [OpenCL];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [AutorunDocs];
}

def UsesGlobalWorkOffset : InheritableAttr {
  let Spellings = [GNU<"uses_global_work_offset">];
  let Args = [BoolArgument<"Enabled">];
  let LangOpts = [OpenCL];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [UsesGlobalWorkOffsetDocs];
}

def OpenCLHostAccessible : Attr {
  let Spellings = [GNU<"intel_host_accessible">];
  let Subjects = SubjectList<[Var]>;
  let Documentation = [OpenCLHostAccessibleDocs];
  let LangOpts = [OpenCL];
}

// End of FPGA-specific attributes

def VecLenHint : InheritableAttr {
  let Spellings = [GNU<"intel_vec_len_hint">];
  let Args = [UnsignedArgument<"VecLen">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [VecLenHintDocs];
}

// HLS function attributes
def Cluster : InheritableAttr {
  let Spellings = [Clang<"cluster">];
  let Args = [StringArgument<"Name" , 1> , BoolArgument<"HasName" , 1>];
  let LangOpts = [OpenCL, HLS];
  let Subjects = SubjectList<[Function]>;
  let PragmaAttributeSupport = 0;
  let Documentation = [Undocumented];
}

def Component : InheritableAttr {
  let Spellings = [GNU<"ihc_component">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
}

def ComponentInterface : InheritableAttr {
  let Spellings = [GNU<"component_interface">];
  // If adding to the end of enum, modify generateValidStrings as well.
  let Args = [EnumArgument<"Type", "ComponentInterfaceType",
                           ["avalon_streaming", "avalon_mm_agent",
                            "always_run"],
                           ["Streaming", "Agent", "AlwaysRun"]>];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static void generateValidStrings(SmallString<256> &Str) {
      auto Last = AlwaysRun;
      for (int I = 0; I <= Last; ++I) {
        Str += ConvertComponentInterfaceTypeToStr(
                static_cast<ComponentInterfaceType>(I));
        if (I != Last) Str += " ";
      }
    }
  }];
}

def HLSForceLoopPipelining : InheritableAttr {
  let Spellings = [GNU<"hls_force_loop_pipelining">];
  let Args = [StringArgument<"ForceLoopPipelining">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
}

def HLSII : InheritableAttr {
  let Spellings = [GNU<"hls_ii">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 1;
    }
    static unsigned getMaxValue() {
      return INT_MAX;
    }
  }];
}

def HLSMaxII : InheritableAttr {
  let Spellings = [GNU<"hls_max_ii">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 1;
    }
    static unsigned getMaxValue() {
      return INT_MAX;
    }
  }];
}

def HLSMinII : InheritableAttr {
  let Spellings = [GNU<"hls_min_ii">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 1;
    }
    static unsigned getMaxValue() {
      return INT_MAX;
    }
  }];
}

def HLSMaxInvocationDelay : InheritableAttr {
  let Spellings = [GNU<"hls_max_invocation_delay">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function]>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 0;
    }
    static unsigned getMaxValue() {
      return INT_MAX;
    }
  }];
}

def HLSLocalNonConstVar : SubsetSubject<Var,
                              [{S->hasLocalStorage() &&
                                S->getKind() != Decl::ImplicitParam &&
                                S->getKind() != Decl::ParmVar &&
                                S->getKind() != Decl::NonTypeTemplateParm &&
                                !S->getType().isConstQualified()}],
                              "local non-const variables">;

def HLSConstVar : SubsetSubject<Var,
                      [{S->getKind() != Decl::ImplicitParam &&
                        S->getKind() != Decl::ParmVar &&
                        S->getKind() != Decl::NonTypeTemplateParm &&
                        (S->getType().isConstQualified() ||
                         S->getType().getAddressSpace() ==
                            LangAS::opencl_constant)}],
                      "constant variables">;

def MaxConcurrency : InheritableAttr {
  let Spellings = [GNU<"max_concurrency">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [OpenCL, HLS];
  let Subjects = SubjectList<[Function, HLSLocalNonConstVar], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 0;
    }
    static unsigned getMaxValue() {
      return 1024*1024;
    }
  }];
}

def MaxInterleaving : InheritableAttr {
  let Spellings = [GNU<"max_interleaving">];
  let Args = [ExprArgument<"Value">];
  let LangOpts = [OpenCL, HLS];
  let Subjects = SubjectList<[Function, HLSLocalNonConstVar], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 0;
    }
    static unsigned getMaxValue() {
      return 1024*1024;
    }
  }];
}

def StallLatency : InheritableAttr {
  let Spellings = [GNU<"stall_latency_enabled">, GNU<"stall_latency_disabled">];
  let LangOpts = [OpenCL, HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let PragmaAttributeSupport = 0;
  let Documentation = [Undocumented];
  let AdditionalMembers = [{
    bool isEnabled() const {
      return getSemanticSpelling() == GNU_stall_latency_enabled;
    }
  }];
}

def StallFreeReturn : InheritableAttr {
  let Spellings = [GNU<"stall_free_return">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
}

def UseSingleClock : InheritableAttr {
  let Spellings = [GNU<"hls_component_use_single_clock">];
  let LangOpts = [HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
}

// HLS parameter attributes
def ArgumentInterface : Attr {
  let Spellings = [GNU<"argument_interface">];
  // If adding to the end of enum, modify generateValidStrings as well.
  let Args = [EnumArgument<"Type", "ArgumentInterfaceType",
                           ["wire","avalon_mm_agent", "avalon_streaming"],
                           ["Wire", "Agent", "Streaming"]>];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let LangOpts = [HLS];
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let AdditionalMembers = [{
    static void generateValidStrings(SmallString<256> &Str) {
      auto Last = Streaming;
      for (int I = 0; I <= Last; ++I) {
        Str += ConvertArgumentInterfaceTypeToStr(
                static_cast<ArgumentInterfaceType>(I));
        if (I != Last) Str += " ";
      }
    }
  }];
}

def StableArgument : Attr {
  let Spellings = [GNU<"stable_argument">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let LangOpts = [HLS];
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let SimpleHandler = 1;
}

def AgentMemoryArgument : Attr {
  let Spellings = [GNU<"agent_memory_argument">];
  let Subjects = SubjectList<[ParmVar], ErrorDiag>;
  let LangOpts = [HLS];
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
  let SimpleHandler = 1;
}

// HLS local or static variable attributes
def HLSLocalOrStaticVar : SubsetSubject<Var,
                              [{S->getKind() != Decl::ImplicitParam &&
                                S->getKind() != Decl::ParmVar &&
                                S->getKind() != Decl::NonTypeTemplateParm &&
                                (S->getStorageClass() == StorageClass::SC_Static ||
                                 S->hasLocalStorage())}],
                              "local variables, static variables">;

def HLSLocalStaticAgentMemVar : SubsetSubject<Var,
      [{S->getKind() != Decl::ImplicitParam &&
        S->getKind() != Decl::NonTypeTemplateParm &&
        (S->getStorageClass() == StorageClass::SC_Static ||
         S->hasLocalStorage())}],
         "local variables, static variables, agent memory arguments">;

def StallFree : InheritableAttr {
  let Spellings = [GNU<"stall_free">];
  let LangOpts = [OpenCL, HLS];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [Undocumented];
  let PragmaAttributeSupport = 0;
}

// One string argument
def ReadWriteMode : Attr {
  let Spellings = [GNU<"readwrite_mode">];
  let Args = [StringArgument<"Type">];
  let Subjects = SubjectList<[HLSLocalStaticAgentMemVar, Field], ErrorDiag>;
  let LangOpts = [HLS];
  let Documentation = [ReadWriteModeDocs];
}

def HLSLocalStaticVar : SubsetSubject<Var,
                              [{S->getStorageClass() == StorageClass::SC_Static &&
                                S->getKind() != Decl::ImplicitParam &&
                                S->getKind() != Decl::ParmVar &&
                                S->getKind() != Decl::NonTypeTemplateParm}],
                              "local static variables">;

// One integral argument, static only.
def StaticArrayReset : Attr {
  let Spellings = [GNU<"static_array_reset">];
  let Args = [ExprArgument<"Value">];
  let Subjects = SubjectList<[HLSConstVar, HLSLocalStaticVar,
                              Field], ErrorDiag>;
  let LangOpts = [HLS];
  let Documentation = [Undocumented];
  let AdditionalMembers = [{
    static unsigned getMinValue() {
      return 0;
    }
    static unsigned getMaxValue() {
      return 1;
    }
  }];
}
def : MutualExclusions<[SYCLIntelRegister, StaticArrayReset]>;

// IntrinsicPromotion implementation.
// This is an AST only attribute that should never appear in customer code.
def TargetPromotion : InheritableAttr {
  let Spellings = [];
  let Args = [StringArgument<"Features">];
  let Documentation = [InternalOnly];
}

def IntelOclBiccAVX : DeclOrTypeAttr {
  let Spellings = [Clang<"intel_ocl_bicc_avx", 0>];
  let Documentation = [IntelOclBiccAVXDocs];
}

def IntelOclBiccAVX512 : DeclOrTypeAttr {
  let Spellings = [Clang<"intel_ocl_bicc_avx512", 0>];
  let Documentation = [IntelOclBiccAVX512Docs];
}

def AllowCpuFeatures : InheritableAttr,
                       TargetSpecificAttr<TargetAnyX86> {
  let Spellings = [Clang<"allow_cpu_features">, Declspec<"allow_cpu_features">];
  let Subjects = SubjectList<[Function], ErrorDiag>;
  let Documentation = [AllowCPUFeatureDocs];
  let Args = [ExprArgument<"Page1">, ExprArgument<"Page2", 1>];
  let AdditionalMembers = [{
    uint64_t getPage1Value() const {
      assert(page1 && "Page1 expression is not optional");
      const auto *P1CE = dyn_cast<ConstantExpr>(page1);

      if (!P1CE)
        return 0;
      return P1CE->getResultAsAPSInt().getZExtValue();
    }
    uint64_t getPage2Value() const {
      const auto *P2CE = dyn_cast_or_null<ConstantExpr>(page2);
      if (!P2CE)
        return 0;
      return P2CE->getResultAsAPSInt().getZExtValue();
    }
  }];
}
