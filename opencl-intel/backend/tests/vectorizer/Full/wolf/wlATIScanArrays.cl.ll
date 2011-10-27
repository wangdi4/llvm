; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIScanArrays.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_ScanLargeArrays_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ScanLargeArrays_parameters = appending global [195 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(3))) *, uint const, uint const, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[195 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i32, float addrspace(1)*)* @ScanLargeArrays to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ScanLargeArrays_locals to i8*), i8* getelementptr inbounds ([195 x i8]* @opencl_ScanLargeArrays_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @ScanLargeArrays(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(3)* %block, i32 %block_size, i32 %length, float addrspace(1)* %sumBuffer) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=5]
  %input.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %block.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=15]
  %block_size.addr = alloca i32, align 4          ; <i32*> [#uses=5]
  %length.addr = alloca i32, align 4              ; <i32*> [#uses=1]
  %sumBuffer.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=13]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=7]
  %bid = alloca i32, align 4                      ; <i32*> [#uses=2]
  %offset = alloca i32, align 4                   ; <i32*> [#uses=9]
  %d = alloca i32, align 4                        ; <i32*> [#uses=5]
  %ai = alloca i32, align 4                       ; <i32*> [#uses=2]
  %bi = alloca i32, align 4                       ; <i32*> [#uses=2]
  %group_id = alloca i32, align 4                 ; <i32*> [#uses=2]
  %d66 = alloca i32, align 4                      ; <i32*> [#uses=5]
  %ai79 = alloca i32, align 4                     ; <i32*> [#uses=3]
  %bi87 = alloca i32, align 4                     ; <i32*> [#uses=3]
  %t = alloca float, align 4                      ; <float*> [#uses=2]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store float addrspace(1)* %input, float addrspace(1)** %input.addr
  store float addrspace(3)* %block, float addrspace(3)** %block.addr
  store i32 %block_size, i32* %block_size.addr
  store i32 %length, i32* %length.addr
  store float addrspace(1)* %sumBuffer, float addrspace(1)** %sumBuffer.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %bid
  store i32 1, i32* %offset
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %mul = mul i32 2, %tmp                          ; <i32> [#uses=1]
  %tmp3 = load float addrspace(1)** %input.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp3, i32 %mul ; <float addrspace(1)*> [#uses=1]
  %tmp4 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  %tmp5 = load i32* %tid                          ; <i32> [#uses=1]
  %mul6 = mul i32 2, %tmp5                        ; <i32> [#uses=1]
  %tmp7 = load float addrspace(3)** %block.addr   ; <float addrspace(3)*> [#uses=1]
  %arrayidx8 = getelementptr inbounds float addrspace(3)* %tmp7, i32 %mul6 ; <float addrspace(3)*> [#uses=1]
  store float %tmp4, float addrspace(3)* %arrayidx8
  %tmp9 = load i32* %gid                          ; <i32> [#uses=1]
  %mul10 = mul i32 2, %tmp9                       ; <i32> [#uses=1]
  %add = add nsw i32 %mul10, 1                    ; <i32> [#uses=1]
  %tmp11 = load float addrspace(1)** %input.addr  ; <float addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds float addrspace(1)* %tmp11, i32 %add ; <float addrspace(1)*> [#uses=1]
  %tmp13 = load float addrspace(1)* %arrayidx12   ; <float> [#uses=1]
  %tmp14 = load i32* %tid                         ; <i32> [#uses=1]
  %mul15 = mul i32 2, %tmp14                      ; <i32> [#uses=1]
  %add16 = add nsw i32 %mul15, 1                  ; <i32> [#uses=1]
  %tmp17 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds float addrspace(3)* %tmp17, i32 %add16 ; <float addrspace(3)*> [#uses=1]
  store float %tmp13, float addrspace(3)* %arrayidx18
  %tmp20 = load i32* %block_size.addr             ; <i32> [#uses=1]
  %shr = lshr i32 %tmp20, 1                       ; <i32> [#uses=1]
  store i32 %shr, i32* %d
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp21 = load i32* %d                           ; <i32> [#uses=1]
  %cmp = icmp sgt i32 %tmp21, 0                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call void @barrier(i32 1)
  %tmp22 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp23 = load i32* %d                           ; <i32> [#uses=1]
  %cmp24 = icmp slt i32 %tmp22, %tmp23            ; <i1> [#uses=1]
  br i1 %cmp24, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp26 = load i32* %offset                      ; <i32> [#uses=1]
  %tmp27 = load i32* %tid                         ; <i32> [#uses=1]
  %mul28 = mul i32 2, %tmp27                      ; <i32> [#uses=1]
  %add29 = add nsw i32 %mul28, 1                  ; <i32> [#uses=1]
  %mul30 = mul i32 %tmp26, %add29                 ; <i32> [#uses=1]
  %sub = sub i32 %mul30, 1                        ; <i32> [#uses=1]
  store i32 %sub, i32* %ai
  %tmp32 = load i32* %offset                      ; <i32> [#uses=1]
  %tmp33 = load i32* %tid                         ; <i32> [#uses=1]
  %mul34 = mul i32 2, %tmp33                      ; <i32> [#uses=1]
  %add35 = add nsw i32 %mul34, 2                  ; <i32> [#uses=1]
  %mul36 = mul i32 %tmp32, %add35                 ; <i32> [#uses=1]
  %sub37 = sub i32 %mul36, 1                      ; <i32> [#uses=1]
  store i32 %sub37, i32* %bi
  %tmp38 = load i32* %ai                          ; <i32> [#uses=1]
  %tmp39 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx40 = getelementptr inbounds float addrspace(3)* %tmp39, i32 %tmp38 ; <float addrspace(3)*> [#uses=1]
  %tmp41 = load float addrspace(3)* %arrayidx40   ; <float> [#uses=1]
  %tmp42 = load i32* %bi                          ; <i32> [#uses=1]
  %tmp43 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds float addrspace(3)* %tmp43, i32 %tmp42 ; <float addrspace(3)*> [#uses=2]
  %tmp45 = load float addrspace(3)* %arrayidx44   ; <float> [#uses=1]
  %add46 = fadd float %tmp45, %tmp41              ; <float> [#uses=1]
  store float %add46, float addrspace(3)* %arrayidx44
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %tmp47 = load i32* %offset                      ; <i32> [#uses=1]
  %mul48 = mul i32 %tmp47, 2                      ; <i32> [#uses=1]
  store i32 %mul48, i32* %offset
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp49 = load i32* %d                           ; <i32> [#uses=1]
  %shr50 = ashr i32 %tmp49, 1                     ; <i32> [#uses=1]
  store i32 %shr50, i32* %d
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @barrier(i32 1)
  %call52 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call52, i32* %group_id
  %tmp53 = load i32* %block_size.addr             ; <i32> [#uses=1]
  %sub54 = sub i32 %tmp53, 1                      ; <i32> [#uses=1]
  %tmp55 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(3)* %tmp55, i32 %sub54 ; <float addrspace(3)*> [#uses=1]
  %tmp57 = load float addrspace(3)* %arrayidx56   ; <float> [#uses=1]
  %tmp58 = load i32* %bid                         ; <i32> [#uses=1]
  %tmp59 = load float addrspace(1)** %sumBuffer.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds float addrspace(1)* %tmp59, i32 %tmp58 ; <float addrspace(1)*> [#uses=1]
  store float %tmp57, float addrspace(1)* %arrayidx60
  call void @barrier(i32 3)
  %tmp61 = load i32* %block_size.addr             ; <i32> [#uses=1]
  %sub62 = sub i32 %tmp61, 1                      ; <i32> [#uses=1]
  %tmp63 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx64 = getelementptr inbounds float addrspace(3)* %tmp63, i32 %sub62 ; <float addrspace(3)*> [#uses=1]
  store float 0.000000e+000, float addrspace(3)* %arrayidx64
  store i32 1, i32* %d66
  br label %for.cond67

for.cond67:                                       ; preds = %for.inc113, %for.end
  %tmp68 = load i32* %d66                         ; <i32> [#uses=1]
  %tmp69 = load i32* %block_size.addr             ; <i32> [#uses=1]
  %cmp70 = icmp ult i32 %tmp68, %tmp69            ; <i1> [#uses=1]
  br i1 %cmp70, label %for.body71, label %for.end116

for.body71:                                       ; preds = %for.cond67
  %tmp72 = load i32* %offset                      ; <i32> [#uses=1]
  %shr73 = ashr i32 %tmp72, 1                     ; <i32> [#uses=1]
  store i32 %shr73, i32* %offset
  call void @barrier(i32 1)
  %tmp74 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp75 = load i32* %d66                         ; <i32> [#uses=1]
  %cmp76 = icmp slt i32 %tmp74, %tmp75            ; <i1> [#uses=1]
  br i1 %cmp76, label %if.then77, label %if.end112

if.then77:                                        ; preds = %for.body71
  %tmp80 = load i32* %offset                      ; <i32> [#uses=1]
  %tmp81 = load i32* %tid                         ; <i32> [#uses=1]
  %mul82 = mul i32 2, %tmp81                      ; <i32> [#uses=1]
  %add83 = add nsw i32 %mul82, 1                  ; <i32> [#uses=1]
  %mul84 = mul i32 %tmp80, %add83                 ; <i32> [#uses=1]
  %sub85 = sub i32 %mul84, 1                      ; <i32> [#uses=1]
  store i32 %sub85, i32* %ai79
  %tmp88 = load i32* %offset                      ; <i32> [#uses=1]
  %tmp89 = load i32* %tid                         ; <i32> [#uses=1]
  %mul90 = mul i32 2, %tmp89                      ; <i32> [#uses=1]
  %add91 = add nsw i32 %mul90, 2                  ; <i32> [#uses=1]
  %mul92 = mul i32 %tmp88, %add91                 ; <i32> [#uses=1]
  %sub93 = sub i32 %mul92, 1                      ; <i32> [#uses=1]
  store i32 %sub93, i32* %bi87
  %tmp95 = load i32* %ai79                        ; <i32> [#uses=1]
  %tmp96 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx97 = getelementptr inbounds float addrspace(3)* %tmp96, i32 %tmp95 ; <float addrspace(3)*> [#uses=1]
  %tmp98 = load float addrspace(3)* %arrayidx97   ; <float> [#uses=1]
  store float %tmp98, float* %t
  %tmp99 = load i32* %bi87                        ; <i32> [#uses=1]
  %tmp100 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx101 = getelementptr inbounds float addrspace(3)* %tmp100, i32 %tmp99 ; <float addrspace(3)*> [#uses=1]
  %tmp102 = load float addrspace(3)* %arrayidx101 ; <float> [#uses=1]
  %tmp103 = load i32* %ai79                       ; <i32> [#uses=1]
  %tmp104 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx105 = getelementptr inbounds float addrspace(3)* %tmp104, i32 %tmp103 ; <float addrspace(3)*> [#uses=1]
  store float %tmp102, float addrspace(3)* %arrayidx105
  %tmp106 = load float* %t                        ; <float> [#uses=1]
  %tmp107 = load i32* %bi87                       ; <i32> [#uses=1]
  %tmp108 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(3)* %tmp108, i32 %tmp107 ; <float addrspace(3)*> [#uses=2]
  %tmp110 = load float addrspace(3)* %arrayidx109 ; <float> [#uses=1]
  %add111 = fadd float %tmp110, %tmp106           ; <float> [#uses=1]
  store float %add111, float addrspace(3)* %arrayidx109
  br label %if.end112

if.end112:                                        ; preds = %if.then77, %for.body71
  br label %for.inc113

for.inc113:                                       ; preds = %if.end112
  %tmp114 = load i32* %d66                        ; <i32> [#uses=1]
  %mul115 = mul i32 %tmp114, 2                    ; <i32> [#uses=1]
  store i32 %mul115, i32* %d66
  br label %for.cond67

for.end116:                                       ; preds = %for.cond67
  call void @barrier(i32 1)
  %tmp117 = load i32* %group_id                   ; <i32> [#uses=1]
  %cmp118 = icmp eq i32 %tmp117, 0                ; <i1> [#uses=1]
  br i1 %cmp118, label %if.then119, label %if.else

if.then119:                                       ; preds = %for.end116
  %tmp120 = load i32* %tid                        ; <i32> [#uses=1]
  %mul121 = mul i32 2, %tmp120                    ; <i32> [#uses=1]
  %tmp122 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx123 = getelementptr inbounds float addrspace(3)* %tmp122, i32 %mul121 ; <float addrspace(3)*> [#uses=1]
  %tmp124 = load float addrspace(3)* %arrayidx123 ; <float> [#uses=1]
  %tmp125 = load i32* %gid                        ; <i32> [#uses=1]
  %mul126 = mul i32 2, %tmp125                    ; <i32> [#uses=1]
  %tmp127 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx128 = getelementptr inbounds float addrspace(1)* %tmp127, i32 %mul126 ; <float addrspace(1)*> [#uses=1]
  store float %tmp124, float addrspace(1)* %arrayidx128
  %tmp129 = load i32* %tid                        ; <i32> [#uses=1]
  %mul130 = mul i32 2, %tmp129                    ; <i32> [#uses=1]
  %add131 = add nsw i32 %mul130, 1                ; <i32> [#uses=1]
  %tmp132 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx133 = getelementptr inbounds float addrspace(3)* %tmp132, i32 %add131 ; <float addrspace(3)*> [#uses=1]
  %tmp134 = load float addrspace(3)* %arrayidx133 ; <float> [#uses=1]
  %tmp135 = load i32* %gid                        ; <i32> [#uses=1]
  %mul136 = mul i32 2, %tmp135                    ; <i32> [#uses=1]
  %add137 = add nsw i32 %mul136, 1                ; <i32> [#uses=1]
  %tmp138 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx139 = getelementptr inbounds float addrspace(1)* %tmp138, i32 %add137 ; <float addrspace(1)*> [#uses=1]
  store float %tmp134, float addrspace(1)* %arrayidx139
  br label %if.end160

if.else:                                          ; preds = %for.end116
  %tmp140 = load i32* %tid                        ; <i32> [#uses=1]
  %mul141 = mul i32 2, %tmp140                    ; <i32> [#uses=1]
  %tmp142 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx143 = getelementptr inbounds float addrspace(3)* %tmp142, i32 %mul141 ; <float addrspace(3)*> [#uses=1]
  %tmp144 = load float addrspace(3)* %arrayidx143 ; <float> [#uses=1]
  %tmp145 = load i32* %gid                        ; <i32> [#uses=1]
  %mul146 = mul i32 2, %tmp145                    ; <i32> [#uses=1]
  %tmp147 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx148 = getelementptr inbounds float addrspace(1)* %tmp147, i32 %mul146 ; <float addrspace(1)*> [#uses=1]
  store float %tmp144, float addrspace(1)* %arrayidx148
  %tmp149 = load i32* %tid                        ; <i32> [#uses=1]
  %mul150 = mul i32 2, %tmp149                    ; <i32> [#uses=1]
  %add151 = add nsw i32 %mul150, 1                ; <i32> [#uses=1]
  %tmp152 = load float addrspace(3)** %block.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx153 = getelementptr inbounds float addrspace(3)* %tmp152, i32 %add151 ; <float addrspace(3)*> [#uses=1]
  %tmp154 = load float addrspace(3)* %arrayidx153 ; <float> [#uses=1]
  %tmp155 = load i32* %gid                        ; <i32> [#uses=1]
  %mul156 = mul i32 2, %tmp155                    ; <i32> [#uses=1]
  %add157 = add nsw i32 %mul156, 1                ; <i32> [#uses=1]
  %tmp158 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx159 = getelementptr inbounds float addrspace(1)* %tmp158, i32 %add157 ; <float addrspace(1)*> [#uses=1]
  store float %tmp154, float addrspace(1)* %arrayidx159
  br label %if.end160

if.end160:                                        ; preds = %if.else, %if.then119
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)
