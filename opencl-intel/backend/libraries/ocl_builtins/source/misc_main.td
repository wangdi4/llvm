// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// This file is used for migration purpose only.
// 1. Invoke `ocl-tblgen -gen-ocl-biheader` to generate `BuiltinList.inc` for
// all builtin declarations under `GENERIC` and `source` folders.
// 2. Invoke `ocl-tblgen -gen-vectorization-info` to generate `VectInfo.gen`
// for all VPlan vectorization info records under `GENERIC` and `source` folders.

// TODO: remove this file when all related records are migrated into `source`.

// Include all files that contain OclBuiltin, VectInfo defs

// Migrated
include "source/main.td"

// To migrate
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/sub_group_builtin.td"
include "GENERIC/ll_intrinsics.td"
include "GENERIC/sycl-spirv.td"
include "GENERIC/intel-matrix_layout_transform.td" // INTEL
