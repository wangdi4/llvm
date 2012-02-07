; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIFloydWarshall.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_floydWarshallPass_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_floydWarshallPass_parameters = appending global [139 x i8] c"unsigned int __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, unsigned int const, unsigned int const\00", section "llvm.metadata" ; <[139 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @floydWarshallPass to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_floydWarshallPass_locals to i8*), i8* getelementptr inbounds ([139 x i8]* @opencl_floydWarshallPass_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @floydWarshallPass(i32 addrspace(1)* %pathDistanceBuffer, i32 addrspace(1)* %pathBuffer, i32 %width, i32 %pass) nounwind {
entry:
  %pathDistanceBuffer.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=9]
  %pathBuffer.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %pass.addr = alloca i32, align 4                ; <i32*> [#uses=2]
  %x = alloca i32, align 4                        ; <i32*> [#uses=8]
  %y = alloca i32, align 4                        ; <i32*> [#uses=2]
  %k = alloca i32, align 4                        ; <i32*> [#uses=6]
  %yXwidth = alloca i32, align 4                  ; <i32*> [#uses=8]
  %distanceYtoX = alloca i32, align 4             ; <i32*> [#uses=2]
  %distanceYtoK = alloca i32, align 4             ; <i32*> [#uses=2]
  %distanceKtoX = alloca i32, align 4             ; <i32*> [#uses=2]
  %indirectDistance = alloca i32, align 4         ; <i32*> [#uses=3]
  store i32 addrspace(1)* %pathDistanceBuffer, i32 addrspace(1)** %pathDistanceBuffer.addr
  store i32 addrspace(1)* %pathBuffer, i32 addrspace(1)** %pathBuffer.addr
  store i32 %width, i32* %width.addr
  store i32 %pass, i32* %pass.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %x
  %call1 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %y
  %tmp = load i32* %pass.addr                     ; <i32> [#uses=1]
  store i32 %tmp, i32* %k
  %tmp3 = load i32* %y                            ; <i32> [#uses=1]
  %tmp4 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  store i32 %mul, i32* %yXwidth
  %tmp6 = load i32* %yXwidth                      ; <i32> [#uses=1]
  %tmp7 = load i32* %x                            ; <i32> [#uses=1]
  %add = add i32 %tmp6, %tmp7                     ; <i32> [#uses=1]
  %tmp8 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp8, i32 %add ; <i32 addrspace(1)*> [#uses=1]
  %tmp9 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  store i32 %tmp9, i32* %distanceYtoX
  %tmp11 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp12 = load i32* %k                           ; <i32> [#uses=1]
  %add13 = add i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  %tmp14 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx15 = getelementptr inbounds i32 addrspace(1)* %tmp14, i32 %add13 ; <i32 addrspace(1)*> [#uses=1]
  %tmp16 = load i32 addrspace(1)* %arrayidx15     ; <i32> [#uses=1]
  store i32 %tmp16, i32* %distanceYtoK
  %tmp18 = load i32* %k                           ; <i32> [#uses=1]
  %tmp19 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul20 = mul i32 %tmp18, %tmp19                 ; <i32> [#uses=1]
  %tmp21 = load i32* %x                           ; <i32> [#uses=1]
  %add22 = add i32 %mul20, %tmp21                 ; <i32> [#uses=1]
  %tmp23 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds i32 addrspace(1)* %tmp23, i32 %add22 ; <i32 addrspace(1)*> [#uses=1]
  %tmp25 = load i32 addrspace(1)* %arrayidx24     ; <i32> [#uses=1]
  store i32 %tmp25, i32* %distanceKtoX
  %tmp27 = load i32* %distanceYtoK                ; <i32> [#uses=1]
  %tmp28 = load i32* %distanceKtoX                ; <i32> [#uses=1]
  %add29 = add i32 %tmp27, %tmp28                 ; <i32> [#uses=1]
  store i32 %add29, i32* %indirectDistance
  %tmp30 = load i32* %indirectDistance            ; <i32> [#uses=1]
  %tmp31 = load i32* %distanceYtoX                ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp30, %tmp31              ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp32 = load i32* %indirectDistance            ; <i32> [#uses=1]
  %tmp33 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp34 = load i32* %x                           ; <i32> [#uses=1]
  %add35 = add i32 %tmp33, %tmp34                 ; <i32> [#uses=1]
  %tmp36 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx37 = getelementptr inbounds i32 addrspace(1)* %tmp36, i32 %add35 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp32, i32 addrspace(1)* %arrayidx37
  %tmp38 = load i32* %k                           ; <i32> [#uses=1]
  %tmp39 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp40 = load i32* %x                           ; <i32> [#uses=1]
  %add41 = add i32 %tmp39, %tmp40                 ; <i32> [#uses=1]
  %tmp42 = load i32 addrspace(1)** %pathBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx43 = getelementptr inbounds i32 addrspace(1)* %tmp42, i32 %add41 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp38, i32 addrspace(1)* %arrayidx43
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp44 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp45 = load i32* %x                           ; <i32> [#uses=1]
  %add46 = add i32 %tmp44, %tmp45                 ; <i32> [#uses=1]
  %tmp47 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx48 = getelementptr inbounds i32 addrspace(1)* %tmp47, i32 %add46 ; <i32 addrspace(1)*> [#uses=1]
  %tmp49 = load i32 addrspace(1)* %arrayidx48     ; <i32> [#uses=1]
  %tmp50 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp51 = load i32* %k                           ; <i32> [#uses=1]
  %add52 = add i32 %tmp50, %tmp51                 ; <i32> [#uses=1]
  %tmp53 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx54 = getelementptr inbounds i32 addrspace(1)* %tmp53, i32 %add52 ; <i32 addrspace(1)*> [#uses=1]
  %tmp55 = load i32 addrspace(1)* %arrayidx54     ; <i32> [#uses=1]
  %tmp56 = load i32* %k                           ; <i32> [#uses=1]
  %tmp57 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul58 = mul i32 %tmp56, %tmp57                 ; <i32> [#uses=1]
  %tmp59 = load i32* %x                           ; <i32> [#uses=1]
  %add60 = add i32 %mul58, %tmp59                 ; <i32> [#uses=1]
  %tmp61 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx62 = getelementptr inbounds i32 addrspace(1)* %tmp61, i32 %add60 ; <i32 addrspace(1)*> [#uses=1]
  %tmp63 = load i32 addrspace(1)* %arrayidx62     ; <i32> [#uses=1]
  %add64 = add i32 %tmp55, %tmp63                 ; <i32> [#uses=1]
  %call65 = call i32 @uintMin(i32 %tmp49, i32 %add64) ; <i32> [#uses=1]
  %tmp66 = load i32* %yXwidth                     ; <i32> [#uses=1]
  %tmp67 = load i32* %x                           ; <i32> [#uses=1]
  %add68 = add i32 %tmp66, %tmp67                 ; <i32> [#uses=1]
  %tmp69 = load i32 addrspace(1)** %pathDistanceBuffer.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds i32 addrspace(1)* %tmp69, i32 %add68 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %call65, i32 addrspace(1)* %arrayidx70
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

define available_externally i32 @uintMin(i32 %a, i32 %b) nounwind inlinehint {
entry:
  %retval = alloca i32, align 4                   ; <i32*> [#uses=2]
  %a.addr = alloca i32, align 4                   ; <i32*> [#uses=3]
  %b.addr = alloca i32, align 4                   ; <i32*> [#uses=3]
  store i32 %a, i32* %a.addr
  store i32 %b, i32* %b.addr
  %tmp = load i32* %b.addr                        ; <i32> [#uses=1]
  %tmp1 = load i32* %a.addr                       ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  %tmp2 = load i32* %b.addr                       ; <i32> [#uses=1]
  %tmp3 = load i32* %a.addr                       ; <i32> [#uses=1]
  %cond = select i1 %cmp, i32 %tmp2, i32 %tmp3    ; <i32> [#uses=1]
  store i32 %cond, i32* %retval
  %0 = load i32* %retval                          ; <i32> [#uses=1]
  ret i32 %0
}
