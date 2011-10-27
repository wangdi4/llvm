; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlReduction.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_reductionIntGrouped_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionIntGrouped_parameters = appending global [145 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_reductionInt_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionInt_parameters = appending global [145 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_reductionInt2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionInt2_parameters = appending global [147 x i8] c"uint2 const __attribute__((address_space(1))) *, uint2 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[147 x i8]*> [#uses=1]
@opencl_reductionInt4_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionInt4_parameters = appending global [147 x i8] c"uint4 const __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[147 x i8]*> [#uses=1]
@opencl_reductionInt8_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionInt8_parameters = appending global [147 x i8] c"uint8 const __attribute__((address_space(1))) *, uint8 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[147 x i8]*> [#uses=1]
@opencl_reductionInt16_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reductionInt16_parameters = appending global [149 x i8] c"uint16 const __attribute__((address_space(1))) *, uint16 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, size_t const\00", section "llvm.metadata" ; <[149 x i8]*> [#uses=1]
@opencl_metadata = appending global [6 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @reductionIntGrouped to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionIntGrouped_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_reductionIntGrouped_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @reductionInt to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionInt_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_reductionInt_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*, i32 addrspace(1)*, i32)* @reductionInt2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionInt2_locals to i8*), i8* getelementptr inbounds ([147 x i8]* @opencl_reductionInt2_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, i32 addrspace(1)*, i32)* @reductionInt4 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionInt4_locals to i8*), i8* getelementptr inbounds ([147 x i8]* @opencl_reductionInt4_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<8 x i32> addrspace(1)*, <8 x i32> addrspace(1)*, i32 addrspace(1)*, i32)* @reductionInt8 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionInt8_locals to i8*), i8* getelementptr inbounds ([147 x i8]* @opencl_reductionInt8_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<16 x i32> addrspace(1)*, <16 x i32> addrspace(1)*, i32 addrspace(1)*, i32)* @reductionInt16 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reductionInt16_locals to i8*), i8* getelementptr inbounds ([149 x i8]* @opencl_reductionInt16_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[6 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @reductionIntGrouped(i32 addrspace(1)* %puiInputArray, i32 addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=5]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=2]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=9]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  %currentIndex = alloca i32, align 4             ; <i32*> [#uses=3]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  %i68 = alloca i32, align 4                      ; <i32*> [#uses=4]
  store i32 addrspace(1)* %puiInputArray, i32 addrspace(1)** %puiInputArray.addr
  store i32 addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load i32 addrspace(1)** %puiInputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %tmp5, i32 %tmp6 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedInputArray
  %tmp8 = load i32 addrspace(1)** %puiTmpOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds i32 addrspace(1)* %tmp8, i32 %tmp9 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr10, i32 addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load i32 addrspace(1)** %shiftedInputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp15, i32 %tmp14 ; <i32 addrspace(1)*> [#uses=1]
  %tmp16 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds i32 addrspace(1)* %tmp18, i32 %tmp17 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp16, i32 addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i32 addrspace(1)* %tmp35, i32 %shl ; <i32 addrspace(1)*> [#uses=1]
  %tmp37 = load i32 addrspace(1)* %arrayidx36     ; <i32> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds i32 addrspace(1)* %tmp40, i32 %add ; <i32 addrspace(1)*> [#uses=1]
  %tmp42 = load i32 addrspace(1)* %arrayidx41     ; <i32> [#uses=1]
  %add43 = add i32 %tmp37, %tmp42                 ; <i32> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds i32 addrspace(1)* %tmp45, i32 %tmp44 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add43, i32 addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %call56 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call56, i32* %currentIndex
  %call58 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call58, i32* %lastIndex
  call void @barrier(i32 2)
  %call59 = call i32 @get_local_id(i32 0)         ; <i32> [#uses=1]
  %cmp60 = icmp ult i32 0, %call59                ; <i1> [#uses=1]
  br i1 %cmp60, label %if.then, label %if.end

if.then:                                          ; preds = %for.end54
  br label %for.end88

if.end:                                           ; preds = %for.end54
  %tmp61 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx62 = getelementptr inbounds i32 addrspace(1)* %tmp61, i32 0 ; <i32 addrspace(1)*> [#uses=1]
  %tmp63 = load i32 addrspace(1)* %arrayidx62     ; <i32> [#uses=1]
  %tmp64 = load i32* %currentIndex                ; <i32> [#uses=1]
  %tmp65 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds i32 addrspace(1)* %tmp65, i32 %tmp64 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp63, i32 addrspace(1)* %arrayidx66
  store i32 1, i32* %i68
  br label %for.cond69

for.cond69:                                       ; preds = %for.inc85, %if.end
  %tmp70 = load i32* %i68                         ; <i32> [#uses=1]
  %tmp71 = load i32* %lastIndex                   ; <i32> [#uses=1]
  %cmp72 = icmp ult i32 %tmp70, %tmp71            ; <i1> [#uses=1]
  br i1 %cmp72, label %for.body73, label %for.end88

for.body73:                                       ; preds = %for.cond69
  %tmp74 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %tmp75 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %add.ptr76 = getelementptr inbounds i32 addrspace(1)* %tmp75, i32 %tmp74 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr76, i32 addrspace(1)** %shiftedTmpOutputArray
  %tmp77 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds i32 addrspace(1)* %tmp77, i32 0 ; <i32 addrspace(1)*> [#uses=1]
  %tmp79 = load i32 addrspace(1)* %arrayidx78     ; <i32> [#uses=1]
  %tmp80 = load i32* %currentIndex                ; <i32> [#uses=1]
  %tmp81 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx82 = getelementptr inbounds i32 addrspace(1)* %tmp81, i32 %tmp80 ; <i32 addrspace(1)*> [#uses=2]
  %tmp83 = load i32 addrspace(1)* %arrayidx82     ; <i32> [#uses=1]
  %add84 = add i32 %tmp83, %tmp79                 ; <i32> [#uses=1]
  store i32 %add84, i32 addrspace(1)* %arrayidx82
  br label %for.inc85

for.inc85:                                        ; preds = %for.body73
  %tmp86 = load i32* %i68                         ; <i32> [#uses=1]
  %inc87 = add i32 %tmp86, 1                      ; <i32> [#uses=1]
  store i32 %inc87, i32* %i68
  br label %for.cond69

for.end88:                                        ; preds = %if.then, %for.cond69
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)

declare i32 @get_local_id(i32)

; CHECK: ret
define void @reductionInt(i32 addrspace(1)* %puiInputArray, i32 addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=6]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store i32 addrspace(1)* %puiInputArray, i32 addrspace(1)** %puiInputArray.addr
  store i32 addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load i32 addrspace(1)** %puiInputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %tmp5, i32 %tmp6 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedInputArray
  %tmp8 = load i32 addrspace(1)** %puiTmpOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds i32 addrspace(1)* %tmp8, i32 %tmp9 ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %add.ptr10, i32 addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load i32 addrspace(1)** %shiftedInputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp15, i32 %tmp14 ; <i32 addrspace(1)*> [#uses=1]
  %tmp16 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds i32 addrspace(1)* %tmp18, i32 %tmp17 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp16, i32 addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i32 addrspace(1)* %tmp35, i32 %shl ; <i32 addrspace(1)*> [#uses=1]
  %tmp37 = load i32 addrspace(1)* %arrayidx36     ; <i32> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds i32 addrspace(1)* %tmp40, i32 %add ; <i32 addrspace(1)*> [#uses=1]
  %tmp42 = load i32 addrspace(1)* %arrayidx41     ; <i32> [#uses=1]
  %add43 = add i32 %tmp37, %tmp42                 ; <i32> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds i32 addrspace(1)* %tmp45, i32 %tmp44 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add43, i32 addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %tmp55 = load i32 addrspace(1)** %shiftedTmpOutputArray ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds i32 addrspace(1)* %tmp55, i32 0 ; <i32 addrspace(1)*> [#uses=1]
  %tmp57 = load i32 addrspace(1)* %arrayidx56     ; <i32> [#uses=1]
  %tmp58 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp59 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds i32 addrspace(1)* %tmp59, i32 %tmp58 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp57, i32 addrspace(1)* %arrayidx60
  ret void
}

; CHECK: ret
define void @reductionInt2(<2 x i32> addrspace(1)* %puiInputArray, <2 x i32> addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca <2 x i32> addrspace(1)*, align 4 ; <<2 x i32> addrspace(1)**> [#uses=7]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store <2 x i32> addrspace(1)* %puiInputArray, <2 x i32> addrspace(1)** %puiInputArray.addr
  store <2 x i32> addrspace(1)* %puiTmpOutputArray, <2 x i32> addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load <2 x i32> addrspace(1)** %puiInputArray.addr ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <2 x i32> addrspace(1)* %tmp5, i32 %tmp6 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> addrspace(1)* %add.ptr, <2 x i32> addrspace(1)** %shiftedInputArray
  %tmp8 = load <2 x i32> addrspace(1)** %puiTmpOutputArray.addr ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp8, i32 %tmp9 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> addrspace(1)* %add.ptr10, <2 x i32> addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load <2 x i32> addrspace(1)** %shiftedInputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <2 x i32> addrspace(1)* %tmp15, i32 %tmp14 ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp16 = load <2 x i32> addrspace(1)* %arrayidx ; <<2 x i32>> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp18, i32 %tmp17 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> %tmp16, <2 x i32> addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp35, i32 %shl ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp37 = load <2 x i32> addrspace(1)* %arrayidx36 ; <<2 x i32>> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp40, i32 %add ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp42 = load <2 x i32> addrspace(1)* %arrayidx41 ; <<2 x i32>> [#uses=1]
  %add43 = add <2 x i32> %tmp37, %tmp42           ; <<2 x i32>> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp45, i32 %tmp44 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> %add43, <2 x i32> addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %tmp55 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp55, i32 0 ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp57 = load <2 x i32> addrspace(1)* %arrayidx56 ; <<2 x i32>> [#uses=1]
  %tmp58 = extractelement <2 x i32> %tmp57, i32 0 ; <i32> [#uses=1]
  %tmp59 = load <2 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<2 x i32> addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds <2 x i32> addrspace(1)* %tmp59, i32 0 ; <<2 x i32> addrspace(1)*> [#uses=1]
  %tmp61 = load <2 x i32> addrspace(1)* %arrayidx60 ; <<2 x i32>> [#uses=1]
  %tmp62 = extractelement <2 x i32> %tmp61, i32 1 ; <i32> [#uses=1]
  %add63 = add i32 %tmp58, %tmp62                 ; <i32> [#uses=1]
  %tmp64 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp65 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds i32 addrspace(1)* %tmp65, i32 %tmp64 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add63, i32 addrspace(1)* %arrayidx66
  ret void
}

; CHECK: ret
define void @reductionInt4(<4 x i32> addrspace(1)* %puiInputArray, <4 x i32> addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=9]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store <4 x i32> addrspace(1)* %puiInputArray, <4 x i32> addrspace(1)** %puiInputArray.addr
  store <4 x i32> addrspace(1)* %puiTmpOutputArray, <4 x i32> addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load <4 x i32> addrspace(1)** %puiInputArray.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <4 x i32> addrspace(1)* %tmp5, i32 %tmp6 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> addrspace(1)* %add.ptr, <4 x i32> addrspace(1)** %shiftedInputArray
  %tmp8 = load <4 x i32> addrspace(1)** %puiTmpOutputArray.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp8, i32 %tmp9 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> addrspace(1)* %add.ptr10, <4 x i32> addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load <4 x i32> addrspace(1)** %shiftedInputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i32> addrspace(1)* %tmp15, i32 %tmp14 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp16 = load <4 x i32> addrspace(1)* %arrayidx ; <<4 x i32>> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp18, i32 %tmp17 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> %tmp16, <4 x i32> addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp35, i32 %shl ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp37 = load <4 x i32> addrspace(1)* %arrayidx36 ; <<4 x i32>> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp40, i32 %add ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp42 = load <4 x i32> addrspace(1)* %arrayidx41 ; <<4 x i32>> [#uses=1]
  %add43 = add <4 x i32> %tmp37, %tmp42           ; <<4 x i32>> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp45, i32 %tmp44 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> %add43, <4 x i32> addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %tmp55 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp55, i32 0 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp57 = load <4 x i32> addrspace(1)* %arrayidx56 ; <<4 x i32>> [#uses=1]
  %tmp58 = extractelement <4 x i32> %tmp57, i32 0 ; <i32> [#uses=1]
  %tmp59 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp59, i32 0 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp61 = load <4 x i32> addrspace(1)* %arrayidx60 ; <<4 x i32>> [#uses=1]
  %tmp62 = extractelement <4 x i32> %tmp61, i32 1 ; <i32> [#uses=1]
  %add63 = add i32 %tmp58, %tmp62                 ; <i32> [#uses=1]
  %tmp64 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp64, i32 0 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp66 = load <4 x i32> addrspace(1)* %arrayidx65 ; <<4 x i32>> [#uses=1]
  %tmp67 = extractelement <4 x i32> %tmp66, i32 2 ; <i32> [#uses=1]
  %add68 = add i32 %add63, %tmp67                 ; <i32> [#uses=1]
  %tmp69 = load <4 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp69, i32 0 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp71 = load <4 x i32> addrspace(1)* %arrayidx70 ; <<4 x i32>> [#uses=1]
  %tmp72 = extractelement <4 x i32> %tmp71, i32 3 ; <i32> [#uses=1]
  %add73 = add i32 %add68, %tmp72                 ; <i32> [#uses=1]
  %tmp74 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp75 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx76 = getelementptr inbounds i32 addrspace(1)* %tmp75, i32 %tmp74 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add73, i32 addrspace(1)* %arrayidx76
  ret void
}

; CHECK: ret
define void @reductionInt8(<8 x i32> addrspace(1)* %puiInputArray, <8 x i32> addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca <8 x i32> addrspace(1)*, align 4 ; <<8 x i32> addrspace(1)**> [#uses=13]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store <8 x i32> addrspace(1)* %puiInputArray, <8 x i32> addrspace(1)** %puiInputArray.addr
  store <8 x i32> addrspace(1)* %puiTmpOutputArray, <8 x i32> addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load <8 x i32> addrspace(1)** %puiInputArray.addr ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <8 x i32> addrspace(1)* %tmp5, i32 %tmp6 ; <<8 x i32> addrspace(1)*> [#uses=1]
  store <8 x i32> addrspace(1)* %add.ptr, <8 x i32> addrspace(1)** %shiftedInputArray
  %tmp8 = load <8 x i32> addrspace(1)** %puiTmpOutputArray.addr ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp8, i32 %tmp9 ; <<8 x i32> addrspace(1)*> [#uses=1]
  store <8 x i32> addrspace(1)* %add.ptr10, <8 x i32> addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load <8 x i32> addrspace(1)** %shiftedInputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x i32> addrspace(1)* %tmp15, i32 %tmp14 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp16 = load <8 x i32> addrspace(1)* %arrayidx ; <<8 x i32>> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp18, i32 %tmp17 ; <<8 x i32> addrspace(1)*> [#uses=1]
  store <8 x i32> %tmp16, <8 x i32> addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp35, i32 %shl ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp37 = load <8 x i32> addrspace(1)* %arrayidx36 ; <<8 x i32>> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp40, i32 %add ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp42 = load <8 x i32> addrspace(1)* %arrayidx41 ; <<8 x i32>> [#uses=1]
  %add43 = add <8 x i32> %tmp37, %tmp42           ; <<8 x i32>> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp45, i32 %tmp44 ; <<8 x i32> addrspace(1)*> [#uses=1]
  store <8 x i32> %add43, <8 x i32> addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %tmp55 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp55, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp57 = load <8 x i32> addrspace(1)* %arrayidx56 ; <<8 x i32>> [#uses=1]
  %tmp58 = extractelement <8 x i32> %tmp57, i32 0 ; <i32> [#uses=1]
  %tmp59 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp59, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp61 = load <8 x i32> addrspace(1)* %arrayidx60 ; <<8 x i32>> [#uses=1]
  %tmp62 = extractelement <8 x i32> %tmp61, i32 1 ; <i32> [#uses=1]
  %add63 = add i32 %tmp58, %tmp62                 ; <i32> [#uses=1]
  %tmp64 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp64, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp66 = load <8 x i32> addrspace(1)* %arrayidx65 ; <<8 x i32>> [#uses=1]
  %tmp67 = extractelement <8 x i32> %tmp66, i32 2 ; <i32> [#uses=1]
  %add68 = add i32 %add63, %tmp67                 ; <i32> [#uses=1]
  %tmp69 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp69, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp71 = load <8 x i32> addrspace(1)* %arrayidx70 ; <<8 x i32>> [#uses=1]
  %tmp72 = extractelement <8 x i32> %tmp71, i32 3 ; <i32> [#uses=1]
  %add73 = add i32 %add68, %tmp72                 ; <i32> [#uses=1]
  %tmp74 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx75 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp74, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp76 = load <8 x i32> addrspace(1)* %arrayidx75 ; <<8 x i32>> [#uses=1]
  %tmp77 = extractelement <8 x i32> %tmp76, i32 4 ; <i32> [#uses=1]
  %add78 = add i32 %add73, %tmp77                 ; <i32> [#uses=1]
  %tmp79 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx80 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp79, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp81 = load <8 x i32> addrspace(1)* %arrayidx80 ; <<8 x i32>> [#uses=1]
  %tmp82 = extractelement <8 x i32> %tmp81, i32 5 ; <i32> [#uses=1]
  %add83 = add i32 %add78, %tmp82                 ; <i32> [#uses=1]
  %tmp84 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx85 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp84, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp86 = load <8 x i32> addrspace(1)* %arrayidx85 ; <<8 x i32>> [#uses=1]
  %tmp87 = extractelement <8 x i32> %tmp86, i32 6 ; <i32> [#uses=1]
  %add88 = add i32 %add83, %tmp87                 ; <i32> [#uses=1]
  %tmp89 = load <8 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<8 x i32> addrspace(1)*> [#uses=1]
  %arrayidx90 = getelementptr inbounds <8 x i32> addrspace(1)* %tmp89, i32 0 ; <<8 x i32> addrspace(1)*> [#uses=1]
  %tmp91 = load <8 x i32> addrspace(1)* %arrayidx90 ; <<8 x i32>> [#uses=1]
  %tmp92 = extractelement <8 x i32> %tmp91, i32 7 ; <i32> [#uses=1]
  %add93 = add i32 %add88, %tmp92                 ; <i32> [#uses=1]
  %tmp94 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp95 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx96 = getelementptr inbounds i32 addrspace(1)* %tmp95, i32 %tmp94 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add93, i32 addrspace(1)* %arrayidx96
  ret void
}

; CHECK: ret
define void @reductionInt16(<16 x i32> addrspace(1)* %puiInputArray, <16 x i32> addrspace(1)* %puiTmpOutputArray, i32 addrspace(1)* %puiOutputArray, i32 %szElementsPerItem) nounwind {
entry:
  %puiInputArray.addr = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=2]
  %puiTmpOutputArray.addr = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=2]
  %puiOutputArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %szElementsPerItem.addr = alloca i32, align 4   ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %szPairsPerItem = alloca i32, align 4           ; <i32*> [#uses=2]
  %startingIndex = alloca i32, align 4            ; <i32*> [#uses=3]
  %shiftedInputArray = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=2]
  %shiftedTmpOutputArray = alloca <16 x i32> addrspace(1)*, align 4 ; <<16 x i32> addrspace(1)**> [#uses=21]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %n = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i28 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store <16 x i32> addrspace(1)* %puiInputArray, <16 x i32> addrspace(1)** %puiInputArray.addr
  store <16 x i32> addrspace(1)* %puiTmpOutputArray, <16 x i32> addrspace(1)** %puiTmpOutputArray.addr
  store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
  store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %szElementsPerItem.addr        ; <i32> [#uses=1]
  %shr = lshr i32 %tmp, 1                         ; <i32> [#uses=1]
  store i32 %shr, i32* %szPairsPerItem
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp3 = load i32* %szElementsPerItem.addr       ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %tmp3                     ; <i32> [#uses=1]
  store i32 %mul, i32* %startingIndex
  %tmp5 = load <16 x i32> addrspace(1)** %puiInputArray.addr ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds <16 x i32> addrspace(1)* %tmp5, i32 %tmp6 ; <<16 x i32> addrspace(1)*> [#uses=1]
  store <16 x i32> addrspace(1)* %add.ptr, <16 x i32> addrspace(1)** %shiftedInputArray
  %tmp8 = load <16 x i32> addrspace(1)** %puiTmpOutputArray.addr ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %startingIndex                ; <i32> [#uses=1]
  %add.ptr10 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp8, i32 %tmp9 ; <<16 x i32> addrspace(1)*> [#uses=1]
  store <16 x i32> addrspace(1)* %add.ptr10, <16 x i32> addrspace(1)** %shiftedTmpOutputArray
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32* %szElementsPerItem.addr      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp12, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load <16 x i32> addrspace(1)** %shiftedInputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <16 x i32> addrspace(1)* %tmp15, i32 %tmp14 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp16 = load <16 x i32> addrspace(1)* %arrayidx ; <<16 x i32>> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp18, i32 %tmp17 ; <<16 x i32> addrspace(1)*> [#uses=1]
  store <16 x i32> %tmp16, <16 x i32> addrspace(1)* %arrayidx19
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp20, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp22 = load i32* %szPairsPerItem              ; <i32> [#uses=1]
  store i32 %tmp22, i32* %n
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc51, %for.end
  %tmp24 = load i32* %n                           ; <i32> [#uses=1]
  %cmp25 = icmp ugt i32 %tmp24, 0                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end54

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %i28
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc47, %for.body26
  %tmp30 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp31 = load i32* %n                           ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end50

for.body33:                                       ; preds = %for.cond29
  %tmp34 = load i32* %i28                         ; <i32> [#uses=1]
  %shl = shl i32 %tmp34, 1                        ; <i32> [#uses=1]
  %tmp35 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp35, i32 %shl ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp37 = load <16 x i32> addrspace(1)* %arrayidx36 ; <<16 x i32>> [#uses=1]
  %tmp38 = load i32* %i28                         ; <i32> [#uses=1]
  %shl39 = shl i32 %tmp38, 1                      ; <i32> [#uses=1]
  %add = add nsw i32 %shl39, 1                    ; <i32> [#uses=1]
  %tmp40 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp40, i32 %add ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp42 = load <16 x i32> addrspace(1)* %arrayidx41 ; <<16 x i32>> [#uses=1]
  %add43 = add <16 x i32> %tmp37, %tmp42          ; <<16 x i32>> [#uses=1]
  %tmp44 = load i32* %i28                         ; <i32> [#uses=1]
  %tmp45 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp45, i32 %tmp44 ; <<16 x i32> addrspace(1)*> [#uses=1]
  store <16 x i32> %add43, <16 x i32> addrspace(1)* %arrayidx46
  br label %for.inc47

for.inc47:                                        ; preds = %for.body33
  %tmp48 = load i32* %i28                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %i28
  br label %for.cond29

for.end50:                                        ; preds = %for.cond29
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %tmp52 = load i32* %n                           ; <i32> [#uses=1]
  %shr53 = lshr i32 %tmp52, 1                     ; <i32> [#uses=1]
  store i32 %shr53, i32* %n
  br label %for.cond23

for.end54:                                        ; preds = %for.cond23
  %tmp55 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp55, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp57 = load <16 x i32> addrspace(1)* %arrayidx56 ; <<16 x i32>> [#uses=1]
  %tmp58 = extractelement <16 x i32> %tmp57, i32 0 ; <i32> [#uses=1]
  %tmp59 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp59, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp61 = load <16 x i32> addrspace(1)* %arrayidx60 ; <<16 x i32>> [#uses=1]
  %tmp62 = extractelement <16 x i32> %tmp61, i32 1 ; <i32> [#uses=1]
  %add63 = add i32 %tmp58, %tmp62                 ; <i32> [#uses=1]
  %tmp64 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp64, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp66 = load <16 x i32> addrspace(1)* %arrayidx65 ; <<16 x i32>> [#uses=1]
  %tmp67 = extractelement <16 x i32> %tmp66, i32 2 ; <i32> [#uses=1]
  %add68 = add i32 %add63, %tmp67                 ; <i32> [#uses=1]
  %tmp69 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp69, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp71 = load <16 x i32> addrspace(1)* %arrayidx70 ; <<16 x i32>> [#uses=1]
  %tmp72 = extractelement <16 x i32> %tmp71, i32 3 ; <i32> [#uses=1]
  %add73 = add i32 %add68, %tmp72                 ; <i32> [#uses=1]
  %tmp74 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx75 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp74, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp76 = load <16 x i32> addrspace(1)* %arrayidx75 ; <<16 x i32>> [#uses=1]
  %tmp77 = extractelement <16 x i32> %tmp76, i32 4 ; <i32> [#uses=1]
  %add78 = add i32 %add73, %tmp77                 ; <i32> [#uses=1]
  %tmp79 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx80 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp79, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp81 = load <16 x i32> addrspace(1)* %arrayidx80 ; <<16 x i32>> [#uses=1]
  %tmp82 = extractelement <16 x i32> %tmp81, i32 5 ; <i32> [#uses=1]
  %add83 = add i32 %add78, %tmp82                 ; <i32> [#uses=1]
  %tmp84 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx85 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp84, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp86 = load <16 x i32> addrspace(1)* %arrayidx85 ; <<16 x i32>> [#uses=1]
  %tmp87 = extractelement <16 x i32> %tmp86, i32 6 ; <i32> [#uses=1]
  %add88 = add i32 %add83, %tmp87                 ; <i32> [#uses=1]
  %tmp89 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx90 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp89, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp91 = load <16 x i32> addrspace(1)* %arrayidx90 ; <<16 x i32>> [#uses=1]
  %tmp92 = extractelement <16 x i32> %tmp91, i32 7 ; <i32> [#uses=1]
  %add93 = add i32 %add88, %tmp92                 ; <i32> [#uses=1]
  %tmp94 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp94, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp96 = load <16 x i32> addrspace(1)* %arrayidx95 ; <<16 x i32>> [#uses=1]
  %tmp97 = extractelement <16 x i32> %tmp96, i32 8 ; <i32> [#uses=1]
  %add98 = add i32 %add93, %tmp97                 ; <i32> [#uses=1]
  %tmp99 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx100 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp99, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp101 = load <16 x i32> addrspace(1)* %arrayidx100 ; <<16 x i32>> [#uses=1]
  %tmp102 = extractelement <16 x i32> %tmp101, i32 9 ; <i32> [#uses=1]
  %add103 = add i32 %add98, %tmp102               ; <i32> [#uses=1]
  %tmp104 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx105 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp104, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp106 = load <16 x i32> addrspace(1)* %arrayidx105 ; <<16 x i32>> [#uses=1]
  %tmp107 = extractelement <16 x i32> %tmp106, i32 10 ; <i32> [#uses=1]
  %add108 = add i32 %add103, %tmp107              ; <i32> [#uses=1]
  %tmp109 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx110 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp109, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp111 = load <16 x i32> addrspace(1)* %arrayidx110 ; <<16 x i32>> [#uses=1]
  %tmp112 = extractelement <16 x i32> %tmp111, i32 11 ; <i32> [#uses=1]
  %add113 = add i32 %add108, %tmp112              ; <i32> [#uses=1]
  %tmp114 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx115 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp114, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp116 = load <16 x i32> addrspace(1)* %arrayidx115 ; <<16 x i32>> [#uses=1]
  %tmp117 = extractelement <16 x i32> %tmp116, i32 12 ; <i32> [#uses=1]
  %add118 = add i32 %add113, %tmp117              ; <i32> [#uses=1]
  %tmp119 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx120 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp119, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp121 = load <16 x i32> addrspace(1)* %arrayidx120 ; <<16 x i32>> [#uses=1]
  %tmp122 = extractelement <16 x i32> %tmp121, i32 13 ; <i32> [#uses=1]
  %add123 = add i32 %add118, %tmp122              ; <i32> [#uses=1]
  %tmp124 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx125 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp124, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp126 = load <16 x i32> addrspace(1)* %arrayidx125 ; <<16 x i32>> [#uses=1]
  %tmp127 = extractelement <16 x i32> %tmp126, i32 14 ; <i32> [#uses=1]
  %add128 = add i32 %add123, %tmp127              ; <i32> [#uses=1]
  %tmp129 = load <16 x i32> addrspace(1)** %shiftedTmpOutputArray ; <<16 x i32> addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds <16 x i32> addrspace(1)* %tmp129, i32 0 ; <<16 x i32> addrspace(1)*> [#uses=1]
  %tmp131 = load <16 x i32> addrspace(1)* %arrayidx130 ; <<16 x i32>> [#uses=1]
  %tmp132 = extractelement <16 x i32> %tmp131, i32 15 ; <i32> [#uses=1]
  %add133 = add i32 %add128, %tmp132              ; <i32> [#uses=1]
  %tmp134 = load i32* %gid                        ; <i32> [#uses=1]
  %tmp135 = load i32 addrspace(1)** %puiOutputArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx136 = getelementptr inbounds i32 addrspace(1)* %tmp135, i32 %tmp134 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add133, i32 addrspace(1)* %arrayidx136
  ret void
}
