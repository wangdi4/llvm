#ifdef INTEL_SPECIFIC_IL0_BACKEND

#include "clang/Parse/Parser.h"
#include "RAIIObjectsForParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/Attributes.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/PrettyStackTrace.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/LoopHint.h"
#include "clang/Sema/PrettyDeclStackTrace.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/TypoCorrection.h"
#include "llvm/ADT/SmallString.h"
#include "clang/AST/StmtCXX.h"
using namespace clang;

void Parser::CheckIntelStmt(StmtVector& Stmts){
  if (getLangOpts().IntelCompat) {
    // Analisys for #pragma ivdep, distribute_point before empty statements (i.e. goto, break, continue, declspec)
    SmallVector<SourceLocation, 4> locs;
    SmallVector<unsigned, 4> messages;
    PragmaStmt *stmt = NULL;
    Stmt *lastStmt = Stmts.back()->stripLabelLikeStatements();
    if (isa<BreakStmt>(lastStmt) || isa<GotoStmt>(lastStmt) 
      || isa<IndirectGotoStmt>(lastStmt) || isa<ContinueStmt>(lastStmt)
      || isa<DeclStmt>(lastStmt)|| isa<CXXCatchStmt>(lastStmt)) {
      size_t Size = Stmts.size();
      for (size_t cnt = 2; cnt <= Size; ++cnt) {
        if (isa<PragmaStmt>(Stmts[Size - cnt])) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]);
        }
        else if (isa<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements())) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements());
        }
        else {
          //cnt -= offset;
          //offset = 1;
          //stmt = NULL;
          break;
        }
        switch(stmt->getPragmaKind()) {
          case (IntelPragmaIvdep):
          case (IntelPragmaNoVector):
          case (IntelPragmaVector):
          case (IntelPragmaNoParallel):
          case (IntelPragmaParallel):
            messages.push_back(diag::x_warn_intel_pragma_wrong_place);
            break;
          case (IntelPragmaDistribute):
          case (IntelPragmaInline):
          case (IntelPragmaLoopCount):
          case (IntelPragmaUnroll):
          case (IntelPragmaUnrollAndJam):
          case (IntelPragmaNoFusion):
            messages.push_back(diag::x_error_intel_pragma_wrong_place);
            break;
          default:
            break;
        }
        switch(stmt->getPragmaKind()) {
          case (IntelPragmaOptimize):
          case (IntelPragmaOptimizationLevel):
          case (IntelPragmaInlineEnd):
          case (IntelPragmaAllocSection):
          case (IntelPragmaSection):
          case (IntelPragmaAllocText):
          case (IntelPragmaAutoInline):
          case (IntelPragmaBCCDSeg):
          case (IntelPragmaCheckStack):
          case (IntelPragmaInitSeg):
          case (IntelPragmaFloatControl):
            break;
          default:
            if (!stmt->isNullOp()) {
              locs.push_back(stmt->getSemiLoc());
              Actions.DeletePragmaOnError(stmt);
            }
            break;
        }
      }
    }
    if (!isa<PragmaStmt>(lastStmt)) {
      bool PragmaFound = false;
      size_t Size = Stmts.size();
      for (size_t cnt = 2; cnt <= Size; ++cnt) {
        if (isa<PragmaStmt>(Stmts[Size - cnt])) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]);
        }
        else if (isa<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements())) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements());
        }
        else {
          break;
        }
        StmtResult Res;
        switch(stmt->getPragmaKind()) {
          case (IntelPragmaInline):
            Res = Actions.ActOnPragmaOptionsEndInline(SourceLocation());
            PragmaFound = true;
            break;
          default:
            break;
        }
        if (Res.isUsable()) {
          Stmts.push_back(Res.get());
        }
      }
      if (PragmaFound) {
        // Insert terminator for closing pragmas
        Stmts.push_back(Actions.ActOnNullStmt(lastStmt->getLocStart()).get());
      }
    }
    else {
      size_t Size = Stmts.size();
      for (size_t cnt = 1; cnt <= Size; ++cnt) {
        if (isa<PragmaStmt>(Stmts[Size - cnt])) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]);
        }
        else if (isa<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements())) {
          stmt = cast<PragmaStmt>(Stmts[Size - cnt]->stripLabelLikeStatements());
        }
        else {
          break;
        }
      }
    }
    for (size_t cnt = locs.size(); cnt > 0; --cnt)
    {
      Diag(locs[cnt - 1], messages[cnt - 1]) << locs[cnt - 1];
    }
  }
}
#endif // INTEL_SPECIFIC_IL0_BACKEND
