//===--- ExecuteCompilerInvocation.cpp ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file holds ExecuteCompilerInvocation(). It is split into its own file to
// minimize the impact of pulling in essentially everything else in Clang.
//
//===----------------------------------------------------------------------===//

#include "clang/FrontendTool/Utils.h"
#include "clang/ARCMigrate/ARCMTActions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/Utils.h"
#include "clang/Rewrite/Frontend/FrontendActions.h"
#include "clang/StaticAnalyzer/Frontend/FrontendActions.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#ifdef INTEL_SPECIFIC_IL0_BACKEND
#include "llvm/Support/raw_ostream.h"

class HelpPragmaPrinter {
  public:
    static void Print(llvm::raw_ostream &OS) {
      OS << "PRAGMA                      ARGUMENTS\n";
      OS << "============================================================\n";
      OS << "align                       = native|natural|packed|power|mac68k|reset\n";
      OS << "alloc_section               ( var, \"section-name\" )\n";
      OS << "alloc_text                  ( \"section-name\", func1[, func2, ...] )\n";
      OS << "auto_inline                 ( on|off )\n";
      OS << "bss_seg                     ( [[{push|pop},] [identifier,]] [\"segment-name\" [,\"segment-class\"] )\n";
      OS << "check_stack                 ([{on|off}] | {+|-})\n";
      OS << "clang __debug               assert|crash|llvm_fatal_error|llvm_unreachable|overflow_stack|handle_crash\n";
      OS << "clang arc_cf_code_audited   begin|end\n";
      OS << "code_seg                    ( [[{push|pop},] [identifier,]] [\"segment-name\" [,\"segment-class\"] )\n";
      OS << "comment                     message\n";
      OS << "component                   <none>\n";
      OS << "conform                     (forScope [, show|push|pop] [, identifier ] [, on|off])\n";
      OS << "const_seg                   ( [[{push|pop},] [identifier,]] [\"segment-name\" [,\"segment-class\"] )\n";
      OS << "data_seg                    ( [[{push|pop},] [identifier,]] [\"segment-name\" [,\"segment-class\"] )\n";
      OS << "deprecated                  <none>\n";
      OS << "distribute_point            <none>\n";
      OS << "endregion                   <none>\n";
      OS << "fenv_access                 on|off\n";
      OS << "float_control               (push|pop)|(precise|except, on|off [,push])\n";
      OS << "forceinline                 [recursive]\n";
      OS << "fp_contract                 ( on|off )\n";
      OS << "GCC|clang dependency        \"filename\"\n";
      OS << "GCC|clang diagnostic        warning|error|ignored|fatal|pop|push \"diagnostic option\"\n";
      OS << "[GCC|clang] poison|POISON   id1 [id2 ...]\n";
      OS << "GCC|clang system_header     <none>\n";
      OS << "GCC visibility              push ( default|hidden|internal|protected ) | pop\n";
      OS << "include_alias               ( {<|\"} oldfilename {\"|>} , {<|\"} newfilename {\"|>} )\n";
      OS << "include_directory           directory_name\n";
      OS << "init_seg                    ( {compiler | lib | user | \"sect-name\"[,\"func-name\"]})\n";
      OS << "inline                      [recursive]\n";
      OS << "ident                       \"id\"\n";
      OS << "ivdep                       [loop]\n";
      OS << "loop_count                  [min|max|avg](n1)[,[min|max|avg](n2)]...\n";
      OS << "mark                        <none>\n";
      OS << "message                     ( \"message\" )\n";
      OS << "ms_struct                   on|off|reset\n";
      OS << "nofusion                    <none>\n";
      OS << "noinline                    <none>\n";
      OS << "noparallel                  <none>\n";
      OS << "nounroll                    <none>\n";
      OS << "nounroll_and_jam            <none>\n";
      OS << "novector                    <none>\n";
      OS << "omp parallel                [for|sections| [clause[[,] clause]...]]\n";
      OS << "once                        <none>\n";
      OS << "optimization_level          0|1|2|3\n";
      OS << "optimization_parameter      <args>\n";
      OS << "optimize                    ( "", {on | off} )\n";
      OS << "options align               = native|natural|packed|power|mac68k|reset\n";
      OS << "pack                        ( n|show|{push|pop[,identifier][,n]} )\n";
      OS << "parallel                    [clause [clause]...]\n";
      OS << "push_macro                  ( \"macroname\" )\n";
      OS << "pop_macro                   ( \"macroname\" )\n";
      OS << "redefine_extname            id1 id2\n";
      OS << "region                      <none>\n";
      OS << "section                     ( \"sect-name\", attribute-list )\n";
      OS << "start_map_region            (<string-literal>)\n";
      OS << "STDC CX_LIMITED_RANGE       ON|OFF|DEFAULT\n";
      OS << "STDC FENV_ACCESS            ON|OFF|DEFAULT\n";
      OS << "STDC|OPENCL FP_CONTRACT     ON|OFF|DEFAULT\n";
      OS << "stop_map_region             <none>\n";
      OS << "OPENCL EXTENSION            id, enable|disable\n";
      OS << "unroll                      [(n|expr)]\n";
      OS << "unroll_and_jam              [ ( n ) ]\n";
      OS << "unused                      ( id1 [, id2 ...] )\n";
      OS << "vector                      always[none]|aligned[none]|unaligned[none]|no_mask_readwrite[none]|mask_readwrite[none]|nontemporal [ ( var [, var]... ) ]\n";
      OS << "vtordisp                    ( on|off )\n";
      OS << "weak                        id1 [= id2]\n";
      OS << "\n";
      OS << "                 Note: \"clause\" can be following cases depend on different pragmas\n";
      OS << "                          if (scalar-expresssion)\n";
      OS << "                          num_threads (integer-expr)\n";
      OS << "                          default (shared|none)\n";
      OS << "                          copyin ([var [,var]...])\n";
      OS << "                          shared ([var [,var]...])\n";
      OS << "                          private ([var [,var]...])\n";
      OS << "                          firstprivate ([var [,var]...])\n";
      OS << "                          lastprivate ([var [,var]...]\n";
      OS << "                          reduction (operator : [var [,var]...])\n";
      OS << "                          ordered|nowait\n";
    }
};
#endif  // INTEL_SPECIFIC_IL0_BACKEND

