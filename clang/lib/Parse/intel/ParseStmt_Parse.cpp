
case (tok::annot_pragma_ivdep):
  Res = HandlePragmaIvdep();
  return (Res);
case (tok::annot_pragma_novector):
  Res = HandlePragmaNoVector();
  return (Res);
case (tok::annot_pragma_vector):
  Res = HandlePragmaVector();
  return (Res);
case (tok::annot_pragma_distribute_point):
  Res = HandlePragmaDistribute();
  return (Res);
case (tok::annot_pragma_inline):
  Res = HandlePragmaInline();
  return (Res);
case (tok::annot_pragma_loop_count):
  Res = HandlePragmaLoopCount();
  return (Res);
case (tok::annot_pragma_optimize):
  //Res = HandlePragmaOptimize();
  HandlePragmaOptimize();
  //return (Res);
  return (StmtEmpty());
case (tok::annot_pragma_optimization_level):
  HandlePragmaOptimizationLevel();
  return (StmtError());
case (tok::annot_pragma_noparallel):
  Res = HandlePragmaNoParallel();
  return (Res);
case (tok::annot_pragma_parallel):
  Res = HandlePragmaParallel();
  return (Res);
case (tok::annot_pragma_unroll):
  Res = HandlePragmaUnroll();
  return (Res);
case (tok::annot_pragma_unroll_and_jam):
  Res = HandlePragmaUnrollAndJam();
  return (Res);
case (tok::annot_pragma_nofusion):
  Res = HandlePragmaNoFusion();
  return (Res);
case (tok::annot_pragma_optimization_parameter):
  HandlePragmaOptimizationParameter();
  return (StmtError());
case (tok::annot_pragma_alloc_section):
  Res = HandlePragmaAllocSection();
  return (Res);
case (tok::annot_pragma_section):
  Res = HandlePragmaSection();
  return (Res);
case (tok::annot_pragma_alloc_text):
  Res = HandlePragmaAllocText();
  return (Res);
case (tok::annot_pragma_auto_inline):
  HandlePragmaAutoInline();
  //Res = HandlePragmaAutoInline();
  //return (Res);
  return (StmtEmpty());
case (tok::annot_pragma_seg):
  Res = HandlePragmaSeg();
  return (Res);
case (tok::annot_pragma_check_stack):
  HandlePragmaCheckStack();
  //Res = HandlePragmaCheckStack();
  //return (Res);
  return (StmtEmpty());
case (tok::annot_pragma_init_seg):
  Res = HandlePragmaInitSeg();
  return (Res);
case (tok::annot_pragma_float_control):
  //Res = HandlePragmaFloatControl();
  //return (Res);
  HandlePragmaFloatControl();
  return (StmtEmpty());
case tok::annot_pragma_intel_fp_contract:
  //Res = HandlePragmaCommonOnOff(Sema::IntelPragmaFPContract, false);
  //return (Res);
  HandlePragmaCommonOnOff(Sema::IntelPragmaFPContract, false);
  return (StmtEmpty());
case (tok::annot_pragma_fenv_access):
  //Res = HandlePragmaCommonOnOff(Sema::IntelPragmaFEnvAccess, false);
  //return (Res);
  HandlePragmaCommonOnOff(Sema::IntelPragmaFEnvAccess, false);
  return (StmtEmpty());
