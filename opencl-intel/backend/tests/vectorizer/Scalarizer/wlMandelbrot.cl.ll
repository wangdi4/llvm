; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlMandelbrot.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_mandelbrotClassic_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_mandelbrotClassic_parameters = appending global [76 x i8] c"int __attribute__((address_space(1))) *, float const, uint const, int const\00", section "llvm.metadata" ; <[76 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, float, i32, i32)* @mandelbrotClassic to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_mandelbrotClassic_locals to i8*), i8* getelementptr inbounds ([76 x i8]* @opencl_mandelbrotClassic_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @mandelbrotClassic(i32 addrspace(1)* %mandelbrotImage, float %scale, i32 %maxIterations, i32 %width) nounwind {
entry:
  %mandelbrotImage.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %scale.addr = alloca float, align 4             ; <float*> [#uses=7]
  %maxIterations.addr = alloca i32, align 4       ; <i32*> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=7]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=2]
  %j = alloca i32, align 4                        ; <i32*> [#uses=2]
  %x0 = alloca float, align 4                     ; <float*> [#uses=3]
  %y0 = alloca float, align 4                     ; <float*> [#uses=3]
  %x = alloca float, align 4                      ; <float*> [#uses=7]
  %y = alloca float, align 4                      ; <float*> [#uses=7]
  %x2 = alloca float, align 4                     ; <float*> [#uses=4]
  %y2 = alloca float, align 4                     ; <float*> [#uses=4]
  %scaleSquare = alloca float, align 4            ; <float*> [#uses=2]
  %iter = alloca i32, align 4                     ; <i32*> [#uses=6]
  store i32 addrspace(1)* %mandelbrotImage, i32 addrspace(1)** %mandelbrotImage.addr
  store float %scale, float* %scale.addr
  store i32 %maxIterations, i32* %maxIterations.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %tmp = load i32* %tid                           ; <i32> [#uses=1]
  %tmp1 = load i32* %width.addr                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp1                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp1         ; <i32> [#uses=1]
  %rem = srem i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %rem, i32* %i
  %tmp3 = load i32* %tid                          ; <i32> [#uses=1]
  %tmp4 = load i32* %width.addr                   ; <i32> [#uses=2]
  %cmp5 = icmp eq i32 0, %tmp4                    ; <i1> [#uses=1]
  %sel6 = select i1 %cmp5, i32 1, i32 %tmp4       ; <i32> [#uses=1]
  %div = sdiv i32 %tmp3, %sel6                    ; <i32> [#uses=1]
  store i32 %div, i32* %j
  %tmp8 = load i32* %i                            ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp8 to float               ; <float> [#uses=1]
  %tmp9 = load float* %scale.addr                 ; <float> [#uses=1]
  %mul = fmul float %conv, %tmp9                  ; <float> [#uses=1]
  %tmp10 = load float* %scale.addr                ; <float> [#uses=1]
  %div11 = fdiv float %tmp10, 2.000000e+000       ; <float> [#uses=1]
  %tmp12 = load i32* %width.addr                  ; <i32> [#uses=1]
  %conv13 = sitofp i32 %tmp12 to float            ; <float> [#uses=1]
  %mul14 = fmul float %div11, %conv13             ; <float> [#uses=1]
  %sub = fsub float %mul, %mul14                  ; <float> [#uses=1]
  %tmp15 = load i32* %width.addr                  ; <i32> [#uses=1]
  %conv16 = sitofp i32 %tmp15 to float            ; <float> [#uses=3]
  %cmp17 = fcmp oeq float 0.000000e+000, %conv16  ; <i1> [#uses=1]
  %sel18 = select i1 %cmp17, float 1.000000e+000, float %conv16 ; <float> [#uses=0]
  %div19 = fdiv float %sub, %conv16               ; <float> [#uses=1]
  store float %div19, float* %x0
  %tmp21 = load i32* %j                           ; <i32> [#uses=1]
  %conv22 = sitofp i32 %tmp21 to float            ; <float> [#uses=1]
  %tmp23 = load float* %scale.addr                ; <float> [#uses=1]
  %mul24 = fmul float %conv22, %tmp23             ; <float> [#uses=1]
  %tmp25 = load float* %scale.addr                ; <float> [#uses=1]
  %div26 = fdiv float %tmp25, 2.000000e+000       ; <float> [#uses=1]
  %tmp27 = load i32* %width.addr                  ; <i32> [#uses=1]
  %conv28 = sitofp i32 %tmp27 to float            ; <float> [#uses=1]
  %mul29 = fmul float %div26, %conv28             ; <float> [#uses=1]
  %sub30 = fsub float %mul24, %mul29              ; <float> [#uses=1]
  %tmp31 = load i32* %width.addr                  ; <i32> [#uses=1]
  %conv32 = sitofp i32 %tmp31 to float            ; <float> [#uses=3]
  %cmp33 = fcmp oeq float 0.000000e+000, %conv32  ; <i1> [#uses=1]
  %sel34 = select i1 %cmp33, float 1.000000e+000, float %conv32 ; <float> [#uses=0]
  %div35 = fdiv float %sub30, %conv32             ; <float> [#uses=1]
  store float %div35, float* %y0
  %tmp37 = load float* %x0                        ; <float> [#uses=1]
  store float %tmp37, float* %x
  %tmp39 = load float* %y0                        ; <float> [#uses=1]
  store float %tmp39, float* %y
  %tmp41 = load float* %x                         ; <float> [#uses=1]
  %tmp42 = load float* %x                         ; <float> [#uses=1]
  %mul43 = fmul float %tmp41, %tmp42              ; <float> [#uses=1]
  store float %mul43, float* %x2
  %tmp45 = load float* %y                         ; <float> [#uses=1]
  %tmp46 = load float* %y                         ; <float> [#uses=1]
  %mul47 = fmul float %tmp45, %tmp46              ; <float> [#uses=1]
  store float %mul47, float* %y2
  %tmp49 = load float* %scale.addr                ; <float> [#uses=1]
  %tmp50 = load float* %scale.addr                ; <float> [#uses=1]
  %mul51 = fmul float %tmp49, %tmp50              ; <float> [#uses=1]
  store float %mul51, float* %scaleSquare
  store i32 0, i32* %iter
  store i32 0, i32* %iter
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp53 = load float* %x2                        ; <float> [#uses=1]
  %tmp54 = load float* %y2                        ; <float> [#uses=1]
  %add = fadd float %tmp53, %tmp54                ; <float> [#uses=1]
  %tmp55 = load float* %scaleSquare               ; <float> [#uses=1]
  %cmp56 = fcmp ole float %add, %tmp55            ; <i1> [#uses=1]
  br i1 %cmp56, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %for.cond
  %tmp58 = load i32* %iter                        ; <i32> [#uses=1]
  %tmp59 = load i32* %maxIterations.addr          ; <i32> [#uses=1]
  %cmp60 = icmp ult i32 %tmp58, %tmp59            ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.cond
  %0 = phi i1 [ false, %for.cond ], [ %cmp60, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %for.body, label %for.end

for.body:                                         ; preds = %land.end
  %tmp62 = load float* %x                         ; <float> [#uses=1]
  %mul63 = fmul float 2.000000e+000, %tmp62       ; <float> [#uses=1]
  %tmp64 = load float* %y                         ; <float> [#uses=1]
  %mul65 = fmul float %mul63, %tmp64              ; <float> [#uses=1]
  %tmp66 = load float* %y0                        ; <float> [#uses=1]
  %add67 = fadd float %mul65, %tmp66              ; <float> [#uses=1]
  store float %add67, float* %y
  %tmp68 = load float* %x2                        ; <float> [#uses=1]
  %tmp69 = load float* %y2                        ; <float> [#uses=1]
  %sub70 = fsub float %tmp68, %tmp69              ; <float> [#uses=1]
  %tmp71 = load float* %x0                        ; <float> [#uses=1]
  %add72 = fadd float %sub70, %tmp71              ; <float> [#uses=1]
  store float %add72, float* %x
  %tmp73 = load float* %x                         ; <float> [#uses=1]
  %tmp74 = load float* %x                         ; <float> [#uses=1]
  %mul75 = fmul float %tmp73, %tmp74              ; <float> [#uses=1]
  store float %mul75, float* %x2
  %tmp76 = load float* %y                         ; <float> [#uses=1]
  %tmp77 = load float* %y                         ; <float> [#uses=1]
  %mul78 = fmul float %tmp76, %tmp77              ; <float> [#uses=1]
  store float %mul78, float* %y2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp79 = load i32* %iter                        ; <i32> [#uses=1]
  %inc = add i32 %tmp79, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %iter
  br label %for.cond

for.end:                                          ; preds = %land.end
  %tmp80 = load i32* %iter                        ; <i32> [#uses=1]
  %mul81 = mul i32 255, %tmp80                    ; <i32> [#uses=1]
  %tmp82 = load i32* %maxIterations.addr          ; <i32> [#uses=2]
  %cmp83 = icmp eq i32 0, %tmp82                  ; <i1> [#uses=1]
  %sel84 = select i1 %cmp83, i32 1, i32 %tmp82    ; <i32> [#uses=1]
  %div85 = udiv i32 %mul81, %sel84                ; <i32> [#uses=1]
  %tmp86 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp87 = load i32 addrspace(1)** %mandelbrotImage.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp87, i32 %tmp86 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %div85, i32 addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)
