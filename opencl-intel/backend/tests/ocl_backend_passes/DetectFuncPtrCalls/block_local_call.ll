; RUN: opt < %s -analyze -detectfuncptrcall  | FileCheck %s

%struct.__block_descriptor = type { i64, i64 }
%struct.__block_literal_generic = type { i8*, i32, i32, i8*, %struct.__block_descriptor* }

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__global_scope_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8

define void @global_scope(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %globalBlock = alloca i32 (i32)*, align 8
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to i32 (i32)*), i32 (i32)** %globalBlock, align 8
  %0 = load i32 (i32)*, i32 (i32)** %globalBlock, align 8
  %block.literal = bitcast i32 (i32)* %0 to %struct.__block_literal_generic*
  %1 = getelementptr inbounds %struct.__block_literal_generic, %struct.__block_literal_generic* %block.literal, i32 0, i32 3
  %2 = bitcast %struct.__block_literal_generic* %block.literal to i8*
  %3 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8
  %4 = load i32, i32 addrspace(1)* %3, align 4
  %5 = load i8*, i8** %1
  %6 = bitcast i8* %5 to i32 (i8*, i32)*

; CHECK: Printing analysis 'Detect Function Pointer Calls'
; CHECK: DetectFuncPtrCalls: Found function pointer calls.
; CHECK-NEXT: DetectFuncPtrCalls: Functions with unresolved pointer calls:
; CHECK-NEXT: global_scope
  %call = call i32 %6(i8* %2, i32 %4)
  %7 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8
  store i32 %call, i32 addrspace(1)* %7, align 4
  ret void
}

define internal i32 @__global_scope_block_invoke(i8* %.block_descriptor, i32 %num) nounwind {
entry:
  %num.addr = alloca i32, align 4
  store i32 %num, i32* %num.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*
  ret i32 1
}