using namespace clang;
using namespace llvm::opt;

static FrontendAction *CreateFrontendBaseAction(CompilerInstance &CI) {
  using namespace clang::frontend;
  StringRef Action("unknown");
  (void)Action;

  switch (CI.getFrontendOpts().ProgramAction) {
  case ASTDeclList:            return new ASTDeclListAction();
  case ASTDump:                return new ASTDumpAction();
  case ASTPrint:               return new ASTPrintAction();
  case ASTView:                return new ASTViewAction();
  case DumpRawTokens:          return new DumpRawTokensAction();
  case DumpTokens:             return new DumpTokensAction();
  case EmitAssembly:           return new EmitAssemblyAction();
  case EmitBC:                 return new EmitBCAction();
  case EmitHTML:               return new HTMLPrintAction();
  case EmitLLVM:               return new EmitLLVMAction();
  case EmitLLVMOnly:           return new EmitLLVMOnlyAction();
  case EmitCodeGenOnly:        return new EmitCodeGenOnlyAction();
  case EmitObj:                return new EmitObjAction();
  case FixIt:                  return new FixItAction();
  case GenerateModule:         return new GenerateModuleAction;
  case GeneratePCH:            return new GeneratePCHAction;
  case GeneratePTH:            return new GeneratePTHAction();
  case InitOnly:               return new InitOnlyAction();
  case ParseSyntaxOnly:        return new SyntaxOnlyAction();
  case ModuleFileInfo:         return new DumpModuleInfoAction();
  case VerifyPCH:              return new VerifyPCHAction();

  case PluginAction: {
    for (FrontendPluginRegistry::iterator it =
           FrontendPluginRegistry::begin(), ie = FrontendPluginRegistry::end();
         it != ie; ++it) {
      if (it->getName() == CI.getFrontendOpts().ActionName) {
        std::unique_ptr<PluginASTAction> P(it->instantiate());
        if (!P->ParseArgs(CI, CI.getFrontendOpts().PluginArgs))
          return nullptr;
        return P.release();
      }
    }

    CI.getDiagnostics().Report(diag::err_fe_invalid_plugin_name)
      << CI.getFrontendOpts().ActionName;
    return nullptr;
  }

  case PrintDeclContext:       return new DeclContextPrintAction();
  case PrintPreamble:          return new PrintPreambleAction();
  case PrintPreprocessedInput: {
    if (CI.getPreprocessorOutputOpts().RewriteIncludes)
      return new RewriteIncludesAction();
    return new PrintPreprocessedAction();
  }

  case RewriteMacros:          return new RewriteMacrosAction();
  case RewriteTest:            return new RewriteTestAction();
#ifdef CLANG_ENABLE_OBJC_REWRITER
  case RewriteObjC:            return new RewriteObjCAction();
#else
  case RewriteObjC:            Action = "RewriteObjC"; break;
#endif
#ifdef CLANG_ENABLE_ARCMT
  case MigrateSource:          return new arcmt::MigrateSourceAction();
#else
  case MigrateSource:          Action = "MigrateSource"; break;
#endif
#ifdef CLANG_ENABLE_STATIC_ANALYZER
  case RunAnalysis:            return new ento::AnalysisAction();
#else
  case RunAnalysis:            Action = "RunAnalysis"; break;
#endif
  case RunPreprocessorOnly:    return new PreprocessOnlyAction();
  }

#if !defined(CLANG_ENABLE_ARCMT) || !defined(CLANG_ENABLE_STATIC_ANALYZER) \
  || !defined(CLANG_ENABLE_OBJC_REWRITER)
  CI.getDiagnostics().Report(diag::err_fe_action_not_available) << Action;
  return 0;
#else
  llvm_unreachable("Invalid program action!");
#endif
}

