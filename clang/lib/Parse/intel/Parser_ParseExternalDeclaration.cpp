
  case tok::annot_pragma_ivdep:
    HandlePragmaIvdepDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_novector:
    HandlePragmaNoVectorDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_vector:
    HandlePragmaVectorDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_distribute_point:
    HandlePragmaDistributeDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_inline:
    HandlePragmaInlineDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_loop_count:
    HandlePragmaLoopCountDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_optimize:
    HandlePragmaOptimizeDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_optimization_level:
    HandlePragmaOptimizationLevelDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_noparallel:
    HandlePragmaNoParallelDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_parallel:
    HandlePragmaParallelDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_unroll:
    HandlePragmaUnrollDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_unroll_and_jam:
    HandlePragmaUnrollAndJamDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_nofusion:
    HandlePragmaNoFusionDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_optimization_parameter:
    HandlePragmaOptimizationParameterDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_alloc_section:
    HandlePragmaAllocSectionDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_section:
    HandlePragmaSectionDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_alloc_text:
    HandlePragmaAllocTextDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_auto_inline:
    HandlePragmaAutoInlineDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_seg:
    HandlePragmaSegDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_check_stack:
    HandlePragmaCheckStackDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_init_seg:
    HandlePragmaInitSegDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_float_control:
    HandlePragmaFloatControlDecl();
    return DeclGroupPtrTy();
  case tok::annot_pragma_intel_fp_contract:
    HandlePragmaCommonOnOffDecl(Sema::IntelPragmaFPContract, false);
    return DeclGroupPtrTy();
  case (tok::annot_pragma_fenv_access):
    HandlePragmaCommonOnOffDecl(Sema::IntelPragmaFEnvAccess, false);
    return DeclGroupPtrTy();
