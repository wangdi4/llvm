  if (OnlyStatement && Res.isUsable() && isa<PragmaStmt>(Res.get())) {
    if (!Attrs.empty())
      Res = Actions.ProcessStmtAttributes(Res.get(), Attrs.getList(), Attrs.Range);
    StmtResult PragmaStmtRes = Res;
    Res = ParseStatement(TrailingElseLoc);
    if (Res.isInvalid())
      return StmtError();
    if (PragmaStmtRes.isInvalid())
      return Res;
    StmtVector Stmts;
    Stmts.push_back(PragmaStmtRes.get());
    Stmts.push_back(Res.get());
    #include "ParseStmt_CheckStmt.cpp"
    if (cast<PragmaStmt>(Stmts[0])->isNullOp())
      return Stmts[1];
    return Actions.ActOnCompoundStmt(Stmts[0]->getLocStart(), Stmts[1]->getLocEnd(),
                                     Stmts, false);
  }
