; regression test that no crash happens in computing blockLiteral size
; RUN: opt -cloneblockinvokefunctokernel <  %s -S

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.__block_descriptor = type { i64, i64 }

; Function Attrs: nounwind
define internal void @__enqueue_simple_block_block_invoke(i8* %.block_descriptor) {
entry:
  %.block_descriptor.addr = alloca i8*, align 8
  %block.addr = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>*, align 8
  store i8* %.block_descriptor, i8** %.block_descriptor.addr, align 8
  %0 = load i8** %.block_descriptor.addr
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>*
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>* %block, <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>** %block.addr, align 8
  %block.capture.addr = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>* %block, i32 0, i32 5
  %1 = load i64* %block.capture.addr, align 8
  %block.capture.addr1 = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>* %block, i32 0, i32 7
  %2 = load i32* %block.capture.addr1, align 4
  %block.capture.addr2 = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>* %block, i32 0, i32 6
  %3 = load i32 addrspace(1)** %block.capture.addr2, align 8
  ret void
}
