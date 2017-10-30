//===--- IntelDeclNodes.td - Intel Decl Nodes -----------------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

def CilkSpawn : Decl {
  code OpenGuard = [{#if INTEL_SPECIFIC_CILKPLUS}];
  code CloseGuard = [{#endif // INTEL_SPECIFIC_CILKPLUS}];
}
def Pragma : Decl {
  code OpenGuard = [{#if INTEL_CUSTOMIZATION}];
  code CloseGuard = [{#endif // INTEL_CUSTOMIZATION}];
}
