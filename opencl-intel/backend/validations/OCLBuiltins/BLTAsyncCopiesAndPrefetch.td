// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

def event_t : OclType<"event_t", "event_t", 1, 0, 0>;
def pevent_t : OclType<"pevent_t", "event_t", 1, 0, 1>;

// OpenCL 1.2 rev 15, Table 6.18
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
             v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def async_work_group_copy_g2l   : REF_OclBuiltin<"async_work_group_copy",   (outs event_t:$ret), (ins pgentype:$dst, pgentype:$src, size_t:$num_gentypes, event_t:$event), [], [], TemplateAttr< [type0,vec0] > >, LOCAL;
def async_work_group_copy_l2g   : REF_OclBuiltin<"async_work_group_copy",   (outs event_t:$ret), (ins pgentype:$dst, pgentype:$src, size_t:$num_gentypes, event_t:$event), [], [], TemplateAttr< [type0,vec0] > >, GLOBAL;
def async_work_group_strided_copy_g2l   : REF_OclBuiltin<"async_work_group_strided_copy",   (outs event_t:$ret), (ins pgentype:$dst, pgentype:$src, size_t:$num_gentypes, size_t:$src_stride, event_t:$event), [], [], TemplateAttr< [type0, vec0]> >, LOCAL;
def async_work_group_strided_copy_l2g   : REF_OclBuiltin<"async_work_group_strided_copy",   (outs event_t:$ret), (ins pgentype:$dst, pgentype:$src, size_t:$num_gentypes, size_t:$src_stride, event_t:$event), [], [], TemplateAttr< [type0, vec0]> >, GLOBAL;
def prefetch  : REF_OclBuiltin<"prefetch",  (outs), (ins pgentype:$p, size_t:$num_gentypes), [], [], TemplateAttr< [] > >, CONST_GLOBAL;
}

// OpenCL 1.2 rev 15, Table 6.18
let Types = [ v1i32 ],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def wait_group_events  : REF_OclBuiltin<"wait_group_events",  (outs), (ins si32gentype:$num_events, pevent_t:$event_list), [], [], TemplateAttr< [] > >, PRIVATE;
}

class Generic {
  code Prolog = [{
/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTAsyncCopiesAndPrefetch.cpp

\*****************************************************************************/

#include "BLTAsyncCopiesAndPrefetch.h"

using namespace llvm;
using std::string;
using std::vector;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif

namespace Validation {
namespace OCLBuiltins {
llvm::GenericValue lle_X_prefetch(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args)
{
    return GenericValue();
}

llvm::GenericValue lle_X_wait_group_events(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args)
{
    llvm::GenericValue arg0 = Args[0];
    llvm::GenericValue arg1 = Args[1];
    size_t num_events = getVal<size_t>(arg0);
    size_t* event_list = static_cast<size_t*>(arg1.PointerVal);
    for (size_t i = 0; i < num_events; ++i, ++event_list)
    {
        *event_list = 0;
    }
    return GenericValue();
}
}
}
extern "C" {
BUILTINS_API void initOCLBuiltinsAsync() {return;}
  }];

  code Epilog = [{
}

  }];

}
  def inc : Generic;
