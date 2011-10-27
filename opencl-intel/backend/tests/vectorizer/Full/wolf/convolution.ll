; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_simpleConvolution_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata"
@opencl_simpleConvolution_parameters = appending global [153 x i8] c"float __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint2 const, uint2 const\00", section "llvm.metadata"
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32 addrspace(1)*, float addrspace(1)*, <2 x i32>, <2 x i32>)* @simpleConvolution to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_simpleConvolution_locals to i8*), i8* getelementptr inbounds ([153 x i8]* @opencl_simpleConvolution_parameters, i32 0, i32 0) }>], section "llvm.metadata"

define void @simpleConvolution(float addrspace(1)* nocapture %output, i32 addrspace(1)* nocapture %input, float addrspace(1)* nocapture %mask, <2 x i32> %inputDimensions, <2 x i32> %maskDimensions) nounwind {
; <label>:0
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = extractelement <2 x i32> %inputDimensions, i32 0
  %3 = extractelement <2 x i32> %inputDimensions, i32 1
  %4 = icmp eq i32 %2, 0
  %5 = select i1 %4, i32 1, i32 %2
  %6 = urem i32 %1, %5
  %7 = udiv i32 %1, %5
  %8 = extractelement <2 x i32> %maskDimensions, i32 0
  %9 = extractelement <2 x i32> %maskDimensions, i32 1
  %10 = add i32 %8, -1
  %11 = lshr i32 %10, 1
  %12 = add i32 %9, -1
  %13 = lshr i32 %12, 1
  %14 = icmp ult i32 %6, %11
  %15 = sub i32 %6, %11
  %16 = select i1 %14, i32 0, i32 %15
  %17 = add i32 %6, %11
  %18 = icmp uge i32 %17, %2
  %19 = add i32 %2, -1
  %20 = select i1 %18, i32 %19, i32 %17
  %21 = add i32 %7, %13
  %22 = icmp uge i32 %21, %3
  %23 = add i32 %3, -1
  %24 = select i1 %22, i32 %23, i32 %21
  %25 = icmp ugt i32 %16, %20
  br i1 %25, label %._crit_edge7, label %bb.nph6

bb.nph6:                                          ; preds = %0
  %26 = sub i32 %7, %13
  %27 = icmp ult i32 %7, %13
  %28 = select i1 %27, i32 0, i32 %26
  %29 = icmp ugt i32 %28, %24
  br i1 %29, label %bb.nph6.split.us, label %bb.nph6.bb.nph6.split_crit_edge

bb.nph6.bb.nph6.split_crit_edge:                  ; preds = %bb.nph6
  %tmp23 = icmp ugt i32 %2, 1
  %umax24 = select i1 %tmp23, i32 %2, i32 1
  %tmp25 = udiv i32 %1, %umax24
  %tmp28 = icmp ugt i32 %tmp25, %13
  %umax29 = select i1 %tmp28, i32 %tmp25, i32 %13
  %tmp44 = add i32 %umax29, 1
  %tmp45 = sub i32 %tmp44, %13
  %tmp50 = icmp ugt i32 %6, %11
  %umax51 = select i1 %tmp50, i32 %6, i32 %11
  %tmp59 = sub i32 %umax29, %13
  %tmp60 = mul i32 %2, %tmp59
  %tmp61 = add i32 %umax51, %tmp60
  %tmp62 = sub i32 %tmp61, %11
  %tmp67 = sub i32 %umax29, %tmp25
  %tmp68 = mul i32 %8, %tmp67
  %tmp69 = add i32 %umax51, %tmp68
  %tmp70 = sub i32 %tmp69, %6
  %tmp74 = add i32 %umax51, 1
  %tmp75 = sub i32 %tmp74, %11
  br label %bb.nph

bb.nph6.split.us:                                 ; preds = %bb.nph6
  %tmp10 = icmp ugt i32 %6, %11
  %umax = select i1 %tmp10, i32 %6, i32 %11
  %tmp11 = add i32 %umax, 1
  %tmp12 = sub i32 %tmp11, %11
  br label %30

; <label>:30                                      ; preds = %bb.nph6.split.us, %30
  %indvar = phi i32 [ %indvar.next, %30 ], [ 0, %bb.nph6.split.us ]
  %tmp13 = add i32 %tmp12, %indvar
  %31 = icmp ugt i32 %tmp13, %20
  %indvar.next = add i32 %indvar, 1
  br i1 %31, label %._crit_edge7.loopexit77, label %30

bb.nph:                                           ; preds = %._crit_edge, %bb.nph6.bb.nph6.split_crit_edge
  %indvar17 = phi i32 [ 0, %bb.nph6.bb.nph6.split_crit_edge ], [ %indvar.next18, %._crit_edge ]
  %sumFX.14 = phi float [ 0.000000e+000, %bb.nph6.bb.nph6.split_crit_edge ], [ %37, %._crit_edge ]
  %tmp63 = add i32 %tmp62, %indvar17
  %tmp71 = add i32 %tmp70, %indvar17
  %tmp76 = add i32 %tmp75, %indvar17
  br label %32

; <label>:32                                      ; preds = %bb.nph, %32
  %indvar14 = phi i32 [ 0, %bb.nph ], [ %indvar.next15, %32 ]
  %sumFX.02 = phi float [ %sumFX.14, %bb.nph ], [ %37, %32 ]
  %tmp47 = mul i32 %2, %indvar14
  %tmp64 = add i32 %tmp63, %tmp47
  %scevgep43 = getelementptr i32 addrspace(1)* %input, i32 %tmp64
  %tmp66 = mul i32 %8, %indvar14
  %tmp72 = add i32 %tmp71, %tmp66
  %scevgep = getelementptr float addrspace(1)* %mask, i32 %tmp72
  %33 = load i32 addrspace(1)* %scevgep43, align 4
  %34 = uitofp i32 %33 to float
  %35 = load float addrspace(1)* %scevgep, align 4
  %36 = fmul float %34, %35
  %37 = fadd float %sumFX.02, %36
  %tmp46 = add i32 %tmp45, %indvar14
  %38 = icmp ugt i32 %tmp46, %24
  %indvar.next15 = add i32 %indvar14, 1
  br i1 %38, label %._crit_edge, label %32

._crit_edge:                                      ; preds = %32
  %39 = icmp ugt i32 %tmp76, %20
  %indvar.next18 = add i32 %indvar17, 1
  br i1 %39, label %._crit_edge7.loopexit, label %bb.nph

._crit_edge7.loopexit:                            ; preds = %._crit_edge
  br label %._crit_edge7

._crit_edge7.loopexit77:                          ; preds = %30
  br label %._crit_edge7

._crit_edge7:                                     ; preds = %._crit_edge7.loopexit77, %._crit_edge7.loopexit, %0
  %sumFX.1.lcssa = phi float [ 0.000000e+000, %0 ], [ %37, %._crit_edge7.loopexit ], [ 0.000000e+000, %._crit_edge7.loopexit77 ]
  %40 = fadd float %sumFX.1.lcssa, 5.000000e-001
  %41 = getelementptr inbounds float addrspace(1)* %output, i32 %1
  store float %40, float addrspace(1)* %41, align 4
; CHECK: ret
  ret void
}

declare i32 @get_global_id(i32)

