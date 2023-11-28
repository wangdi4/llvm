//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   Volcano OpenCL Runtime related code. Currently, there is no way to get
//   OpenCL Runtime information from VPlan. This file holds the static
//   information that we need. Using this file, we can also keep OpenCL-related
//   code in Xmain, reducing code divergence between OpenCL and Xmain compilers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVOLCANOOPENCL_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVOLCANOOPENCL_H

namespace llvm {

// Scalar select functions in Volcano OpenCL
const char *volcanoScalarSelect[] = {
    "_Z6selectccc", "_Z6selectcch", "_Z6selecthhc",
    "_Z6selecthhh", "_Z6selectsss", "_Z6selectsst",
    "_Z6selecttts", "_Z6selectttt", "_Z6selectiii",
    "_Z6selectiij", "_Z6selectjji", "_Z6selectjjj",
    "_Z6selectlll", "_Z6selectllm", "_Z6selectmml",
    "_Z6selectmmm", "_Z6selectffi", "_Z6selectffj",
    "_Z6selectddl", "_Z6selectddm"};

} // end llvm namespace

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVOLCANOOPENCL_H
