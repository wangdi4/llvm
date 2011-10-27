; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\Mandelbrot.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type <{ <2 x i32>, <2 x float>, <2 x float>, i32, float, float, i32, <2 x float>, <2 x float>, float }>

@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [95 x i8] c"kernelArgs __attribute__((address_space(2))) *, kernelArgs __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[95 x i8]*> [#uses=1]
@opencl_Mandelbrot2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_Mandelbrot2D_parameters = appending global [91 x i8] c"float4 __attribute__((address_space(1))) *, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_Mandelbrot_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_Mandelbrot_parameters = appending global [103 x i8] c"float4 __attribute__((address_space(1))) *, uint const, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[103 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct.anon addrspace(2)*, %struct.anon addrspace(1)*)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([95 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, %struct.anon addrspace(2)*)* @Mandelbrot2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_Mandelbrot2D_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_Mandelbrot2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, i32, %struct.anon addrspace(2)*)* @Mandelbrot to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_Mandelbrot_locals to i8*), i8* getelementptr inbounds ([103 x i8]* @opencl_Mandelbrot_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @evaluateDependents(%struct.anon addrspace(2)* %pIn, %struct.anon addrspace(1)* %pOut) nounwind {
entry:
  %pIn.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=23]
  %pOut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=13]
  %opt_x0 = alloca float, align 4                 ; <float*> [#uses=7]
  %opt_y0 = alloca float, align 4                 ; <float*> [#uses=7]
  %opt_zoomInv = alloca float, align 4            ; <float*> [#uses=3]
  %x1 = alloca float, align 4                     ; <float*> [#uses=5]
  %x2 = alloca float, align 4                     ; <float*> [#uses=2]
  %spanX = alloca float, align 4                  ; <float*> [#uses=4]
  %spanY = alloca float, align 4                  ; <float*> [#uses=5]
  %y1 = alloca float, align 4                     ; <float*> [#uses=4]
  %y2 = alloca float, align 4                     ; <float*> [#uses=1]
  store %struct.anon addrspace(2)* %pIn, %struct.anon addrspace(2)** %pIn.addr
  store %struct.anon addrspace(1)* %pOut, %struct.anon addrspace(1)** %pOut.addr
  %tmp = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp1 = getelementptr inbounds %struct.anon addrspace(2)* %tmp, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp2 = load <2 x i32> addrspace(2)* %tmp1      ; <<2 x i32>> [#uses=1]
  %tmp3 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp4 = getelementptr inbounds %struct.anon addrspace(1)* %tmp3, i32 0, i32 0 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> %tmp2, <2 x i32> addrspace(1)* %tmp4
  %tmp5 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp6 = getelementptr inbounds %struct.anon addrspace(2)* %tmp5, i32 0, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp7 = load <2 x float> addrspace(2)* %tmp6    ; <<2 x float>> [#uses=1]
  %tmp8 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp9 = getelementptr inbounds %struct.anon addrspace(1)* %tmp8, i32 0, i32 1 ; <<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %tmp7, <2 x float> addrspace(1)* %tmp9
  %tmp10 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp11 = getelementptr inbounds %struct.anon addrspace(2)* %tmp10, i32 0, i32 2 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp12 = load <2 x float> addrspace(2)* %tmp11  ; <<2 x float>> [#uses=1]
  %tmp13 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp14 = getelementptr inbounds %struct.anon addrspace(1)* %tmp13, i32 0, i32 2 ; <<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %tmp12, <2 x float> addrspace(1)* %tmp14
  %tmp15 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp16 = getelementptr inbounds %struct.anon addrspace(2)* %tmp15, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp17 = load i32 addrspace(2)* %tmp16          ; <i32> [#uses=1]
  %tmp18 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon addrspace(1)* %tmp18, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp17, i32 addrspace(1)* %tmp19
  %tmp20 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp21 = getelementptr inbounds %struct.anon addrspace(2)* %tmp20, i32 0, i32 4 ; <float addrspace(2)*> [#uses=1]
  %tmp22 = load float addrspace(2)* %tmp21        ; <float> [#uses=1]
  %tmp23 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp24 = getelementptr inbounds %struct.anon addrspace(1)* %tmp23, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  store float %tmp22, float addrspace(1)* %tmp24
  %tmp25 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp26 = getelementptr inbounds %struct.anon addrspace(2)* %tmp25, i32 0, i32 5 ; <float addrspace(2)*> [#uses=1]
  %tmp27 = load float addrspace(2)* %tmp26        ; <float> [#uses=1]
  %tmp28 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp29 = getelementptr inbounds %struct.anon addrspace(1)* %tmp28, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  store float %tmp27, float addrspace(1)* %tmp29
  %tmp30 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp31 = getelementptr inbounds %struct.anon addrspace(2)* %tmp30, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp32 = load i32 addrspace(2)* %tmp31          ; <i32> [#uses=1]
  %tmp33 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp34 = getelementptr inbounds %struct.anon addrspace(1)* %tmp33, i32 0, i32 6 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp32, i32 addrspace(1)* %tmp34
  %tmp38 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp39 = getelementptr inbounds %struct.anon addrspace(2)* %tmp38, i32 0, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp40 = load <2 x float> addrspace(2)* %tmp39  ; <<2 x float>> [#uses=1]
  %tmp41 = extractelement <2 x float> %tmp40, i32 0 ; <float> [#uses=1]
  store float %tmp41, float* %opt_x0
  %tmp42 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp43 = getelementptr inbounds %struct.anon addrspace(2)* %tmp42, i32 0, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp44 = load <2 x float> addrspace(2)* %tmp43  ; <<2 x float>> [#uses=1]
  %tmp45 = extractelement <2 x float> %tmp44, i32 1 ; <float> [#uses=1]
  store float %tmp45, float* %opt_y0
  %tmp46 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp47 = getelementptr inbounds %struct.anon addrspace(2)* %tmp46, i32 0, i32 4 ; <float addrspace(2)*> [#uses=1]
  %tmp48 = load float addrspace(2)* %tmp47        ; <float> [#uses=1]
  %tmp49 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp50 = getelementptr inbounds %struct.anon addrspace(2)* %tmp49, i32 0, i32 5 ; <float addrspace(2)*> [#uses=1]
  %tmp51 = load float addrspace(2)* %tmp50        ; <float> [#uses=1]
  %add = fadd float %tmp48, %tmp51                ; <float> [#uses=1]
  %call = call float @_Z3expf(float %add)         ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %call      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %call ; <float> [#uses=0]
  %div = fdiv float 2.000000e+000, %call          ; <float> [#uses=1]
  store float %div, float* %opt_zoomInv
  %tmp52 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp53 = getelementptr inbounds %struct.anon addrspace(2)* %tmp52, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp54 = load i32 addrspace(2)* %tmp53          ; <i32> [#uses=1]
  %cmp55 = icmp eq i32 %tmp54, 1                  ; <i1> [#uses=1]
  br i1 %cmp55, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store float 0xBFE7C2BE80000000, float* %opt_x0
  store float 0xBFC2650640000000, float* %opt_y0
  br label %if.end75

if.else:                                          ; preds = %entry
  %tmp56 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp57 = getelementptr inbounds %struct.anon addrspace(2)* %tmp56, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp58 = load i32 addrspace(2)* %tmp57          ; <i32> [#uses=1]
  %cmp59 = icmp eq i32 %tmp58, 2                  ; <i1> [#uses=1]
  br i1 %cmp59, label %if.then60, label %if.else61

if.then60:                                        ; preds = %if.else
  store float 0x3FD73AA100000000, float* %opt_x0
  store float 0xBFE4A87120000000, float* %opt_y0
  br label %if.end74

if.else61:                                        ; preds = %if.else
  %tmp62 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp63 = getelementptr inbounds %struct.anon addrspace(2)* %tmp62, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp64 = load i32 addrspace(2)* %tmp63          ; <i32> [#uses=1]
  %cmp65 = icmp eq i32 %tmp64, 3                  ; <i1> [#uses=1]
  br i1 %cmp65, label %if.then66, label %if.else67

if.then66:                                        ; preds = %if.else61
  store float 0x3FD4999DC0000000, float* %opt_x0
  store float 0x3FA2033900000000, float* %opt_y0
  br label %if.end73

if.else67:                                        ; preds = %if.else61
  %tmp68 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp69 = getelementptr inbounds %struct.anon addrspace(2)* %tmp68, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp70 = load i32 addrspace(2)* %tmp69          ; <i32> [#uses=1]
  %cmp71 = icmp eq i32 %tmp70, 4                  ; <i1> [#uses=1]
  br i1 %cmp71, label %if.then72, label %if.end

if.then72:                                        ; preds = %if.else67
  store float 0xBFFAC6A4E0000000, float* %opt_x0
  store float 0xBF35BFCD00000000, float* %opt_y0
  br label %if.end

if.end:                                           ; preds = %if.then72, %if.else67
  br label %if.end73

if.end73:                                         ; preds = %if.end, %if.then66
  br label %if.end74

if.end74:                                         ; preds = %if.end73, %if.then60
  br label %if.end75

if.end75:                                         ; preds = %if.end74, %if.then
  %tmp77 = load float* %opt_x0                    ; <float> [#uses=1]
  %tmp78 = load float* %opt_zoomInv               ; <float> [#uses=1]
  %sub = fsub float %tmp77, %tmp78                ; <float> [#uses=1]
  store float %sub, float* %x1
  %tmp80 = load float* %opt_x0                    ; <float> [#uses=1]
  %tmp81 = load float* %opt_zoomInv               ; <float> [#uses=1]
  %add82 = fadd float %tmp80, %tmp81              ; <float> [#uses=1]
  store float %add82, float* %x2
  %tmp84 = load float* %x2                        ; <float> [#uses=1]
  %tmp85 = load float* %x1                        ; <float> [#uses=1]
  %sub86 = fsub float %tmp84, %tmp85              ; <float> [#uses=1]
  store float %sub86, float* %spanX
  %tmp88 = load float* %spanX                     ; <float> [#uses=1]
  %tmp89 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp90 = getelementptr inbounds %struct.anon addrspace(2)* %tmp89, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp91 = load <2 x i32> addrspace(2)* %tmp90    ; <<2 x i32>> [#uses=1]
  %tmp92 = extractelement <2 x i32> %tmp91, i32 1 ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp92 to float              ; <float> [#uses=1]
  %tmp93 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp94 = getelementptr inbounds %struct.anon addrspace(2)* %tmp93, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp95 = load <2 x i32> addrspace(2)* %tmp94    ; <<2 x i32>> [#uses=1]
  %tmp96 = extractelement <2 x i32> %tmp95, i32 0 ; <i32> [#uses=1]
  %conv97 = uitofp i32 %tmp96 to float            ; <float> [#uses=3]
  %cmp98 = fcmp oeq float 0.000000e+000, %conv97  ; <i1> [#uses=1]
  %sel99 = select i1 %cmp98, float 1.000000e+000, float %conv97 ; <float> [#uses=0]
  %div100 = fdiv float %conv, %conv97             ; <float> [#uses=1]
  %mul = fmul float %tmp88, %div100               ; <float> [#uses=1]
  store float %mul, float* %spanY
  %tmp102 = load float* %opt_y0                   ; <float> [#uses=1]
  %tmp103 = load float* %spanY                    ; <float> [#uses=1]
  %div104 = fdiv float %tmp103, 2.000000e+000     ; <float> [#uses=1]
  %sub105 = fsub float %tmp102, %div104           ; <float> [#uses=1]
  store float %sub105, float* %y1
  %tmp107 = load float* %opt_y0                   ; <float> [#uses=1]
  %tmp108 = load float* %spanY                    ; <float> [#uses=1]
  %div109 = fdiv float %tmp108, 2.000000e+000     ; <float> [#uses=1]
  %add110 = fadd float %tmp107, %div109           ; <float> [#uses=1]
  store float %add110, float* %y2
  %tmp111 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp112 = getelementptr inbounds %struct.anon addrspace(2)* %tmp111, i32 0, i32 2 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp113 = load <2 x float> addrspace(2)* %tmp112 ; <<2 x float>> [#uses=1]
  %tmp114 = extractelement <2 x float> %tmp113, i32 0 ; <float> [#uses=1]
  %tmp115 = load float* %spanX                    ; <float> [#uses=1]
  %mul116 = fmul float %tmp114, %tmp115           ; <float> [#uses=1]
  %tmp117 = load float* %x1                       ; <float> [#uses=1]
  %add118 = fadd float %tmp117, %mul116           ; <float> [#uses=1]
  store float %add118, float* %x1
  %tmp119 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp120 = getelementptr inbounds %struct.anon addrspace(2)* %tmp119, i32 0, i32 2 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp121 = load <2 x float> addrspace(2)* %tmp120 ; <<2 x float>> [#uses=1]
  %tmp122 = extractelement <2 x float> %tmp121, i32 1 ; <float> [#uses=1]
  %tmp123 = load float* %spanY                    ; <float> [#uses=1]
  %mul124 = fmul float %tmp122, %tmp123           ; <float> [#uses=1]
  %tmp125 = load float* %y1                       ; <float> [#uses=1]
  %add126 = fadd float %tmp125, %mul124           ; <float> [#uses=1]
  store float %add126, float* %y1
  %tmp127 = load float* %x1                       ; <float> [#uses=1]
  %tmp128 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp129 = getelementptr inbounds %struct.anon addrspace(1)* %tmp128, i32 0, i32 7 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp130 = load <2 x float> addrspace(1)* %tmp129 ; <<2 x float>> [#uses=1]
  %tmp131 = insertelement <2 x float> %tmp130, float %tmp127, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp131, <2 x float> addrspace(1)* %tmp129
  %tmp132 = load float* %y1                       ; <float> [#uses=1]
  %tmp133 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp134 = getelementptr inbounds %struct.anon addrspace(1)* %tmp133, i32 0, i32 7 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp135 = load <2 x float> addrspace(1)* %tmp134 ; <<2 x float>> [#uses=1]
  %tmp136 = insertelement <2 x float> %tmp135, float %tmp132, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp136, <2 x float> addrspace(1)* %tmp134
  %tmp137 = load float* %spanX                    ; <float> [#uses=1]
  %tmp138 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp139 = getelementptr inbounds %struct.anon addrspace(2)* %tmp138, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp140 = load <2 x i32> addrspace(2)* %tmp139  ; <<2 x i32>> [#uses=1]
  %tmp141 = extractelement <2 x i32> %tmp140, i32 0 ; <i32> [#uses=1]
  %conv142 = uitofp i32 %tmp141 to float          ; <float> [#uses=3]
  %cmp143 = fcmp oeq float 0.000000e+000, %conv142 ; <i1> [#uses=1]
  %sel144 = select i1 %cmp143, float 1.000000e+000, float %conv142 ; <float> [#uses=0]
  %div145 = fdiv float %tmp137, %conv142          ; <float> [#uses=1]
  %tmp146 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp147 = getelementptr inbounds %struct.anon addrspace(1)* %tmp146, i32 0, i32 8 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp148 = load <2 x float> addrspace(1)* %tmp147 ; <<2 x float>> [#uses=1]
  %tmp149 = insertelement <2 x float> %tmp148, float %div145, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp149, <2 x float> addrspace(1)* %tmp147
  %tmp150 = load float* %spanY                    ; <float> [#uses=1]
  %tmp151 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp152 = getelementptr inbounds %struct.anon addrspace(2)* %tmp151, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp153 = load <2 x i32> addrspace(2)* %tmp152  ; <<2 x i32>> [#uses=1]
  %tmp154 = extractelement <2 x i32> %tmp153, i32 1 ; <i32> [#uses=1]
  %conv155 = uitofp i32 %tmp154 to float          ; <float> [#uses=3]
  %cmp156 = fcmp oeq float 0.000000e+000, %conv155 ; <i1> [#uses=1]
  %sel157 = select i1 %cmp156, float 1.000000e+000, float %conv155 ; <float> [#uses=0]
  %div158 = fdiv float %tmp150, %conv155          ; <float> [#uses=1]
  %tmp159 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp160 = getelementptr inbounds %struct.anon addrspace(1)* %tmp159, i32 0, i32 8 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp161 = load <2 x float> addrspace(1)* %tmp160 ; <<2 x float>> [#uses=1]
  %tmp162 = insertelement <2 x float> %tmp161, float %div158, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp162, <2 x float> addrspace(1)* %tmp160
  %tmp163 = load %struct.anon addrspace(2)** %pIn.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp164 = getelementptr inbounds %struct.anon addrspace(2)* %tmp163, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp165 = load i32 addrspace(2)* %tmp164        ; <i32> [#uses=1]
  %conv166 = sitofp i32 %tmp165 to float          ; <float> [#uses=3]
  %cmp167 = fcmp oeq float 0.000000e+000, %conv166 ; <i1> [#uses=1]
  %sel168 = select i1 %cmp167, float 1.000000e+000, float %conv166 ; <float> [#uses=0]
  %div169 = fdiv float 1.000000e+000, %conv166    ; <float> [#uses=1]
  %tmp170 = load %struct.anon addrspace(1)** %pOut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp171 = getelementptr inbounds %struct.anon addrspace(1)* %tmp170, i32 0, i32 9 ; <float addrspace(1)*> [#uses=1]
  store float %div169, float addrspace(1)* %tmp171
  ret void
}

declare float @_Z3expf(float)

define <4 x float> @evaluatePixel(<2 x float> %outCrd, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=20]
  %p = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %aa = alloca float, align 4                     ; <float*> [#uses=3]
  %bb = alloca float, align 4                     ; <float*> [#uses=3]
  %zz = alloca float, align 4                     ; <float*> [#uses=0]
  %twoab = alloca float, align 4                  ; <float*> [#uses=2]
  %x0 = alloca float, align 4                     ; <float*> [#uses=7]
  %y0 = alloca float, align 4                     ; <float*> [#uses=7]
  %zoom = alloca float, align 4                   ; <float*> [#uses=3]
  %x1 = alloca float, align 4                     ; <float*> [#uses=5]
  %x2 = alloca float, align 4                     ; <float*> [#uses=2]
  %spanX = alloca float, align 4                  ; <float*> [#uses=4]
  %spanY = alloca float, align 4                  ; <float*> [#uses=5]
  %y1 = alloca float, align 4                     ; <float*> [#uses=4]
  %y2 = alloca float, align 4                     ; <float*> [#uses=1]
  %xy1 = alloca <2 x float>, align 8              ; <<2 x float>*> [#uses=2]
  %spanXY = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=12]
  %z0 = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=3]
  %n = alloca i32, align 4                        ; <i32*> [#uses=8]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp1 = getelementptr inbounds %struct.anon addrspace(2)* %tmp, i32 0, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp2 = load <2 x float> addrspace(2)* %tmp1    ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 0 ; <float> [#uses=1]
  store float %tmp3, float* %x0
  %tmp5 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp6 = getelementptr inbounds %struct.anon addrspace(2)* %tmp5, i32 0, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp7 = load <2 x float> addrspace(2)* %tmp6    ; <<2 x float>> [#uses=1]
  %tmp8 = extractelement <2 x float> %tmp7, i32 1 ; <float> [#uses=1]
  store float %tmp8, float* %y0
  %tmp9 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp10 = getelementptr inbounds %struct.anon addrspace(2)* %tmp9, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp11 = load i32 addrspace(2)* %tmp10          ; <i32> [#uses=1]
  %cmp = icmp eq i32 %tmp11, 1                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store float 0xBFE7C2BE80000000, float* %x0
  store float 0xBFC2650640000000, float* %y0
  br label %if.end31

if.else:                                          ; preds = %entry
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp14 = load i32 addrspace(2)* %tmp13          ; <i32> [#uses=1]
  %cmp15 = icmp eq i32 %tmp14, 2                  ; <i1> [#uses=1]
  br i1 %cmp15, label %if.then16, label %if.else17

if.then16:                                        ; preds = %if.else
  store float 0x3FD73AA100000000, float* %x0
  store float 0xBFE4A87120000000, float* %y0
  br label %if.end30

if.else17:                                        ; preds = %if.else
  %tmp18 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon addrspace(2)* %tmp18, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp20 = load i32 addrspace(2)* %tmp19          ; <i32> [#uses=1]
  %cmp21 = icmp eq i32 %tmp20, 3                  ; <i1> [#uses=1]
  br i1 %cmp21, label %if.then22, label %if.else23

if.then22:                                        ; preds = %if.else17
  store float 0x3FD4999DC0000000, float* %x0
  store float 0x3FA2033900000000, float* %y0
  br label %if.end29

if.else23:                                        ; preds = %if.else17
  %tmp24 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp25 = getelementptr inbounds %struct.anon addrspace(2)* %tmp24, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp26 = load i32 addrspace(2)* %tmp25          ; <i32> [#uses=1]
  %cmp27 = icmp eq i32 %tmp26, 4                  ; <i1> [#uses=1]
  br i1 %cmp27, label %if.then28, label %if.end

if.then28:                                        ; preds = %if.else23
  store float 0xBFFAC6A4E0000000, float* %x0
  store float 0xBF35BFCD00000000, float* %y0
  br label %if.end

if.end:                                           ; preds = %if.then28, %if.else23
  br label %if.end29

if.end29:                                         ; preds = %if.end, %if.then22
  br label %if.end30

if.end30:                                         ; preds = %if.end29, %if.then16
  br label %if.end31

if.end31:                                         ; preds = %if.end30, %if.then
  %tmp33 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp34 = getelementptr inbounds %struct.anon addrspace(2)* %tmp33, i32 0, i32 4 ; <float addrspace(2)*> [#uses=1]
  %tmp35 = load float addrspace(2)* %tmp34        ; <float> [#uses=1]
  %tmp36 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp37 = getelementptr inbounds %struct.anon addrspace(2)* %tmp36, i32 0, i32 5 ; <float addrspace(2)*> [#uses=1]
  %tmp38 = load float addrspace(2)* %tmp37        ; <float> [#uses=1]
  %add = fadd float %tmp35, %tmp38                ; <float> [#uses=1]
  %call = call float @_Z3expf(float %add)         ; <float> [#uses=1]
  store float %call, float* %zoom
  %tmp40 = load float* %x0                        ; <float> [#uses=1]
  %tmp41 = load float* %zoom                      ; <float> [#uses=3]
  %cmp42 = fcmp oeq float 0.000000e+000, %tmp41   ; <i1> [#uses=1]
  %sel = select i1 %cmp42, float 1.000000e+000, float %tmp41 ; <float> [#uses=0]
  %div = fdiv float 2.000000e+000, %tmp41         ; <float> [#uses=1]
  %sub = fsub float %tmp40, %div                  ; <float> [#uses=1]
  store float %sub, float* %x1
  %tmp44 = load float* %x0                        ; <float> [#uses=1]
  %tmp45 = load float* %zoom                      ; <float> [#uses=3]
  %cmp46 = fcmp oeq float 0.000000e+000, %tmp45   ; <i1> [#uses=1]
  %sel47 = select i1 %cmp46, float 1.000000e+000, float %tmp45 ; <float> [#uses=0]
  %div48 = fdiv float 2.000000e+000, %tmp45       ; <float> [#uses=1]
  %add49 = fadd float %tmp44, %div48              ; <float> [#uses=1]
  store float %add49, float* %x2
  %tmp51 = load float* %x2                        ; <float> [#uses=1]
  %tmp52 = load float* %x1                        ; <float> [#uses=1]
  %sub53 = fsub float %tmp51, %tmp52              ; <float> [#uses=1]
  store float %sub53, float* %spanX
  %tmp55 = load float* %spanX                     ; <float> [#uses=1]
  %tmp56 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp57 = getelementptr inbounds %struct.anon addrspace(2)* %tmp56, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp58 = load <2 x i32> addrspace(2)* %tmp57    ; <<2 x i32>> [#uses=1]
  %tmp59 = extractelement <2 x i32> %tmp58, i32 1 ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp59 to float              ; <float> [#uses=1]
  %tmp60 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp61 = getelementptr inbounds %struct.anon addrspace(2)* %tmp60, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp62 = load <2 x i32> addrspace(2)* %tmp61    ; <<2 x i32>> [#uses=1]
  %tmp63 = extractelement <2 x i32> %tmp62, i32 0 ; <i32> [#uses=1]
  %conv64 = uitofp i32 %tmp63 to float            ; <float> [#uses=3]
  %cmp65 = fcmp oeq float 0.000000e+000, %conv64  ; <i1> [#uses=1]
  %sel66 = select i1 %cmp65, float 1.000000e+000, float %conv64 ; <float> [#uses=0]
  %div67 = fdiv float %conv, %conv64              ; <float> [#uses=1]
  %mul = fmul float %tmp55, %div67                ; <float> [#uses=1]
  store float %mul, float* %spanY
  %tmp69 = load float* %y0                        ; <float> [#uses=1]
  %tmp70 = load float* %spanY                     ; <float> [#uses=1]
  %div71 = fdiv float %tmp70, 2.000000e+000       ; <float> [#uses=1]
  %sub72 = fsub float %tmp69, %div71              ; <float> [#uses=1]
  store float %sub72, float* %y1
  %tmp74 = load float* %y0                        ; <float> [#uses=1]
  %tmp75 = load float* %spanY                     ; <float> [#uses=1]
  %div76 = fdiv float %tmp75, 2.000000e+000       ; <float> [#uses=1]
  %add77 = fadd float %tmp74, %div76              ; <float> [#uses=1]
  store float %add77, float* %y2
  %tmp78 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp79 = getelementptr inbounds %struct.anon addrspace(2)* %tmp78, i32 0, i32 2 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp80 = load <2 x float> addrspace(2)* %tmp79  ; <<2 x float>> [#uses=1]
  %tmp81 = extractelement <2 x float> %tmp80, i32 0 ; <float> [#uses=1]
  %tmp82 = load float* %spanX                     ; <float> [#uses=1]
  %mul83 = fmul float %tmp81, %tmp82              ; <float> [#uses=1]
  %tmp84 = load float* %x1                        ; <float> [#uses=1]
  %add85 = fadd float %tmp84, %mul83              ; <float> [#uses=1]
  store float %add85, float* %x1
  %tmp86 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp87 = getelementptr inbounds %struct.anon addrspace(2)* %tmp86, i32 0, i32 2 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp88 = load <2 x float> addrspace(2)* %tmp87  ; <<2 x float>> [#uses=1]
  %tmp89 = extractelement <2 x float> %tmp88, i32 1 ; <float> [#uses=1]
  %tmp90 = load float* %spanY                     ; <float> [#uses=1]
  %mul91 = fmul float %tmp89, %tmp90              ; <float> [#uses=1]
  %tmp92 = load float* %y1                        ; <float> [#uses=1]
  %add93 = fadd float %tmp92, %mul91              ; <float> [#uses=1]
  store float %add93, float* %y1
  %tmp95 = load float* %x1                        ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %tmp95, i32 0 ; <<2 x float>> [#uses=1]
  %tmp96 = load float* %y1                        ; <float> [#uses=1]
  %vecinit97 = insertelement <2 x float> %vecinit, float %tmp96, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit97, <2 x float>* %xy1
  %tmp99 = load float* %spanX                     ; <float> [#uses=1]
  %tmp100 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp101 = getelementptr inbounds %struct.anon addrspace(2)* %tmp100, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp102 = load <2 x i32> addrspace(2)* %tmp101  ; <<2 x i32>> [#uses=1]
  %tmp103 = extractelement <2 x i32> %tmp102, i32 0 ; <i32> [#uses=1]
  %conv104 = uitofp i32 %tmp103 to float          ; <float> [#uses=3]
  %cmp105 = fcmp oeq float 0.000000e+000, %conv104 ; <i1> [#uses=1]
  %sel106 = select i1 %cmp105, float 1.000000e+000, float %conv104 ; <float> [#uses=0]
  %div107 = fdiv float %tmp99, %conv104           ; <float> [#uses=1]
  %vecinit108 = insertelement <2 x float> undef, float %div107, i32 0 ; <<2 x float>> [#uses=1]
  %tmp109 = load float* %spanY                    ; <float> [#uses=1]
  %tmp110 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp111 = getelementptr inbounds %struct.anon addrspace(2)* %tmp110, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp112 = load <2 x i32> addrspace(2)* %tmp111  ; <<2 x i32>> [#uses=1]
  %tmp113 = extractelement <2 x i32> %tmp112, i32 1 ; <i32> [#uses=1]
  %conv114 = uitofp i32 %tmp113 to float          ; <float> [#uses=3]
  %cmp115 = fcmp oeq float 0.000000e+000, %conv114 ; <i1> [#uses=1]
  %sel116 = select i1 %cmp115, float 1.000000e+000, float %conv114 ; <float> [#uses=0]
  %div117 = fdiv float %tmp109, %conv114          ; <float> [#uses=1]
  %vecinit118 = insertelement <2 x float> %vecinit108, float %div117, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit118, <2 x float>* %spanXY
  %tmp120 = load <2 x float>* %xy1                ; <<2 x float>> [#uses=1]
  %tmp121 = load <2 x float>* %outCrd.addr        ; <<2 x float>> [#uses=1]
  %tmp122 = load <2 x float>* %spanXY             ; <<2 x float>> [#uses=1]
  %mul123 = fmul <2 x float> %tmp121, %tmp122     ; <<2 x float>> [#uses=1]
  %add124 = fadd <2 x float> %tmp120, %mul123     ; <<2 x float>> [#uses=1]
  store <2 x float> %add124, <2 x float>* %z
  %tmp126 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp126, <2 x float>* %z0
  store i32 0, i32* %n
  store <4 x float> <float 0.000000e+000, float 0.000000e+000, float 0.000000e+000, float 1.000000e+000>, <4 x float>* %dst
  br label %while.cond

while.cond:                                       ; preds = %if.end207, %if.end31
  %tmp129 = load i32* %n                          ; <i32> [#uses=1]
  %tmp130 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp131 = getelementptr inbounds %struct.anon addrspace(2)* %tmp130, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp132 = load i32 addrspace(2)* %tmp131        ; <i32> [#uses=1]
  %cmp133 = icmp slt i32 %tmp129, %tmp132         ; <i1> [#uses=1]
  br i1 %cmp133, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %tmp135 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp136 = extractelement <2 x float> %tmp135, i32 0 ; <float> [#uses=1]
  %tmp137 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp138 = extractelement <2 x float> %tmp137, i32 0 ; <float> [#uses=1]
  %mul139 = fmul float %tmp136, %tmp138           ; <float> [#uses=1]
  store float %mul139, float* %aa
  %tmp140 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp141 = extractelement <2 x float> %tmp140, i32 1 ; <float> [#uses=1]
  %tmp142 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp143 = extractelement <2 x float> %tmp142, i32 1 ; <float> [#uses=1]
  %mul144 = fmul float %tmp141, %tmp143           ; <float> [#uses=1]
  store float %mul144, float* %bb
  %tmp145 = load float* %aa                       ; <float> [#uses=1]
  %tmp146 = load float* %bb                       ; <float> [#uses=1]
  %add147 = fadd float %tmp145, %tmp146           ; <float> [#uses=1]
  %cmp148 = fcmp ogt float %add147, 1.000000e+001 ; <i1> [#uses=1]
  br i1 %cmp148, label %if.then150, label %if.end207

if.then150:                                       ; preds = %while.body
  %tmp151 = load i32* %n                          ; <i32> [#uses=1]
  %conv152 = sitofp i32 %tmp151 to float          ; <float> [#uses=1]
  %tmp153 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp154 = getelementptr inbounds %struct.anon addrspace(2)* %tmp153, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp155 = load i32 addrspace(2)* %tmp154        ; <i32> [#uses=1]
  %conv156 = sitofp i32 %tmp155 to float          ; <float> [#uses=3]
  %cmp157 = fcmp oeq float 0.000000e+000, %conv156 ; <i1> [#uses=1]
  %sel158 = select i1 %cmp157, float 1.000000e+000, float %conv156 ; <float> [#uses=0]
  %div159 = fdiv float %conv152, %conv156         ; <float> [#uses=1]
  %conv160 = fpext float %div159 to double        ; <double> [#uses=1]
  %sub161 = fsub double 1.000000e+000, %conv160   ; <double> [#uses=1]
  %conv162 = fptrunc double %sub161 to float      ; <float> [#uses=1]
  %vecinit163 = insertelement <4 x float> undef, float %conv162, i32 0 ; <<4 x float>> [#uses=1]
  %tmp164 = load i32* %n                          ; <i32> [#uses=1]
  %conv165 = sitofp i32 %tmp164 to float          ; <float> [#uses=1]
  %tmp166 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp167 = getelementptr inbounds %struct.anon addrspace(2)* %tmp166, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp168 = load i32 addrspace(2)* %tmp167        ; <i32> [#uses=1]
  %conv169 = sitofp i32 %tmp168 to float          ; <float> [#uses=3]
  %cmp170 = fcmp oeq float 0.000000e+000, %conv169 ; <i1> [#uses=1]
  %sel171 = select i1 %cmp170, float 1.000000e+000, float %conv169 ; <float> [#uses=0]
  %div172 = fdiv float %conv165, %conv169         ; <float> [#uses=1]
  %conv173 = fpext float %div172 to double        ; <double> [#uses=1]
  %sub174 = fsub double 1.000000e+000, %conv173   ; <double> [#uses=1]
  %conv175 = fptrunc double %sub174 to float      ; <float> [#uses=1]
  %vecinit176 = insertelement <4 x float> %vecinit163, float %conv175, i32 1 ; <<4 x float>> [#uses=1]
  %tmp177 = load i32* %n                          ; <i32> [#uses=1]
  %conv178 = sitofp i32 %tmp177 to float          ; <float> [#uses=1]
  %tmp179 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp180 = getelementptr inbounds %struct.anon addrspace(2)* %tmp179, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp181 = load i32 addrspace(2)* %tmp180        ; <i32> [#uses=1]
  %conv182 = sitofp i32 %tmp181 to float          ; <float> [#uses=3]
  %cmp183 = fcmp oeq float 0.000000e+000, %conv182 ; <i1> [#uses=1]
  %sel184 = select i1 %cmp183, float 1.000000e+000, float %conv182 ; <float> [#uses=0]
  %div185 = fdiv float %conv178, %conv182         ; <float> [#uses=1]
  %conv186 = fpext float %div185 to double        ; <double> [#uses=1]
  %sub187 = fsub double 1.000000e+000, %conv186   ; <double> [#uses=1]
  %conv188 = fptrunc double %sub187 to float      ; <float> [#uses=1]
  %vecinit189 = insertelement <4 x float> %vecinit176, float %conv188, i32 2 ; <<4 x float>> [#uses=1]
  %tmp190 = load i32* %n                          ; <i32> [#uses=1]
  %conv191 = sitofp i32 %tmp190 to float          ; <float> [#uses=1]
  %tmp192 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp193 = getelementptr inbounds %struct.anon addrspace(2)* %tmp192, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp194 = load i32 addrspace(2)* %tmp193        ; <i32> [#uses=1]
  %conv195 = sitofp i32 %tmp194 to float          ; <float> [#uses=3]
  %cmp196 = fcmp oeq float 0.000000e+000, %conv195 ; <i1> [#uses=1]
  %sel197 = select i1 %cmp196, float 1.000000e+000, float %conv195 ; <float> [#uses=0]
  %div198 = fdiv float %conv191, %conv195         ; <float> [#uses=1]
  %conv199 = fpext float %div198 to double        ; <double> [#uses=1]
  %sub200 = fsub double 1.000000e+000, %conv199   ; <double> [#uses=1]
  %conv201 = fptrunc double %sub200 to float      ; <float> [#uses=1]
  %vecinit202 = insertelement <4 x float> %vecinit189, float %conv201, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit202, <4 x float>* %.compoundliteral
  %tmp203 = load <4 x float>* %.compoundliteral   ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp203, <4 x float>* %p
  %tmp204 = load <4 x float>* %p                  ; <<4 x float>> [#uses=1]
  %tmp205 = insertelement <4 x float> %tmp204, float 1.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp205, <4 x float>* %p
  %tmp206 = load <4 x float>* %p                  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp206, <4 x float>* %dst
  br label %while.end

if.end207:                                        ; preds = %while.body
  %tmp208 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp209 = extractelement <2 x float> %tmp208, i32 0 ; <float> [#uses=1]
  %conv210 = fpext float %tmp209 to double        ; <double> [#uses=1]
  %mul211 = fmul double 2.000000e+000, %conv210   ; <double> [#uses=1]
  %tmp212 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp213 = extractelement <2 x float> %tmp212, i32 1 ; <float> [#uses=1]
  %conv214 = fpext float %tmp213 to double        ; <double> [#uses=1]
  %mul215 = fmul double %mul211, %conv214         ; <double> [#uses=1]
  %conv216 = fptrunc double %mul215 to float      ; <float> [#uses=1]
  store float %conv216, float* %twoab
  %tmp217 = load float* %aa                       ; <float> [#uses=1]
  %tmp218 = load float* %bb                       ; <float> [#uses=1]
  %sub219 = fsub float %tmp217, %tmp218           ; <float> [#uses=1]
  %tmp220 = load <2 x float>* %z0                 ; <<2 x float>> [#uses=1]
  %tmp221 = extractelement <2 x float> %tmp220, i32 0 ; <float> [#uses=1]
  %add222 = fadd float %sub219, %tmp221           ; <float> [#uses=1]
  %tmp223 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp224 = insertelement <2 x float> %tmp223, float %add222, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp224, <2 x float>* %z
  %tmp225 = load float* %twoab                    ; <float> [#uses=1]
  %tmp226 = load <2 x float>* %z0                 ; <<2 x float>> [#uses=1]
  %tmp227 = extractelement <2 x float> %tmp226, i32 1 ; <float> [#uses=1]
  %add228 = fadd float %tmp225, %tmp227           ; <float> [#uses=1]
  %tmp229 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp230 = insertelement <2 x float> %tmp229, float %add228, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp230, <2 x float>* %z
  %tmp231 = load i32* %n                          ; <i32> [#uses=1]
  %add232 = add nsw i32 %tmp231, 1                ; <i32> [#uses=1]
  store i32 %add232, i32* %n
  br label %while.cond

while.end:                                        ; preds = %if.then150, %while.cond
  %tmp233 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp233, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

define <4 x float> @evaluatePixelOpt(<2 x float> %outCrd, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=8]
  %p = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %aa = alloca float, align 4                     ; <float*> [#uses=3]
  %bb = alloca float, align 4                     ; <float*> [#uses=3]
  %zz = alloca float, align 4                     ; <float*> [#uses=0]
  %twoab = alloca float, align 4                  ; <float*> [#uses=2]
  %z = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=12]
  %z0 = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=3]
  %n = alloca i32, align 4                        ; <i32*> [#uses=8]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp1 = getelementptr inbounds %struct.anon addrspace(2)* %tmp, i32 0, i32 7 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp2 = load <2 x float> addrspace(2)* %tmp1    ; <<2 x float>> [#uses=1]
  %tmp3 = load <2 x float>* %outCrd.addr          ; <<2 x float>> [#uses=1]
  %tmp4 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp5 = getelementptr inbounds %struct.anon addrspace(2)* %tmp4, i32 0, i32 8 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp6 = load <2 x float> addrspace(2)* %tmp5    ; <<2 x float>> [#uses=1]
  %mul = fmul <2 x float> %tmp3, %tmp6            ; <<2 x float>> [#uses=1]
  %add = fadd <2 x float> %tmp2, %mul             ; <<2 x float>> [#uses=1]
  store <2 x float> %add, <2 x float>* %z
  %tmp8 = load <2 x float>* %z                    ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp8, <2 x float>* %z0
  store i32 0, i32* %n
  store <4 x float> <float 0.000000e+000, float 0.000000e+000, float 0.000000e+000, float 1.000000e+000>, <4 x float>* %dst
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %tmp11 = load i32* %n                           ; <i32> [#uses=1]
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 6 ; <i32 addrspace(2)*> [#uses=1]
  %tmp14 = load i32 addrspace(2)* %tmp13          ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp11, %tmp14              ; <i1> [#uses=1]
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %tmp15 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp16 = extractelement <2 x float> %tmp15, i32 0 ; <float> [#uses=1]
  %tmp17 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp18 = extractelement <2 x float> %tmp17, i32 0 ; <float> [#uses=1]
  %mul19 = fmul float %tmp16, %tmp18              ; <float> [#uses=1]
  store float %mul19, float* %aa
  %tmp20 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp21 = extractelement <2 x float> %tmp20, i32 1 ; <float> [#uses=1]
  %tmp22 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp23 = extractelement <2 x float> %tmp22, i32 1 ; <float> [#uses=1]
  %mul24 = fmul float %tmp21, %tmp23              ; <float> [#uses=1]
  store float %mul24, float* %bb
  %tmp25 = load float* %aa                        ; <float> [#uses=1]
  %tmp26 = load float* %bb                        ; <float> [#uses=1]
  %add27 = fadd float %tmp25, %tmp26              ; <float> [#uses=1]
  %cmp28 = fcmp ogt float %add27, 1.000000e+001   ; <i1> [#uses=1]
  br i1 %cmp28, label %if.then, label %if.end

if.then:                                          ; preds = %while.body
  %tmp29 = load i32* %n                           ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp29 to float              ; <float> [#uses=1]
  %tmp30 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp31 = getelementptr inbounds %struct.anon addrspace(2)* %tmp30, i32 0, i32 9 ; <float addrspace(2)*> [#uses=1]
  %tmp32 = load float addrspace(2)* %tmp31        ; <float> [#uses=1]
  %mul33 = fmul float %conv, %tmp32               ; <float> [#uses=1]
  %conv34 = fpext float %mul33 to double          ; <double> [#uses=1]
  %sub = fsub double 1.000000e+000, %conv34       ; <double> [#uses=1]
  %conv35 = fptrunc double %sub to float          ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %conv35, i32 0 ; <<4 x float>> [#uses=1]
  %tmp36 = load i32* %n                           ; <i32> [#uses=1]
  %conv37 = sitofp i32 %tmp36 to float            ; <float> [#uses=1]
  %tmp38 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp39 = getelementptr inbounds %struct.anon addrspace(2)* %tmp38, i32 0, i32 9 ; <float addrspace(2)*> [#uses=1]
  %tmp40 = load float addrspace(2)* %tmp39        ; <float> [#uses=1]
  %mul41 = fmul float %conv37, %tmp40             ; <float> [#uses=1]
  %conv42 = fpext float %mul41 to double          ; <double> [#uses=1]
  %sub43 = fsub double 1.000000e+000, %conv42     ; <double> [#uses=1]
  %conv44 = fptrunc double %sub43 to float        ; <float> [#uses=1]
  %vecinit45 = insertelement <4 x float> %vecinit, float %conv44, i32 1 ; <<4 x float>> [#uses=1]
  %tmp46 = load i32* %n                           ; <i32> [#uses=1]
  %conv47 = sitofp i32 %tmp46 to float            ; <float> [#uses=1]
  %tmp48 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp49 = getelementptr inbounds %struct.anon addrspace(2)* %tmp48, i32 0, i32 9 ; <float addrspace(2)*> [#uses=1]
  %tmp50 = load float addrspace(2)* %tmp49        ; <float> [#uses=1]
  %mul51 = fmul float %conv47, %tmp50             ; <float> [#uses=1]
  %conv52 = fpext float %mul51 to double          ; <double> [#uses=1]
  %sub53 = fsub double 1.000000e+000, %conv52     ; <double> [#uses=1]
  %conv54 = fptrunc double %sub53 to float        ; <float> [#uses=1]
  %vecinit55 = insertelement <4 x float> %vecinit45, float %conv54, i32 2 ; <<4 x float>> [#uses=1]
  %tmp56 = load i32* %n                           ; <i32> [#uses=1]
  %conv57 = sitofp i32 %tmp56 to float            ; <float> [#uses=1]
  %tmp58 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp59 = getelementptr inbounds %struct.anon addrspace(2)* %tmp58, i32 0, i32 9 ; <float addrspace(2)*> [#uses=1]
  %tmp60 = load float addrspace(2)* %tmp59        ; <float> [#uses=1]
  %mul61 = fmul float %conv57, %tmp60             ; <float> [#uses=1]
  %conv62 = fpext float %mul61 to double          ; <double> [#uses=1]
  %sub63 = fsub double 1.000000e+000, %conv62     ; <double> [#uses=1]
  %conv64 = fptrunc double %sub63 to float        ; <float> [#uses=1]
  %vecinit65 = insertelement <4 x float> %vecinit55, float %conv64, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit65, <4 x float>* %.compoundliteral
  %tmp66 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp66, <4 x float>* %p
  %tmp67 = load <4 x float>* %p                   ; <<4 x float>> [#uses=1]
  %tmp68 = insertelement <4 x float> %tmp67, float 1.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68, <4 x float>* %p
  %tmp69 = load <4 x float>* %p                   ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69, <4 x float>* %dst
  br label %while.end

if.end:                                           ; preds = %while.body
  %tmp70 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp71 = extractelement <2 x float> %tmp70, i32 0 ; <float> [#uses=1]
  %conv72 = fpext float %tmp71 to double          ; <double> [#uses=1]
  %mul73 = fmul double 2.000000e+000, %conv72     ; <double> [#uses=1]
  %tmp74 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp75 = extractelement <2 x float> %tmp74, i32 1 ; <float> [#uses=1]
  %conv76 = fpext float %tmp75 to double          ; <double> [#uses=1]
  %mul77 = fmul double %mul73, %conv76            ; <double> [#uses=1]
  %conv78 = fptrunc double %mul77 to float        ; <float> [#uses=1]
  store float %conv78, float* %twoab
  %tmp79 = load float* %aa                        ; <float> [#uses=1]
  %tmp80 = load float* %bb                        ; <float> [#uses=1]
  %sub81 = fsub float %tmp79, %tmp80              ; <float> [#uses=1]
  %tmp82 = load <2 x float>* %z0                  ; <<2 x float>> [#uses=1]
  %tmp83 = extractelement <2 x float> %tmp82, i32 0 ; <float> [#uses=1]
  %add84 = fadd float %sub81, %tmp83              ; <float> [#uses=1]
  %tmp85 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp86 = insertelement <2 x float> %tmp85, float %add84, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp86, <2 x float>* %z
  %tmp87 = load float* %twoab                     ; <float> [#uses=1]
  %tmp88 = load <2 x float>* %z0                  ; <<2 x float>> [#uses=1]
  %tmp89 = extractelement <2 x float> %tmp88, i32 1 ; <float> [#uses=1]
  %add90 = fadd float %tmp87, %tmp89              ; <float> [#uses=1]
  %tmp91 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp92 = insertelement <2 x float> %tmp91, float %add90, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp92, <2 x float>* %z
  %tmp93 = load i32* %n                           ; <i32> [#uses=1]
  %add94 = add nsw i32 %tmp93, 1                  ; <i32> [#uses=1]
  store i32 %add94, i32* %n
  br label %while.cond

while.end:                                        ; preds = %if.then, %while.cond
  %tmp95 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp95, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

define void @Mandelbrot2D(<4 x float> addrspace(1)* %output, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %gid0_col = alloca i32, align 4                 ; <i32*> [#uses=3]
  %gid1_row = alloca i32, align 4                 ; <i32*> [#uses=3]
  %imgWidth = alloca i32, align 4                 ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_col
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid1_row
  %call2 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call2, i32* %imgWidth
  %tmp = load i32* %gid1_row                      ; <i32> [#uses=1]
  %tmp3 = load i32* %imgWidth                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp3                      ; <i32> [#uses=1]
  %tmp4 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp4                  ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp6 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp6 to float               ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv, i32 0 ; <<2 x float>> [#uses=1]
  %tmp7 = load i32* %gid1_row                     ; <i32> [#uses=1]
  %conv8 = sitofp i32 %tmp7 to float              ; <float> [#uses=1]
  %vecinit9 = insertelement <2 x float> %vecinit, float %conv8, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit9, <2 x float>* %curCrd
  %tmp10 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp11 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call12 = call <4 x float> @evaluatePixel(<2 x float> %tmp10, %struct.anon addrspace(2)* %tmp11) ; <<4 x float>> [#uses=1]
  %tmp13 = load i32* %index                       ; <i32> [#uses=1]
  %tmp14 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp14, i32 %tmp13 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call12, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

define void @Mandelbrot(<4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=5]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=7]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load i32* %row                          ; <i32> [#uses=1]
  %tmp4 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  %tmp5 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp6 = getelementptr inbounds %struct.anon addrspace(2)* %tmp5, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp7 = load <2 x i32> addrspace(2)* %tmp6      ; <<2 x i32>> [#uses=1]
  %tmp8 = extractelement <2 x i32> %tmp7, i32 1   ; <i32> [#uses=1]
  %call9 = call i32 @_Z3minjj(i32 %add, i32 %tmp8) ; <i32> [#uses=1]
  store i32 %call9, i32* %lastRow
  %tmp11 = load i32* %row                         ; <i32> [#uses=1]
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp14 = load <2 x i32> addrspace(2)* %tmp13    ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 0 ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp11, %tmp15                 ; <i32> [#uses=1]
  store i32 %mul16, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc44, %entry
  %tmp17 = load i32* %row                         ; <i32> [#uses=1]
  %tmp18 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp17, %tmp18              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end47

for.body:                                         ; preds = %for.cond
  %tmp20 = load i32* %row                         ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp20 to float              ; <float> [#uses=1]
  %tmp21 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp22 = insertelement <2 x float> %tmp21, float %conv, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp22, <2 x float>* %curCrd
  store i32 0, i32* %col
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc, %for.body
  %tmp25 = load i32* %col                         ; <i32> [#uses=1]
  %tmp26 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp27 = getelementptr inbounds %struct.anon addrspace(2)* %tmp26, i32 0, i32 0 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp28 = load <2 x i32> addrspace(2)* %tmp27    ; <<2 x i32>> [#uses=1]
  %tmp29 = extractelement <2 x i32> %tmp28, i32 0 ; <i32> [#uses=1]
  %cmp30 = icmp ult i32 %tmp25, %tmp29            ; <i1> [#uses=1]
  br i1 %cmp30, label %for.body32, label %for.end

for.body32:                                       ; preds = %for.cond24
  %tmp33 = load i32* %col                         ; <i32> [#uses=1]
  %conv34 = uitofp i32 %tmp33 to float            ; <float> [#uses=1]
  %tmp35 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp36 = insertelement <2 x float> %tmp35, float %conv34, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp36, <2 x float>* %curCrd
  %tmp37 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp38 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call39 = call <4 x float> @evaluatePixel(<2 x float> %tmp37, %struct.anon addrspace(2)* %tmp38) ; <<4 x float>> [#uses=1]
  %tmp40 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add i32 %tmp40, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp41 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp41, i32 %tmp40 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call39, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body32
  %tmp42 = load i32* %col                         ; <i32> [#uses=1]
  %inc43 = add i32 %tmp42, 1                      ; <i32> [#uses=1]
  store i32 %inc43, i32* %col
  br label %for.cond24

for.end:                                          ; preds = %for.cond24
  br label %for.inc44

for.inc44:                                        ; preds = %for.end
  %tmp45 = load i32* %row                         ; <i32> [#uses=1]
  %inc46 = add i32 %tmp45, 1                      ; <i32> [#uses=1]
  store i32 %inc46, i32* %row
  br label %for.cond

for.end47:                                        ; preds = %for.cond
  ret void
}

declare i32 @_Z3minjj(i32, i32)
