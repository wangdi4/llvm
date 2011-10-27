; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32:64"

@msg_fp2 = internal constant [7 x i8] c"%f %f\0A\00"
@msg_fp3 = internal constant [10 x i8] c"%f %f %f\0A\00"

declare void @printf(i8*, ...)

define i32 @main() {
  %msg_fp2 = getelementptr [7 x i8]* @msg_fp2, i32 0, i32 0
  ;================ test for vectors of doubles ===============================
  %X = alloca < 2 x double >
  store < 2 x double > <double 3.4567e+2, double -1.9823e-2>, < 2 x double >* %X
  %D = load < 2 x double >* %X

  ; fadd
  %V = fadd < 2 x double > %D, <double 1.000000e+00, double -1.000000e+00>
  %V0 = extractelement <2 x double> %V, i32 0
  %V1 = extractelement <2 x double> %V, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %V0, double %V1)
  ; CHECK: 346.670000 -1.019823

  ; fsub
  %W = fsub < 2 x double > %V, %D
  %W0 = extractelement <2 x double> %W, i32 0
  %W1 = extractelement <2 x double> %W, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %W0, double %W1)
  ; CHECK: 1.000000 -1.000000

  %U = fmul < 2 x double > %D, %V
  %U0 = extractelement <2 x double> %U, i32 0
  %U1 = extractelement <2 x double> %U, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %U0, double %U1)
  ; CHECK: 119833.418900 0.020216

  %Y = fdiv < 2 x double > %U, %V
  %Y0 = extractelement <2 x double> %Y, i32 0
  %Y1 = extractelement <2 x double> %Y, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %Y0, double %Y1)
  ; CHECK: 345.670000 -0.019823

  %Z = frem < 2 x double > %U, %Y
  %Z0 = extractelement <2 x double> %Z, i32 0
  %Z1 = extractelement <2 x double> %Z, i32 1
  call void (i8*, ...)* @printf(i8* %msg_fp2, double %Z0, double %Z1)
  ; CHECK: 231.598900 0.000393

  call void ()* @testFloat()

  ret i32 0
}

define void @testFloat() {
  %msg_fp3 = getelementptr [10 x i8]* @msg_fp3, i32 0, i32 0
  ;================ test for vectors of floats ===============================
  %X = alloca < 3 x float >
  store < 3 x float > <float 3.456e+03, float -1.93e+2, float 5.6e+02>, < 3 x float >* %X
  %D = load < 3 x float >* %X

  ; fadd
  %V = fadd < 3 x float > %D, <float 1.000000e+00, float -1.000000e+00, float 0.e+0>
  %V0_ = extractelement <3 x float> %V, i32 0
  %V0 = fpext float %V0_ to double
  %V1_ = extractelement <3 x float> %V, i32 1
  %V1 = fpext float %V1_ to double
  %V2_ = extractelement <3 x float> %V, i32 2
  %V2 = fpext float %V2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %V0, double %V1, double %V2)
  ; CHECK: 3457.000000 -194.000000 560.000000

  ; fsub
  %W = fsub < 3 x float > %V, %D
  %W0_ = extractelement <3 x float> %W, i32 0
  %W0 = fpext float %W0_ to double
  %W1_ = extractelement <3 x float> %W, i32 1
  %W1 = fpext float %W1_ to double
  %W2_ = extractelement <3 x float> %W, i32 2
  %W2 = fpext float %W2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %W0, double %W1, double %W2)
  ; CHECK: 1.000000 -1.000000 0.000000

  %U = fmul < 3 x float > %D, %V
  %U0_ = extractelement <3 x float> %U, i32 0
  %U0 = fpext float %U0_ to double
  %U1_ = extractelement <3 x float> %U, i32 1
  %U1 = fpext float %U1_ to double
  %U2_ = extractelement <3 x float> %U, i32 2
  %U2 = fpext float %U2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %U0, double %U1, double %U2)
  ; CHECK: 11947392.000000 37442.000000 313600.000000

  %Y = fdiv < 3 x float > %U, %V
  %Y0_ = extractelement <3 x float> %Y, i32 0
  %Y0 = fpext float %Y0_ to double
  %Y1_ = extractelement <3 x float> %Y, i32 1
  %Y1 = fpext float %Y1_ to double
  %Y2_ = extractelement <3 x float> %Y, i32 2
  %Y2 = fpext float %Y2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %Y0, double %Y1, double %Y2)
  ; CHECK: 3456.000000 -193.000000 560.000000

  %Z = frem < 3 x float > %U, %Y
  %Z0_ = extractelement <3 x float> %Z, i32 0
  %Z0 = fpext float %Z0_ to double
  %Z1_ = extractelement <3 x float> %Z, i32 1
  %Z1 = fpext float %Z1_ to double
  %Z2_ = extractelement <3 x float> %Z, i32 2
  %Z2 = fpext float %Z2_ to double
  call void (i8*, ...)* @printf(i8* %msg_fp3, double %Z0, double %Z1, double %Z2)
  ; CHECK: 0.000000 0.000000 0.000000

  ret void
}
