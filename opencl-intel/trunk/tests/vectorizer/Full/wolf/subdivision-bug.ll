; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'C:\Users\nrotem\Desktop\runme\runtime_tests64\Subdivision.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK: ret
%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._Edge = type { [2 x i32], [2 x i32] }
%struct._Face = type { [4 x i32] }
%struct._Vertex = type { float, [3 x float], [4 x float], i32 }

@opencl_TestSubdivisionKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_TestSubdivisionKernel_parameters = appending global [220 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, float, int\00", section "llvm.metadata" ; <[220 x i8]*> [#uses=1]
@opencl_UpdateFacesKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_UpdateFacesKernel_parameters = appending global [169 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[169 x i8]*> [#uses=1]
@opencl_UpdateEdgesKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_UpdateEdgesKernel_parameters = appending global [169 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Edge __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[169 x i8]*> [#uses=1]
@opencl_GenerateFacePointsKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_GenerateFacePointsKernel_parameters = appending global [427 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int\00", section "llvm.metadata" ; <[427 x i8]*> [#uses=1]
@opencl_GenerateEdgePointsKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_GenerateEdgePointsKernel_parameters = appending global [680 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Edge __attribute__((address_space(1))) *, Edge __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[680 x i8]*> [#uses=1]
@opencl_UpdateVerticesKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_UpdateVerticesKernel_parameters = appending global [133 x i8] c"int __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[133 x i8]*> [#uses=1]
@opencl_GetScanTotalsKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_GetScanTotalsKernel_parameters = appending global [173 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int\00", section "llvm.metadata" ; <[173 x i8]*> [#uses=1]
@opencl_ParallelScanKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ParallelScanKernel_parameters = appending global [91 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_SequentialScanKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_SequentialScanKernel_parameters = appending global [86 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[86 x i8]*> [#uses=1]
@opencl_metadata = appending global [9 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, %struct._Vertex addrspace(1)*, %struct._Face addrspace(1)*, float, i32)* @TestSubdivisionKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_TestSubdivisionKernel_locals to i8*), i8* getelementptr inbounds ([220 x i8]* @opencl_TestSubdivisionKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, %struct._Face addrspace(1)*, i32)* @UpdateFacesKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_UpdateFacesKernel_locals to i8*), i8* getelementptr inbounds ([169 x i8]* @opencl_UpdateFacesKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, %struct._Edge addrspace(1)*, i32)* @UpdateEdgesKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_UpdateEdgesKernel_locals to i8*), i8* getelementptr inbounds ([169 x i8]* @opencl_UpdateEdgesKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, %struct._Face addrspace(1)*, %struct._Face addrspace(1)*, %struct._Vertex addrspace(1)*, %struct._Vertex addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @GenerateFacePointsKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_GenerateFacePointsKernel_locals to i8*), i8* getelementptr inbounds ([427 x i8]* @opencl_GenerateFacePointsKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, %struct._Edge addrspace(1)*, %struct._Edge addrspace(1)*, %struct._Face addrspace(1)*, %struct._Face addrspace(1)*, %struct._Vertex addrspace(1)*, %struct._Vertex addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, i32)* @GenerateEdgePointsKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_GenerateEdgePointsKernel_locals to i8*), i8* getelementptr inbounds ([680 x i8]* @opencl_GenerateEdgePointsKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, %struct._Vertex addrspace(1)*, %struct._Vertex addrspace(1)*, i32)* @UpdateVerticesKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_UpdateVerticesKernel_locals to i8*), i8* getelementptr inbounds ([133 x i8]* @opencl_UpdateVerticesKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @GetScanTotalsKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_GetScanTotalsKernel_locals to i8*), i8* getelementptr inbounds ([173 x i8]* @opencl_GetScanTotalsKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @ParallelScanKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ParallelScanKernel_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_ParallelScanKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @SequentialScanKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_SequentialScanKernel_locals to i8*), i8* getelementptr inbounds ([86 x i8]* @opencl_SequentialScanKernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[9 x %opencl_metadata_type]*> [#uses=0]

define i32 @NewEdgeIndex(i32 %e, i32 %m_edges, i32 addrspace(1)* nocapture %dEdge, i32 addrspace(1)* nocapture %m_pEdgeScanned) nounwind readonly {
  %1 = sext i32 %e to i64                         ; <i64> [#uses=3]
  %2 = getelementptr inbounds i32 addrspace(1)* %dEdge, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %3 = load i32 addrspace(1)* %2                  ; <i32> [#uses=1]
  %4 = icmp eq i32 %3, 1                          ; <i1> [#uses=1]
  br i1 %4, label %5, label %14

; <label>:5                                       ; preds = %0
  %6 = sext i32 %m_edges to i64                   ; <i64> [#uses=1]
  %7 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %6 ; <i32 addrspace(1)*> [#uses=1]
  %8 = load i32 addrspace(1)* %7                  ; <i32> [#uses=1]
  %9 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %10 = load i32 addrspace(1)* %9                 ; <i32> [#uses=1]
  %11 = shl i32 %10, 2                            ; <i32> [#uses=1]
  %12 = sub i32 %m_edges, %8                      ; <i32> [#uses=1]
  %13 = add nsw i32 %12, %11                      ; <i32> [#uses=1]
  br label %18

; <label>:14                                      ; preds = %0
  %15 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %16 = load i32 addrspace(1)* %15                ; <i32> [#uses=1]
  %17 = sub i32 %e, %16                           ; <i32> [#uses=1]
  br label %18

; <label>:18                                      ; preds = %14, %5
  %newEdgeIndex.0 = phi i32 [ %13, %5 ], [ %17, %14 ] ; <i32> [#uses=3]
  %19 = icmp slt i32 %newEdgeIndex.0, 0           ; <i1> [#uses=1]
  br i1 %19, label %23, label %20

; <label>:20                                      ; preds = %18
  %21 = shl i32 %m_edges, 2                       ; <i32> [#uses=1]
  %22 = icmp slt i32 %21, %newEdgeIndex.0         ; <i1> [#uses=1]
  %retval = select i1 %22, i32 0, i32 %newEdgeIndex.0 ; <i32> [#uses=1]
  ret i32 %retval

; <label>:23                                      ; preds = %18
  ret i32 0
}

define i32 @NewFaceIndex(i32 %f, i32 %m_faces, i32 addrspace(1)* nocapture %dFace3, i32 addrspace(1)* nocapture %dFace4, i32 addrspace(1)* nocapture %m_pFace3Scanned, i32 addrspace(1)* nocapture %m_pFace4Scanned) nounwind readonly {
  %1 = sext i32 %f to i64                         ; <i64> [#uses=6]
  %2 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %3 = load i32 addrspace(1)* %2                  ; <i32> [#uses=1]
  %4 = icmp eq i32 %3, 1                          ; <i1> [#uses=1]
  br i1 %4, label %9, label %5

; <label>:5                                       ; preds = %0
  %6 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %7 = load i32 addrspace(1)* %6                  ; <i32> [#uses=1]
  %8 = icmp eq i32 %7, 1                          ; <i1> [#uses=1]
  br i1 %8, label %9, label %25

; <label>:9                                       ; preds = %5, %0
  %10 = sext i32 %m_faces to i64                  ; <i64> [#uses=2]
  %11 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %10 ; <i32 addrspace(1)*> [#uses=1]
  %12 = load i32 addrspace(1)* %11                ; <i32> [#uses=1]
  %13 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %10 ; <i32 addrspace(1)*> [#uses=1]
  %14 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %15 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %16 = load i32 addrspace(1)* %15                ; <i32> [#uses=1]
  %17 = mul i32 %16, 3                            ; <i32> [#uses=1]
  %18 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %19 = load i32 addrspace(1)* %18                ; <i32> [#uses=1]
  %20 = shl i32 %19, 2                            ; <i32> [#uses=1]
  %21 = sub i32 %m_faces, %12                     ; <i32> [#uses=1]
  %22 = sub i32 %21, %14                          ; <i32> [#uses=1]
  %23 = add nsw i32 %22, %17                      ; <i32> [#uses=1]
  %24 = add nsw i32 %23, %20                      ; <i32> [#uses=1]
  ret i32 %24

; <label>:25                                      ; preds = %5
  %26 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %27 = load i32 addrspace(1)* %26                ; <i32> [#uses=1]
  %28 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %29 = load i32 addrspace(1)* %28                ; <i32> [#uses=1]
  %30 = sub i32 %f, %27                           ; <i32> [#uses=1]
  %31 = sub i32 %30, %29                          ; <i32> [#uses=1]
  ret i32 %31
}

define void @TestSubdivisionKernel(i32 addrspace(1)* nocapture %dPotentiallyActive, i32 addrspace(1)* nocapture %dActive, i32 addrspace(1)* nocapture %dVertex, %struct._Vertex addrspace(1)* nocapture %pVB, %struct._Face addrspace(1)* %pFace, float %shadingRate, i32 %faces) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=1]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=1]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=1]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=2]
  %7 = icmp slt i32 %6, %faces                    ; <i1> [#uses=1]
  br i1 %7, label %8, label %115

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=4]
  %10 = getelementptr inbounds %struct._Face addrspace(1)* %pFace, i64 %9, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=5]
  %11 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %12 = sext i32 %11 to i64                       ; <i64> [#uses=3]
  %.1298.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %12, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp320 = load float addrspace(1)* %.1298.0, align 4 ; <float> [#uses=1]
  %.1298.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %12, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp321 = load float addrspace(1)* %.1298.1, align 4 ; <float> [#uses=1]
  %.1298.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %12, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp322 = load float addrspace(1)* %.1298.2, align 4 ; <float> [#uses=1]
  %13 = getelementptr inbounds %struct._Face addrspace(1)* %pFace, i64 %9, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=5]
  %14 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %15 = sext i32 %14 to i64                       ; <i64> [#uses=3]
  %.1259.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %15, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp282 = load float addrspace(1)* %.1259.0, align 4 ; <float> [#uses=1]
  %.1259.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %15, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp283 = load float addrspace(1)* %.1259.1, align 4 ; <float> [#uses=1]
  %.1259.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %15, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp284 = load float addrspace(1)* %.1259.2, align 4 ; <float> [#uses=1]
  %16 = fsub float %tmp320, %tmp282               ; <float> [#uses=2]
  %17 = fsub float %tmp321, %tmp283               ; <float> [#uses=2]
  %18 = fsub float %tmp322, %tmp284               ; <float> [#uses=2]
  %19 = fmul float %16, %16                       ; <float> [#uses=1]
  %20 = fmul float %17, %17                       ; <float> [#uses=1]
  %21 = fadd float %19, %20                       ; <float> [#uses=1]
  %22 = fmul float %18, %18                       ; <float> [#uses=1]
  %23 = fadd float %21, %22                       ; <float> [#uses=1]
  %24 = tail call float @_Z4sqrtf(float %23) nounwind ; <float> [#uses=1]
  %25 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %26 = sext i32 %25 to i64                       ; <i64> [#uses=3]
  %.1220.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %26, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp243 = load float addrspace(1)* %.1220.0, align 4 ; <float> [#uses=1]
  %.1220.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %26, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp244 = load float addrspace(1)* %.1220.1, align 4 ; <float> [#uses=1]
  %.1220.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %26, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp245 = load float addrspace(1)* %.1220.2, align 4 ; <float> [#uses=1]
  %27 = getelementptr inbounds %struct._Face addrspace(1)* %pFace, i64 %9, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=5]
  %28 = load i32 addrspace(1)* %27                ; <i32> [#uses=1]
  %29 = sext i32 %28 to i64                       ; <i64> [#uses=3]
  %.1181.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %29, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp204 = load float addrspace(1)* %.1181.0, align 4 ; <float> [#uses=1]
  %.1181.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %29, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp205 = load float addrspace(1)* %.1181.1, align 4 ; <float> [#uses=1]
  %.1181.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %29, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp206 = load float addrspace(1)* %.1181.2, align 4 ; <float> [#uses=1]
  %30 = fsub float %tmp243, %tmp204               ; <float> [#uses=2]
  %31 = fsub float %tmp244, %tmp205               ; <float> [#uses=2]
  %32 = fsub float %tmp245, %tmp206               ; <float> [#uses=2]
  %33 = fmul float %30, %30                       ; <float> [#uses=1]
  %34 = fmul float %31, %31                       ; <float> [#uses=1]
  %35 = fadd float %33, %34                       ; <float> [#uses=1]
  %36 = fmul float %32, %32                       ; <float> [#uses=1]
  %37 = fadd float %35, %36                       ; <float> [#uses=1]
  %38 = tail call float @_Z4sqrtf(float %37) nounwind ; <float> [#uses=1]
  %39 = load i32 addrspace(1)* %27                ; <i32> [#uses=1]
  %40 = sext i32 %39 to i64                       ; <i64> [#uses=3]
  %.1142.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %40, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp165 = load float addrspace(1)* %.1142.0, align 4 ; <float> [#uses=1]
  %.1142.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %40, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp166 = load float addrspace(1)* %.1142.1, align 4 ; <float> [#uses=1]
  %.1142.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %40, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp167 = load float addrspace(1)* %.1142.2, align 4 ; <float> [#uses=1]
  %41 = getelementptr inbounds %struct._Face addrspace(1)* %pFace, i64 %9, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=5]
  %42 = load i32 addrspace(1)* %41                ; <i32> [#uses=1]
  %43 = sext i32 %42 to i64                       ; <i64> [#uses=3]
  %.1103.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %43, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp126 = load float addrspace(1)* %.1103.0, align 4 ; <float> [#uses=1]
  %.1103.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %43, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp127 = load float addrspace(1)* %.1103.1, align 4 ; <float> [#uses=1]
  %.1103.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %43, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp128 = load float addrspace(1)* %.1103.2, align 4 ; <float> [#uses=1]
  %44 = fsub float %tmp165, %tmp126               ; <float> [#uses=2]
  %45 = fsub float %tmp166, %tmp127               ; <float> [#uses=2]
  %46 = fsub float %tmp167, %tmp128               ; <float> [#uses=2]
  %47 = fmul float %44, %44                       ; <float> [#uses=1]
  %48 = fmul float %45, %45                       ; <float> [#uses=1]
  %49 = fadd float %47, %48                       ; <float> [#uses=1]
  %50 = fmul float %46, %46                       ; <float> [#uses=1]
  %51 = fadd float %49, %50                       ; <float> [#uses=1]
  %52 = tail call float @_Z4sqrtf(float %51) nounwind ; <float> [#uses=1]
  %53 = load i32 addrspace(1)* %41                ; <i32> [#uses=1]
  %54 = sext i32 %53 to i64                       ; <i64> [#uses=3]
  %.164.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %54, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp87 = load float addrspace(1)* %.164.0, align 4 ; <float> [#uses=1]
  %.164.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %54, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp88 = load float addrspace(1)* %.164.1, align 4 ; <float> [#uses=1]
  %.164.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %54, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp89 = load float addrspace(1)* %.164.2, align 4 ; <float> [#uses=1]
  %55 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %56 = sext i32 %55 to i64                       ; <i64> [#uses=3]
  %.132.0 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %56, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %tmp48 = load float addrspace(1)* %.132.0, align 4 ; <float> [#uses=1]
  %.132.1 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %56, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %tmp49 = load float addrspace(1)* %.132.1, align 4 ; <float> [#uses=1]
  %.132.2 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %56, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %tmp50 = load float addrspace(1)* %.132.2, align 4 ; <float> [#uses=1]
  %57 = fsub float %tmp87, %tmp48                 ; <float> [#uses=2]
  %58 = fsub float %tmp88, %tmp49                 ; <float> [#uses=2]
  %59 = fsub float %tmp89, %tmp50                 ; <float> [#uses=2]
  %60 = fmul float %57, %57                       ; <float> [#uses=1]
  %61 = fmul float %58, %58                       ; <float> [#uses=1]
  %62 = fadd float %60, %61                       ; <float> [#uses=1]
  %63 = fmul float %59, %59                       ; <float> [#uses=1]
  %64 = fadd float %62, %63                       ; <float> [#uses=1]
  %65 = tail call float @_Z4sqrtf(float %64) nounwind ; <float> [#uses=1]
  %66 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %67 = sext i32 %66 to i64                       ; <i64> [#uses=1]
  %68 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %67, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %68
  %69 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %70 = sext i32 %69 to i64                       ; <i64> [#uses=1]
  %71 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %70, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %71
  %72 = load i32 addrspace(1)* %27                ; <i32> [#uses=1]
  %73 = sext i32 %72 to i64                       ; <i64> [#uses=1]
  %74 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %73, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %74
  %75 = load i32 addrspace(1)* %41                ; <i32> [#uses=1]
  %76 = sext i32 %75 to i64                       ; <i64> [#uses=1]
  %77 = getelementptr inbounds %struct._Vertex addrspace(1)* %pVB, i64 %76, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %77
  %78 = fcmp ogt float %24, %shadingRate          ; <i1> [#uses=1]
  %79 = fcmp ogt float %38, %shadingRate          ; <i1> [#uses=1]
  %or.cond = or i1 %78, %79                       ; <i1> [#uses=1]
  %80 = fcmp ogt float %52, %shadingRate          ; <i1> [#uses=1]
  %or.cond1 = or i1 %or.cond, %80                 ; <i1> [#uses=1]
  %81 = fcmp ogt float %65, %shadingRate          ; <i1> [#uses=1]
  %or.cond2 = or i1 %or.cond1, %81                ; <i1> [#uses=1]
  br i1 %or.cond2, label %82, label %115

; <label>:82                                      ; preds = %8
  %83 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %84 = sext i32 %83 to i64                       ; <i64> [#uses=2]
  %85 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %84 ; <i32 addrspace(1)*> [#uses=1]
  %86 = load i32 addrspace(1)* %85                ; <i32> [#uses=1]
  %87 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %84 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %86, i32 addrspace(1)* %87
  %88 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %89 = sext i32 %88 to i64                       ; <i64> [#uses=2]
  %90 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %89 ; <i32 addrspace(1)*> [#uses=1]
  %91 = load i32 addrspace(1)* %90                ; <i32> [#uses=1]
  %92 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %89 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %91, i32 addrspace(1)* %92
  %93 = load i32 addrspace(1)* %27                ; <i32> [#uses=1]
  %94 = sext i32 %93 to i64                       ; <i64> [#uses=2]
  %95 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %94 ; <i32 addrspace(1)*> [#uses=1]
  %96 = load i32 addrspace(1)* %95                ; <i32> [#uses=1]
  %97 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %94 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %96, i32 addrspace(1)* %97
  %98 = load i32 addrspace(1)* %41                ; <i32> [#uses=1]
  %99 = sext i32 %98 to i64                       ; <i64> [#uses=2]
  %100 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %99 ; <i32 addrspace(1)*> [#uses=1]
  %101 = load i32 addrspace(1)* %100              ; <i32> [#uses=1]
  %102 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %99 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %101, i32 addrspace(1)* %102
  %103 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %104 = sext i32 %103 to i64                     ; <i64> [#uses=1]
  %105 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %104 ; <i32 addrspace(1)*> [#uses=1]
  store i32 1, i32 addrspace(1)* %105
  %106 = load i32 addrspace(1)* %13               ; <i32> [#uses=1]
  %107 = sext i32 %106 to i64                     ; <i64> [#uses=1]
  %108 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %107 ; <i32 addrspace(1)*> [#uses=1]
  store i32 1, i32 addrspace(1)* %108
  %109 = load i32 addrspace(1)* %27               ; <i32> [#uses=1]
  %110 = sext i32 %109 to i64                     ; <i64> [#uses=1]
  %111 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %110 ; <i32 addrspace(1)*> [#uses=1]
  store i32 1, i32 addrspace(1)* %111
  %112 = load i32 addrspace(1)* %41               ; <i32> [#uses=1]
  %113 = sext i32 %112 to i64                     ; <i64> [#uses=1]
  %114 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %113 ; <i32 addrspace(1)*> [#uses=1]
  store i32 1, i32 addrspace(1)* %114
  ret void

; <label>:115                                     ; preds = %8, %0
  ret void
}

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_group_id(i32)

define void @UpdateFacesKernel(i32 addrspace(1)* nocapture %dActive, i32 addrspace(1)* nocapture %dFace3, i32 addrspace(1)* nocapture %dFace4, %struct._Face addrspace(1)* nocapture %m_pInFB, i32 %faces) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=1]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=1]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=1]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=2]
  %7 = icmp slt i32 %6, %faces                    ; <i1> [#uses=1]
  br i1 %7, label %8, label %37

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=6]
  %10 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %11 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %12 = sext i32 %11 to i64                       ; <i64> [#uses=1]
  %13 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %12 ; <i32 addrspace(1)*> [#uses=1]
  %14 = load i32 addrspace(1)* %13                ; <i32> [#uses=1]
  %15 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %16 = load i32 addrspace(1)* %15                ; <i32> [#uses=1]
  %17 = sext i32 %16 to i64                       ; <i64> [#uses=1]
  %18 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %17 ; <i32 addrspace(1)*> [#uses=1]
  %19 = load i32 addrspace(1)* %18                ; <i32> [#uses=1]
  %20 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %21 = load i32 addrspace(1)* %20                ; <i32> [#uses=1]
  %22 = sext i32 %21 to i64                       ; <i64> [#uses=1]
  %23 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %22 ; <i32 addrspace(1)*> [#uses=1]
  %24 = load i32 addrspace(1)* %23                ; <i32> [#uses=1]
  %25 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %26 = load i32 addrspace(1)* %25                ; <i32> [#uses=1]
  %27 = sext i32 %26 to i64                       ; <i64> [#uses=1]
  %28 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %27 ; <i32 addrspace(1)*> [#uses=1]
  %29 = load i32 addrspace(1)* %28                ; <i32> [#uses=1]
  %30 = add nsw i32 %19, %14                      ; <i32> [#uses=1]
  %31 = add nsw i32 %30, %24                      ; <i32> [#uses=1]
  %32 = add nsw i32 %31, %29                      ; <i32> [#uses=1]
  %33 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %9 ; <i32 addrspace(1)*> [#uses=2]
  store i32 0, i32 addrspace(1)* %33
  %34 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %9 ; <i32 addrspace(1)*> [#uses=2]
  store i32 0, i32 addrspace(1)* %34
  switch i32 %32, label %37 [
    i32 2, label %35
    i32 1, label %36
  ]

; <label>:35                                      ; preds = %8
  store i32 1, i32 addrspace(1)* %34
  ret void

; <label>:36                                      ; preds = %8
  store i32 1, i32 addrspace(1)* %33
  ret void

; <label>:37                                      ; preds = %8, %0
  ret void
}

define void @UpdateEdgesKernel(i32 addrspace(1)* nocapture %dActive, i32 addrspace(1)* nocapture %dVertex, i32 addrspace(1)* nocapture %dEdge, %struct._Edge addrspace(1)* nocapture %pEdges, i32 %edges) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=1]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=1]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=1]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=2]
  %7 = icmp slt i32 %6, %edges                    ; <i1> [#uses=1]
  br i1 %7, label %8, label %32

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=3]
  %10 = getelementptr inbounds i32 addrspace(1)* %dEdge, i64 %9 ; <i32 addrspace(1)*> [#uses=2]
  store i32 0, i32 addrspace(1)* %10
  %11 = getelementptr inbounds %struct._Edge addrspace(1)* %pEdges, i64 %9, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %12 = load i32 addrspace(1)* %11                ; <i32> [#uses=2]
  %13 = icmp eq i32 %12, -1                       ; <i1> [#uses=1]
  br i1 %13, label %32, label %14

; <label>:14                                      ; preds = %8
  %15 = sext i32 %12 to i64                       ; <i64> [#uses=2]
  %16 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %15 ; <i32 addrspace(1)*> [#uses=1]
  %17 = load i32 addrspace(1)* %16                ; <i32> [#uses=1]
  %18 = icmp eq i32 %17, 0                        ; <i1> [#uses=1]
  br i1 %18, label %19, label %26

; <label>:19                                      ; preds = %14
  %20 = getelementptr inbounds %struct._Edge addrspace(1)* %pEdges, i64 %9, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=2]
  %21 = load i32 addrspace(1)* %20                ; <i32> [#uses=1]
  %22 = sext i32 %21 to i64                       ; <i64> [#uses=1]
  %23 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %22 ; <i32 addrspace(1)*> [#uses=1]
  %24 = load i32 addrspace(1)* %23                ; <i32> [#uses=1]
  %25 = icmp eq i32 %24, 0                        ; <i1> [#uses=1]
  br i1 %25, label %27, label %26

; <label>:26                                      ; preds = %19, %14
  store i32 1, i32 addrspace(1)* %10
  ret void

; <label>:27                                      ; preds = %19
  %28 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %15 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %28
  %29 = load i32 addrspace(1)* %20                ; <i32> [#uses=1]
  %30 = sext i32 %29 to i64                       ; <i64> [#uses=1]
  %31 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %30 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %31
  ret void

; <label>:32                                      ; preds = %8, %0
  ret void
}

define void @GenerateFacePointsKernel(i32 addrspace(1)* nocapture %dFace3, i32 addrspace(1)* nocapture %dFace4, i32 addrspace(1)* nocapture %dActive, %struct._Face addrspace(1)* %m_pInFB, %struct._Face addrspace(1)* nocapture %m_pOutFB, %struct._Vertex addrspace(1)* nocapture %m_pInVB, %struct._Vertex addrspace(1)* %m_pOutVB, i32 addrspace(1)* nocapture %m_pFace3Scanned, i32 addrspace(1)* nocapture %m_pFace4Scanned, i32 addrspace(1)* nocapture %m_pEdgeScanned, i32 %m_vertices, i32 %faces) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=4]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=4]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=4]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=3]
  %7 = icmp slt i32 %6, %faces                    ; <i1> [#uses=1]
  br i1 %7, label %8, label %225

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=20]
  %10 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %9 ; <i32 addrspace(1)*> [#uses=2]
  %11 = load i32 addrspace(1)* %10                ; <i32> [#uses=2]
  %12 = icmp eq i32 %11, 1                        ; <i1> [#uses=1]
  br i1 %12, label %17, label %13

; <label>:13                                      ; preds = %8
  %14 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %15 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %16 = icmp eq i32 %15, 1                        ; <i1> [#uses=1]
  br i1 %16, label %17, label %33

; <label>:17                                      ; preds = %13, %8
  %18 = sext i32 %faces to i64                    ; <i64> [#uses=2]
  %19 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %18 ; <i32 addrspace(1)*> [#uses=1]
  %20 = load i32 addrspace(1)* %19                ; <i32> [#uses=1]
  %21 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %18 ; <i32 addrspace(1)*> [#uses=1]
  %22 = load i32 addrspace(1)* %21                ; <i32> [#uses=1]
  %23 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %24 = load i32 addrspace(1)* %23                ; <i32> [#uses=2]
  %25 = mul i32 %24, 3                            ; <i32> [#uses=1]
  %26 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %27 = load i32 addrspace(1)* %26                ; <i32> [#uses=2]
  %28 = shl i32 %27, 2                            ; <i32> [#uses=1]
  %29 = sub i32 %faces, %20                       ; <i32> [#uses=1]
  %30 = sub i32 %29, %22                          ; <i32> [#uses=1]
  %31 = add nsw i32 %30, %25                      ; <i32> [#uses=1]
  %32 = add nsw i32 %31, %28                      ; <i32> [#uses=1]
  br label %NewFaceIndex.exit

; <label>:33                                      ; preds = %13
  %34 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %35 = load i32 addrspace(1)* %34                ; <i32> [#uses=2]
  %36 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %37 = load i32 addrspace(1)* %36                ; <i32> [#uses=2]
  %38 = sub i32 %6, %35                           ; <i32> [#uses=1]
  %39 = sub i32 %38, %37                          ; <i32> [#uses=1]
  br label %NewFaceIndex.exit

NewFaceIndex.exit:                                ; preds = %17, %33
  %40 = phi i32 [ %37, %33 ], [ %27, %17 ]        ; <i32> [#uses=1]
  %41 = phi i32 [ %35, %33 ], [ %24, %17 ]        ; <i32> [#uses=1]
  %42 = phi i32 [ %39, %33 ], [ %32, %17 ]        ; <i32> [#uses=8]
  %43 = icmp eq i32 %11, 0                        ; <i1> [#uses=1]
  br i1 %43, label %44, label %bb.nph12

; <label>:44                                      ; preds = %NewFaceIndex.exit
  %45 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %46 = load i32 addrspace(1)* %45                ; <i32> [#uses=1]
  %47 = icmp eq i32 %46, 0                        ; <i1> [#uses=1]
  br i1 %47, label %187, label %bb.nph12

bb.nph12:                                         ; preds = %NewFaceIndex.exit, %44
  %48 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=2]
  %49 = load i32 addrspace(1)* %48                ; <i32> [#uses=1]
  %50 = sext i32 %49 to i64                       ; <i64> [#uses=3]
  %51 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %50, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %52 = load float addrspace(1)* %51              ; <float> [#uses=1]
  %53 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %54 = load i32 addrspace(1)* %53                ; <i32> [#uses=1]
  %55 = sext i32 %54 to i64                       ; <i64> [#uses=3]
  %56 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %55, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %57 = load float addrspace(1)* %56              ; <float> [#uses=1]
  %58 = fadd float %52, %57                       ; <float> [#uses=1]
  %59 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %60 = load i32 addrspace(1)* %59                ; <i32> [#uses=1]
  %61 = sext i32 %60 to i64                       ; <i64> [#uses=3]
  %62 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %61, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %63 = load float addrspace(1)* %62              ; <float> [#uses=1]
  %64 = fadd float %58, %63                       ; <float> [#uses=1]
  %65 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %66 = load i32 addrspace(1)* %65                ; <i32> [#uses=1]
  %67 = sext i32 %66 to i64                       ; <i64> [#uses=3]
  %68 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %67, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %69 = load float addrspace(1)* %68              ; <float> [#uses=1]
  %70 = fadd float %64, %69                       ; <float> [#uses=1]
  %71 = fdiv float %70, 4.000000e+000             ; <float> [#uses=2]
  %72 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %50, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %73 = load float addrspace(1)* %72              ; <float> [#uses=1]
  %74 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %55, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %75 = load float addrspace(1)* %74              ; <float> [#uses=1]
  %76 = fadd float %73, %75                       ; <float> [#uses=1]
  %77 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %61, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %78 = load float addrspace(1)* %77              ; <float> [#uses=1]
  %79 = fadd float %76, %78                       ; <float> [#uses=1]
  %80 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %67, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %81 = load float addrspace(1)* %80              ; <float> [#uses=1]
  %82 = fadd float %79, %81                       ; <float> [#uses=1]
  %83 = fdiv float %82, 4.000000e+000             ; <float> [#uses=2]
  %84 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %50, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %85 = load float addrspace(1)* %84              ; <float> [#uses=1]
  %86 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %55, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %87 = load float addrspace(1)* %86              ; <float> [#uses=1]
  %88 = fadd float %85, %87                       ; <float> [#uses=1]
  %89 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %61, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %90 = load float addrspace(1)* %89              ; <float> [#uses=1]
  %91 = fadd float %88, %90                       ; <float> [#uses=1]
  %92 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %67, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %93 = load float addrspace(1)* %92              ; <float> [#uses=1]
  %94 = fadd float %91, %93                       ; <float> [#uses=1]
  %95 = fdiv float %94, 4.000000e+000             ; <float> [#uses=2]
  %96 = add nsw i32 %41, %m_vertices              ; <i32> [#uses=1]
  %97 = add nsw i32 %96, %40                      ; <i32> [#uses=8]
  %98 = sext i32 %97 to i64                       ; <i64> [#uses=5]
  %99 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %98, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  store float %71, float addrspace(1)* %99
  %100 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %98, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  store float %83, float addrspace(1)* %100
  %101 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %98, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  store float %95, float addrspace(1)* %101
  %102 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %98, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float 4.000000e+000, float addrspace(1)* %102
  %103 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %98, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %103
  %tmp32 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp33 = add i64 %1, %tmp32                     ; <i64> [#uses=1]
  %sext = shl i64 %tmp33, 32                      ; <i64> [#uses=1]
  %tmp35 = ashr i64 %sext, 32                     ; <i64> [#uses=1]
  br label %.preheader5

.preheader5:                                      ; preds = %132, %bb.nph12
  %indvar29 = phi i64 [ 0, %bb.nph12 ], [ %indvar.next30, %132 ] ; <i64> [#uses=2]
  %scevgep36 = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp35, i32 0, i64 %indvar29 ; <i32 addrspace(1)*> [#uses=5]
  br label %104

; <label>:104                                     ; preds = %.preheader5, %104
  %105 = load i32 addrspace(1)* %scevgep36        ; <i32> [#uses=1]
  %106 = sext i32 %105 to i64                     ; <i64> [#uses=1]
  %107 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %106, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %108 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %107, i32 0, i32 1) nounwind ; <i32> [#uses=1]
  %109 = icmp eq i32 %108, 0                      ; <i1> [#uses=1]
  br i1 %109, label %110, label %104

; <label>:110                                     ; preds = %104
  %111 = load i32 addrspace(1)* %scevgep36        ; <i32> [#uses=1]
  %112 = sext i32 %111 to i64                     ; <i64> [#uses=1]
  %113 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %112, i32 1, i64 0 ; <float addrspace(1)*> [#uses=2]
  %114 = load float addrspace(1)* %113            ; <float> [#uses=1]
  %115 = fadd float %114, %71                     ; <float> [#uses=1]
  store float %115, float addrspace(1)* %113
  %116 = load i32 addrspace(1)* %scevgep36        ; <i32> [#uses=1]
  %117 = sext i32 %116 to i64                     ; <i64> [#uses=1]
  %118 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %117, i32 1, i64 1 ; <float addrspace(1)*> [#uses=2]
  %119 = load float addrspace(1)* %118            ; <float> [#uses=1]
  %120 = fadd float %119, %83                     ; <float> [#uses=1]
  store float %120, float addrspace(1)* %118
  %121 = load i32 addrspace(1)* %scevgep36        ; <i32> [#uses=1]
  %122 = sext i32 %121 to i64                     ; <i64> [#uses=1]
  %123 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %122, i32 1, i64 2 ; <float addrspace(1)*> [#uses=2]
  %124 = load float addrspace(1)* %123            ; <float> [#uses=1]
  %125 = fadd float %124, %95                     ; <float> [#uses=1]
  store float %125, float addrspace(1)* %123
  br label %126

; <label>:126                                     ; preds = %126, %110
  %127 = load i32 addrspace(1)* %scevgep36        ; <i32> [#uses=1]
  %128 = sext i32 %127 to i64                     ; <i64> [#uses=1]
  %129 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %128, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %130 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %129, i32 1, i32 0) nounwind ; <i32> [#uses=1]
  %131 = icmp eq i32 %130, 1                      ; <i1> [#uses=1]
  br i1 %131, label %132, label %126

; <label>:132                                     ; preds = %126
  %indvar.next30 = add i64 %indvar29, 1           ; <i64> [#uses=2]
  %exitcond31 = icmp eq i64 %indvar.next30, 4     ; <i1> [#uses=1]
  br i1 %exitcond31, label %._crit_edge13, label %.preheader5

._crit_edge13:                                    ; preds = %132
  %133 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %134 = load i32 addrspace(1)* %133              ; <i32> [#uses=1]
  %135 = icmp eq i32 %134, 0                      ; <i1> [#uses=1]
  br i1 %135, label %136, label %.loopexit

; <label>:136                                     ; preds = %._crit_edge13
  %137 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %138 = icmp eq i32 %137, 0                      ; <i1> [#uses=1]
  br i1 %138, label %225, label %.preheader4

.preheader4:                                      ; preds = %136
  %139 = load i32 addrspace(1)* %48               ; <i32> [#uses=1]
  %140 = sext i32 %139 to i64                     ; <i64> [#uses=1]
  %141 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %140 ; <i32 addrspace(1)*> [#uses=1]
  %142 = load i32 addrspace(1)* %141              ; <i32> [#uses=1]
  %143 = icmp eq i32 %142, 0                      ; <i1> [#uses=1]
  br i1 %143, label %bb.nph8, label %150

._crit_edge:                                      ; preds = %144
  %tmp23 = trunc i64 %tmp22 to i32                ; <i32> [#uses=1]
  br label %150

bb.nph8:                                          ; preds = %.preheader4
  %tmp24 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp25 = add i64 %1, %tmp24                     ; <i64> [#uses=1]
  %sext38 = shl i64 %tmp25, 32                    ; <i64> [#uses=1]
  %tmp27 = ashr i64 %sext38, 32                   ; <i64> [#uses=1]
  br label %144

; <label>:144                                     ; preds = %bb.nph8, %144
  %indvar20 = phi i64 [ 0, %bb.nph8 ], [ %tmp22, %144 ] ; <i64> [#uses=1]
  %tmp22 = add i64 %indvar20, 1                   ; <i64> [#uses=3]
  %scevgep28 = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp27, i32 0, i64 %tmp22 ; <i32 addrspace(1)*> [#uses=1]
  %145 = load i32 addrspace(1)* %scevgep28        ; <i32> [#uses=1]
  %146 = sext i32 %145 to i64                     ; <i64> [#uses=1]
  %147 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %146 ; <i32 addrspace(1)*> [#uses=1]
  %148 = load i32 addrspace(1)* %147              ; <i32> [#uses=1]
  %149 = icmp eq i32 %148, 0                      ; <i1> [#uses=1]
  br i1 %149, label %144, label %._crit_edge

; <label>:150                                     ; preds = %._crit_edge, %.preheader4
  %active.0.lcssa = phi i32 [ %tmp23, %._crit_edge ], [ 0, %.preheader4 ] ; <i32> [#uses=4]
  %151 = sext i32 %active.0.lcssa to i64          ; <i64> [#uses=1]
  %152 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 %151 ; <i32 addrspace(1)*> [#uses=1]
  %153 = load i32 addrspace(1)* %152              ; <i32> [#uses=1]
  %154 = sext i32 %42 to i64                      ; <i64> [#uses=4]
  %155 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %154, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %153, i32 addrspace(1)* %155
  %156 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %154, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %156
  %157 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %154, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %157
  %158 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %154, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %158
  %159 = add nsw i32 %active.0.lcssa, 1           ; <i32> [#uses=1]
  %160 = srem i32 %159, 4                         ; <i32> [#uses=1]
  %161 = sext i32 %160 to i64                     ; <i64> [#uses=1]
  %162 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 %161 ; <i32 addrspace(1)*> [#uses=1]
  %163 = load i32 addrspace(1)* %162              ; <i32> [#uses=1]
  %164 = add nsw i32 %42, 1                       ; <i32> [#uses=1]
  %165 = sext i32 %164 to i64                     ; <i64> [#uses=4]
  %166 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %165, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %163, i32 addrspace(1)* %166
  %167 = add nsw i32 %active.0.lcssa, 2           ; <i32> [#uses=1]
  %168 = srem i32 %167, 4                         ; <i32> [#uses=1]
  %169 = sext i32 %168 to i64                     ; <i64> [#uses=1]
  %170 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 %169 ; <i32 addrspace(1)*> [#uses=2]
  %171 = load i32 addrspace(1)* %170              ; <i32> [#uses=1]
  %172 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %165, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %171, i32 addrspace(1)* %172
  %173 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %165, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %173
  %174 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %165, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %174
  %175 = add nsw i32 %active.0.lcssa, 3           ; <i32> [#uses=1]
  %176 = srem i32 %175, 4                         ; <i32> [#uses=1]
  %177 = sext i32 %176 to i64                     ; <i64> [#uses=1]
  %178 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 %177 ; <i32 addrspace(1)*> [#uses=1]
  %179 = load i32 addrspace(1)* %178              ; <i32> [#uses=1]
  %180 = add nsw i32 %42, 2                       ; <i32> [#uses=1]
  %181 = sext i32 %180 to i64                     ; <i64> [#uses=4]
  %182 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %181, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %179, i32 addrspace(1)* %182
  %183 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %181, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %183
  %184 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %181, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %184
  %185 = load i32 addrspace(1)* %170              ; <i32> [#uses=1]
  %186 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %181, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %185, i32 addrspace(1)* %186
  ret void

; <label>:187                                     ; preds = %44
  %188 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %189 = load i32 addrspace(1)* %188              ; <i32> [#uses=1]
  %190 = sext i32 %42 to i64                      ; <i64> [#uses=4]
  %191 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %190, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %189, i32 addrspace(1)* %191
  %192 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %193 = load i32 addrspace(1)* %192              ; <i32> [#uses=1]
  %194 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %190, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %193, i32 addrspace(1)* %194
  %195 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %196 = load i32 addrspace(1)* %195              ; <i32> [#uses=1]
  %197 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %190, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %196, i32 addrspace(1)* %197
  %198 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %9, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %199 = load i32 addrspace(1)* %198              ; <i32> [#uses=1]
  %200 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %190, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %199, i32 addrspace(1)* %200
  ret void

.loopexit:                                        ; preds = %._crit_edge13
  %tmp = mul i64 %2, %3                           ; <i64> [#uses=1]
  %tmp14 = add i64 %1, %tmp                       ; <i64> [#uses=1]
  %sext37 = shl i64 %tmp14, 32                    ; <i64> [#uses=1]
  %tmp16 = ashr i64 %sext37, 32                   ; <i64> [#uses=4]
  %scevgep = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp16, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %201 = load i32 addrspace(1)* %scevgep          ; <i32> [#uses=1]
  %202 = sext i32 %42 to i64                      ; <i64> [#uses=4]
  %203 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %202, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %201, i32 addrspace(1)* %203
  %204 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %202, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %204
  %205 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %202, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %205
  %206 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %202, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %206
  %scevgep.1 = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp16, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %tmp18.1 = add i32 %42, 1                       ; <i32> [#uses=1]
  %207 = load i32 addrspace(1)* %scevgep.1        ; <i32> [#uses=1]
  %208 = sext i32 %tmp18.1 to i64                 ; <i64> [#uses=4]
  %209 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %208, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %207, i32 addrspace(1)* %209
  %210 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %208, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %210
  %211 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %208, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %211
  %212 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %208, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %212
  %scevgep.2 = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp16, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %tmp18.2 = add i32 %42, 2                       ; <i32> [#uses=1]
  %213 = load i32 addrspace(1)* %scevgep.2        ; <i32> [#uses=1]
  %214 = sext i32 %tmp18.2 to i64                 ; <i64> [#uses=4]
  %215 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %214, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %213, i32 addrspace(1)* %215
  %216 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %214, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %216
  %217 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %214, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %217
  %218 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %214, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %218
  %scevgep.3 = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %tmp16, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp18.3 = add i32 %42, 3                       ; <i32> [#uses=1]
  %219 = load i32 addrspace(1)* %scevgep.3        ; <i32> [#uses=1]
  %220 = sext i32 %tmp18.3 to i64                 ; <i64> [#uses=4]
  %221 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %220, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %219, i32 addrspace(1)* %221
  %222 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %220, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %97, i32 addrspace(1)* %222
  %223 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %220, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %223
  %224 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %220, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %224
  ret void

; <label>:225                                     ; preds = %136, %0
  ret void
}

declare i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)*, i32, i32)

define void @GenerateEdgePointsKernel(i32 addrspace(1)* nocapture %dEdge, i32 addrspace(1)* nocapture %dFace3, i32 addrspace(1)* nocapture %dFace4, i32 addrspace(1)* nocapture %dActive, i32 addrspace(1)* nocapture %dPotentiallyActive, i32 addrspace(1)* nocapture %dNewPotentiallyActive, i32 addrspace(1)* nocapture %dVertex, %struct._Edge addrspace(1)* %m_pInEB, %struct._Edge addrspace(1)* %m_pOutEB, %struct._Face addrspace(1)* %m_pInFB, %struct._Face addrspace(1)* nocapture %m_pOutFB, %struct._Vertex addrspace(1)* %m_pInVB, %struct._Vertex addrspace(1)* %m_pOutVB, i32 addrspace(1)* nocapture %m_pFace3Scanned, i32 addrspace(1)* nocapture %m_pFace4Scanned, i32 addrspace(1)* nocapture %m_pEdgeScanned, i32 %m_vertices, i32 %m_faces, i32 %m_edges) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=4]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=4]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=4]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=3]
  %7 = icmp slt i32 %6, %m_edges                  ; <i1> [#uses=1]
  br i1 %7, label %8, label %520

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=8]
  %10 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=9]
  %11 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %12 = sext i32 %11 to i64                       ; <i64> [#uses=1]
  %13 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %12 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %13
  %14 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=12]
  %15 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %16 = sext i32 %15 to i64                       ; <i64> [#uses=1]
  %17 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %16 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %17
  %18 = getelementptr inbounds i32 addrspace(1)* %dEdge, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %19 = load i32 addrspace(1)* %18                ; <i32> [#uses=2]
  %20 = icmp eq i32 %19, 1                        ; <i1> [#uses=1]
  br i1 %20, label %21, label %30

; <label>:21                                      ; preds = %8
  %22 = sext i32 %m_edges to i64                  ; <i64> [#uses=1]
  %23 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %22 ; <i32 addrspace(1)*> [#uses=1]
  %24 = load i32 addrspace(1)* %23                ; <i32> [#uses=1]
  %25 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %26 = load i32 addrspace(1)* %25                ; <i32> [#uses=1]
  %27 = shl i32 %26, 2                            ; <i32> [#uses=1]
  %28 = sub i32 %m_edges, %24                     ; <i32> [#uses=1]
  %29 = add nsw i32 %28, %27                      ; <i32> [#uses=1]
  br label %34

; <label>:30                                      ; preds = %8
  %31 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %32 = load i32 addrspace(1)* %31                ; <i32> [#uses=1]
  %33 = sub i32 %6, %32                           ; <i32> [#uses=1]
  br label %34

; <label>:34                                      ; preds = %30, %21
  %newEdgeIndex.0.i = phi i32 [ %29, %21 ], [ %33, %30 ] ; <i32> [#uses=3]
  %35 = icmp slt i32 %newEdgeIndex.0.i, 0         ; <i1> [#uses=1]
  br i1 %35, label %NewEdgeIndex.exit, label %36

; <label>:36                                      ; preds = %34
  %37 = shl i32 %m_edges, 2                       ; <i32> [#uses=1]
  %38 = icmp slt i32 %37, %newEdgeIndex.0.i       ; <i1> [#uses=1]
  %retval.i = select i1 %38, i32 0, i32 %newEdgeIndex.0.i ; <i32> [#uses=1]
  br label %NewEdgeIndex.exit

NewEdgeIndex.exit:                                ; preds = %34, %36
  %39 = phi i32 [ %retval.i, %36 ], [ 0, %34 ]    ; <i32> [#uses=8]
  %40 = load i32 addrspace(1)* %10                ; <i32> [#uses=3]
  %41 = icmp eq i32 %40, -1                       ; <i1> [#uses=1]
  br i1 %41, label %42, label %46

; <label>:42                                      ; preds = %NewEdgeIndex.exit
  %43 = sext i32 %39 to i64                       ; <i64> [#uses=1]
  %44 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %43, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %45 = bitcast i32 addrspace(1)* %44 to i8*      ; <i8*> [#uses=1]
  call void @llvm.memset.i64(i8* %45, i8 -1, i64 16, i32 1)
  ret void

; <label>:46                                      ; preds = %NewEdgeIndex.exit
  %47 = icmp eq i32 %19, 0                        ; <i1> [#uses=1]
  br i1 %47, label %bb.nph24, label %bb.nph22

bb.nph22:                                         ; preds = %46
  %48 = sext i32 %40 to i64                       ; <i64> [#uses=3]
  %49 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %50 = load float addrspace(1)* %49              ; <float> [#uses=1]
  %51 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %52 = sext i32 %51 to i64                       ; <i64> [#uses=3]
  %53 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %54 = load float addrspace(1)* %53              ; <float> [#uses=1]
  %55 = fadd float %50, %54                       ; <float> [#uses=2]
  %56 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %57 = load float addrspace(1)* %56              ; <float> [#uses=1]
  %58 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %59 = load float addrspace(1)* %58              ; <float> [#uses=1]
  %60 = fadd float %57, %59                       ; <float> [#uses=2]
  %61 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %62 = load float addrspace(1)* %61              ; <float> [#uses=1]
  %63 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %64 = load float addrspace(1)* %63              ; <float> [#uses=1]
  %65 = fadd float %62, %64                       ; <float> [#uses=2]
  %tmp61 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp62 = add i64 %1, %tmp61                     ; <i64> [#uses=1]
  %sext = shl i64 %tmp62, 32                      ; <i64> [#uses=1]
  %tmp64 = ashr i64 %sext, 32                     ; <i64> [#uses=1]
  br label %.preheader

.preheader:                                       ; preds = %94, %bb.nph22
  %indvar58 = phi i64 [ 0, %bb.nph22 ], [ %indvar.next59, %94 ] ; <i64> [#uses=2]
  %scevgep65 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp64, i32 1, i64 %indvar58 ; <i32 addrspace(1)*> [#uses=5]
  br label %66

; <label>:66                                      ; preds = %.preheader, %66
  %67 = load i32 addrspace(1)* %scevgep65         ; <i32> [#uses=1]
  %68 = sext i32 %67 to i64                       ; <i64> [#uses=1]
  %69 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %68, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %70 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %69, i32 0, i32 1) nounwind ; <i32> [#uses=1]
  %71 = icmp eq i32 %70, 0                        ; <i1> [#uses=1]
  br i1 %71, label %72, label %66

; <label>:72                                      ; preds = %66
  %73 = load i32 addrspace(1)* %scevgep65         ; <i32> [#uses=1]
  %74 = sext i32 %73 to i64                       ; <i64> [#uses=1]
  %75 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %74, i32 1, i64 0 ; <float addrspace(1)*> [#uses=2]
  %76 = load float addrspace(1)* %75              ; <float> [#uses=1]
  %77 = fadd float %76, %55                       ; <float> [#uses=1]
  store float %77, float addrspace(1)* %75
  %78 = load i32 addrspace(1)* %scevgep65         ; <i32> [#uses=1]
  %79 = sext i32 %78 to i64                       ; <i64> [#uses=1]
  %80 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %79, i32 1, i64 1 ; <float addrspace(1)*> [#uses=2]
  %81 = load float addrspace(1)* %80              ; <float> [#uses=1]
  %82 = fadd float %81, %60                       ; <float> [#uses=1]
  store float %82, float addrspace(1)* %80
  %83 = load i32 addrspace(1)* %scevgep65         ; <i32> [#uses=1]
  %84 = sext i32 %83 to i64                       ; <i64> [#uses=1]
  %85 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %84, i32 1, i64 2 ; <float addrspace(1)*> [#uses=2]
  %86 = load float addrspace(1)* %85              ; <float> [#uses=1]
  %87 = fadd float %86, %65                       ; <float> [#uses=1]
  store float %87, float addrspace(1)* %85
  br label %88

; <label>:88                                      ; preds = %88, %72
  %89 = load i32 addrspace(1)* %scevgep65         ; <i32> [#uses=1]
  %90 = sext i32 %89 to i64                       ; <i64> [#uses=1]
  %91 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %90, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %92 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %91, i32 1, i32 0) nounwind ; <i32> [#uses=1]
  %93 = icmp eq i32 %92, 1                        ; <i1> [#uses=1]
  br i1 %93, label %94, label %88

; <label>:94                                      ; preds = %88
  %indvar.next59 = add i64 %indvar58, 1           ; <i64> [#uses=2]
  %exitcond60 = icmp eq i64 %indvar.next59, 2     ; <i1> [#uses=1]
  br i1 %exitcond60, label %95, label %.preheader

; <label>:95                                      ; preds = %94
  %.phi.trans.insert = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %.pre = load i32 addrspace(1)* %.phi.trans.insert ; <i32> [#uses=1]
  %96 = sext i32 %m_faces to i64                  ; <i64> [#uses=2]
  %97 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %96 ; <i32 addrspace(1)*> [#uses=2]
  %98 = load i32 addrspace(1)* %97                ; <i32> [#uses=1]
  %99 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %96 ; <i32 addrspace(1)*> [#uses=2]
  %100 = load i32 addrspace(1)* %99               ; <i32> [#uses=1]
  %101 = add nsw i32 %98, %m_vertices             ; <i32> [#uses=1]
  %102 = add nsw i32 %101, %100                   ; <i32> [#uses=1]
  %103 = add nsw i32 %102, %.pre                  ; <i32> [#uses=11]
  %104 = sext i32 %103 to i64                     ; <i64> [#uses=10]
  %105 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %104 ; <i32 addrspace(1)*> [#uses=1]
  store i32 1, i32 addrspace(1)* %105
  %106 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=3]
  %107 = load i32 addrspace(1)* %106              ; <i32> [#uses=2]
  %108 = icmp eq i32 %107, -1                     ; <i1> [#uses=1]
  br i1 %108, label %113, label %109

; <label>:109                                     ; preds = %95
  %110 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=3]
  %111 = load i32 addrspace(1)* %110              ; <i32> [#uses=2]
  %112 = icmp eq i32 %111, -1                     ; <i1> [#uses=1]
  br i1 %112, label %113, label %128

; <label>:113                                     ; preds = %109, %95
  %114 = fdiv float %55, 2.000000e+000            ; <float> [#uses=1]
  %115 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  store float %114, float addrspace(1)* %115
  %116 = fdiv float %60, 2.000000e+000            ; <float> [#uses=1]
  %117 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  store float %116, float addrspace(1)* %117
  %118 = fdiv float %65, 2.000000e+000            ; <float> [#uses=1]
  %119 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  store float %118, float addrspace(1)* %119
  %120 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float 3.000000e+000, float addrspace(1)* %120
  %121 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %121
  %122 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %123 = sext i32 %122 to i64                     ; <i64> [#uses=1]
  %124 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %123 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %124
  %125 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %126 = sext i32 %125 to i64                     ; <i64> [#uses=1]
  %127 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %126 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %127
  br label %bb.nph

; <label>:128                                     ; preds = %109
  %129 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %130 = sext i32 %129 to i64                     ; <i64> [#uses=1]
  %131 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %130, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %132 = load float addrspace(1)* %131            ; <float> [#uses=1]
  %133 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %134 = sext i32 %133 to i64                     ; <i64> [#uses=1]
  %135 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %134, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %136 = load float addrspace(1)* %135            ; <float> [#uses=1]
  %137 = fadd float %132, %136                    ; <float> [#uses=1]
  %138 = sext i32 %107 to i64                     ; <i64> [#uses=4]
  %139 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %140 = load i32 addrspace(1)* %139              ; <i32> [#uses=1]
  %141 = sext i32 %140 to i64                     ; <i64> [#uses=1]
  %142 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %141, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %143 = load float addrspace(1)* %142            ; <float> [#uses=1]
  %144 = fadd float %137, %143                    ; <float> [#uses=1]
  %145 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %146 = load i32 addrspace(1)* %145              ; <i32> [#uses=1]
  %147 = sext i32 %146 to i64                     ; <i64> [#uses=1]
  %148 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %147, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %149 = load float addrspace(1)* %148            ; <float> [#uses=1]
  %150 = fadd float %144, %149                    ; <float> [#uses=1]
  %151 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %152 = load i32 addrspace(1)* %151              ; <i32> [#uses=1]
  %153 = sext i32 %152 to i64                     ; <i64> [#uses=1]
  %154 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %153, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %155 = load float addrspace(1)* %154            ; <float> [#uses=1]
  %156 = fadd float %150, %155                    ; <float> [#uses=1]
  %157 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %158 = load i32 addrspace(1)* %157              ; <i32> [#uses=1]
  %159 = sext i32 %158 to i64                     ; <i64> [#uses=1]
  %160 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %159, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %161 = load float addrspace(1)* %160            ; <float> [#uses=1]
  %162 = fadd float %156, %161                    ; <float> [#uses=1]
  %163 = sext i32 %111 to i64                     ; <i64> [#uses=4]
  %164 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %165 = load i32 addrspace(1)* %164              ; <i32> [#uses=1]
  %166 = sext i32 %165 to i64                     ; <i64> [#uses=1]
  %167 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %166, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %168 = load float addrspace(1)* %167            ; <float> [#uses=1]
  %169 = fadd float %162, %168                    ; <float> [#uses=1]
  %170 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %171 = load i32 addrspace(1)* %170              ; <i32> [#uses=1]
  %172 = sext i32 %171 to i64                     ; <i64> [#uses=1]
  %173 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %172, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %174 = load float addrspace(1)* %173            ; <float> [#uses=1]
  %175 = fadd float %169, %174                    ; <float> [#uses=1]
  %176 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %177 = load i32 addrspace(1)* %176              ; <i32> [#uses=1]
  %178 = sext i32 %177 to i64                     ; <i64> [#uses=1]
  %179 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %178, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %180 = load float addrspace(1)* %179            ; <float> [#uses=1]
  %181 = fadd float %175, %180                    ; <float> [#uses=1]
  %182 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %183 = load i32 addrspace(1)* %182              ; <i32> [#uses=1]
  %184 = sext i32 %183 to i64                     ; <i64> [#uses=1]
  %185 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %184, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %186 = load float addrspace(1)* %185            ; <float> [#uses=1]
  %187 = fadd float %181, %186                    ; <float> [#uses=1]
  %188 = fdiv float %187, 1.000000e+001           ; <float> [#uses=1]
  %189 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  store float %188, float addrspace(1)* %189
  %190 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %191 = sext i32 %190 to i64                     ; <i64> [#uses=1]
  %192 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %191, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %193 = load float addrspace(1)* %192            ; <float> [#uses=1]
  %194 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %195 = sext i32 %194 to i64                     ; <i64> [#uses=1]
  %196 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %195, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %197 = load float addrspace(1)* %196            ; <float> [#uses=1]
  %198 = fadd float %193, %197                    ; <float> [#uses=1]
  %199 = load i32 addrspace(1)* %106              ; <i32> [#uses=1]
  %200 = sext i32 %199 to i64                     ; <i64> [#uses=4]
  %201 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %202 = load i32 addrspace(1)* %201              ; <i32> [#uses=1]
  %203 = sext i32 %202 to i64                     ; <i64> [#uses=1]
  %204 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %203, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %205 = load float addrspace(1)* %204            ; <float> [#uses=1]
  %206 = fadd float %198, %205                    ; <float> [#uses=1]
  %207 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %208 = load i32 addrspace(1)* %207              ; <i32> [#uses=1]
  %209 = sext i32 %208 to i64                     ; <i64> [#uses=1]
  %210 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %209, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %211 = load float addrspace(1)* %210            ; <float> [#uses=1]
  %212 = fadd float %206, %211                    ; <float> [#uses=1]
  %213 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %214 = load i32 addrspace(1)* %213              ; <i32> [#uses=1]
  %215 = sext i32 %214 to i64                     ; <i64> [#uses=1]
  %216 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %215, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %217 = load float addrspace(1)* %216            ; <float> [#uses=1]
  %218 = fadd float %212, %217                    ; <float> [#uses=1]
  %219 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %220 = load i32 addrspace(1)* %219              ; <i32> [#uses=1]
  %221 = sext i32 %220 to i64                     ; <i64> [#uses=1]
  %222 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %221, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %223 = load float addrspace(1)* %222            ; <float> [#uses=1]
  %224 = fadd float %218, %223                    ; <float> [#uses=1]
  %225 = load i32 addrspace(1)* %110              ; <i32> [#uses=1]
  %226 = sext i32 %225 to i64                     ; <i64> [#uses=4]
  %227 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %228 = load i32 addrspace(1)* %227              ; <i32> [#uses=1]
  %229 = sext i32 %228 to i64                     ; <i64> [#uses=1]
  %230 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %229, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %231 = load float addrspace(1)* %230            ; <float> [#uses=1]
  %232 = fadd float %224, %231                    ; <float> [#uses=1]
  %233 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %234 = load i32 addrspace(1)* %233              ; <i32> [#uses=1]
  %235 = sext i32 %234 to i64                     ; <i64> [#uses=1]
  %236 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %235, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %237 = load float addrspace(1)* %236            ; <float> [#uses=1]
  %238 = fadd float %232, %237                    ; <float> [#uses=1]
  %239 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %240 = load i32 addrspace(1)* %239              ; <i32> [#uses=1]
  %241 = sext i32 %240 to i64                     ; <i64> [#uses=1]
  %242 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %241, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %243 = load float addrspace(1)* %242            ; <float> [#uses=1]
  %244 = fadd float %238, %243                    ; <float> [#uses=1]
  %245 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %246 = load i32 addrspace(1)* %245              ; <i32> [#uses=1]
  %247 = sext i32 %246 to i64                     ; <i64> [#uses=1]
  %248 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %247, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %249 = load float addrspace(1)* %248            ; <float> [#uses=1]
  %250 = fadd float %244, %249                    ; <float> [#uses=1]
  %251 = fdiv float %250, 1.000000e+001           ; <float> [#uses=1]
  %252 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  store float %251, float addrspace(1)* %252
  %253 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %254 = sext i32 %253 to i64                     ; <i64> [#uses=1]
  %255 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %254, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %256 = load float addrspace(1)* %255            ; <float> [#uses=1]
  %257 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %258 = sext i32 %257 to i64                     ; <i64> [#uses=1]
  %259 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %258, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %260 = load float addrspace(1)* %259            ; <float> [#uses=1]
  %261 = fadd float %256, %260                    ; <float> [#uses=1]
  %262 = load i32 addrspace(1)* %106              ; <i32> [#uses=1]
  %263 = sext i32 %262 to i64                     ; <i64> [#uses=4]
  %264 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %265 = load i32 addrspace(1)* %264              ; <i32> [#uses=1]
  %266 = sext i32 %265 to i64                     ; <i64> [#uses=1]
  %267 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %266, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %268 = load float addrspace(1)* %267            ; <float> [#uses=1]
  %269 = fadd float %261, %268                    ; <float> [#uses=1]
  %270 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %271 = load i32 addrspace(1)* %270              ; <i32> [#uses=1]
  %272 = sext i32 %271 to i64                     ; <i64> [#uses=1]
  %273 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %272, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %274 = load float addrspace(1)* %273            ; <float> [#uses=1]
  %275 = fadd float %269, %274                    ; <float> [#uses=1]
  %276 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %277 = load i32 addrspace(1)* %276              ; <i32> [#uses=1]
  %278 = sext i32 %277 to i64                     ; <i64> [#uses=1]
  %279 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %278, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %280 = load float addrspace(1)* %279            ; <float> [#uses=1]
  %281 = fadd float %275, %280                    ; <float> [#uses=1]
  %282 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %283 = load i32 addrspace(1)* %282              ; <i32> [#uses=1]
  %284 = sext i32 %283 to i64                     ; <i64> [#uses=1]
  %285 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %284, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %286 = load float addrspace(1)* %285            ; <float> [#uses=1]
  %287 = fadd float %281, %286                    ; <float> [#uses=1]
  %288 = load i32 addrspace(1)* %110              ; <i32> [#uses=1]
  %289 = sext i32 %288 to i64                     ; <i64> [#uses=4]
  %290 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %291 = load i32 addrspace(1)* %290              ; <i32> [#uses=1]
  %292 = sext i32 %291 to i64                     ; <i64> [#uses=1]
  %293 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %292, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %294 = load float addrspace(1)* %293            ; <float> [#uses=1]
  %295 = fadd float %287, %294                    ; <float> [#uses=1]
  %296 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %297 = load i32 addrspace(1)* %296              ; <i32> [#uses=1]
  %298 = sext i32 %297 to i64                     ; <i64> [#uses=1]
  %299 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %298, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %300 = load float addrspace(1)* %299            ; <float> [#uses=1]
  %301 = fadd float %295, %300                    ; <float> [#uses=1]
  %302 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %303 = load i32 addrspace(1)* %302              ; <i32> [#uses=1]
  %304 = sext i32 %303 to i64                     ; <i64> [#uses=1]
  %305 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %304, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %306 = load float addrspace(1)* %305            ; <float> [#uses=1]
  %307 = fadd float %301, %306                    ; <float> [#uses=1]
  %308 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %309 = load i32 addrspace(1)* %308              ; <i32> [#uses=1]
  %310 = sext i32 %309 to i64                     ; <i64> [#uses=1]
  %311 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %310, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %312 = load float addrspace(1)* %311            ; <float> [#uses=1]
  %313 = fadd float %307, %312                    ; <float> [#uses=1]
  %314 = fdiv float %313, 1.000000e+001           ; <float> [#uses=1]
  %315 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  store float %314, float addrspace(1)* %315
  %316 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float 4.000000e+000, float addrspace(1)* %316
  br label %bb.nph

bb.nph:                                           ; preds = %113, %128
  %317 = load i32 addrspace(1)* %10               ; <i32> [#uses=1]
  %318 = sext i32 %39 to i64                      ; <i64> [#uses=3]
  %319 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %317, i32 addrspace(1)* %319
  %320 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %320
  %321 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %322 = add nsw i32 %39, 1                       ; <i32> [#uses=2]
  %323 = sext i32 %322 to i64                     ; <i64> [#uses=2]
  %324 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %323, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %321, i32 addrspace(1)* %324
  %325 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %323, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %325
  %326 = add nsw i32 %39, 2                       ; <i32> [#uses=2]
  %327 = sext i32 %326 to i64                     ; <i64> [#uses=2]
  %328 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %327, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %328
  %329 = add nsw i32 %39, 3                       ; <i32> [#uses=1]
  %330 = sext i32 %329 to i64                     ; <i64> [#uses=2]
  %331 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %330, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %331
  %332 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %327, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -2, i32 addrspace(1)* %332
  %333 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %330, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -2, i32 addrspace(1)* %333
  %334 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %tmp36 = zext i32 %322 to i64                   ; <i64> [#uses=1]
  %tmp39 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp40 = add i64 %1, %tmp39                     ; <i64> [#uses=1]
  %sext76 = shl i64 %tmp40, 32                    ; <i64> [#uses=1]
  %tmp42 = ashr i64 %sext76, 32                   ; <i64> [#uses=1]
  %tmp45 = zext i32 %39 to i64                    ; <i64> [#uses=1]
  %tmp55 = zext i32 %326 to i64                   ; <i64> [#uses=1]
  br label %335

; <label>:335                                     ; preds = %.critedge, %bb.nph
  %indvar32 = phi i64 [ 0, %bb.nph ], [ %indvar.next33, %.critedge ] ; <i64> [#uses=7]
  %tmp37 = sub i64 %tmp36, %indvar32              ; <i64> [#uses=1]
  %scevgep43 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp42, i32 0, i64 %indvar32 ; <i32 addrspace(1)*> [#uses=1]
  %tmp46 = add i64 %tmp45, %indvar32              ; <i64> [#uses=1]
  %sext87 = shl i64 %tmp46, 32                    ; <i64> [#uses=2]
  %336 = ashr i64 %sext87, 30                     ; <i64> [#uses=1]
  %scevgep44.sum = add i64 %indvar32, %336        ; <i64> [#uses=1]
  %scevgep50 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 0, i32 0, i64 %scevgep44.sum ; <i32 addrspace(1)*> [#uses=3]
  %sext77 = shl i64 %tmp37, 32                    ; <i64> [#uses=1]
  %337 = ashr i64 %sext77, 30                     ; <i64> [#uses=1]
  %scevgep44.sum78 = add i64 %indvar32, %337      ; <i64> [#uses=1]
  %scevgep53 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 0, i32 0, i64 %scevgep44.sum78 ; <i32 addrspace(1)*> [#uses=3]
  %tmp56 = add i64 %tmp55, %indvar32              ; <i64> [#uses=1]
  %tmp57 = trunc i64 %tmp56 to i32                ; <i32> [#uses=2]
  %338 = load i32 addrspace(1)* %scevgep43        ; <i32> [#uses=3]
  %339 = icmp eq i32 %338, -1                     ; <i1> [#uses=1]
  br i1 %339, label %428, label %340

; <label>:340                                     ; preds = %335
  %341 = sext i32 %338 to i64                     ; <i64> [#uses=9]
  %342 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %343 = load i32 addrspace(1)* %342              ; <i32> [#uses=1]
  %344 = icmp eq i32 %343, 1                      ; <i1> [#uses=1]
  br i1 %344, label %349, label %345

; <label>:345                                     ; preds = %340
  %346 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %347 = load i32 addrspace(1)* %346              ; <i32> [#uses=1]
  %348 = icmp eq i32 %347, 1                      ; <i1> [#uses=1]
  br i1 %348, label %349, label %362

; <label>:349                                     ; preds = %345, %340
  %350 = load i32 addrspace(1)* %97               ; <i32> [#uses=1]
  %351 = load i32 addrspace(1)* %99               ; <i32> [#uses=1]
  %352 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %353 = load i32 addrspace(1)* %352              ; <i32> [#uses=1]
  %354 = mul i32 %353, 3                          ; <i32> [#uses=1]
  %355 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %356 = load i32 addrspace(1)* %355              ; <i32> [#uses=1]
  %357 = shl i32 %356, 2                          ; <i32> [#uses=1]
  %358 = sub i32 %m_faces, %350                   ; <i32> [#uses=1]
  %359 = sub i32 %358, %351                       ; <i32> [#uses=1]
  %360 = add nsw i32 %359, %354                   ; <i32> [#uses=1]
  %361 = add nsw i32 %360, %357                   ; <i32> [#uses=1]
  br label %NewFaceIndex.exit3

; <label>:362                                     ; preds = %345
  %363 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %364 = load i32 addrspace(1)* %363              ; <i32> [#uses=1]
  %365 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %366 = load i32 addrspace(1)* %365              ; <i32> [#uses=1]
  %367 = sub i32 %338, %364                       ; <i32> [#uses=1]
  %368 = sub i32 %367, %366                       ; <i32> [#uses=1]
  br label %NewFaceIndex.exit3

NewFaceIndex.exit3:                               ; preds = %349, %362
  %369 = phi i32 [ %368, %362 ], [ %361, %349 ]   ; <i32> [#uses=9]
  %370 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %341 ; <i32 addrspace(1)*> [#uses=1]
  %371 = add nsw i32 %369, 1                      ; <i32> [#uses=3]
  %372 = sext i32 %371 to i64                     ; <i64> [#uses=2]
  %373 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %372, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %374 = sext i32 %369 to i64                     ; <i64> [#uses=3]
  %375 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %374, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %376 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %372, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %377 = sext i32 %tmp57 to i64                   ; <i64> [#uses=3]
  %378 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %377, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=3]
  %379 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %377, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=3]
  %380 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %374, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=3]
  %381 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %377, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=3]
  %382 = add nsw i32 %369, 2                      ; <i32> [#uses=3]
  %383 = sext i32 %382 to i64                     ; <i64> [#uses=1]
  %384 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %383, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %385 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %374, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp25 = zext i32 %369 to i64                   ; <i64> [#uses=1]
  br label %386

; <label>:386                                     ; preds = %408, %426, %424, %403, %395, %NewFaceIndex.exit3
  %indvar = phi i64 [ 0, %NewFaceIndex.exit3 ], [ %tmp28, %395 ], [ %tmp28, %403 ], [ %tmp28, %424 ], [ %tmp28, %426 ], [ %tmp28, %408 ] ; <i64> [#uses=4]
  %found.1 = phi i8 [ 0, %NewFaceIndex.exit3 ], [ 1, %408 ], [ 1, %424 ], [ 1, %426 ], [ %found.1, %403 ], [ %found.1, %395 ] ; <i8> [#uses=3]
  %tmp26 = add i64 %tmp25, %indvar                ; <i64> [#uses=1]
  %tmp27 = trunc i64 %tmp26 to i32                ; <i32> [#uses=3]
  %tmp28 = add i64 %indvar, 1                     ; <i64> [#uses=6]
  %tmp29 = trunc i64 %tmp28 to i32                ; <i32> [#uses=1]
  %scevgep = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %341, i32 0, i64 %indvar ; <i32 addrspace(1)*> [#uses=1]
  %v1.0 = trunc i64 %indvar to i32                ; <i32> [#uses=1]
  %387 = icmp slt i32 %v1.0, 4                    ; <i1> [#uses=1]
  br i1 %387, label %388, label %.critedge

; <label>:388                                     ; preds = %386
  %tmp = and i8 %found.1, 1                       ; <i8> [#uses=1]
  %389 = icmp eq i8 %tmp, 0                       ; <i1> [#uses=1]
  br i1 %389, label %390, label %.critedge

; <label>:390                                     ; preds = %388
  %391 = srem i32 %tmp29, 4                       ; <i32> [#uses=2]
  %392 = load i32 addrspace(1)* %scevgep          ; <i32> [#uses=2]
  %393 = load i32 addrspace(1)* %10               ; <i32> [#uses=4]
  %394 = icmp eq i32 %392, %393                   ; <i1> [#uses=1]
  br i1 %394, label %398, label %395

; <label>:395                                     ; preds = %390
  %396 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %397 = icmp eq i32 %392, %396                   ; <i1> [#uses=1]
  br i1 %397, label %398, label %386

; <label>:398                                     ; preds = %395, %390
  %399 = sext i32 %391 to i64                     ; <i64> [#uses=1]
  %400 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %341, i32 0, i64 %399 ; <i32 addrspace(1)*> [#uses=1]
  %401 = load i32 addrspace(1)* %400              ; <i32> [#uses=2]
  %402 = icmp eq i32 %401, %393                   ; <i1> [#uses=1]
  br i1 %402, label %._crit_edge80, label %403

; <label>:403                                     ; preds = %398
  %404 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %405 = icmp eq i32 %401, %404                   ; <i1> [#uses=1]
  br i1 %405, label %._crit_edge80, label %386

._crit_edge80:                                    ; preds = %403, %398
  %406 = load i32 addrspace(1)* %370              ; <i32> [#uses=1]
  %407 = icmp eq i32 %406, 0                      ; <i1> [#uses=1]
  br i1 %407, label %415, label %408

; <label>:408                                     ; preds = %._crit_edge80
  %409 = sext i32 %tmp27 to i64                   ; <i64> [#uses=1]
  %410 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %409, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %410
  %411 = add nsw i32 %391, %369                   ; <i32> [#uses=3]
  %412 = sext i32 %411 to i64                     ; <i64> [#uses=1]
  %413 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %412, i32 0, i64 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %103, i32 addrspace(1)* %413
  store i32 %tmp27, i32 addrspace(1)* %scevgep50
  store i32 %411, i32 addrspace(1)* %scevgep53
  store i32 %tmp27, i32 addrspace(1)* %378
  store i32 %411, i32 addrspace(1)* %379
  %414 = load i32 addrspace(1)* %380              ; <i32> [#uses=1]
  store i32 %414, i32 addrspace(1)* %381
  br label %386

; <label>:415                                     ; preds = %._crit_edge80
  %416 = sext i32 %393 to i64                     ; <i64> [#uses=1]
  %417 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %416 ; <i32 addrspace(1)*> [#uses=1]
  %418 = load i32 addrspace(1)* %417              ; <i32> [#uses=1]
  %419 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %420 = icmp ne i32 %418, 0                      ; <i1> [#uses=1]
  %421 = select i1 %420, i32 %419, i32 %393       ; <i32> [#uses=1]
  %422 = load i32 addrspace(1)* %373              ; <i32> [#uses=1]
  %423 = icmp eq i32 %421, %422                   ; <i1> [#uses=1]
  br i1 %423, label %424, label %426

; <label>:424                                     ; preds = %415
  store i32 %103, i32 addrspace(1)* %375
  store i32 %103, i32 addrspace(1)* %376
  store i32 %369, i32 addrspace(1)* %scevgep50
  store i32 %371, i32 addrspace(1)* %scevgep53
  store i32 %369, i32 addrspace(1)* %378
  store i32 %371, i32 addrspace(1)* %379
  %425 = load i32 addrspace(1)* %380              ; <i32> [#uses=1]
  store i32 %425, i32 addrspace(1)* %381
  br label %386

; <label>:426                                     ; preds = %415
  store i32 %103, i32 addrspace(1)* %384
  store i32 %103, i32 addrspace(1)* %385
  store i32 %382, i32 addrspace(1)* %scevgep50
  store i32 %369, i32 addrspace(1)* %scevgep53
  store i32 %382, i32 addrspace(1)* %378
  store i32 %369, i32 addrspace(1)* %379
  %427 = load i32 addrspace(1)* %380              ; <i32> [#uses=1]
  store i32 %427, i32 addrspace(1)* %381
  br label %386

; <label>:428                                     ; preds = %335
  %tmp48 = ashr i64 %sext87, 32                   ; <i64> [#uses=1]
  %429 = sext i32 %tmp57 to i64                   ; <i64> [#uses=1]
  %430 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %429, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  %431 = bitcast i32 addrspace(1)* %430 to i8*    ; <i8*> [#uses=1]
  call void @llvm.memset.i64(i8* %431, i8 -1, i64 16, i32 1)
  store i32 -1, i32 addrspace(1)* %334
  %432 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %tmp48, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 -1, i32 addrspace(1)* %432
  br label %.critedge

.critedge:                                        ; preds = %386, %388, %428
  %indvar.next33 = add i64 %indvar32, 1           ; <i64> [#uses=2]
  %exitcond = icmp eq i64 %indvar.next33, 2       ; <i1> [#uses=1]
  br i1 %exitcond, label %.loopexit, label %335

bb.nph24:                                         ; preds = %46
  %433 = sext i32 %39 to i64                      ; <i64> [#uses=3]
  %434 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %40, i32 addrspace(1)* %434
  %435 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %436 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %435, i32 addrspace(1)* %436
  %437 = sext i32 %m_faces to i64                 ; <i64> [#uses=2]
  %438 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %437 ; <i32 addrspace(1)*> [#uses=1]
  %439 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %437 ; <i32 addrspace(1)*> [#uses=1]
  %440 = sext i32 %m_edges to i64                 ; <i64> [#uses=1]
  %441 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %440 ; <i32 addrspace(1)*> [#uses=1]
  %tmp69 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp70 = add i64 %1, %tmp69                     ; <i64> [#uses=1]
  %sext79 = shl i64 %tmp70, 32                    ; <i64> [#uses=1]
  %tmp72 = ashr i64 %sext79, 32                   ; <i64> [#uses=1]
  br label %442

; <label>:442                                     ; preds = %510, %bb.nph24
  %indvar66 = phi i64 [ 0, %bb.nph24 ], [ %indvar.next67, %510 ] ; <i64> [#uses=3]
  %scevgep73 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp72, i32 0, i64 %indvar66 ; <i32 addrspace(1)*> [#uses=2]
  %scevgep75 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 0, i64 %indvar66 ; <i32 addrspace(1)*> [#uses=3]
  %443 = load i32 addrspace(1)* %scevgep73        ; <i32> [#uses=3]
  %444 = icmp eq i32 %443, -1                     ; <i1> [#uses=1]
  br i1 %444, label %509, label %445

; <label>:445                                     ; preds = %442
  %446 = load i32 addrspace(1)* %10               ; <i32> [#uses=2]
  %447 = sext i32 %446 to i64                     ; <i64> [#uses=1]
  %448 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %447 ; <i32 addrspace(1)*> [#uses=1]
  %449 = load i32 addrspace(1)* %448              ; <i32> [#uses=1]
  %450 = load i32 addrspace(1)* %14               ; <i32> [#uses=1]
  %451 = sext i32 %443 to i64                     ; <i64> [#uses=7]
  %452 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %453 = load i32 addrspace(1)* %452              ; <i32> [#uses=2]
  %454 = icmp eq i32 %453, 1                      ; <i1> [#uses=1]
  br i1 %454, label %459, label %455

; <label>:455                                     ; preds = %445
  %456 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %457 = load i32 addrspace(1)* %456              ; <i32> [#uses=1]
  %458 = icmp eq i32 %457, 1                      ; <i1> [#uses=1]
  br i1 %458, label %459, label %472

; <label>:459                                     ; preds = %455, %445
  %460 = load i32 addrspace(1)* %438              ; <i32> [#uses=1]
  %461 = load i32 addrspace(1)* %439              ; <i32> [#uses=1]
  %462 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %463 = load i32 addrspace(1)* %462              ; <i32> [#uses=1]
  %464 = mul i32 %463, 3                          ; <i32> [#uses=1]
  %465 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %466 = load i32 addrspace(1)* %465              ; <i32> [#uses=1]
  %467 = shl i32 %466, 2                          ; <i32> [#uses=1]
  %468 = sub i32 %m_faces, %460                   ; <i32> [#uses=1]
  %469 = sub i32 %468, %461                       ; <i32> [#uses=1]
  %470 = add nsw i32 %469, %464                   ; <i32> [#uses=1]
  %471 = add nsw i32 %470, %467                   ; <i32> [#uses=1]
  br label %NewFaceIndex.exit

; <label>:472                                     ; preds = %455
  %473 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %474 = load i32 addrspace(1)* %473              ; <i32> [#uses=1]
  %475 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %451 ; <i32 addrspace(1)*> [#uses=1]
  %476 = load i32 addrspace(1)* %475              ; <i32> [#uses=1]
  %477 = sub i32 %443, %474                       ; <i32> [#uses=1]
  %478 = sub i32 %477, %476                       ; <i32> [#uses=1]
  br label %NewFaceIndex.exit

NewFaceIndex.exit:                                ; preds = %459, %472
  %479 = phi i32 [ %478, %472 ], [ %471, %459 ]   ; <i32> [#uses=6]
  %480 = icmp eq i32 %453, 0                      ; <i1> [#uses=1]
  br i1 %480, label %508, label %481

; <label>:481                                     ; preds = %NewFaceIndex.exit
  %482 = icmp ne i32 %449, 0                      ; <i1> [#uses=1]
  %483 = select i1 %482, i32 %446, i32 %450       ; <i32> [#uses=1]
  %484 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %451, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %485 = load i32 addrspace(1)* %484              ; <i32> [#uses=1]
  %486 = icmp eq i32 %485, %483                   ; <i1> [#uses=3]
  %487 = add nsw i32 %479, 2                      ; <i32> [#uses=2]
  %.pre82 = add nsw i32 %479, 1                   ; <i32> [#uses=1]
  %488 = add nsw i32 %479, 1                      ; <i32> [#uses=2]
  %.pre83 = add nsw i32 %479, 2                   ; <i32> [#uses=1]
  %.pre-phi84 = select i1 %486, i32 %.pre83, i32 %487 ; <i32> [#uses=1]
  %.pre-phi = select i1 %486, i32 %488, i32 %.pre82 ; <i32> [#uses=2]
  %storemerge = select i1 %486, i32 %488, i32 %487 ; <i32> [#uses=1]
  store i32 %storemerge, i32 addrspace(1)* %scevgep75
  %489 = load i32 addrspace(1)* %441              ; <i32> [#uses=1]
  %490 = mul i32 %489, 3                          ; <i32> [#uses=1]
  %491 = load i32 addrspace(1)* %scevgep73        ; <i32> [#uses=1]
  %492 = sext i32 %491 to i64                     ; <i64> [#uses=1]
  %493 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %492 ; <i32 addrspace(1)*> [#uses=1]
  %494 = load i32 addrspace(1)* %493              ; <i32> [#uses=1]
  %495 = add nsw i32 %490, %m_edges               ; <i32> [#uses=1]
  %496 = add nsw i32 %495, %494                   ; <i32> [#uses=1]
  %497 = sext i32 %496 to i64                     ; <i64> [#uses=4]
  %498 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %497, i32 0, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %.pre-phi, i32 addrspace(1)* %498
  %499 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %497, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %.pre-phi84, i32 addrspace(1)* %499
  %500 = sext i32 %.pre-phi to i64                ; <i64> [#uses=1]
  %501 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %500, i32 0, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %502 = load i32 addrspace(1)* %501              ; <i32> [#uses=1]
  %503 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %497, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %502, i32 addrspace(1)* %503
  %504 = sext i32 %479 to i64                     ; <i64> [#uses=1]
  %505 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %504, i32 0, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  %506 = load i32 addrspace(1)* %505              ; <i32> [#uses=1]
  %507 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %497, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %506, i32 addrspace(1)* %507
  br label %510

; <label>:508                                     ; preds = %NewFaceIndex.exit
  store i32 %479, i32 addrspace(1)* %scevgep75
  br label %510

; <label>:509                                     ; preds = %442
  store i32 -1, i32 addrspace(1)* %scevgep75
  br label %510

; <label>:510                                     ; preds = %509, %508, %481
  %indvar.next67 = add i64 %indvar66, 1           ; <i64> [#uses=2]
  %exitcond68 = icmp eq i64 %indvar.next67, 2     ; <i1> [#uses=1]
  br i1 %exitcond68, label %.loopexit, label %442

.loopexit:                                        ; preds = %510, %.critedge
  %511 = sext i32 %39 to i64                      ; <i64> [#uses=3]
  %512 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 0 ; <i32 addrspace(1)*> [#uses=2]
  %513 = load i32 addrspace(1)* %512              ; <i32> [#uses=1]
  %514 = icmp slt i32 %513, 0                     ; <i1> [#uses=1]
  br i1 %514, label %._crit_edge86, label %515

; <label>:515                                     ; preds = %.loopexit
  %516 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  %517 = load i32 addrspace(1)* %516              ; <i32> [#uses=1]
  %518 = icmp slt i32 %517, 0                     ; <i1> [#uses=1]
  br i1 %518, label %._crit_edge86, label %520

._crit_edge86:                                    ; preds = %.loopexit, %515
  store i32 0, i32 addrspace(1)* %512
  %519 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %519
  ret void

; <label>:520                                     ; preds = %515, %0
  ret void
}

define void @UpdateVerticesKernel(i32 addrspace(1)* nocapture %dVertex, %struct._Vertex addrspace(1)* nocapture %m_pInVB, %struct._Vertex addrspace(1)* nocapture %m_pOutVB, i32 %vertices) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=2]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=2]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=2]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=2]
  %7 = icmp slt i32 %6, %vertices                 ; <i1> [#uses=1]
  br i1 %7, label %8, label %41

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=4]
  %10 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %9, i32 0 ; <float addrspace(1)*> [#uses=1]
  %11 = load float addrspace(1)* %10              ; <float> [#uses=8]
  %12 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %9, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %11, float addrspace(1)* %12
  %13 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %9, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %13
  %14 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %15 = load i32 addrspace(1)* %14                ; <i32> [#uses=1]
  %16 = icmp eq i32 %15, 0                        ; <i1> [#uses=1]
  %tmp14 = mul i64 %2, %3                         ; <i64> [#uses=1]
  %tmp15 = add i64 %1, %tmp14                     ; <i64> [#uses=1]
  %sext20 = shl i64 %tmp15, 32                    ; <i64> [#uses=1]
  %tmp17 = ashr i64 %sext20, 32                   ; <i64> [#uses=10]
  %scevgep18 = getelementptr %struct._Vertex addrspace(1)* %m_pInVB, i64 %tmp17, i32 1, i64 0 ; <float addrspace(1)*> [#uses=1]
  %scevgep19 = getelementptr %struct._Vertex addrspace(1)* %m_pOutVB, i64 %tmp17, i32 1, i64 0 ; <float addrspace(1)*> [#uses=3]
  %17 = load float addrspace(1)* %scevgep18       ; <float> [#uses=3]
  br i1 %16, label %.loopexit3, label %.loopexit

.loopexit:                                        ; preds = %8
  %18 = fadd float %11, -3.000000e+000            ; <float> [#uses=3]
  %19 = fmul float %17, %18                       ; <float> [#uses=1]
  %20 = load float addrspace(1)* %scevgep19       ; <float> [#uses=1]
  %21 = fsub float %20, %17                       ; <float> [#uses=1]
  %22 = fdiv float %21, %11                       ; <float> [#uses=1]
  %23 = fadd float %19, %22                       ; <float> [#uses=1]
  %24 = fdiv float %23, %11                       ; <float> [#uses=1]
  store float %24, float addrspace(1)* %scevgep19
  %scevgep.1 = getelementptr %struct._Vertex addrspace(1)* %m_pInVB, i64 %tmp17, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %scevgep10.1 = getelementptr %struct._Vertex addrspace(1)* %m_pOutVB, i64 %tmp17, i32 1, i64 1 ; <float addrspace(1)*> [#uses=2]
  %25 = load float addrspace(1)* %scevgep.1       ; <float> [#uses=2]
  %26 = fmul float %25, %18                       ; <float> [#uses=1]
  %27 = load float addrspace(1)* %scevgep10.1     ; <float> [#uses=1]
  %28 = fsub float %27, %25                       ; <float> [#uses=1]
  %29 = fdiv float %28, %11                       ; <float> [#uses=1]
  %30 = fadd float %26, %29                       ; <float> [#uses=1]
  %31 = fdiv float %30, %11                       ; <float> [#uses=1]
  store float %31, float addrspace(1)* %scevgep10.1
  %scevgep.2 = getelementptr %struct._Vertex addrspace(1)* %m_pInVB, i64 %tmp17, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %scevgep10.2 = getelementptr %struct._Vertex addrspace(1)* %m_pOutVB, i64 %tmp17, i32 1, i64 2 ; <float addrspace(1)*> [#uses=2]
  %32 = load float addrspace(1)* %scevgep.2       ; <float> [#uses=2]
  %33 = fmul float %32, %18                       ; <float> [#uses=1]
  %34 = load float addrspace(1)* %scevgep10.2     ; <float> [#uses=1]
  %35 = fsub float %34, %32                       ; <float> [#uses=1]
  %36 = fdiv float %35, %11                       ; <float> [#uses=1]
  %37 = fadd float %33, %36                       ; <float> [#uses=1]
  %38 = fdiv float %37, %11                       ; <float> [#uses=1]
  store float %38, float addrspace(1)* %scevgep10.2
  ret void

.loopexit3:                                       ; preds = %8
  store float %17, float addrspace(1)* %scevgep19
  %scevgep18.1 = getelementptr %struct._Vertex addrspace(1)* %m_pInVB, i64 %tmp17, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %scevgep19.1 = getelementptr %struct._Vertex addrspace(1)* %m_pOutVB, i64 %tmp17, i32 1, i64 1 ; <float addrspace(1)*> [#uses=1]
  %39 = load float addrspace(1)* %scevgep18.1     ; <float> [#uses=1]
  store float %39, float addrspace(1)* %scevgep19.1
  %scevgep18.2 = getelementptr %struct._Vertex addrspace(1)* %m_pInVB, i64 %tmp17, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %scevgep19.2 = getelementptr %struct._Vertex addrspace(1)* %m_pOutVB, i64 %tmp17, i32 1, i64 2 ; <float addrspace(1)*> [#uses=1]
  %40 = load float addrspace(1)* %scevgep18.2     ; <float> [#uses=1]
  store float %40, float addrspace(1)* %scevgep19.2
  ret void

; <label>:41                                      ; preds = %0
  ret void
}

define void @GetScanTotalsKernel(i32 addrspace(1)* nocapture %dScanTotals, i32 addrspace(1)* nocapture %dEdgeScanned, i32 addrspace(1)* nocapture %dFace3Scanned, i32 addrspace(1)* nocapture %dFace4Scanned, i32 %edges, i32 %faces) nounwind {
  %1 = sext i32 %edges to i64                     ; <i64> [#uses=1]
  %2 = getelementptr inbounds i32 addrspace(1)* %dEdgeScanned, i64 %1 ; <i32 addrspace(1)*> [#uses=1]
  %3 = load i32 addrspace(1)* %2                  ; <i32> [#uses=1]
  store i32 %3, i32 addrspace(1)* %dScanTotals
  %4 = sext i32 %faces to i64                     ; <i64> [#uses=2]
  %5 = getelementptr inbounds i32 addrspace(1)* %dFace4Scanned, i64 %4 ; <i32 addrspace(1)*> [#uses=1]
  %6 = load i32 addrspace(1)* %5                  ; <i32> [#uses=1]
  %7 = getelementptr inbounds i32 addrspace(1)* %dScanTotals, i64 1 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %6, i32 addrspace(1)* %7
  %8 = getelementptr inbounds i32 addrspace(1)* %dFace3Scanned, i64 %4 ; <i32 addrspace(1)*> [#uses=1]
  %9 = load i32 addrspace(1)* %8                  ; <i32> [#uses=1]
  %10 = getelementptr inbounds i32 addrspace(1)* %dScanTotals, i64 2 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %9, i32 addrspace(1)* %10
  ret void
}

define void @ParallelScanKernel(i32 addrspace(1)* nocapture %dIn, i32 addrspace(1)* nocapture %dOut, i32 %start, i32 %numFlags) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind ; <i64> [#uses=1]
  %2 = tail call i64 @get_local_size(i32 0) nounwind ; <i64> [#uses=1]
  %3 = tail call i64 @get_group_id(i32 0) nounwind ; <i64> [#uses=1]
  %4 = mul i64 %3, %2                             ; <i64> [#uses=1]
  %5 = add i64 %4, %1                             ; <i64> [#uses=1]
  %6 = trunc i64 %5 to i32                        ; <i32> [#uses=5]
  %7 = icmp slt i32 %6, %start                    ; <i1> [#uses=1]
  br i1 %7, label %8, label %13

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64                         ; <i64> [#uses=2]
  %10 = getelementptr inbounds i32 addrspace(1)* %dIn, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  %11 = load i32 addrspace(1)* %10                ; <i32> [#uses=1]
  %12 = getelementptr inbounds i32 addrspace(1)* %dOut, i64 %9 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %11, i32 addrspace(1)* %12
  ret void

; <label>:13                                      ; preds = %0
  %14 = icmp slt i32 %6, %numFlags                ; <i1> [#uses=1]
  br i1 %14, label %15, label %25

; <label>:15                                      ; preds = %13
  %16 = sext i32 %6 to i64                        ; <i64> [#uses=2]
  %17 = getelementptr inbounds i32 addrspace(1)* %dIn, i64 %16 ; <i32 addrspace(1)*> [#uses=1]
  %18 = load i32 addrspace(1)* %17                ; <i32> [#uses=1]
  %19 = sub i32 %6, %start                        ; <i32> [#uses=1]
  %20 = sext i32 %19 to i64                       ; <i64> [#uses=1]
  %21 = getelementptr inbounds i32 addrspace(1)* %dIn, i64 %20 ; <i32 addrspace(1)*> [#uses=1]
  %22 = load i32 addrspace(1)* %21                ; <i32> [#uses=1]
  %23 = add nsw i32 %22, %18                      ; <i32> [#uses=1]
  %24 = getelementptr inbounds i32 addrspace(1)* %dOut, i64 %16 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %23, i32 addrspace(1)* %24
  ret void

; <label>:25                                      ; preds = %13
  ret void
}

define void @SequentialScanKernel(i32 addrspace(1)* nocapture %dResult, i32 addrspace(1)* nocapture %dInput, i32 %count) nounwind {
  %1 = add nsw i32 %count, 1                      ; <i32> [#uses=2]
  %2 = icmp sgt i32 %1, 0                         ; <i1> [#uses=1]
  br i1 %2, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %0
  %tmp = zext i32 %1 to i64                       ; <i64> [#uses=1]
  br label %3

; <label>:3                                       ; preds = %3, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %3 ] ; <i64> [#uses=3]
  %scan.01 = phi i32 [ 0, %bb.nph ], [ %5, %3 ]   ; <i32> [#uses=2]
  %scevgep = getelementptr i32 addrspace(1)* %dResult, i64 %indvar ; <i32 addrspace(1)*> [#uses=1]
  store i32 %scan.01, i32 addrspace(1)* %scevgep
  %scevgep3 = getelementptr i32 addrspace(1)* %dInput, i64 %indvar ; <i32 addrspace(1)*> [#uses=1]
  %4 = load i32 addrspace(1)* %scevgep3           ; <i32> [#uses=1]
  %5 = add nsw i32 %4, %scan.01                   ; <i32> [#uses=1]
  %indvar.next = add i64 %indvar, 1               ; <i64> [#uses=2]
  %exitcond = icmp eq i64 %indvar.next, %tmp      ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge, label %3

._crit_edge:                                      ; preds = %3, %0
  ret void
}

declare float @_Z4sqrtf(float)

declare void @llvm.memset.i64(i8* nocapture, i8, i64, i32) nounwind
