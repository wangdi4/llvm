; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtime=dx  -runtimelib %p/../../dxruntime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; CHECK: dx_soa_load_input_uniform_indirect_4_float4_vs
;;LLVMIR start
; ModuleID = 'dx2llvm'

define fastcc void @aos_shader(i8* noalias nocapture %gc, i8* noalias nocapture %lc) nounwind {
entry:
%r = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r
%r1 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r1
%r2 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r2
%r3 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r3
%r4 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r4
%r5 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r5
%r6 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r6
%r7 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r7
%r8 = alloca <4 x float>
store <4 x float> zeroinitializer, <4 x float>* %r8
%load_call = call <4 x float> @dx_soa_load_input_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 0)
%0 = extractelement <4 x float> %load_call, i32 2
%bitcast = bitcast float %0 to i32
%1 = icmp ne i32 %bitcast, 0
br i1 %1, label %ifblock, label %else

retfrommain:                                      ; preds = %endif
ret void

endif:                                            ; preds = %endif20, %ifblock
br label %retfrommain

ifblock:                                          ; preds = %entry
br label %endif

else:                                             ; preds = %entry
%2 = load <4 x float>* %r1
%bitcastindex = bitcast <4 x float> %2 to <4 x i32>
%3 = extractelement <4 x i32> %bitcastindex, i32 2
%4 = add i32 %3, 0
%5 = and i32 31, %4
%load_call9 = call <4 x float> @dx_soa_load_input_nonuniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 %5)
%load_call10 = call <4 x float> @dx_soa_load_input_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 1)
%6 = shufflevector <4 x float> %load_call9, <4 x float> %load_call9, <4 x i32> zeroinitializer
%bitcast11 = bitcast <4 x float> %6 to <4 x i32>
%7 = shufflevector <4 x float> %load_call10, <4 x float> %load_call10, <4 x i32> <i32 2, i32 1, i32 0, i32 0>
%bitcast12 = bitcast <4 x float> %7 to <4 x i32>
%8 = icmp ne <4 x i32> %bitcast11, %bitcast12
%9 = sext <4 x i1> %8 to <4 x i32>
%10 = load <4 x float>* %r1
%bitcastindex13 = bitcast <4 x float> %10 to <4 x i32>
%11 = extractelement <4 x i32> %bitcastindex13, i32 2
%12 = add i32 %11, 0
%13 = and i32 31, %12
%load_call14 = call <4 x float> @dx_soa_load_output_nonuniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 %13)
%bitcast15 = bitcast <4 x i32> %9 to <4 x float>
%14 = shufflevector <4 x float> %load_call14, <4 x float> %bitcast15, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
%15 = load <4 x float>* %r1
%bitcastindex16 = bitcast <4 x float> %15 to <4 x i32>
%16 = extractelement <4 x i32> %bitcastindex16, i32 2
%17 = add i32 %16, 0
%18 = and i32 31, %17
call void @dx_soa_store_output_nonuniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 %18, <4 x float> %14)
%19 = load <4 x float>* %r1
%bitcastindex17 = bitcast <4 x float> %19 to <4 x i32>
%20 = extractelement <4 x i32> %bitcastindex17, i32 1
%21 = add i32 %20, 0
%22 = and i32 31, %21
%load_call18 = call <4 x float> @dx_soa_load_input_nonuniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 %22)
%23 = extractelement <4 x float> %load_call18, i32 1
%bitcast19 = bitcast float %23 to i32
%24 = icmp ne i32 %bitcast19, 0
br i1 %24, label %ifblock21, label %endif20

endif20:                                          ; preds = %else, %ifblock21
br label %endif

ifblock21:                                        ; preds = %else
%load_call22 = call <4 x float> @dx_soa_load_input_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 0)
%25 = load <4 x float>* %r5
%bitcastindex23 = bitcast <4 x float> %25 to <4 x i32>
%26 = extractelement <4 x i32> %bitcastindex23, i32 1
%27 = add i32 %26, 5
%28 = and i32 31, %27
%load_call24 = call <4 x float> @dx_soa_load_input_nonuniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 %28)
%29 = shufflevector <4 x float> %load_call22, <4 x float> %load_call22, <4 x i32> <i32 2, i32 2, i32 2, i32 2>
%bitcast25 = bitcast <4 x float> %29 to <4 x i32>
%30 = shufflevector <4 x float> %load_call24, <4 x float> %load_call24, <4 x i32> <i32 3, i32 3, i32 3, i32 3>
%bitcast26 = bitcast <4 x float> %30 to <4 x i32>
%31 = icmp slt <4 x i32> %bitcast25, %bitcast26
%32 = sext <4 x i1> %31 to <4 x i32>
%load_call27 = call <4 x float> @dx_soa_load_output_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 3)
%bitcast28 = bitcast <4 x i32> %32 to <4 x float>
%33 = shufflevector <4 x float> %load_call27, <4 x float> %bitcast28, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
call void @dx_soa_store_output_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 3, <4 x float> %33)
br label %endif20
}

declare <4 x float> @dx_soa_load_input_uniform_indirect_1_float4_vs(i8*, i32, i32)

declare <4 x float> @dx_soa_load_input_nonuniform_indirect_1_float4_vs(i8*, i32, i32)

declare <4 x float> @dx_soa_load_output_nonuniform_indirect_1_float4_vs(i8*, i32, i32)

declare void @dx_soa_store_output_nonuniform_indirect_1_float4_vs(i8*, i32, i32, <4 x float>)

declare <4 x float> @dx_soa_load_output_uniform_indirect_1_float4_vs(i8*, i32, i32)

declare void @dx_soa_store_output_uniform_indirect_1_float4_vs(i8*, i32, i32, <4 x float>)
;;LLVMIR end
