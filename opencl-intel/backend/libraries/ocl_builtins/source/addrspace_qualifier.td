// Copyright (C) 2023 Intel Corporation
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


// Prototypes
let Types = [v1i8], Attrs = [INLINE], NeedForwardDecl = true in {
    def global_qualifier_builtin : OclBuiltin<"__to_global", (outs gpgentype:$ret), (ins pgentype:$ptr)>;
    def local_qualifier_builtin : OclBuiltin<"__to_local", (outs lpgentype:$ret), (ins pgentype:$ptr)>;
    def private_qualifier_builtin : OclBuiltin<"__to_private", (outs ppgentype:$ret), (ins pgentype:$ptr)>;
}

let Types = [v1i8], NeedForwardDecl = true in {
    def get_buffer_info_builtin : OclBuiltin<"__get_buffer_range_info", (outs pvoid:$ret), (ins void)>;
}

let Types = [v1i8], NeedForwardDecl = true in {
    def cast_to_global_builtin : OclBuiltin<"__to_global_addrspacecast", (outs gpgentype:$ret), (ins pgentype:$pData)>;
    def cast_to_local_builtin : OclBuiltin<"__to_local_addrspacecast", (outs lpgentype:$ret), (ins pgentype:$pData)>;
    def cast_to_private_builtin : OclBuiltin<"__to_private_addrspacecast", (outs ppgentype:$ret), (ins pgentype:$pData)>;
}

/* 
    BufferRanges's struct is as follow. (Each element's size 8 Byte)
        [LocalArgCount] [GlobalArgCount]
        [LocalBufferBaseAddr] [LocalBufferSize]
        [LocalArg1Addr ][ LocalArg1Size ] [LocalArg2Addr ][ LocalArg2Size ]...
        [GlobalArg1Addr][ GlobalArg1Size] [GlobalArg2Addr][ GlobalArg2Size]...
*/

defvar to_global_code = [{
    // get buffer's addrspace info
    void* infoPtr = __get_buffer_range_info();
    size_t* pInfo = (size_t*)infoPtr;
    
    // get input ptr's addr
    void* prt = (void*)$Arg0VarName;
    size_t ptrAddr = (size_t)prt;
    
    // get local arg's count and global arg's count
    size_t localcount = *(pInfo ++);
    size_t globalcount = *(pInfo ++);

    // get global arg's addrspace info begin addr
    pInfo = pInfo + localcount * 2;

    for(size_t idx = 0; idx < globalcount; idx++){
        size_t start = pInfo[idx * 2];
        size_t end = pInfo[idx * 2 + 1] + start;
        if(ptrAddr >= start && ptrAddr < end)
            return __to_global_addrspacecast($Arg0VarName);
    }
    return NULL;
}];

defvar to_local_code = [{
    // get buffer's addrspace info
    void* infoPtr = __get_buffer_range_info();
    size_t* pInfo = (size_t*)infoPtr;
    
    // get input ptr's addr
    void* prt = (void*)$Arg0VarName;
    size_t ptrAddr = (size_t)prt;
    
    // get local arg's count
    size_t localcount = *pInfo;

    // get local arg's addrspace info begin addr
    pInfo = pInfo + 2;

    for(size_t idx = 0; idx < localcount; idx++){
        size_t start = pInfo[idx * 2];
        size_t end = pInfo[idx * 2 + 1] + start;
        if(ptrAddr >= start && ptrAddr < end){
            return __to_local_addrspacecast($Arg0VarName);
        }
    }
    return NULL;
}];

defvar to_private_code = [{
    // get buffer's addrspace info
    void* infoPtr = __get_buffer_range_info();
    size_t* pInfo = (size_t*)infoPtr;
    
    // get input ptr's addr
    void* prt = (void*)$Arg0VarName;
    size_t ptrAddr = (size_t)prt;
    
    // get local arg's count and global arg's count
    size_t localcount = *(pInfo ++);
    size_t globalcount = *(pInfo ++);

    for(size_t idx = 0; idx < localcount; idx++){
        size_t start = pInfo[idx * 2];
        size_t end = pInfo[idx * 2 + 1] + start;
        if(ptrAddr >= start && ptrAddr < end){
            return NULL;
        }
    }

    // get global arg's addrspace info begin addr
    pInfo = pInfo + localcount * 2;

    for(size_t idx = 0; idx < globalcount; idx++){
        size_t start = pInfo[idx * 2];
        size_t end = pInfo[idx * 2 + 1] + start;
        if(ptrAddr >= start && ptrAddr < end){
            return NULL;
        }
    }
    return __to_private_addrspacecast($Arg0VarName);
}];

// Implementations

def : OclBuiltinImpl<cast_to_global_builtin, cast_to_global_builtin.Types, /*IsDeclOnly*/true, [{}]>, SHARED;
def : OclBuiltinImpl<cast_to_local_builtin, cast_to_local_builtin.Types, /*IsDeclOnly*/true, [{}]>, SHARED;
def : OclBuiltinImpl<cast_to_private_builtin, cast_to_private_builtin.Types, /*IsDeclOnly*/true, [{}]>, SHARED;

def : OclBuiltinImpl<get_buffer_info_builtin, get_buffer_info_builtin.Types, /*IsDeclOnly*/true, [{}]>, SHARED;

def : OclBuiltinImpl<global_qualifier_builtin, global_qualifier_builtin.Types, false, to_global_code>, SHARED;
def : OclBuiltinImpl<local_qualifier_builtin, local_qualifier_builtin.Types, false, to_local_code>, SHARED;
def : OclBuiltinImpl<private_qualifier_builtin, private_qualifier_builtin.Types, false, to_private_code>, SHARED;
