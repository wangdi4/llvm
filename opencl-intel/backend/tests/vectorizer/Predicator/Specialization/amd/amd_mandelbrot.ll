; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_mandelbrot.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"2000\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, float, i32, i32, ...)* @mandelbrot to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @mandelbrot
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                           ; preds = %header{{[0-9]*}}

define void @mandelbrot(i32 addrspace(1)* %mandelbrotImage, float %scale, i32 %maxIterations, i32 %width, ...) nounwind {
entry:
	%mandelbrotImage.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%scale.addr = alloca float		; <float*> [#uses=7]
	%maxIterations.addr = alloca i32		; <i32*> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=7]
	%tid = alloca i32, align 4		; <i32*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=2]
	%j = alloca i32, align 4		; <i32*> [#uses=2]
	%x0 = alloca float, align 4		; <float*> [#uses=3]
	%y0 = alloca float, align 4		; <float*> [#uses=3]
	%x = alloca float, align 4		; <float*> [#uses=7]
	%y = alloca float, align 4		; <float*> [#uses=7]
	%x2 = alloca float, align 4		; <float*> [#uses=4]
	%y2 = alloca float, align 4		; <float*> [#uses=4]
	%scaleSquare = alloca float, align 4		; <float*> [#uses=2]
	%iter = alloca i32, align 4		; <i32*> [#uses=6]
	store i32 addrspace(1)* %mandelbrotImage, i32 addrspace(1)** %mandelbrotImage.addr
	store float %scale, float* %scale.addr
	store i32 %maxIterations, i32* %maxIterations.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %tid
	%tmp = load i32* %tid		; <i32> [#uses=1]
	%tmp1 = load i32* %width.addr		; <i32> [#uses=3]
	%ashr = ashr i32 %tmp1, 31		; <i32> [#uses=1]
	%cmp = icmp eq i32 %ashr, %tmp1		; <i1> [#uses=2]
	%nonzero = select i1 %cmp, i32 1, i32 %tmp1		; <i32> [#uses=1]
	%rem = srem i32 %tmp, %nonzero		; <i32> [#uses=1]
	%grem = select i1 %cmp, i32 0, i32 %rem		; <i32> [#uses=1]
	store i32 %grem, i32* %i
	%tmp3 = load i32* %tid		; <i32> [#uses=2]
	%tmp4 = load i32* %width.addr		; <i32> [#uses=3]
	%ashr5 = ashr i32 %tmp4, 31		; <i32> [#uses=1]
	%cmp6 = icmp eq i32 %ashr5, %tmp4		; <i1> [#uses=2]
	%nonzero7 = select i1 %cmp6, i32 1, i32 %tmp4		; <i32> [#uses=1]
	%div = sdiv i32 %tmp3, %nonzero7		; <i32> [#uses=1]
	%neg = sub i32 0, %tmp3		; <i32> [#uses=1]
	%gdiv = select i1 %cmp6, i32 %neg, i32 %div		; <i32> [#uses=1]
	store i32 %gdiv, i32* %j
	%tmp9 = load i32* %i		; <i32> [#uses=1]
	%conv = sitofp i32 %tmp9 to float		; <float> [#uses=1]
	%tmp10 = load float* %scale.addr		; <float> [#uses=1]
	%mul = fmul float %conv, %tmp10		; <float> [#uses=1]
	%tmp11 = load float* %scale.addr		; <float> [#uses=1]
	%div12 = fdiv float %tmp11, 2.000000e+000		; <float> [#uses=1]
	%tmp13 = load i32* %width.addr		; <i32> [#uses=1]
	%conv14 = sitofp i32 %tmp13 to float		; <float> [#uses=1]
	%mul15 = fmul float %div12, %conv14		; <float> [#uses=1]
	%sub = fsub float %mul, %mul15		; <float> [#uses=1]
	%tmp16 = load i32* %width.addr		; <i32> [#uses=1]
	%conv17 = sitofp i32 %tmp16 to float		; <float> [#uses=1]
	%div18 = fdiv float %sub, %conv17		; <float> [#uses=1]
	store float %div18, float* %x0
	%tmp20 = load i32* %j		; <i32> [#uses=1]
	%conv21 = sitofp i32 %tmp20 to float		; <float> [#uses=1]
	%tmp22 = load float* %scale.addr		; <float> [#uses=1]
	%mul23 = fmul float %conv21, %tmp22		; <float> [#uses=1]
	%tmp24 = load float* %scale.addr		; <float> [#uses=1]
	%div25 = fdiv float %tmp24, 2.000000e+000		; <float> [#uses=1]
	%tmp26 = load i32* %width.addr		; <i32> [#uses=1]
	%conv27 = sitofp i32 %tmp26 to float		; <float> [#uses=1]
	%mul28 = fmul float %div25, %conv27		; <float> [#uses=1]
	%sub29 = fsub float %mul23, %mul28		; <float> [#uses=1]
	%tmp30 = load i32* %width.addr		; <i32> [#uses=1]
	%conv31 = sitofp i32 %tmp30 to float		; <float> [#uses=1]
	%div32 = fdiv float %sub29, %conv31		; <float> [#uses=1]
	store float %div32, float* %y0
	%tmp34 = load float* %x0		; <float> [#uses=1]
	store float %tmp34, float* %x
	%tmp36 = load float* %y0		; <float> [#uses=1]
	store float %tmp36, float* %y
	%tmp38 = load float* %x		; <float> [#uses=1]
	%tmp39 = load float* %x		; <float> [#uses=1]
	%mul40 = fmul float %tmp38, %tmp39		; <float> [#uses=1]
	store float %mul40, float* %x2
	%tmp42 = load float* %y		; <float> [#uses=1]
	%tmp43 = load float* %y		; <float> [#uses=1]
	%mul44 = fmul float %tmp42, %tmp43		; <float> [#uses=1]
	store float %mul44, float* %y2
	%tmp46 = load float* %scale.addr		; <float> [#uses=1]
	%tmp47 = load float* %scale.addr		; <float> [#uses=1]
	%mul48 = fmul float %tmp46, %tmp47		; <float> [#uses=1]
	store float %mul48, float* %scaleSquare
	store i32 0, i32* %iter
	store i32 0, i32* %iter
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp50 = load float* %x2		; <float> [#uses=1]
	%tmp51 = load float* %y2		; <float> [#uses=1]
	%add = fadd float %tmp50, %tmp51		; <float> [#uses=1]
	%tmp52 = load float* %scaleSquare		; <float> [#uses=1]
	%cmp53 = fcmp ole float %add, %tmp52		; <i1> [#uses=1]
	br i1 %cmp53, label %land.lhs.true, label %for.end

land.lhs.true:		; preds = %for.cond
	%tmp55 = load i32* %iter		; <i32> [#uses=1]
	%tmp56 = load i32* %maxIterations.addr		; <i32> [#uses=1]
	%cmp57 = icmp ult i32 %tmp55, %tmp56		; <i1> [#uses=1]
	br i1 %cmp57, label %for.body, label %for.end

for.body:		; preds = %land.lhs.true
	%tmp59 = load float* %x		; <float> [#uses=1]
	%mul60 = fmul float 2.000000e+000, %tmp59		; <float> [#uses=1]
	%tmp61 = load float* %y		; <float> [#uses=1]
	%mul62 = fmul float %mul60, %tmp61		; <float> [#uses=1]
	%tmp63 = load float* %y0		; <float> [#uses=1]
	%add64 = fadd float %mul62, %tmp63		; <float> [#uses=1]
	store float %add64, float* %y
	%tmp65 = load float* %x2		; <float> [#uses=1]
	%tmp66 = load float* %y2		; <float> [#uses=1]
	%sub67 = fsub float %tmp65, %tmp66		; <float> [#uses=1]
	%tmp68 = load float* %x0		; <float> [#uses=1]
	%add69 = fadd float %sub67, %tmp68		; <float> [#uses=1]
	store float %add69, float* %x
	%tmp70 = load float* %x		; <float> [#uses=1]
	%tmp71 = load float* %x		; <float> [#uses=1]
	%mul72 = fmul float %tmp70, %tmp71		; <float> [#uses=1]
	store float %mul72, float* %x2
	%tmp73 = load float* %y		; <float> [#uses=1]
	%tmp74 = load float* %y		; <float> [#uses=1]
	%mul75 = fmul float %tmp73, %tmp74		; <float> [#uses=1]
	store float %mul75, float* %y2
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp76 = load i32* %iter		; <i32> [#uses=1]
	%inc = add i32 %tmp76, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %iter
	br label %for.cond

for.end:		; preds = %land.lhs.true, %for.cond
	%tmp77 = load i32* %tid		; <i32> [#uses=1]
	%tmp78 = load i32 addrspace(1)** %mandelbrotImage.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i32 addrspace(1)* %tmp78, i32 %tmp77		; <i32 addrspace(1)*> [#uses=1]
	%tmp79 = load i32* %iter		; <i32> [#uses=1]
	%mul80 = mul i32 255, %tmp79		; <i32> [#uses=1]
	%tmp81 = load i32* %maxIterations.addr		; <i32> [#uses=2]
	%cmp82 = icmp ne i32 %tmp81, 0		; <i1> [#uses=1]
	%nonzero83 = select i1 %cmp82, i32 %tmp81, i32 1		; <i32> [#uses=1]
	%div84 = udiv i32 %mul80, %nonzero83		; <i32> [#uses=1]
	store i32 %div84, i32 addrspace(1)* %arrayidx
	ret void
}

declare i32 @get_global_id(i32)
