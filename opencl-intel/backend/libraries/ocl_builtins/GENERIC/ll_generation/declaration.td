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

// Basic tablegen declaration classes for .ll builtin generation

include "GENERIC/ll_generation/type_value.td"

class LLDeclare<string name, list<Type> param_types, Type return_type, string mangle_suffix = ""> {
  string Name = name;
  list<Type> ParamTypes = param_types;
  Type RetType = return_type;

  bit EmitMangledName = false;
  string MangledName = "_Z" # !size(name) # name # mangle_suffix;

  bit HasDefine = false;

  list<string> FuncAttrs = [];
}
