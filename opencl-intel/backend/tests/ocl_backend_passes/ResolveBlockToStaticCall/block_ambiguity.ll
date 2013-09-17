; Negative scenario for OCL2.0
; block call can NOT be resolved at compile time
; RUN: opt -resolve-block-call -S < %s | FileCheck %s

;kernel void global_scope(__global int* res)
;{
;int (^globalBlock)(int);
;
;int (^globalBlock1)(int) = ^int(int num)
;{
;   return 1;
;};
;int (^globalBlock2)(int) = ^int(int num)
;{
;   return 2;
;};
;
;globalBlock = *res ? globalBlock1 : globalBlock2;
;  
;*res = globalBlock(*res);
;}


%struct.__block_descriptor = type { i64, i64 }
%struct.__block_literal_generic = type { i8*, i32, i32, i8*, %struct.__block_descriptor* }

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__global_scope_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8
@__block_descriptor_tmp1 = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global2 = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__global_scope_block_invoke_2 to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp1 to %struct.__block_descriptor*) }, align 8

define void @global_scope(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %globalBlock = alloca i32 (i32)*, align 8
  %globalBlock1 = alloca i32 (i32)*, align 8
  %globalBlock2 = alloca i32 (i32)*, align 8
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to i32 (i32)*), i32 (i32)** %globalBlock1, align 8
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global2 to i32 (i32)*), i32 (i32)** %globalBlock2, align 8
  %0 = load i32 addrspace(1)** %res.addr, align 8
  %1 = load i32 addrspace(1)* %0, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %2 = load i32 (i32)** %globalBlock1, align 8
  br label %cond.end

cond.false:                                       ; preds = %entry
  %3 = load i32 (i32)** %globalBlock2, align 8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 (i32)* [ %2, %cond.true ], [ %3, %cond.false ]
  store i32 (i32)* %cond, i32 (i32)** %globalBlock, align 8
  %4 = load i32 (i32)** %globalBlock, align 8
  %block.literal = bitcast i32 (i32)* %4 to %struct.__block_literal_generic*
  %5 = getelementptr inbounds %struct.__block_literal_generic* %block.literal, i32 0, i32 3
  %6 = bitcast %struct.__block_literal_generic* %block.literal to i8*
  %7 = load i32 addrspace(1)** %res.addr, align 8
  %8 = load i32 addrspace(1)* %7, align 4
  %9 = load i8** %5
  %10 = bitcast i8* %9 to i32 (i8*, i32)*
; check call is left indirect
; CHECK: call i32 %
  %call = call i32 %10(i8* %6, i32 %8)
  %11 = load i32 addrspace(1)** %res.addr, align 8
  store i32 %call, i32 addrspace(1)* %11, align 4
  ret void
}

define internal i32 @__global_scope_block_invoke(i8* %.block_descriptor, i32 %num) nounwind {
  ret i32 1
}

define internal i32 @__global_scope_block_invoke_2(i8* %.block_descriptor, i32 %num) nounwind {
  ret i32 2
}

