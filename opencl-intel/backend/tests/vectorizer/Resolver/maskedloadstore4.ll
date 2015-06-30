; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve -runtimelib %p/../Full/runtime.bc %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @testldst
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_load_long8
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_long8
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_long8
; CHECK: ret

declare i32 @_Z13get_global_idj(i32) nounwind
declare <8 x i64> @masked_load_align8_1(<8 x i1>, <8 x i64> addrspace(1)*)
declare void @masked_store_align8_2(<8 x i1>, <8 x i64>, <8 x i64> addrspace(1)*)
declare void @masked_store_align8_3(<8 x i1>, <8 x i64>, <8 x i64> addrspace(1)*)

define void @testldst(i64 addrspace(1)* nocapture %a, i64 addrspace(1)* nocapture %b, i64 addrspace(1)* nocapture %c) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <8 x i32> undef, i32 %call, i32 0
  %vector = shufflevector <8 x i32> %temp, <8 x i32> undef, <8 x i32> zeroinitializer
  %0 = add <8 x i32> %vector, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %0, i32 0
  %1 = getelementptr inbounds i64 addrspace(1)* %c, i32 %extract
  %ptrTypeCast = bitcast i64 addrspace(1)* %1 to <8 x i64> addrspace(1)*
  %2 = load <8 x i64> addrspace(1)* %ptrTypeCast, align 8
  %rem16 = srem <8 x i64> %2, <i64 100, i64 100, i64 100, i64 100, i64 100, i64 100, i64 100, i64 100>
  %tobool = icmp eq <8 x i64> %rem16, zeroinitializer
  %Mneg17 = xor <8 x i1> %tobool, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %entry_to_if.then18 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %Mneg17
  %entry_to_if.else19 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %tobool
  br label %if.then

if.then:                                          ; preds = %entry
  %rem220 = srem <8 x i64> %2, <i64 40, i64 40, i64 40, i64 40, i64 40, i64 40, i64 40, i64 40>
  br label %if.else

if.else:                                          ; preds = %if.then
  %3 = getelementptr inbounds i64 addrspace(1)* %a, i32 %extract
  %ptrTypeCast21 = bitcast i64 addrspace(1)* %3 to <8 x i64> addrspace(1)*
  %4 = call <8 x i64> @masked_load_align8_1(<8 x i1> %entry_to_if.else19, <8 x i64> addrspace(1)* %ptrTypeCast21)
  %mul22 = mul nsw <8 x i64> %4, <i64 80, i64 80, i64 80, i64 80, i64 80, i64 80, i64 80, i64 80>
  br label %if.end

if.end:                                           ; preds = %if.else
  %merge23 = select <8 x i1> %entry_to_if.then18, <8 x i64> %rem220, <8 x i64> %mul22
  %cmp = icmp sgt <8 x i64> %2, <i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5>
  %Mneg224 = xor <8 x i1> %cmp, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %if.end_to_if.else725 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %Mneg224
  %if.end_to_if.then526 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %cmp
  br label %if.then5

if.then5:                                         ; preds = %if.end
  %add27 = add nsw <8 x i64> %merge23, <i64 45, i64 45, i64 45, i64 45, i64 45, i64 45, i64 45, i64 45>
  %5 = getelementptr inbounds i64 addrspace(1)* %b, i32 %extract
  %ptrTypeCast28 = bitcast i64 addrspace(1)* %5 to <8 x i64> addrspace(1)*
  call void @masked_store_align8_2(<8 x i1> %if.end_to_if.then526, <8 x i64> %add27, <8 x i64> addrspace(1)* %ptrTypeCast28)
  br label %if.else7

if.else7:                                         ; preds = %if.then5
  %div29 = sdiv <8 x i64> %merge23, <i64 97, i64 97, i64 97, i64 97, i64 97, i64 97, i64 97, i64 97>
  %6 = getelementptr inbounds i64 addrspace(1)* %b, i32 %extract
  %ptrTypeCast30 = bitcast i64 addrspace(1)* %6 to <8 x i64> addrspace(1)*
  call void @masked_store_align8_3(<8 x i1> %if.end_to_if.else725, <8 x i64> %div29, <8 x i64> addrspace(1)* %ptrTypeCast30)
  br label %if.end9

if.end9:                                          ; preds = %if.else7
  ret void
}

