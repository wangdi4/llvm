; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlHistogram.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_histogramScalar_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramScalar_parameters = appending global [103 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[103 x i8]*> [#uses=1]
@opencl_histogramGrouped_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramGrouped_parameters = appending global [173 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[173 x i8]*> [#uses=1]
@opencl_histogramStep1_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep1_parameters = appending global [117 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const, size_t const\00", section "llvm.metadata" ; <[117 x i8]*> [#uses=1]
@opencl_histogramStep2int_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep2int_parameters = appending global [125 x i8] c"uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[125 x i8]*> [#uses=1]
@opencl_histogramStep2int2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep2int2_parameters = appending global [127 x i8] c"uint2 __attribute__((address_space(1))) *, uint2 __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[127 x i8]*> [#uses=1]
@opencl_histogramStep2int4_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep2int4_parameters = appending global [127 x i8] c"uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[127 x i8]*> [#uses=1]
@opencl_histogramStep2int8_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep2int8_parameters = appending global [127 x i8] c"uint8 __attribute__((address_space(1))) *, uint8 __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[127 x i8]*> [#uses=1]
@opencl_histogramStep2int16_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogramStep2int16_parameters = appending global [129 x i8] c"uint16 __attribute__((address_space(1))) *, uint16 __attribute__((address_space(1))) *, size_t const, size_t const, size_t const\00", section "llvm.metadata" ; <[129 x i8]*> [#uses=1]
@opencl_metadata = appending global [8 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @histogramScalar to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramScalar_locals to i8*), i8* getelementptr inbounds ([103 x i8]* @opencl_histogramScalar_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, i32)* @histogramGrouped to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramGrouped_locals to i8*), i8* getelementptr inbounds ([173 x i8]* @opencl_histogramGrouped_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @histogramStep1 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep1_locals to i8*), i8* getelementptr inbounds ([117 x i8]* @opencl_histogramStep1_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, i32)* @histogramStep2int to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep2int_locals to i8*), i8* getelementptr inbounds ([125 x i8]* @opencl_histogramStep2int_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*, i32, i32, i32)* @histogramStep2int2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep2int2_locals to i8*), i8* getelementptr inbounds ([127 x i8]* @opencl_histogramStep2int2_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, i32, i32, i32)* @histogramStep2int4 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep2int4_locals to i8*), i8* getelementptr inbounds ([127 x i8]* @opencl_histogramStep2int4_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<8 x i32> addrspace(1)*, <8 x i32> addrspace(1)*, i32, i32, i32)* @histogramStep2int8 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep2int8_locals to i8*), i8* getelementptr inbounds ([127 x i8]* @opencl_histogramStep2int8_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<16 x i32> addrspace(1)*, <16 x i32> addrspace(1)*, i32, i32, i32)* @histogramStep2int16 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogramStep2int16_locals to i8*), i8* getelementptr inbounds ([129 x i8]* @opencl_histogramStep2int16_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[8 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @histogramScalar(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiOutputArray, i32 %szMatrix) nounwind {
entry:
  %puiInputMatrix.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szMatrix.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  store i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)** %puiInputMatrix.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szMatrix, i32* %szMatrix.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %cmp = icmp ne i32 0, %call                     ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end

if.end:                                           ; preds = %entry
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %tmp1 = load i32* %szMatrix.addr                ; <i32> [#uses=1]
  %cmp2 = icmp ult i32 %tmp, %tmp1                ; <i1> [#uses=1]
  br i1 %cmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %tmp4 = load i32 addrspace(1)** %puiInputMatrix.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp4, i32 %tmp3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp5 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  %tmp6 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx7 = getelementptr inbounds i32 addrspace(1)* %tmp6, i32 %tmp5 ; <i32 addrspace(1)*> [#uses=2]
  %tmp8 = load i32 addrspace(1)* %arrayidx7       ; <i32> [#uses=1]
  %inc = add i32 %tmp8, 1                         ; <i32> [#uses=1]
  store i32 %inc, i32 addrspace(1)* %arrayidx7
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp9 = load i32* %i                            ; <i32> [#uses=1]
  %inc10 = add i32 %tmp9, 1                       ; <i32> [#uses=1]
  store i32 %inc10, i32* %i
  br label %for.cond

for.end:                                          ; preds = %if.then, %for.cond
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @histogramGrouped(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)* %puiGroupOutputArray, i32 %szBin, i32 %szBinsPerItem, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiInputMatrix.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiTmpArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %puiGroupOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=7]
  %szBinsPerItem.addr = alloca i32, align 4       ; <i32*> [#uses=3]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=4]
  %lid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %shiftedTmpArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=4]
  %outerLastIndex = alloca i32, align 4           ; <i32*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %innerLastIndex = alloca i32, align 4           ; <i32*> [#uses=2]
  %shiftedOutTmpArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %shiftedGroupOutputArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i58 = alloca i32, align 4                      ; <i32*> [#uses=6]
  store i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)** %puiInputMatrix.addr
  store i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)** %puiTmpArray.addr
  store i32 addrspace(1)* %puiGroupOutputArray, i32 addrspace(1)** %puiGroupOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsPerItem, i32* %szBinsPerItem.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %call1 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %lid
  %tmp = load i32 addrspace(1)** %puiTmpArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %tmp, i32 %mul ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedTmpArray
  %tmp5 = load i32* %gid                          ; <i32> [#uses=1]
  %add = add i32 %tmp5, 1                         ; <i32> [#uses=1]
  %tmp6 = load i32* %szBinsPerItem.addr           ; <i32> [#uses=1]
  %mul7 = mul i32 %add, %tmp6                     ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %mul7, %tmp8                    ; <i32> [#uses=1]
  store i32 %mul9, i32* %outerLastIndex
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szBin.addr                  ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  %tmp14 = load i32* %szBinsPerItem.addr          ; <i32> [#uses=1]
  %mul15 = mul i32 %mul13, %tmp14                 ; <i32> [#uses=1]
  store i32 %mul15, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp16 = load i32* %i                           ; <i32> [#uses=1]
  %tmp17 = load i32* %outerLastIndex              ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp16, %tmp17              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp18 = load i32* %i                           ; <i32> [#uses=1]
  %tmp19 = load i32 addrspace(1)** %puiInputMatrix.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp19, i32 %tmp18 ; <i32 addrspace(1)*> [#uses=1]
  %tmp20 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp21 = load i32 addrspace(1)** %shiftedTmpArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds i32 addrspace(1)* %tmp21, i32 %tmp20 ; <i32 addrspace(1)*> [#uses=2]
  %tmp23 = load i32 addrspace(1)* %arrayidx22     ; <i32> [#uses=1]
  %inc = add i32 %tmp23, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32 addrspace(1)* %arrayidx22
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp24 = load i32* %i                           ; <i32> [#uses=1]
  %inc25 = add i32 %tmp24, 1                      ; <i32> [#uses=1]
  store i32 %inc25, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call26 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call26, i32* %outerLastIndex
  %tmp28 = load i32* %lid                         ; <i32> [#uses=1]
  %add29 = add i32 %tmp28, 1                      ; <i32> [#uses=1]
  %tmp30 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul31 = mul i32 %add29, %tmp30                 ; <i32> [#uses=1]
  store i32 %mul31, i32* %innerLastIndex
  %tmp32 = load i32 addrspace(1)** %puiTmpArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %call33 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  %call34 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  %mul35 = mul i32 %call33, %call34               ; <i32> [#uses=1]
  %tmp36 = load i32* %szBin.addr                  ; <i32> [#uses=1]
  %mul37 = mul i32 %mul35, %tmp36                 ; <i32> [#uses=1]
  %add.ptr38 = getelementptr inbounds i32 addrspace(1)* %tmp32, i32 %mul37 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr38, i32 addrspace(1)** %shiftedTmpArray
  store i32 addrspace(1)* null, i32 addrspace(1)** %shiftedOutTmpArray
  %tmp41 = load i32 addrspace(1)** %puiGroupOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %call42 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  %tmp43 = load i32* %szBin.addr                  ; <i32> [#uses=1]
  %mul44 = mul i32 %call42, %tmp43                ; <i32> [#uses=1]
  %add.ptr45 = getelementptr inbounds i32 addrspace(1)* %tmp41, i32 %mul44 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr45, i32 addrspace(1)** %shiftedGroupOutputArray
  store i32 0, i32* %t
  br label %for.cond47

for.cond47:                                       ; preds = %for.inc80, %for.end
  %tmp48 = load i32* %t                           ; <i32> [#uses=1]
  %tmp49 = load i32* %outerLastIndex              ; <i32> [#uses=1]
  %cmp50 = icmp ult i32 %tmp48, %tmp49            ; <i1> [#uses=1]
  br i1 %cmp50, label %for.body51, label %for.end83

for.body51:                                       ; preds = %for.cond47
  %tmp52 = load i32 addrspace(1)** %shiftedTmpArray ; <i32 addrspace(1)*> [#uses=1]
  %tmp53 = load i32* %t                           ; <i32> [#uses=1]
  %tmp54 = load i32* %szBin.addr                  ; <i32> [#uses=1]
  %mul55 = mul i32 %tmp53, %tmp54                 ; <i32> [#uses=1]
  %add.ptr56 = getelementptr inbounds i32 addrspace(1)* %tmp52, i32 %mul55 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr56, i32 addrspace(1)** %shiftedOutTmpArray
  %tmp59 = load i32* %lid                         ; <i32> [#uses=1]
  %tmp60 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul61 = mul i32 %tmp59, %tmp60                 ; <i32> [#uses=1]
  store i32 %mul61, i32* %i58
  br label %for.cond62

for.cond62:                                       ; preds = %for.inc76, %for.body51
  %tmp63 = load i32* %i58                         ; <i32> [#uses=1]
  %tmp64 = load i32* %innerLastIndex              ; <i32> [#uses=1]
  %cmp65 = icmp ult i32 %tmp63, %tmp64            ; <i1> [#uses=1]
  br i1 %cmp65, label %for.body66, label %for.end79

for.body66:                                       ; preds = %for.cond62
  %tmp67 = load i32* %i58                         ; <i32> [#uses=1]
  %tmp68 = load i32 addrspace(1)** %shiftedOutTmpArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx69 = getelementptr inbounds i32 addrspace(1)* %tmp68, i32 %tmp67 ; <i32 addrspace(1)*> [#uses=1]
  %tmp70 = load i32 addrspace(1)* %arrayidx69     ; <i32> [#uses=1]
  %tmp71 = load i32* %i58                         ; <i32> [#uses=1]
  %tmp72 = load i32 addrspace(1)** %shiftedGroupOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds i32 addrspace(1)* %tmp72, i32 %tmp71 ; <i32 addrspace(1)*> [#uses=2]
  %tmp74 = load i32 addrspace(1)* %arrayidx73     ; <i32> [#uses=1]
  %add75 = add i32 %tmp74, %tmp70                 ; <i32> [#uses=1]
  store i32 %add75, i32 addrspace(1)* %arrayidx73
  br label %for.inc76

for.inc76:                                        ; preds = %for.body66
  %tmp77 = load i32* %i58                         ; <i32> [#uses=1]
  %inc78 = add i32 %tmp77, 1                      ; <i32> [#uses=1]
  store i32 %inc78, i32* %i58
  br label %for.cond62

for.end79:                                        ; preds = %for.cond62
  br label %for.inc80

for.inc80:                                        ; preds = %for.end79
  %tmp81 = load i32* %t                           ; <i32> [#uses=1]
  %inc82 = add i32 %tmp81, 1                      ; <i32> [#uses=1]
  store i32 %inc82, i32* %t
  br label %for.cond47

for.end83:                                        ; preds = %for.cond47
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

declare i32 @get_group_id(i32)

; CHECK: ret
define void @histogramStep1(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiTmpArray, i32 %szBin, i32 %szBinsPerItem) nounwind {
entry:
  %puiInputMatrix.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiTmpArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %szBinsPerItem.addr = alloca i32, align 4       ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=4]
  %shiftedTmpArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  store i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)** %puiInputMatrix.addr
  store i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)** %puiTmpArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsPerItem, i32* %szBinsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32 addrspace(1)** %puiTmpArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp1 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp2 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp1, %tmp2                     ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %tmp, i32 %mul ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedTmpArray
  %tmp4 = load i32* %gid                          ; <i32> [#uses=1]
  %add = add i32 %tmp4, 1                         ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsPerItem.addr           ; <i32> [#uses=1]
  %mul6 = mul i32 %add, %tmp5                     ; <i32> [#uses=1]
  %tmp7 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul8 = mul i32 %mul6, %tmp7                    ; <i32> [#uses=1]
  store i32 %mul8, i32* %lastIndex
  %tmp10 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp11 = load i32* %szBin.addr                  ; <i32> [#uses=1]
  %mul12 = mul i32 %tmp10, %tmp11                 ; <i32> [#uses=1]
  %tmp13 = load i32* %szBinsPerItem.addr          ; <i32> [#uses=1]
  %mul14 = mul i32 %mul12, %tmp13                 ; <i32> [#uses=1]
  store i32 %mul14, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp15, %tmp16              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load i32 addrspace(1)** %puiInputMatrix.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp18, i32 %tmp17 ; <i32 addrspace(1)*> [#uses=1]
  %tmp19 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp20 = load i32 addrspace(1)** %shiftedTmpArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx21 = getelementptr inbounds i32 addrspace(1)* %tmp20, i32 %tmp19 ; <i32 addrspace(1)*> [#uses=2]
  %tmp22 = load i32 addrspace(1)* %arrayidx21     ; <i32> [#uses=1]
  %inc = add i32 %tmp22, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32 addrspace(1)* %arrayidx21
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp23 = load i32* %i                           ; <i32> [#uses=1]
  %inc24 = add i32 %tmp23, 1                      ; <i32> [#uses=1]
  store i32 %inc24, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @histogramStep2int(i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiTmpArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %szBinsInTmp.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %shiftedTmpArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)** %puiTmpArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsInTmp, i32* %szBinsInTmp.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %tmp1 = load i32* %szElemenetsPerItem.addr      ; <i32> [#uses=1]
  %mul = mul i32 %add, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %lastIndex
  store i32 addrspace(1)* null, i32 addrspace(1)** %shiftedTmpArray
  store i32 0, i32* %t
  br label %for.cond

for.cond:                                         ; preds = %for.inc28, %entry
  %tmp4 = load i32* %t                            ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsInTmp.addr             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp4, %tmp5                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end31

for.body:                                         ; preds = %for.cond
  %tmp6 = load i32 addrspace(1)** %puiTmpArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp7 = load i32* %t                            ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %tmp7, %tmp8                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %tmp6, i32 %mul9 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedTmpArray
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %mul13, i32* %i
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc, %for.body
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond14
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %tmp20 = load i32 addrspace(1)** %shiftedTmpArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp20, i32 %tmp19 ; <i32 addrspace(1)*> [#uses=1]
  %tmp21 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds i32 addrspace(1)* %tmp23, i32 %tmp22 ; <i32 addrspace(1)*> [#uses=2]
  %tmp25 = load i32 addrspace(1)* %arrayidx24     ; <i32> [#uses=1]
  %add26 = add i32 %tmp25, %tmp21                 ; <i32> [#uses=1]
  store i32 %add26, i32 addrspace(1)* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp27, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond14

for.end:                                          ; preds = %for.cond14
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %tmp29 = load i32* %t                           ; <i32> [#uses=1]
  %inc30 = add i32 %tmp29, 1                      ; <i32> [#uses=1]
  store i32 %inc30, i32* %t
  br label %for.cond

for.end31:                                        ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @histogramStep2int2(<2 x i32> addrspace(1)* %puiTmpArray, <2 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiTmpArray.addr = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %szBinsInTmp.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %shiftedTmpArray = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=3]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store <2 x i32> addrspace(1)* %puiTmpArray, <2 x i32> addrspace(1)** %puiTmpArray.addr
  store <2 x i32> addrspace(1)* %puiOutputArray, <2 x i32> addrspace(1)** %puiOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsInTmp, i32* %szBinsInTmp.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %tmp1 = load i32* %szElemenetsPerItem.addr      ; <i32> [#uses=1]
  %mul = mul i32 %add, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %lastIndex
  store <2 x i32> addrspace(1)* null, <2 x i32> addrspace(1)** %shiftedTmpArray
  store i32 0, i32* %t
  br label %for.cond

for.cond:                                         ; preds = %for.inc28, %entry
  %tmp4 = load i32* %t                            ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsInTmp.addr             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp4, %tmp5                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end31

for.body:                                         ; preds = %for.cond
  %tmp6 = load <2 x i32> addrspace(1)** %puiTmpArray.addr ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp7 = load i32* %t                            ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %tmp7, %tmp8                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <2 x i32> addrspace(1)* %tmp6, i32 %mul9 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> addrspace(1)* %add.ptr, <2 x i32> addrspace(1)** %shiftedTmpArray
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %mul13, i32* %i
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc, %for.body
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond14
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %tmp20 = load <2 x i32> addrspace(1)** %shiftedTmpArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <2 x i32> addrspace(1)* %tmp20, i32 %tmp19 ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp21 = load <2 x i32> addrspace(1)* %arrayidx ; <<2 x i32>> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load <2 x i32> addrspace(1)** %puiOutputArray.addr ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp23, i32 %tmp22 ; <<2 x i32> addrspace(1)*> [#uses=2]
  %tmp25 = load <2 x i32> addrspace(1)* %arrayidx24 ; <<2 x i32>> [#uses=1]
  %add26 = add <2 x i32> %tmp25, %tmp21           ; <<2 x i32>> [#uses=1]
  store <2 x i32> %add26, <2 x i32> addrspace(1)* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp27, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond14

for.end:                                          ; preds = %for.cond14
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %tmp29 = load i32* %t                           ; <i32> [#uses=1]
  %inc30 = add i32 %tmp29, 1                      ; <i32> [#uses=1]
  store i32 %inc30, i32* %t
  br label %for.cond

for.end31:                                        ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @histogramStep2int4(<4 x i32> addrspace(1)* %puiTmpArray, <4 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiTmpArray.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %szBinsInTmp.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %shiftedTmpArray = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=3]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store <4 x i32> addrspace(1)* %puiTmpArray, <4 x i32> addrspace(1)** %puiTmpArray.addr
  store <4 x i32> addrspace(1)* %puiOutputArray, <4 x i32> addrspace(1)** %puiOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsInTmp, i32* %szBinsInTmp.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %tmp1 = load i32* %szElemenetsPerItem.addr      ; <i32> [#uses=1]
  %mul = mul i32 %add, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %lastIndex
  store <4 x i32> addrspace(1)* null, <4 x i32> addrspace(1)** %shiftedTmpArray
  store i32 0, i32* %t
  br label %for.cond

for.cond:                                         ; preds = %for.inc28, %entry
  %tmp4 = load i32* %t                            ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsInTmp.addr             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp4, %tmp5                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end31

for.body:                                         ; preds = %for.cond
  %tmp6 = load <4 x i32> addrspace(1)** %puiTmpArray.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp7 = load i32* %t                            ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %tmp7, %tmp8                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <4 x i32> addrspace(1)* %tmp6, i32 %mul9 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> addrspace(1)* %add.ptr, <4 x i32> addrspace(1)** %shiftedTmpArray
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %mul13, i32* %i
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc, %for.body
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond14
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %tmp20 = load <4 x i32> addrspace(1)** %shiftedTmpArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i32> addrspace(1)* %tmp20, i32 %tmp19 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp21 = load <4 x i32> addrspace(1)* %arrayidx ; <<4 x i32>> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load <4 x i32> addrspace(1)** %puiOutputArray.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp23, i32 %tmp22 ; <<4 x i32> addrspace(1)*> [#uses=2]
  %tmp25 = load <4 x i32> addrspace(1)* %arrayidx24 ; <<4 x i32>> [#uses=1]
  %add26 = add <4 x i32> %tmp25, %tmp21           ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add26, <4 x i32> addrspace(1)* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp27, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond14

for.end:                                          ; preds = %for.cond14
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %tmp29 = load i32* %t                           ; <i32> [#uses=1]
  %inc30 = add i32 %tmp29, 1                      ; <i32> [#uses=1]
  store i32 %inc30, i32* %t
  br label %for.cond

for.end31:                                        ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @histogramStep2int8(<8 x i32> addrspace(1)* %puiTmpArray, <8 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiTmpArray.addr = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %szBinsInTmp.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %shiftedTmpArray = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=3]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store <8 x i32> addrspace(1)* %puiTmpArray, <8 x i32> addrspace(1)** %puiTmpArray.addr
  store <8 x i32> addrspace(1)* %puiOutputArray, <8 x i32> addrspace(1)** %puiOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsInTmp, i32* %szBinsInTmp.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %tmp1 = load i32* %szElemenetsPerItem.addr      ; <i32> [#uses=1]
  %mul = mul i32 %add, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %lastIndex
  store <8 x i32> addrspace(1)* null, <8 x i32> addrspace(1)** %shiftedTmpArray
  store i32 0, i32* %t
  br label %for.cond

for.cond:                                         ; preds = %for.inc28, %entry
  %tmp4 = load i32* %t                            ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsInTmp.addr             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp4, %tmp5                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end31

for.body:                                         ; preds = %for.cond
  %tmp6 = load <8 x i32> addrspace(1)** %puiTmpArray.addr ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp7 = load i32* %t                            ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %tmp7, %tmp8                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <8 x i32> addrspace(1)* %tmp6, i32 %mul9 ; <<8 x i32> addrspace(1)*> [#uses=1]
  store <8 x i32> addrspace(1)* %add.ptr, <8 x i32> addrspace(1)** %shiftedTmpArray
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %mul13, i32* %i
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc, %for.body
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond14
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %tmp20 = load <8 x i32> addrspace(1)** %shiftedTmpArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x i32> addrspace(1)* %tmp20, i32 %tmp19 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp21 = load <8 x i32> addrspace(1)* %arrayidx ; <<8 x i32>> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load <8 x i32> addrspace(1)** %puiOutputArray.addr ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp23, i32 %tmp22 ; <<8 x i32> addrspace(1)*> [#uses=2]
  %tmp25 = load <8 x i32> addrspace(1)* %arrayidx24 ; <<8 x i32>> [#uses=1]
  %add26 = add <8 x i32> %tmp25, %tmp21           ; <<8 x i32>> [#uses=1]
  store <8 x i32> %add26, <8 x i32> addrspace(1)* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp27, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond14

for.end:                                          ; preds = %for.cond14
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %tmp29 = load i32* %t                           ; <i32> [#uses=1]
  %inc30 = add i32 %tmp29, 1                      ; <i32> [#uses=1]
  store i32 %inc30, i32* %t
  br label %for.cond

for.end31:                                        ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @histogramStep2int16(<16 x i32> addrspace(1)* %puiTmpArray, <16 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem) nounwind {
entry:
  %puiTmpArray.addr = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=2]
  %szBin.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %szBinsInTmp.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %szElemenetsPerItem.addr = alloca i32, align 4  ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %shiftedTmpArray = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=3]
  %t = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store <16 x i32> addrspace(1)* %puiTmpArray, <16 x i32> addrspace(1)** %puiTmpArray.addr
  store <16 x i32> addrspace(1)* %puiOutputArray, <16 x i32> addrspace(1)** %puiOutputArray.addr
  store i32 %szBin, i32* %szBin.addr
  store i32 %szBinsInTmp, i32* %szBinsInTmp.addr
  store i32 %szElemenetsPerItem, i32* %szElemenetsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %tmp1 = load i32* %szElemenetsPerItem.addr      ; <i32> [#uses=1]
  %mul = mul i32 %add, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %lastIndex
  store <16 x i32> addrspace(1)* null, <16 x i32> addrspace(1)** %shiftedTmpArray
  store i32 0, i32* %t
  br label %for.cond

for.cond:                                         ; preds = %for.inc28, %entry
  %tmp4 = load i32* %t                            ; <i32> [#uses=1]
  %tmp5 = load i32* %szBinsInTmp.addr             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp4, %tmp5                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end31

for.body:                                         ; preds = %for.cond
  %tmp6 = load <16 x i32> addrspace(1)** %puiTmpArray.addr ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp7 = load i32* %t                            ; <i32> [#uses=1]
  %tmp8 = load i32* %szBin.addr                   ; <i32> [#uses=1]
  %mul9 = mul i32 %tmp7, %tmp8                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <16 x i32> addrspace(1)* %tmp6, i32 %mul9 ; <<16 x i32> addrspace(1)*> [#uses=1]
  store <16 x i32> addrspace(1)* %add.ptr, <16 x i32> addrspace(1)** %shiftedTmpArray
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp12 = load i32* %szElemenetsPerItem.addr     ; <i32> [#uses=1]
  %mul13 = mul i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %mul13, i32* %i
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc, %for.body
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond14
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %tmp20 = load <16 x i32> addrspace(1)** %shiftedTmpArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <16 x i32> addrspace(1)* %tmp20, i32 %tmp19 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp21 = load <16 x i32> addrspace(1)* %arrayidx ; <<16 x i32>> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load <16 x i32> addrspace(1)** %puiOutputArray.addr ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp23, i32 %tmp22 ; <<16 x i32> addrspace(1)*> [#uses=2]
  %tmp25 = load <16 x i32> addrspace(1)* %arrayidx24 ; <<16 x i32>> [#uses=1]
  %add26 = add <16 x i32> %tmp25, %tmp21          ; <<16 x i32>> [#uses=1]
  store <16 x i32> %add26, <16 x i32> addrspace(1)* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp27, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond14

for.end:                                          ; preds = %for.cond14
  br label %for.inc28

for.inc28:                                        ; preds = %for.end
  %tmp29 = load i32* %t                           ; <i32> [#uses=1]
  %inc30 = add i32 %tmp29, 1                      ; <i32> [#uses=1]
  store i32 %inc30, i32* %t
  br label %for.cond

for.end31:                                        ; preds = %for.cond
  ret void
}
