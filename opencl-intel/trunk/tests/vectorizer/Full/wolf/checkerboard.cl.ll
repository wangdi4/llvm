; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\checkerboard.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_checkerboard2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_checkerboard2D_parameters = appending global [85 x i8] c"float4 __attribute__((address_space(1))) *, float2 const, float4 const, float4 const\00", section "llvm.metadata" ; <[85 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <2 x float>, <4 x float>, <4 x float>)* @checkerboard2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_checkerboard2D_locals to i8*), i8* getelementptr inbounds ([85 x i8]* @opencl_checkerboard2D_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @evaluatePixel(<2 x float> %outCrd, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %checkerSize.addr = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %color1.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %color2.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %checkerLocation = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=2]
  %f2_2 = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  %modLocation = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=3]
  %f0_0 = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  %f1_1 = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  %setColor1 = alloca i32, align 4                ; <i32*> [#uses=2]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store <2 x float> %checkerSize, <2 x float>* %checkerSize.addr
  store <4 x float> %color1, <4 x float>* %color1.addr
  store <4 x float> %color2, <4 x float>* %color2.addr
  %tmp = load <2 x float>* %outCrd.addr           ; <<2 x float>> [#uses=1]
  %tmp1 = load <2 x float>* %checkerSize.addr     ; <<2 x float>> [#uses=3]
  %cmp = fcmp oeq <2 x float> zeroinitializer, %tmp1 ; <<2 x i1>> [#uses=1]
  %sel = select <2 x i1> %cmp, <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float> %tmp1 ; <<2 x float>> [#uses=0]
  %div = fdiv <2 x float> %tmp, %tmp1             ; <<2 x float>> [#uses=1]
  %call = call <2 x float> @_Z5floorU8__vector2f(<2 x float> %div) ; <<2 x float>> [#uses=1]
  store <2 x float> %call, <2 x float>* %checkerLocation
  store <2 x float> <float 2.000000e+000, float 2.000000e+000>, <2 x float>* %f2_2
  %tmp4 = load <2 x float>* %checkerLocation      ; <<2 x float>> [#uses=1]
  %tmp5 = load <2 x float>* %f2_2                 ; <<2 x float>> [#uses=1]
  %call6 = call <2 x float> @_Z4fmodU8__vector2fS_(<2 x float> %tmp4, <2 x float> %tmp5) ; <<2 x float>> [#uses=1]
  store <2 x float> %call6, <2 x float>* %modLocation
  store <2 x float> zeroinitializer, <2 x float>* %f0_0
  store <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float>* %f1_1
  %tmp10 = load <2 x float>* %modLocation         ; <<2 x float>> [#uses=1]
  %tmp11 = load <2 x float>* %f0_0                ; <<2 x float>> [#uses=1]
  %call12 = call <2 x i32> @_Z7isequalU8__vector2fS_(<2 x float> %tmp10, <2 x float> %tmp11) ; <<2 x i32>> [#uses=1]
  %call13 = call i32 @_Z3allU8__vector2i(<2 x i32> %call12) ; <i32> [#uses=1]
  %tobool = icmp ne i32 %call13, 0                ; <i1> [#uses=1]
  br i1 %tobool, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %tmp14 = load <2 x float>* %modLocation         ; <<2 x float>> [#uses=1]
  %tmp15 = load <2 x float>* %f1_1                ; <<2 x float>> [#uses=1]
  %call16 = call <2 x i32> @_Z7isequalU8__vector2fS_(<2 x float> %tmp14, <2 x float> %tmp15) ; <<2 x i32>> [#uses=1]
  %call17 = call i32 @_Z3allU8__vector2i(<2 x i32> %call16) ; <i32> [#uses=1]
  %tobool18 = icmp ne i32 %call17, 0              ; <i1> [#uses=1]
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %entry
  %0 = phi i1 [ true, %entry ], [ %tobool18, %lor.rhs ] ; <i1> [#uses=1]
  %lor.ext = zext i1 %0 to i32                    ; <i32> [#uses=1]
  store i32 %lor.ext, i32* %setColor1
  %tmp20 = load i32* %setColor1                   ; <i32> [#uses=1]
  %tobool21 = icmp ne i32 %tmp20, 0               ; <i1> [#uses=1]
  br i1 %tobool21, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.end
  %tmp22 = load <4 x float>* %color1.addr         ; <<4 x float>> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %lor.end
  %tmp23 = load <4 x float>* %color2.addr         ; <<4 x float>> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi <4 x float> [ %tmp22, %cond.true ], [ %tmp23, %cond.false ] ; <<4 x float>> [#uses=1]
  store <4 x float> %cond, <4 x float>* %dst
  %tmp24 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp24, <4 x float>* %retval
  %1 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %1
}

declare <2 x float> @_Z5floorU8__vector2f(<2 x float>)

declare <2 x float> @_Z4fmodU8__vector2fS_(<2 x float>, <2 x float>)

declare i32 @_Z3allU8__vector2i(<2 x i32>)

declare <2 x i32> @_Z7isequalU8__vector2fS_(<2 x float>, <2 x float>)

; CHECK: ret
define void @checkerboard2D(<4 x float> addrspace(1)* %output, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
entry:
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %checkerSize.addr = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %color1.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %color2.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %gid0_col = alloca i32, align 4                 ; <i32*> [#uses=3]
  %gid1_row = alloca i32, align 4                 ; <i32*> [#uses=3]
  %imgWidth = alloca i32, align 4                 ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store <2 x float> %checkerSize, <2 x float>* %checkerSize.addr
  store <4 x float> %color1, <4 x float>* %color1.addr
  store <4 x float> %color2, <4 x float>* %color2.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_col
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid1_row
  %call2 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call2, i32* %imgWidth
  %tmp = load i32* %gid1_row                      ; <i32> [#uses=1]
  %tmp3 = load i32* %imgWidth                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp3                      ; <i32> [#uses=1]
  %tmp4 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp4                  ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp6 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp6 to float               ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv, i32 0 ; <<2 x float>> [#uses=1]
  %tmp7 = load i32* %gid1_row                     ; <i32> [#uses=1]
  %conv8 = sitofp i32 %tmp7 to float              ; <float> [#uses=1]
  %vecinit9 = insertelement <2 x float> %vecinit, float %conv8, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit9, <2 x float>* %curCrd
  %tmp10 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp11 = load <2 x float>* %checkerSize.addr    ; <<2 x float>> [#uses=1]
  %tmp12 = load <4 x float>* %color1.addr         ; <<4 x float>> [#uses=1]
  %tmp13 = load <4 x float>* %color2.addr         ; <<4 x float>> [#uses=1]
  %call14 = call <4 x float> @evaluatePixel(<2 x float> %tmp10, <2 x float> %tmp11, <4 x float> %tmp12, <4 x float> %tmp13) ; <<4 x float>> [#uses=1]
  %tmp15 = load i32* %index                       ; <i32> [#uses=1]
  %tmp16 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp16, i32 %tmp15 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call14, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)
