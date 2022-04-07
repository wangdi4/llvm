; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt  -predicate %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'do_while.ll'

@gMaxIteration = global i32 500, align 4
@gDimX = global i32 1024, align 4
@gDimY = global i32 1024, align 4
@lowerBoundX = global float -2.000000e+00, align 4
@lowerBoundY = global float -2.000000e+00, align 4
@scaleFactor = global float 4.000000e+00, align 4

declare i32 @_Z13get_global_idj(i32)

; CHECK: @root.wrapper.indexed
; CHECK-NOT: %new_phi24 = phi i8 [ %merge29, %phi-split-bb15 ], [ -1, %.critedge.i.i ]

define void @root.wrapper.indexed(<4 x i8>* nocapture %out, i32 %x, i32 %y) {
init:
  %0 = tail call i32 @_Z13get_global_idj(i32 0)
  %outElement = getelementptr inbounds <4 x i8>, <4 x i8>* %out, i32 %0
  %currentX = add i32 %0, %x
  %1 = load float, float* @lowerBoundX, align 4
  %2 = uitofp i32 %currentX to float
  %3 = load i32, i32* @gDimX, align 4
  %4 = uitofp i32 %3 to float
  %5 = fdiv float %2, %4
  %6 = load float, float* @scaleFactor, align 4
  %7 = fmul float %5, %6
  %8 = fadd float %1, %7
  %9 = load float, float* @lowerBoundY, align 4
  %10 = uitofp i32 %y to float
  %11 = load i32, i32* @gDimY, align 4
  %12 = uitofp i32 %11 to float
  %13 = fdiv float %10, %12
  %14 = fmul float %6, %13
  %15 = fadd float %9, %14
  %16 = load i32, i32* @gMaxIteration, align 4
  %17 = icmp eq i32 %16, 0
  br i1 %17, label %root.wrapper.exit, label %.lr.ph.i.i.preheader

.lr.ph.i.i.preheader:                             ; preds = %init
  br label %.lr.ph.i.i

.lr.ph.i.i:                                       ; preds = %.lr.ph.i.i.preheader, %.lr.ph.i.i
  %18 = phi float [ %27, %.lr.ph.i.i ], [ 0.000000e+00, %.lr.ph.i.i.preheader ]
  %19 = phi float [ %26, %.lr.ph.i.i ], [ 0.000000e+00, %.lr.ph.i.i.preheader ]
  %iter.04.i.i = phi i32 [ %25, %.lr.ph.i.i ], [ 0, %.lr.ph.i.i.preheader ]
  %t.03.i.i3 = phi float [ %21, %.lr.ph.i.i ], [ 0.000000e+00, %.lr.ph.i.i.preheader ]
  %t.03.i.i4 = phi float [ %24, %.lr.ph.i.i ], [ 0.000000e+00, %.lr.ph.i.i.preheader ]
  %20 = fsub float %19, %18
  %21 = fadd float %8, %20
  %22 = fmul float %t.03.i.i3, 2.000000e+00
  %23 = fmul float %t.03.i.i4, %22
  %24 = fadd float %15, %23
  %25 = add nsw i32 %iter.04.i.i, 1
  %26 = fmul float %21, %21
  %27 = fmul float %24, %24
  %28 = fadd float %26, %27
  %29 = fcmp olt float %28, 4.000000e+00
  %30 = icmp ult i32 %25, %16
  %or.cond.i.i = and i1 %29, %30
  br i1 %or.cond.i.i, label %.lr.ph.i.i, label %.critedge.i.i

.critedge.i.i:                                    ; preds = %.lr.ph.i.i
  br i1 %30, label %31, label %phi-split-bb20

; <label>:31                                      ; preds = %.critedge.i.i
  %32 = uitofp i32 %16 to float
  %33 = fdiv float %32, 3.000000e+00
  %34 = udiv i32 %16, 3
  %35 = icmp ugt i32 %25, %34
  br i1 %35, label %41, label %36

; <label>:36                                      ; preds = %31
  %37 = sitofp i32 %25 to float
  %38 = fdiv float %37, %33
  %39 = fmul float %38, 2.550000e+02
  %40 = fptoui float %39 to i8
  br label %phi-split-bb15

; <label>:41                                      ; preds = %31
  %42 = shl nuw i32 %34, 1
  %43 = icmp ugt i32 %25, %42
  %44 = sitofp i32 %25 to float
  br i1 %43, label %52, label %45

; <label>:45                                      ; preds = %41
  %46 = fsub float %44, %33
  %47 = fdiv float %46, %33
  %48 = fmul float %47, 2.550000e+02
  %49 = fsub float 2.550000e+02, %48
  %50 = fptoui float %49 to i8
  %51 = fptoui float %48 to i8
  br label %phi-split-bb

; <label>:52                                      ; preds = %41
  %53 = fmul float %33, 2.000000e+00
  %54 = fsub float %44, %53
  %55 = fdiv float %54, %33
  %56 = fmul float %55, 2.550000e+02
  %57 = fsub float 2.550000e+02, %56
  %58 = fptoui float %57 to i8
  %59 = fptoui float %56 to i8
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %45, %52
  %new_phi14 = phi i8 [ -1, %52 ], [ -1, %45 ]
  %new_phi13 = phi i8 [ %59, %52 ], [ 0, %45 ]
  %new_phi12 = phi i8 [ %58, %52 ], [ %51, %45 ]
  %new_phi = phi i8 [ 0, %52 ], [ %50, %45 ]
  br label %phi-split-bb15

phi-split-bb15:                                   ; preds = %36, %phi-split-bb
  %new_phi19 = phi i8 [ %new_phi14, %phi-split-bb ], [ -1, %36 ]
  %new_phi18 = phi i8 [ %new_phi13, %phi-split-bb ], [ 0, %36 ]
  %new_phi17 = phi i8 [ %new_phi12, %phi-split-bb ], [ 0, %36 ]
  %new_phi16 = phi i8 [ %new_phi, %phi-split-bb ], [ %40, %36 ]
  br label %phi-split-bb20

phi-split-bb20:                                   ; preds = %.critedge.i.i, %phi-split-bb15
  %new_phi24 = phi i8 [ %new_phi19, %phi-split-bb15 ], [ -1, %.critedge.i.i ]
  %new_phi23 = phi i8 [ %new_phi18, %phi-split-bb15 ], [ 0, %.critedge.i.i ]
  %new_phi22 = phi i8 [ %new_phi17, %phi-split-bb15 ], [ 0, %.critedge.i.i ]
  %new_phi21 = phi i8 [ %new_phi16, %phi-split-bb15 ], [ 0, %.critedge.i.i ]
  br label %root.wrapper.exit

root.wrapper.exit:                                ; preds = %phi-split-bb20, %init
  %.0.i.i5 = phi i8 [ 0, %init ], [ %new_phi21, %phi-split-bb20 ]
  %.0.i.i6 = phi i8 [ 0, %init ], [ %new_phi22, %phi-split-bb20 ]
  %.0.i.i7 = phi i8 [ 0, %init ], [ %new_phi23, %phi-split-bb20 ]
  %.0.i.i8 = phi i8 [ -1, %init ], [ %new_phi24, %phi-split-bb20 ]
  %assembled.vect = insertelement <4 x i8> undef, i8 %.0.i.i5, i32 0
  %assembled.vect9 = insertelement <4 x i8> %assembled.vect, i8 %.0.i.i6, i32 1
  %assembled.vect10 = insertelement <4 x i8> %assembled.vect9, i8 %.0.i.i7, i32 2
  %assembled.vect11 = insertelement <4 x i8> %assembled.vect10, i8 %.0.i.i8, i32 3
  store <4 x i8> %assembled.vect11, <4 x i8>* %outElement
  ret void
}

