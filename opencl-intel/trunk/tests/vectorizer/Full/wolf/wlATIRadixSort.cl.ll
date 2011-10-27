; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIRadixSort.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_histogram_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_histogram_parameters = appending global [139 x i8] c"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint, ushort __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[139 x i8]*> [#uses=1]
@opencl_permute_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_permute_parameters = appending global [187 x i8] c"uint const __attribute__((address_space(1))) *, uint const __attribute__((address_space(1))) *, uint, ushort __attribute__((address_space(3))) *, uint __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[187 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*)* @histogram to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_histogram_locals to i8*), i8* getelementptr inbounds ([139 x i8]* @opencl_histogram_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*, i32 addrspace(1)*)* @permute to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_permute_locals to i8*), i8* getelementptr inbounds ([187 x i8]* @opencl_permute_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @histogram(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %buckets, i32 %shiftCount, i16 addrspace(3)* %sharedArray) nounwind {
entry:
  %unsortedData.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %buckets.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %shiftCount.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %sharedArray.addr = alloca i16 addrspace(3)*, align 4 ; <i16 addrspace(3)**> [#uses=4]
  %localId = alloca i32, align 4                  ; <i32*> [#uses=5]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=2]
  %groupId = alloca i32, align 4                  ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i8 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %value = alloca i32, align 4                    ; <i32*> [#uses=4]
  %i36 = alloca i32, align 4                      ; <i32*> [#uses=6]
  %bucketPos = alloca i32, align 4                ; <i32*> [#uses=2]
  store i32 addrspace(1)* %unsortedData, i32 addrspace(1)** %unsortedData.addr
  store i32 addrspace(1)* %buckets, i32 addrspace(1)** %buckets.addr
  store i32 %shiftCount, i32* %shiftCount.addr
  store i16 addrspace(3)* %sharedArray, i16 addrspace(3)** %sharedArray.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %localId
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalId
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %groupId
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 256                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp3 = load i32* %localId                      ; <i32> [#uses=1]
  %mul = mul i32 %tmp3, 256                       ; <i32> [#uses=1]
  %tmp4 = load i32* %i                            ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp4                      ; <i32> [#uses=1]
  %tmp5 = load i16 addrspace(3)** %sharedArray.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(3)* %tmp5, i32 %add ; <i16 addrspace(3)*> [#uses=1]
  store i16 0, i16 addrspace(3)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp6, 1                     ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @barrier(i32 1)
  store i32 0, i32* %i8
  br label %for.cond9

for.cond9:                                        ; preds = %for.inc31, %for.end
  %tmp10 = load i32* %i8                          ; <i32> [#uses=1]
  %cmp11 = icmp slt i32 %tmp10, 256               ; <i1> [#uses=1]
  br i1 %cmp11, label %for.body12, label %for.end34

for.body12:                                       ; preds = %for.cond9
  %tmp14 = load i32* %globalId                    ; <i32> [#uses=1]
  %mul15 = mul i32 %tmp14, 256                    ; <i32> [#uses=1]
  %tmp16 = load i32* %i8                          ; <i32> [#uses=1]
  %add17 = add i32 %mul15, %tmp16                 ; <i32> [#uses=1]
  %tmp18 = load i32 addrspace(1)** %unsortedData.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds i32 addrspace(1)* %tmp18, i32 %add17 ; <i32 addrspace(1)*> [#uses=1]
  %tmp20 = load i32 addrspace(1)* %arrayidx19     ; <i32> [#uses=1]
  store i32 %tmp20, i32* %value
  %tmp21 = load i32* %value                       ; <i32> [#uses=1]
  %tmp22 = load i32* %shiftCount.addr             ; <i32> [#uses=1]
  %shr = lshr i32 %tmp21, %tmp22                  ; <i32> [#uses=1]
  %and = and i32 %shr, 255                        ; <i32> [#uses=1]
  store i32 %and, i32* %value
  %tmp23 = load i32* %localId                     ; <i32> [#uses=1]
  %mul24 = mul i32 %tmp23, 256                    ; <i32> [#uses=1]
  %tmp25 = load i32* %value                       ; <i32> [#uses=1]
  %add26 = add i32 %mul24, %tmp25                 ; <i32> [#uses=1]
  %tmp27 = load i16 addrspace(3)** %sharedArray.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx28 = getelementptr inbounds i16 addrspace(3)* %tmp27, i32 %add26 ; <i16 addrspace(3)*> [#uses=2]
  %tmp29 = load i16 addrspace(3)* %arrayidx28     ; <i16> [#uses=1]
  %inc30 = add i16 %tmp29, 1                      ; <i16> [#uses=1]
  store i16 %inc30, i16 addrspace(3)* %arrayidx28
  br label %for.inc31

for.inc31:                                        ; preds = %for.body12
  %tmp32 = load i32* %i8                          ; <i32> [#uses=1]
  %inc33 = add nsw i32 %tmp32, 1                  ; <i32> [#uses=1]
  store i32 %inc33, i32* %i8
  br label %for.cond9

for.end34:                                        ; preds = %for.cond9
  call void @barrier(i32 1)
  store i32 0, i32* %i36
  br label %for.cond37

for.cond37:                                       ; preds = %for.inc60, %for.end34
  %tmp38 = load i32* %i36                         ; <i32> [#uses=1]
  %cmp39 = icmp slt i32 %tmp38, 256               ; <i1> [#uses=1]
  br i1 %cmp39, label %for.body40, label %for.end63

for.body40:                                       ; preds = %for.cond37
  %tmp42 = load i32* %groupId                     ; <i32> [#uses=1]
  %mul43 = mul i32 %tmp42, 256                    ; <i32> [#uses=1]
  %mul44 = mul i32 %mul43, 16                     ; <i32> [#uses=1]
  %tmp45 = load i32* %localId                     ; <i32> [#uses=1]
  %mul46 = mul i32 %tmp45, 256                    ; <i32> [#uses=1]
  %add47 = add i32 %mul44, %mul46                 ; <i32> [#uses=1]
  %tmp48 = load i32* %i36                         ; <i32> [#uses=1]
  %add49 = add i32 %add47, %tmp48                 ; <i32> [#uses=1]
  store i32 %add49, i32* %bucketPos
  %tmp50 = load i32* %localId                     ; <i32> [#uses=1]
  %mul51 = mul i32 %tmp50, 256                    ; <i32> [#uses=1]
  %tmp52 = load i32* %i36                         ; <i32> [#uses=1]
  %add53 = add i32 %mul51, %tmp52                 ; <i32> [#uses=1]
  %tmp54 = load i16 addrspace(3)** %sharedArray.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds i16 addrspace(3)* %tmp54, i32 %add53 ; <i16 addrspace(3)*> [#uses=1]
  %tmp56 = load i16 addrspace(3)* %arrayidx55     ; <i16> [#uses=1]
  %conv = zext i16 %tmp56 to i32                  ; <i32> [#uses=1]
  %tmp57 = load i32* %bucketPos                   ; <i32> [#uses=1]
  %tmp58 = load i32 addrspace(1)** %buckets.addr  ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx59 = getelementptr inbounds i32 addrspace(1)* %tmp58, i32 %tmp57 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %conv, i32 addrspace(1)* %arrayidx59
  br label %for.inc60

for.inc60:                                        ; preds = %for.body40
  %tmp61 = load i32* %i36                         ; <i32> [#uses=1]
  %inc62 = add nsw i32 %tmp61, 1                  ; <i32> [#uses=1]
  store i32 %inc62, i32* %i36
  br label %for.cond37

for.end63:                                        ; preds = %for.cond37
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)

; CHECK: ret
define void @permute(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %prescanedBuckets, i32 %shiftCount, i16 addrspace(3)* %sharedBuckets, i32 addrspace(1)* %sortedData) nounwind {
entry:
  %unsortedData.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %prescanedBuckets.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %shiftCount.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %sharedBuckets.addr = alloca i16 addrspace(3)*, align 4 ; <i16 addrspace(3)**> [#uses=4]
  %sortedData.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %groupId = alloca i32, align 4                  ; <i32*> [#uses=2]
  %localId = alloca i32, align 4                  ; <i32*> [#uses=5]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %bucketPos = alloca i32, align 4                ; <i32*> [#uses=2]
  %i21 = alloca i32, align 4                      ; <i32*> [#uses=6]
  %value = alloca i32, align 4                    ; <i32*> [#uses=5]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  store i32 addrspace(1)* %unsortedData, i32 addrspace(1)** %unsortedData.addr
  store i32 addrspace(1)* %prescanedBuckets, i32 addrspace(1)** %prescanedBuckets.addr
  store i32 %shiftCount, i32* %shiftCount.addr
  store i16 addrspace(3)* %sharedBuckets, i16 addrspace(3)** %sharedBuckets.addr
  store i32 addrspace(1)* %sortedData, i32 addrspace(1)** %sortedData.addr
  %call = call i32 @get_group_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %groupId
  %call1 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %localId
  %call2 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call2, i32* %globalId
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 256                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp4 = load i32* %groupId                      ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, 256                       ; <i32> [#uses=1]
  %mul5 = mul i32 %mul, 16                        ; <i32> [#uses=1]
  %tmp6 = load i32* %localId                      ; <i32> [#uses=1]
  %mul7 = mul i32 %tmp6, 256                      ; <i32> [#uses=1]
  %add = add i32 %mul5, %mul7                     ; <i32> [#uses=1]
  %tmp8 = load i32* %i                            ; <i32> [#uses=1]
  %add9 = add i32 %add, %tmp8                     ; <i32> [#uses=1]
  store i32 %add9, i32* %bucketPos
  %tmp10 = load i32* %bucketPos                   ; <i32> [#uses=1]
  %tmp11 = load i32 addrspace(1)** %prescanedBuckets.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp11, i32 %tmp10 ; <i32 addrspace(1)*> [#uses=1]
  %tmp12 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %conv = trunc i32 %tmp12 to i16                 ; <i16> [#uses=1]
  %tmp13 = load i32* %localId                     ; <i32> [#uses=1]
  %mul14 = mul i32 %tmp13, 256                    ; <i32> [#uses=1]
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %add16 = add i32 %mul14, %tmp15                 ; <i32> [#uses=1]
  %tmp17 = load i16 addrspace(3)** %sharedBuckets.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds i16 addrspace(3)* %tmp17, i32 %add16 ; <i16 addrspace(3)*> [#uses=1]
  store i16 %conv, i16 addrspace(3)* %arrayidx18
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp19 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp19, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @barrier(i32 1)
  store i32 0, i32* %i21
  br label %for.cond22

for.cond22:                                       ; preds = %for.inc65, %for.end
  %tmp23 = load i32* %i21                         ; <i32> [#uses=1]
  %cmp24 = icmp slt i32 %tmp23, 256               ; <i1> [#uses=1]
  br i1 %cmp24, label %for.body26, label %for.end68

for.body26:                                       ; preds = %for.cond22
  %tmp28 = load i32* %globalId                    ; <i32> [#uses=1]
  %mul29 = mul i32 %tmp28, 256                    ; <i32> [#uses=1]
  %tmp30 = load i32* %i21                         ; <i32> [#uses=1]
  %add31 = add i32 %mul29, %tmp30                 ; <i32> [#uses=1]
  %tmp32 = load i32 addrspace(1)** %unsortedData.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx33 = getelementptr inbounds i32 addrspace(1)* %tmp32, i32 %add31 ; <i32 addrspace(1)*> [#uses=1]
  %tmp34 = load i32 addrspace(1)* %arrayidx33     ; <i32> [#uses=1]
  store i32 %tmp34, i32* %value
  %tmp35 = load i32* %value                       ; <i32> [#uses=1]
  %tmp36 = load i32* %shiftCount.addr             ; <i32> [#uses=1]
  %shr = lshr i32 %tmp35, %tmp36                  ; <i32> [#uses=1]
  %and = and i32 %shr, 255                        ; <i32> [#uses=1]
  store i32 %and, i32* %value
  %tmp38 = load i32* %localId                     ; <i32> [#uses=1]
  %mul39 = mul i32 %tmp38, 256                    ; <i32> [#uses=1]
  %tmp40 = load i32* %value                       ; <i32> [#uses=1]
  %add41 = add i32 %mul39, %tmp40                 ; <i32> [#uses=1]
  %tmp42 = load i16 addrspace(3)** %sharedBuckets.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx43 = getelementptr inbounds i16 addrspace(3)* %tmp42, i32 %add41 ; <i16 addrspace(3)*> [#uses=1]
  %tmp44 = load i16 addrspace(3)* %arrayidx43     ; <i16> [#uses=1]
  %conv45 = zext i16 %tmp44 to i32                ; <i32> [#uses=1]
  store i32 %conv45, i32* %index
  %tmp46 = load i32* %globalId                    ; <i32> [#uses=1]
  %mul47 = mul i32 %tmp46, 256                    ; <i32> [#uses=1]
  %tmp48 = load i32* %i21                         ; <i32> [#uses=1]
  %add49 = add i32 %mul47, %tmp48                 ; <i32> [#uses=1]
  %tmp50 = load i32 addrspace(1)** %unsortedData.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds i32 addrspace(1)* %tmp50, i32 %add49 ; <i32 addrspace(1)*> [#uses=1]
  %tmp52 = load i32 addrspace(1)* %arrayidx51     ; <i32> [#uses=1]
  %tmp53 = load i32* %index                       ; <i32> [#uses=1]
  %tmp54 = load i32 addrspace(1)** %sortedData.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds i32 addrspace(1)* %tmp54, i32 %tmp53 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp52, i32 addrspace(1)* %arrayidx55
  %tmp56 = load i32* %index                       ; <i32> [#uses=1]
  %add57 = add i32 %tmp56, 1                      ; <i32> [#uses=1]
  %conv58 = trunc i32 %add57 to i16               ; <i16> [#uses=1]
  %tmp59 = load i32* %localId                     ; <i32> [#uses=1]
  %mul60 = mul i32 %tmp59, 256                    ; <i32> [#uses=1]
  %tmp61 = load i32* %value                       ; <i32> [#uses=1]
  %add62 = add i32 %mul60, %tmp61                 ; <i32> [#uses=1]
  %tmp63 = load i16 addrspace(3)** %sharedBuckets.addr ; <i16 addrspace(3)*> [#uses=1]
  %arrayidx64 = getelementptr inbounds i16 addrspace(3)* %tmp63, i32 %add62 ; <i16 addrspace(3)*> [#uses=1]
  store i16 %conv58, i16 addrspace(3)* %arrayidx64
  br label %for.inc65

for.inc65:                                        ; preds = %for.body26
  %tmp66 = load i32* %i21                         ; <i32> [#uses=1]
  %inc67 = add nsw i32 %tmp66, 1                  ; <i32> [#uses=1]
  store i32 %inc67, i32* %i21
  br label %for.cond22

for.end68:                                        ; preds = %for.cond22
  ret void
}
