; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve -runtimelib %p/../Full/runtime.bc %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @testldst
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_load_double4
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_double4
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_double4
; CHECK: ret

declare i32 @_Z13get_global_idj(i32) nounwind
declare <4 x double> @masked_load_align8_1(<4 x i1>, <4 x double> addrspace(1)*)
declare void @masked_store_align8_2(<4 x i1>, <4 x double>, <4 x double> addrspace(1)*)
declare void @masked_store_align8_3(<4 x i1>, <4 x double>, <4 x double> addrspace(1)*)

define void @testldst(double addrspace(1)* nocapture %a, double addrspace(1)* nocapture %b, double addrspace(1)* nocapture %c) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <4 x i32> undef, i32 %call, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = add <4 x i32> %vector, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %0, i32 0
  %1 = getelementptr inbounds double addrspace(1)* %c, i32 %extract
  %ptrTypeCast = bitcast double addrspace(1)* %1 to <4 x double> addrspace(1)*
  %2 = load <4 x double> addrspace(1)* %ptrTypeCast, align 8
  %cmp = fcmp ogt <4 x double> %2, <double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02>
  %Mneg12 = xor <4 x i1> %cmp, <i1 true, i1 true, i1 true, i1 true>
  %entry_to_if.else13 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %Mneg12
  %entry_to_if.then14 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %cmp
  br label %if.then

if.then:                                          ; preds = %entry
  %add15 = fadd <4 x double> %2, <double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01>
  br label %if.else

if.else:                                          ; preds = %if.then
  %3 = getelementptr inbounds double addrspace(1)* %a, i32 %extract
  %ptrTypeCast16 = bitcast double addrspace(1)* %3 to <4 x double> addrspace(1)*
  %4 = call <4 x double> @masked_load_align8_1(<4 x i1> %entry_to_if.else13, <4 x double> addrspace(1)* %ptrTypeCast16)
  %mul17 = fmul <4 x double> %4, <double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01>
  br label %if.end

if.end:                                           ; preds = %if.else
  %merge18 = select <4 x i1> %entry_to_if.then14, <4 x double> %add15, <4 x double> %mul17
  %cmp4 = fcmp ogt <4 x double> %2, <double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00>
  %Mneg219 = xor <4 x i1> %cmp4, <i1 true, i1 true, i1 true, i1 true>
  %if.end_to_if.else820 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %Mneg219
  %if.end_to_if.then521 = and <4 x i1> <i1 true, i1 true, i1 true, i1 true>, %cmp4
  br label %if.then5

if.then5:                                         ; preds = %if.end
  %add622 = fadd <4 x double> %merge18, <double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01>
  %5 = getelementptr inbounds double addrspace(1)* %b, i32 %extract
  %ptrTypeCast23 = bitcast double addrspace(1)* %5 to <4 x double> addrspace(1)*
  call void @masked_store_align8_2(<4 x i1> %if.end_to_if.then521, <4 x double> %add622, <4 x double> addrspace(1)* %ptrTypeCast23)
  br label %if.else8

if.else8:                                         ; preds = %if.then5
  %div24 = fdiv <4 x double> %merge18, <double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01>
  %6 = getelementptr inbounds double addrspace(1)* %b, i32 %extract
  %ptrTypeCast25 = bitcast double addrspace(1)* %6 to <4 x double> addrspace(1)*
  call void @masked_store_align8_3(<4 x i1> %if.end_to_if.else820, <4 x double> %div24, <4 x double> addrspace(1)* %ptrTypeCast25)
  br label %if.end10

if.end10:                                         ; preds = %if.else8
  ret void
}
