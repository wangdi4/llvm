; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_mandelbrot.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"2000\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @mandelbrot
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @mandelbrot(i32 addrspace(1)* %mandelbrotImage, float %scale, i32 %maxIterations, i32 %width, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %ashr = ashr i32 %width, 31
  %cmp = icmp eq i32 %ashr, %width
  %nonzero = select i1 %cmp, i32 1, i32 %width
  %rem = srem i32 %call, %nonzero
  %div = sdiv i32 %call, %nonzero
  %neg = sub i32 0, %call
  %gdiv = select i1 %cmp, i32 %neg, i32 %div
  %0 = sitofp i32 %rem to float
  %conv = select i1 %cmp, float 0.000000e+00, float %0
  %mul = fmul float %conv, %scale
  %div12 = fmul float %scale, 5.000000e-01
  %conv14 = sitofp i32 %width to float
  %mul15 = fmul float %div12, %conv14
  %sub = fsub float %mul, %mul15
  %div18 = fdiv float %sub, %conv14
  %conv21 = sitofp i32 %gdiv to float
  %mul23 = fmul float %conv21, %scale
  %sub29 = fsub float %mul23, %mul15
  %div32 = fdiv float %sub29, %conv14
  %mul40 = fmul float %div18, %div18
  %mul44 = fmul float %div32, %div32
  %mul48 = fmul float %scale, %scale
  %add5 = fadd float %mul40, %mul44
  %cmp536 = fcmp ole float %add5, %mul48
  %cmp577 = icmp ne i32 %maxIterations, 0
  %or.cond8 = and i1 %cmp536, %cmp577
  br i1 %or.cond8, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge13 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %add69112 = phi float [ %add69, %for.body ], [ %div18, %for.body.preheader ]
  %tmp73211 = phi float [ %add64, %for.body ], [ %div32, %for.body.preheader ]
  %mul72310 = phi float [ %mul72, %for.body ], [ %mul40, %for.body.preheader ]
  %mul7549 = phi float [ %mul75, %for.body ], [ %mul44, %for.body.preheader ]
  %mul60 = fmul float %add69112, 2.000000e+00
  %mul62 = fmul float %mul60, %tmp73211
  %add64 = fadd float %mul62, %div32
  %sub67 = fsub float %mul72310, %mul7549
  %add69 = fadd float %sub67, %div18
  %mul72 = fmul float %add69, %add69
  %mul75 = fmul float %add64, %add64
  %inc = add i32 %storemerge13, 1
  %add = fadd float %mul72, %mul75
  %cmp53 = fcmp ole float %add, %mul48
  %cmp57 = icmp ult i32 %inc, %maxIterations
  %or.cond = and i1 %cmp53, %cmp57
  br i1 %or.cond, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %phitmp = mul i32 %inc, 255
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %storemerge.lcssa = phi i32 [ %phitmp, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  %arrayidx = getelementptr i32 addrspace(1)* %mandelbrotImage, i32 %call
  %nonzero83 = select i1 %cmp577, i32 %maxIterations, i32 1
  %div84 = udiv i32 %storemerge.lcssa, %nonzero83
  store i32 %div84, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

declare i32 @get_global_id(i32)
