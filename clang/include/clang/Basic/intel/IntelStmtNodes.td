//===--- IntelStmtNodes.td - Intel Stmt Nodes -----------------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

def CilkSyncStmt : Stmt {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CilkForGrainsizeStmt : Stmt {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CilkForStmt : Stmt {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def SIMDForStmt : Stmt {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CilkSpawnExpr : DStmt<Expr> {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CilkRankedStmt : Stmt {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CEANIndexExpr : DStmt<Expr> {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def CEANBuiltinExpr : DStmt<Expr> {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def PragmaStmt : Stmt {
  code OpenGuard = [{#if INTEL_CUSTOMIZATION}];
  code CloseGuard = [{#endif // INTEL_CUSTOMIZATION}];
}