static FrontendAction *CreateFrontendAction(CompilerInstance &CI) {
  // Create the underlying action.
  FrontendAction *Act = CreateFrontendBaseAction(CI);
  if (!Act)
    return nullptr;

  const FrontendOptions &FEOpts = CI.getFrontendOpts();

  if (FEOpts.FixAndRecompile) {
    Act = new FixItRecompile(Act);
  }
  
#ifdef CLANG_ENABLE_ARCMT
  if (CI.getFrontendOpts().ProgramAction != frontend::MigrateSource &&
      CI.getFrontendOpts().ProgramAction != frontend::GeneratePCH) {
    // Potentially wrap the base FE action in an ARC Migrate Tool action.
    switch (FEOpts.ARCMTAction) {
    case FrontendOptions::ARCMT_None:
      break;
    case FrontendOptions::ARCMT_Check:
      Act = new arcmt::CheckAction(Act);
      break;
    case FrontendOptions::ARCMT_Modify:
      Act = new arcmt::ModifyAction(Act);
      break;
    case FrontendOptions::ARCMT_Migrate:
      Act = new arcmt::MigrateAction(Act,
                                     FEOpts.MTMigrateDir,
                                     FEOpts.ARCMTMigrateReportOut,
                                     FEOpts.ARCMTMigrateEmitARCErrors);
      break;
    }

    if (FEOpts.ObjCMTAction != FrontendOptions::ObjCMT_None) {
      Act = new arcmt::ObjCMigrateAction(Act, FEOpts.MTMigrateDir,
                                         FEOpts.ObjCMTAction);
    }
  }
#endif

  // If there are any AST files to merge, create a frontend action
  // adaptor to perform the merge.
  if (!FEOpts.ASTMergeFiles.empty())
    Act = new ASTMergeAction(Act, FEOpts.ASTMergeFiles);

  return Act;
}

bool clang::ExecuteCompilerInvocation(CompilerInstance *Clang) {
  // Honor -help.
  if (Clang->getFrontendOpts().ShowHelp) {
    std::unique_ptr<OptTable> Opts(driver::createDriverOptTable());
    Opts->PrintHelp(llvm::outs(), "clang -cc1",
                    "LLVM 'Clang' Compiler: http://clang.llvm.org",
                    /*Include=*/ driver::options::CC1Option, /*Exclude=*/ 0);
    return true;
  }

  // Honor -version.
  //
  // FIXME: Use a better -version message?
  if (Clang->getFrontendOpts().ShowVersion) {
    llvm::cl::PrintVersionMessage();
    return true;
  }
#ifdef INTEL_SPECIFIC_IL0_BACKEND
  if (Clang->getFrontendOpts().HelpPragma) {
    HelpPragmaPrinter::Print(llvm::outs());
    return 0;
  }
#endif  // INTEL_SPECIFIC_IL0_BACKEND
  // Load any requested plugins.
  for (unsigned i = 0,
         e = Clang->getFrontendOpts().Plugins.size(); i != e; ++i) {
    const std::string &Path = Clang->getFrontendOpts().Plugins[i];
    std::string Error;
    if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(Path.c_str(), &Error))
      Clang->getDiagnostics().Report(diag::err_fe_unable_to_load_plugin)
        << Path << Error;
  }

  // Honor -mllvm.
  //
  // FIXME: Remove this, one day.
  // This should happen AFTER plugins have been loaded!
  if (!Clang->getFrontendOpts().LLVMArgs.empty()) {
    unsigned NumArgs = Clang->getFrontendOpts().LLVMArgs.size();
    auto Args = llvm::make_unique<const char*[]>(NumArgs + 2);
    Args[0] = "clang (LLVM option parsing)";
    for (unsigned i = 0; i != NumArgs; ++i)
      Args[i + 1] = Clang->getFrontendOpts().LLVMArgs[i].c_str();
    Args[NumArgs + 1] = nullptr;
    llvm::cl::ParseCommandLineOptions(NumArgs + 1, Args.get());
  }

#ifdef CLANG_ENABLE_STATIC_ANALYZER
  // Honor -analyzer-checker-help.
  // This should happen AFTER plugins have been loaded!
  if (Clang->getAnalyzerOpts()->ShowCheckerHelp) {
    ento::printCheckerHelp(llvm::outs(), Clang->getFrontendOpts().Plugins);
    return true;
  }
#endif

  // If there were errors in processing arguments, don't do anything else.
  if (Clang->getDiagnostics().hasErrorOccurred())
    return false;
  // Create and execute the frontend action.
  std::unique_ptr<FrontendAction> Act(CreateFrontendAction(*Clang));
  if (!Act)
    return false;
  bool Success = Clang->ExecuteAction(*Act);
  if (Clang->getFrontendOpts().DisableFree)
    BuryPointer(std::move(Act));
  return Success;
}
