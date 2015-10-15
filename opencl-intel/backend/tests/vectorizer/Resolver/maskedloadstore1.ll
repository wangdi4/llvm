; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve -runtimelib %p/../Full/runtime.bc %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @testldst
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_load_int4
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_int4
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_int4
; CHECK: ret

declare i32 @_Z13get_global_idj(i32) nounwind
declare <4 x i32> @masked_load_align4_1(<4 x i1>, <4 x i32> addrspace(1)*)
declare void @masked_store_align4_2(<4 x i1>, <4 x i32>, <4 x i32> addrspace(1)*)
declare void @masked_store_align4_3(<4 x i1>, <4 x i32>, <4 x i32> addrspace(1)*)

define void @testldst(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b, i32 addrspace(1)* nocapture %c) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <4 x i32> undef, i32 %call, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = add <4 x i32> %vector, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %0, i32 0
  %1 = getelementptr inbounds i32 addrspace(1)* %c, i32 %extract
  %ptrTypeCast = bitcast i32 addrspace(1)* %1 to <4 x i32> addrspace(1)*
  %2 = load <4 x i32> addrspace(1)* %ptrTypeCast, align 4
  %rem12 = srem <4 x i32> %2, <i32 100, i32 100, i32 100, i32 100>
  %tobool = icmp eq <4 x i32> %rem12, zeroinitializer
  %Mneg13 = xor <4 x i1> %tobool, <i1 true, i1 true, i1 true, i1 true>
  %entry_to_if.then14 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %Mneg13
  %entry_to_if.else15 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %tobool
  br label %if.then

if.then:                                          ; preds = %entry
  %rem216 = srem <4 x i32> %2, <i32 40, i32 40, i32 40, i32 40>
  br label %if.else

if.else:                                          ; preds = %if.then
  %3 = getelementptr inbounds i32 addrspace(1)* %a, i32 %extract
  %ptrTypeCast17 = bitcast i32 addrspace(1)* %3 to <4 x i32> addrspace(1)*
  %4 = call <4 x i32> @masked_load_align4_1(<4 x i1> %entry_to_if.else15, <4 x i32> addrspace(1)* %ptrTypeCast17)
  %mul18 = mul nsw <4 x i32> %4, <i32 80, i32 80, i32 80, i32 80>
  br label %if.end

if.end:                                           ; preds = %if.else
  %merge19 = select <4 x i1> %entry_to_if.then14, <4 x i32> %rem216, <4 x i32> %mul18
  %cmp = icmp sgt <4 x i32> %2, <i32 5, i32 5, i32 5, i32 5>
  %Mneg220 = xor <4 x i1> %cmp, <i1 true, i1 true, i1 true, i1 true>
  %if.end_to_if.else721 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %Mneg220
  %if.end_to_if.then522 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %cmp
  br label %if.then5

if.then5:                                         ; preds = %if.end
  %add23 = add nsw <4 x i32> %merge19, <i32 45, i32 45, i32 45, i32 45>
  %5 = getelementptr inbounds i32 addrspace(1)* %b, i32 %extract
  %ptrTypeCast24 = bitcast i32 addrspace(1)* %5 to <4 x i32> addrspace(1)*
  call void @masked_store_align4_2(<4 x i1> %if.end_to_if.then522, <4 x i32> %add23, <4 x i32> addrspace(1)* %ptrTypeCast24)
  br label %if.else7

if.else7:                                         ; preds = %if.then5
  %div25 = sdiv <4 x i32> %merge19, <i32 97, i32 97, i32 97, i32 97>
  %6 = getelementptr inbounds i32 addrspace(1)* %b, i32 %extract
  %ptrTypeCast26 = bitcast i32 addrspace(1)* %6 to <4 x i32> addrspace(1)*
  call void @masked_store_align4_3(<4 x i1> %if.end_to_if.else721, <4 x i32> %div25, <4 x i32> addrspace(1)* %ptrTypeCast26)
  br label %if.end9

if.end9:                                          ; preds = %if.else7
  ret void
}



