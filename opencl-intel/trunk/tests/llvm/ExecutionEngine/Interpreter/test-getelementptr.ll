; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

%ST = type {<2 x i32>, float}

@.LC1 = internal global <3 x i32> <i32 8, i32 9, i32 10>
@.LC2 = addrspace(0) constant %ST {<2 x i32> <i32 8, i32 9>, float 1.0e+0}
@.LC3 = internal global {i32, float} {i32 8, float 0.0e+0}
@.LC4 = internal global <3 x float> <float 8.0e+0, float 10.0e+0, float 1.0e+3>
@.LC5 = internal global <2 x double> <double 18.0e+0, double 31.0e+3>

@msg = internal global [4 x i8] c"%d\0A\00"		; <[4 x i8]*> [#uses=1]
@msgfp = internal global [4 x i8] c"%f\0A\00"		; <[4 x i8]*> [#uses=1]

declare void @printf([4 x i8]*, ...)


define i32 @main() {
	%reg10 =  getelementptr <3 x i32>* @.LC1, i32 0, i32 2
	%reg2 = load i32* %reg10
	%reg11 =  getelementptr {<2 x i32>, float}* @.LC2, i32 0, i32 0, i32 1
	%reg3 = load i32* %reg11
	%reg12 =  getelementptr {i32, float}* @.LC3, i32 0, i32 0
	%reg4 = load i32* %reg12
	%reg14 =  getelementptr <3 x float>* @.LC4, i32 0, i32 1
	%reg5 = load float* %reg14
	%reg6 = fpext float %reg5 to double
	%reg15 =  getelementptr <2 x double>* @.LC5, i32 0, i32 1
	%reg7 = load double* %reg15

	call void ([4 x i8]*, ...)* @printf( [4 x i8]* @msg, i32 %reg2 )
; CHECK: 10
	call void ([4 x i8]*, ...)* @printf( [4 x i8]* @msg, i32 %reg3 )
; CHECK: 9
	call void ([4 x i8]*, ...)* @printf( [4 x i8]* @msg, i32 %reg4 )
; CHECK: 8
	call void ([4 x i8]*, ...)* @printf( [4 x i8]* @msgfp, double %reg6 )
; CHECK: 10.000000
	call void ([4 x i8]*, ...)* @printf( [4 x i8]* @msgfp, double %reg7 )
; CHECK: 31000.000000

	ret i32 0
}
