#!/bin/bash

# Copyright (C) 2022 Intel Corporation
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

# Usage: ./show_impl_status.sh opencl_lib_dir function_name

# check params
[[ $# -ne 2 ]] && echo "Usage: ./show_impl_status.sh opencl_lib_dir function_name" && exit 1;

lib_dir=$1
[[ ! -f $lib_dir/clbltfnshared.rtl ]] && echo "clbltfnshared.rtl not found under $lib_dir" && exit 1;

func_name=$2

llvm_nm=`which llvm-nm`
[[ $? -ne 0 ]] && echo "Please make sure llvm-nm is added to the PATH!" && exit 1;

llvm_nm_args='--defined-only --demangle'

grep_pattern="${func_name}(.*)"

collect_impls() {
  echo "$2 ($lib_dir/$1):"
  $llvm_nm $lib_dir/$1 $llvm_nm_args | grep -wo $grep_pattern | sort -V | sed 's/^/  /g'
}

collect_impls "clbltfnshared.rtl" "shared"
collect_impls "x64/clbltfnh8.rtl" "sse"
collect_impls "x64/clbltfne9.rtl" "avx"
collect_impls "x64/clbltfnl9.rtl" "avx2"
collect_impls "x64/clbltfnz0.rtl" "avx512"
collect_impls "x64/clbltfnz1.rtl" "amx"
