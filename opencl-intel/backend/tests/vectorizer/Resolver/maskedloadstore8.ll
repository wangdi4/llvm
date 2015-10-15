; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve -runtimelib %p/../Full/runtime.bc %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @testldst
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_load_double8
; CHECK: sext
; CHECK: addrspacecast
; CHECK: @__ocl_masked_store_double8
; CHECK: sext
; CHECK: addrspacecast
; CHECK: ret

declare i32 @_Z13get_global_idj(i32) nounwind
declare <8 x double> @masked_load_align8_1(<8 x i1>, <8 x double> addrspace(1)*)
declare void @masked_store_align8_2(<8 x i1>, <8 x double>, <8 x double> addrspace(1)*)
declare void @masked_store_align8_3(<8 x i1>, <8 x double>, <8 x double> addrspace(1)*)

define void @testldst(double addrspace(1)* nocapture %a, double addrspace(1)* nocapture %b, double addrspace(1)* nocapture %c) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <8 x i32> undef, i32 %call, i32 0
  %vector = shufflevector <8 x i32> %temp, <8 x i32> undef, <8 x i32> zeroinitializer
  %0 = add <8 x i32> %vector, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %0, i32 0
  %1 = getelementptr inbounds double addrspace(1)* %c, i32 %extract
  %ptrTypeCast = bitcast double addrspace(1)* %1 to <8 x double> addrspace(1)*
  %2 = load <8 x double> addrspace(1)* %ptrTypeCast, align 8
  %cmp = fcmp ogt <8 x double> %2, <double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02>
  %Mneg16 = xor <8 x i1> %cmp, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %entry_to_if.else17 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %Mneg16
  %entry_to_if.then18 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %cmp
  br label %if.then

if.then:                                          ; preds = %entry
  %add19 = fadd <8 x double> %2, <double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01, double 4.000000e+01>
  br label %if.else

if.else:                                          ; preds = %if.then
  %3 = getelementptr inbounds double addrspace(1)* %a, i32 %extract
  %ptrTypeCast20 = bitcast double addrspace(1)* %3 to <8 x double> addrspace(1)*
  %4 = call <8 x double> @masked_load_align8_1(<8 x i1> %entry_to_if.else17, <8 x double> addrspace(1)* %ptrTypeCast20)
  %mul21 = fmul <8 x double> %4, <double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01, double 8.000000e+01>
  br label %if.end

if.end:                                           ; preds = %if.else
  %merge22 = select <8 x i1> %entry_to_if.then18, <8 x double> %add19, <8 x double> %mul21
  %cmp4 = fcmp ogt <8 x double> %2, <double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00>
  %Mneg223 = xor <8 x i1> %cmp4, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %if.end_to_if.else824 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %Mneg223
  %if.end_to_if.then525 = and <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %cmp4
  br label %if.then5

if.then5:                                         ; preds = %if.end
  %add626 = fadd <8 x double> %merge22, <double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01, double 4.500000e+01>
  %5 = getelementptr inbounds double addrspace(1)* %b, i32 %extract
  %ptrTypeCast27 = bitcast double addrspace(1)* %5 to <8 x double> addrspace(1)*
  call void @masked_store_align8_2(<8 x i1> %if.end_to_if.then525, <8 x double> %add626, <8 x double> addrspace(1)* %ptrTypeCast27)
  br label %if.else8

if.else8:                                         ; preds = %if.then5
  %div28 = fdiv <8 x double> %merge22, <double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01, double 9.700000e+01>
  %6 = getelementptr inbounds double addrspace(1)* %b, i32 %extract
  %ptrTypeCast29 = bitcast double addrspace(1)* %6 to <8 x double> addrspace(1)*
  call void @masked_store_align8_3(<8 x i1> %if.end_to_if.else824, <8 x double> %div28, <8 x double> addrspace(1)* %ptrTypeCast29)
  br label %if.end10

if.end10:                                         ; preds = %if.else8
  ret void
}
