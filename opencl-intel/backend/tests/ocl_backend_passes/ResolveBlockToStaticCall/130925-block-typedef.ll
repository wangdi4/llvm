; Regression test. Optimization pass crashed on following code.
; test that pass does not crash
; RUN: opt -resolve-block-call -S < %s


; typedef int (^block_t)(int);
; kernel void block_typedef_reassign(__global int* res)
; {
;   int tid = get_global_id(0);
;   res[tid] = -1;
;   block_t b = ^(int i) { return i + 1; };
;   int a = 0;
;   a = b(a);
;   b = ^(int i) { return i + 2; };
;   a = b(a);
;   block_t c = ^(int i) { return i + 3; };
;   b = c;
;   a = b(a);
;   res[tid] = a - 6;
; }


; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"

%struct.__block_descriptor.103 = type { i64, i64 }
%struct.__block_literal_generic.104 = type { i8*, i32, i32, i8*, %struct.__block_descriptor.103* }

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor.103* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__block_typedef_reassign_block_invoke to i8*), %struct.__block_descriptor.103* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor.103*) }, align 8
@__block_descriptor_tmp1 = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global2 = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor.103* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__block_typedef_reassign_block_invoke_2 to i8*), %struct.__block_descriptor.103* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp1 to %struct.__block_descriptor.103*) }, align 8
@__block_descriptor_tmp3 = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global4 = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor.103* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @__block_typedef_reassign_block_invoke_3 to i8*), %struct.__block_descriptor.103* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp3 to %struct.__block_descriptor.103*) }, align 8

define spir_kernel void @block_typedef_reassign(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %tid = alloca i32, align 4
  %b = alloca i32 (i32)*, align 8
  %a = alloca i32, align 4
  %c = alloca i32 (i32)*, align 8
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %idxprom = sext i32 %0 to i64
  %1 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %1, i64 %idxprom
  store i32 -1, i32 addrspace(1)* %arrayidx, align 4
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor.103* }* @__block_literal_global to i32 (i32)*), i32 (i32)** %b, align 8
  store i32 0, i32* %a, align 4
  %2 = load i32 (i32)** %b, align 8
  %block.literal = bitcast i32 (i32)* %2 to %struct.__block_literal_generic.104*
  %3 = getelementptr inbounds %struct.__block_literal_generic.104* %block.literal, i32 0, i32 3
  %4 = bitcast %struct.__block_literal_generic.104* %block.literal to i8*
  %5 = load i32* %a, align 4
  %6 = load i8** %3
  %7 = bitcast i8* %6 to i32 (i8*, i32)*
  %call1 = call i32 %7(i8* %4, i32 %5)
  store i32 %call1, i32* %a, align 4
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor.103* }* @__block_literal_global2 to i32 (i32)*), i32 (i32)** %b, align 8
  %8 = load i32 (i32)** %b, align 8
  %block.literal2 = bitcast i32 (i32)* %8 to %struct.__block_literal_generic.104*
  %9 = getelementptr inbounds %struct.__block_literal_generic.104* %block.literal2, i32 0, i32 3
  %10 = bitcast %struct.__block_literal_generic.104* %block.literal2 to i8*
  %11 = load i32* %a, align 4
  %12 = load i8** %9
  %13 = bitcast i8* %12 to i32 (i8*, i32)*
  %call3 = call i32 %13(i8* %10, i32 %11)
  store i32 %call3, i32* %a, align 4
  store i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor.103* }* @__block_literal_global4 to i32 (i32)*), i32 (i32)** %c, align 8
  %14 = load i32 (i32)** %c, align 8
  store i32 (i32)* %14, i32 (i32)** %b, align 8
  %15 = load i32 (i32)** %b, align 8
  %block.literal4 = bitcast i32 (i32)* %15 to %struct.__block_literal_generic.104*
  %16 = getelementptr inbounds %struct.__block_literal_generic.104* %block.literal4, i32 0, i32 3
  %17 = bitcast %struct.__block_literal_generic.104* %block.literal4 to i8*
  %18 = load i32* %a, align 4
  %19 = load i8** %16
  %20 = bitcast i8* %19 to i32 (i8*, i32)*
  %call5 = call i32 %20(i8* %17, i32 %18)
  store i32 %call5, i32* %a, align 4
  %21 = load i32* %a, align 4
  %sub = sub nsw i32 %21, 6
  %22 = load i32* %tid, align 4
  %idxprom6 = sext i32 %22 to i64
  %23 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx7 = getelementptr inbounds i32 addrspace(1)* %23, i64 %idxprom6
  store i32 %sub, i32 addrspace(1)* %arrayidx7, align 4
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) nounwind readnone

define internal i32 @__block_typedef_reassign_block_invoke(i8* %.block_descriptor, i32 %i) nounwind {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor.103* }>*
  %0 = load i32* %i.addr, align 4
  %add = add nsw i32 %0, 1
  ret i32 %add
}

define internal i32 @__block_typedef_reassign_block_invoke_2(i8* %.block_descriptor, i32 %i) nounwind {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor.103* }>*
  %0 = load i32* %i.addr, align 4
  %add = add nsw i32 %0, 2
  ret i32 %add
}

define internal i32 @__block_typedef_reassign_block_invoke_3(i8* %.block_descriptor, i32 %i) nounwind {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor.103* }>*
  %0 = load i32* %i.addr, align 4
  %add = add nsw i32 %0, 3
  ret i32 %add
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!9}

!0 = metadata !{void (i32 addrspace(1)*)* @block_typedef_reassign, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"res"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 2, i32 0}
!8 = metadata !{}
!9 = metadata !{metadata !"-cl-std=CL2.0"}
