//===---------------- IntelVPlanSyncDependenceAnalysis.cpp ----------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// This file instantiates SyncDependenceAnalysis for VPlan's vpo::VPValue class
/// hierarchy. We cannot do that in the original community implementation
/// because that would spoil Analysis library with VPlan symbols.
///
// ===--------------------------------------------------------------------=== //
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"
#define SDA_VPLAN_INSTANCE
#include "../../../Analysis/SyncDependenceAnalysis.cpp"
