; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'file.s'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @GenerateEdgePointsKernel
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: ret

%struct._Edge = type { [2 x i32], [2 x i32] }
%struct._Face = type { [4 x i32] }
%struct._Vertex = type { float, [3 x float], [4 x float], i32 }

@opencl_GenerateFacePointsKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata"
@opencl_GenerateFacePointsKernel_parameters = appending global [427 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int\00", section "llvm.metadata"
@opencl_GenerateEdgePointsKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata"
@opencl_GenerateEdgePointsKernel_parameters = appending global [680 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, Edge __attribute__((address_space(1))) *, Edge __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Face __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, Vertex __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata"

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_group_id(i32)

declare i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)*, i32, i32)

define void @GenerateEdgePointsKernel(i32 addrspace(1)* nocapture %dEdge, i32 addrspace(1)* nocapture %dFace3, i32 addrspace(1)* nocapture %dFace4, i32 addrspace(1)* nocapture %dActive, i32 addrspace(1)* nocapture %dPotentiallyActive, i32 addrspace(1)* nocapture %dNewPotentiallyActive, i32 addrspace(1)* nocapture %dVertex, %struct._Edge addrspace(1)* nocapture %m_pInEB, %struct._Edge addrspace(1)* %m_pOutEB, %struct._Face addrspace(1)* %m_pInFB, %struct._Face addrspace(1)* nocapture %m_pOutFB, %struct._Vertex addrspace(1)* %m_pInVB, %struct._Vertex addrspace(1)* %m_pOutVB, i32 addrspace(1)* nocapture %m_pFace3Scanned, i32 addrspace(1)* nocapture %m_pFace4Scanned, i32 addrspace(1)* nocapture %m_pEdgeScanned, i32 %m_vertices, i32 %m_faces, i32 %m_edges) nounwind {
  %1 = tail call i64 @get_local_id(i32 0) nounwind
  %2 = tail call i64 @get_local_size(i32 0) nounwind
  %3 = tail call i64 @get_group_id(i32 0) nounwind
  %4 = mul i64 %3, %2
  %5 = add i64 %4, %1
  %6 = trunc i64 %5 to i32
  %7 = icmp slt i32 %6, %m_edges
  br i1 %7, label %8, label %520

; <label>:8                                       ; preds = %0
  %9 = sext i32 %6 to i64
  %10 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 1, i64 0
  %11 = load i32 addrspace(1)* %10, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %12
  store i32 0, i32 addrspace(1)* %13, align 4
  %14 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 1, i64 1
  %15 = load i32 addrspace(1)* %14, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %16
  store i32 0, i32 addrspace(1)* %17, align 4
  %18 = getelementptr inbounds i32 addrspace(1)* %dEdge, i64 %9
  %19 = load i32 addrspace(1)* %18, align 4
  %20 = icmp eq i32 %19, 1
  br i1 %20, label %21, label %30

; <label>:21                                      ; preds = %8
  %22 = sext i32 %m_edges to i64
  %23 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %22
  %24 = load i32 addrspace(1)* %23, align 4
  %25 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9
  %26 = load i32 addrspace(1)* %25, align 4
  %27 = shl i32 %26, 2
  %28 = sub i32 %m_edges, %24
  %29 = add nsw i32 %28, %27
  br label %34

; <label>:30                                      ; preds = %8
  %31 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9
  %32 = load i32 addrspace(1)* %31, align 4
  %33 = sub i32 %6, %32
  br label %34

; <label>:34                                      ; preds = %30, %21
  %newEdgeIndex.0.i = phi i32 [ %29, %21 ], [ %33, %30 ]
  %35 = icmp slt i32 %newEdgeIndex.0.i, 0
  br i1 %35, label %NewEdgeIndex.exit, label %36

; <label>:36                                      ; preds = %34
  %37 = shl i32 %m_edges, 2
  %38 = icmp slt i32 %37, %newEdgeIndex.0.i
  %retval.i = select i1 %38, i32 0, i32 %newEdgeIndex.0.i
  br label %NewEdgeIndex.exit

NewEdgeIndex.exit:                                ; preds = %36, %34
  %39 = phi i32 [ %retval.i, %36 ], [ 0, %34 ]
  %40 = load i32 addrspace(1)* %10, align 4
  %41 = icmp eq i32 %40, -1
  br i1 %41, label %42, label %46

; <label>:42                                      ; preds = %NewEdgeIndex.exit
  %43 = sext i32 %39 to i64
  %44 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %43, i32 0, i64 0
  %45 = bitcast i32 addrspace(1)* %44 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %45, i8 -1, i64 16, i32 1, i1 false)
  br label %UnifiedReturnBlock

; <label>:46                                      ; preds = %NewEdgeIndex.exit
  %47 = icmp eq i32 %19, 0
  br i1 %47, label %bb.nph24, label %bb.nph22

bb.nph22:                                         ; preds = %46
  %48 = sext i32 %40 to i64
  %49 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 0
  %50 = load float addrspace(1)* %49, align 4
  %51 = load i32 addrspace(1)* %14, align 4
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 0
  %54 = load float addrspace(1)* %53, align 4
  %55 = fadd float %50, %54
  %56 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 1
  %57 = load float addrspace(1)* %56, align 4
  %58 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 1
  %59 = load float addrspace(1)* %58, align 4
  %60 = fadd float %57, %59
  %61 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %48, i32 1, i64 2
  %62 = load float addrspace(1)* %61, align 4
  %63 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %52, i32 1, i64 2
  %64 = load float addrspace(1)* %63, align 4
  %65 = fadd float %62, %64
  %tmp53 = add i64 %1, %4
  %sext = shl i64 %tmp53, 32
  %tmp55 = ashr i64 %sext, 32
  br label %.preheader

.preheader:                                       ; preds = %94, %bb.nph22
  %indvar58 = phi i64 [ 0, %bb.nph22 ], [ %indvar.next59, %94 ]
  %scevgep65 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp55, i32 1, i64 %indvar58
  br label %66

; <label>:66                                      ; preds = %66, %.preheader
  %67 = load i32 addrspace(1)* %scevgep65, align 4
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %68, i32 3
  %70 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %69, i32 0, i32 1) nounwind
  %71 = icmp eq i32 %70, 0
  br i1 %71, label %72, label %66

; <label>:72                                      ; preds = %66
  %73 = load i32 addrspace(1)* %scevgep65, align 4
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %74, i32 1, i64 0
  %76 = load float addrspace(1)* %75, align 4
  %77 = fadd float %76, %55
  store float %77, float addrspace(1)* %75, align 4
  %78 = load i32 addrspace(1)* %scevgep65, align 4
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %79, i32 1, i64 1
  %81 = load float addrspace(1)* %80, align 4
  %82 = fadd float %81, %60
  store float %82, float addrspace(1)* %80, align 4
  %83 = load i32 addrspace(1)* %scevgep65, align 4
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %84, i32 1, i64 2
  %86 = load float addrspace(1)* %85, align 4
  %87 = fadd float %86, %65
  store float %87, float addrspace(1)* %85, align 4
  br label %88

; <label>:88                                      ; preds = %88, %72
  %89 = load i32 addrspace(1)* %scevgep65, align 4
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %90, i32 3
  %92 = tail call i32 @_Z12atom_cmpxchgPo1iii(i32 addrspace(1)* %91, i32 1, i32 0) nounwind
  %93 = icmp eq i32 %92, 1
  br i1 %93, label %94, label %88

; <label>:94                                      ; preds = %88
  %indvar.next59 = add i64 %indvar58, 1
  %exitcond = icmp eq i64 %indvar.next59, 2
  br i1 %exitcond, label %95, label %.preheader

; <label>:95                                      ; preds = %94
  %.phi.trans.insert = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %9
  %.pre = load i32 addrspace(1)* %.phi.trans.insert, align 4
  %96 = sext i32 %m_faces to i64
  %97 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %96
  %98 = load i32 addrspace(1)* %97, align 4
  %99 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %96
  %100 = load i32 addrspace(1)* %99, align 4
  %101 = add nsw i32 %.pre, %m_vertices
  %102 = add nsw i32 %101, %98
  %103 = add nsw i32 %102, %100
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds i32 addrspace(1)* %dNewPotentiallyActive, i64 %104
  store i32 1, i32 addrspace(1)* %105, align 4
  %106 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 0, i64 0
  %107 = load i32 addrspace(1)* %106, align 4
  %108 = icmp eq i32 %107, -1
  br i1 %108, label %113, label %109

; <label>:109                                     ; preds = %95
  %110 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pInEB, i64 %9, i32 0, i64 1
  %111 = load i32 addrspace(1)* %110, align 4
  %112 = icmp eq i32 %111, -1
  br i1 %112, label %113, label %128

; <label>:113                                     ; preds = %109, %95
  %114 = fdiv float %55, 2.000000e+000
  %115 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 0
  store float %114, float addrspace(1)* %115, align 4
  %116 = fdiv float %60, 2.000000e+000
  %117 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 1
  store float %116, float addrspace(1)* %117, align 4
  %118 = fdiv float %65, 2.000000e+000
  %119 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 2
  store float %118, float addrspace(1)* %119, align 4
  %120 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 0
  store float 3.000000e+000, float addrspace(1)* %120, align 4
  %121 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 3
  store i32 0, i32 addrspace(1)* %121, align 4
  %122 = load i32 addrspace(1)* %10, align 4
  %123 = sext i32 %122 to i64
  %124 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %123
  store i32 0, i32 addrspace(1)* %124, align 4
  %125 = load i32 addrspace(1)* %14, align 4
  %126 = sext i32 %125 to i64
  %127 = getelementptr inbounds i32 addrspace(1)* %dVertex, i64 %126
  store i32 0, i32 addrspace(1)* %127, align 4
  br label %bb.nph

; <label>:128                                     ; preds = %109
  %129 = load i32 addrspace(1)* %10, align 4
  %130 = sext i32 %129 to i64
  %131 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %130, i32 1, i64 0
  %132 = load float addrspace(1)* %131, align 4
  %133 = load i32 addrspace(1)* %14, align 4
  %134 = sext i32 %133 to i64
  %135 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %134, i32 1, i64 0
  %136 = load float addrspace(1)* %135, align 4
  %137 = fadd float %132, %136
  %138 = sext i32 %107 to i64
  %139 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 0
  %140 = load i32 addrspace(1)* %139, align 4
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %141, i32 1, i64 0
  %143 = load float addrspace(1)* %142, align 4
  %144 = fadd float %137, %143
  %145 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 1
  %146 = load i32 addrspace(1)* %145, align 4
  %147 = sext i32 %146 to i64
  %148 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %147, i32 1, i64 0
  %149 = load float addrspace(1)* %148, align 4
  %150 = fadd float %144, %149
  %151 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 2
  %152 = load i32 addrspace(1)* %151, align 4
  %153 = sext i32 %152 to i64
  %154 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %153, i32 1, i64 0
  %155 = load float addrspace(1)* %154, align 4
  %156 = fadd float %150, %155
  %157 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %138, i32 0, i64 3
  %158 = load i32 addrspace(1)* %157, align 4
  %159 = sext i32 %158 to i64
  %160 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %159, i32 1, i64 0
  %161 = load float addrspace(1)* %160, align 4
  %162 = fadd float %156, %161
  %163 = sext i32 %111 to i64
  %164 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 0
  %165 = load i32 addrspace(1)* %164, align 4
  %166 = sext i32 %165 to i64
  %167 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %166, i32 1, i64 0
  %168 = load float addrspace(1)* %167, align 4
  %169 = fadd float %162, %168
  %170 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 1
  %171 = load i32 addrspace(1)* %170, align 4
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %172, i32 1, i64 0
  %174 = load float addrspace(1)* %173, align 4
  %175 = fadd float %169, %174
  %176 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 2
  %177 = load i32 addrspace(1)* %176, align 4
  %178 = sext i32 %177 to i64
  %179 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %178, i32 1, i64 0
  %180 = load float addrspace(1)* %179, align 4
  %181 = fadd float %175, %180
  %182 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %163, i32 0, i64 3
  %183 = load i32 addrspace(1)* %182, align 4
  %184 = sext i32 %183 to i64
  %185 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %184, i32 1, i64 0
  %186 = load float addrspace(1)* %185, align 4
  %187 = fadd float %181, %186
  %188 = fdiv float %187, 1.000000e+001
  %189 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 0
  store float %188, float addrspace(1)* %189, align 4
  %190 = load i32 addrspace(1)* %10, align 4
  %191 = sext i32 %190 to i64
  %192 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %191, i32 1, i64 1
  %193 = load float addrspace(1)* %192, align 4
  %194 = load i32 addrspace(1)* %14, align 4
  %195 = sext i32 %194 to i64
  %196 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %195, i32 1, i64 1
  %197 = load float addrspace(1)* %196, align 4
  %198 = fadd float %193, %197
  %199 = load i32 addrspace(1)* %106, align 4
  %200 = sext i32 %199 to i64
  %201 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 0
  %202 = load i32 addrspace(1)* %201, align 4
  %203 = sext i32 %202 to i64
  %204 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %203, i32 1, i64 1
  %205 = load float addrspace(1)* %204, align 4
  %206 = fadd float %198, %205
  %207 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 1
  %208 = load i32 addrspace(1)* %207, align 4
  %209 = sext i32 %208 to i64
  %210 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %209, i32 1, i64 1
  %211 = load float addrspace(1)* %210, align 4
  %212 = fadd float %206, %211
  %213 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 2
  %214 = load i32 addrspace(1)* %213, align 4
  %215 = sext i32 %214 to i64
  %216 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %215, i32 1, i64 1
  %217 = load float addrspace(1)* %216, align 4
  %218 = fadd float %212, %217
  %219 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %200, i32 0, i64 3
  %220 = load i32 addrspace(1)* %219, align 4
  %221 = sext i32 %220 to i64
  %222 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %221, i32 1, i64 1
  %223 = load float addrspace(1)* %222, align 4
  %224 = fadd float %218, %223
  %225 = load i32 addrspace(1)* %110, align 4
  %226 = sext i32 %225 to i64
  %227 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 0
  %228 = load i32 addrspace(1)* %227, align 4
  %229 = sext i32 %228 to i64
  %230 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %229, i32 1, i64 1
  %231 = load float addrspace(1)* %230, align 4
  %232 = fadd float %224, %231
  %233 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 1
  %234 = load i32 addrspace(1)* %233, align 4
  %235 = sext i32 %234 to i64
  %236 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %235, i32 1, i64 1
  %237 = load float addrspace(1)* %236, align 4
  %238 = fadd float %232, %237
  %239 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 2
  %240 = load i32 addrspace(1)* %239, align 4
  %241 = sext i32 %240 to i64
  %242 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %241, i32 1, i64 1
  %243 = load float addrspace(1)* %242, align 4
  %244 = fadd float %238, %243
  %245 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %226, i32 0, i64 3
  %246 = load i32 addrspace(1)* %245, align 4
  %247 = sext i32 %246 to i64
  %248 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %247, i32 1, i64 1
  %249 = load float addrspace(1)* %248, align 4
  %250 = fadd float %244, %249
  %251 = fdiv float %250, 1.000000e+001
  %252 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 1
  store float %251, float addrspace(1)* %252, align 4
  %253 = load i32 addrspace(1)* %10, align 4
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %254, i32 1, i64 2
  %256 = load float addrspace(1)* %255, align 4
  %257 = load i32 addrspace(1)* %14, align 4
  %258 = sext i32 %257 to i64
  %259 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %258, i32 1, i64 2
  %260 = load float addrspace(1)* %259, align 4
  %261 = fadd float %256, %260
  %262 = load i32 addrspace(1)* %106, align 4
  %263 = sext i32 %262 to i64
  %264 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 0
  %265 = load i32 addrspace(1)* %264, align 4
  %266 = sext i32 %265 to i64
  %267 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %266, i32 1, i64 2
  %268 = load float addrspace(1)* %267, align 4
  %269 = fadd float %261, %268
  %270 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 1
  %271 = load i32 addrspace(1)* %270, align 4
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %272, i32 1, i64 2
  %274 = load float addrspace(1)* %273, align 4
  %275 = fadd float %269, %274
  %276 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 2
  %277 = load i32 addrspace(1)* %276, align 4
  %278 = sext i32 %277 to i64
  %279 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %278, i32 1, i64 2
  %280 = load float addrspace(1)* %279, align 4
  %281 = fadd float %275, %280
  %282 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %263, i32 0, i64 3
  %283 = load i32 addrspace(1)* %282, align 4
  %284 = sext i32 %283 to i64
  %285 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %284, i32 1, i64 2
  %286 = load float addrspace(1)* %285, align 4
  %287 = fadd float %281, %286
  %288 = load i32 addrspace(1)* %110, align 4
  %289 = sext i32 %288 to i64
  %290 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 0
  %291 = load i32 addrspace(1)* %290, align 4
  %292 = sext i32 %291 to i64
  %293 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %292, i32 1, i64 2
  %294 = load float addrspace(1)* %293, align 4
  %295 = fadd float %287, %294
  %296 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 1
  %297 = load i32 addrspace(1)* %296, align 4
  %298 = sext i32 %297 to i64
  %299 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %298, i32 1, i64 2
  %300 = load float addrspace(1)* %299, align 4
  %301 = fadd float %295, %300
  %302 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 2
  %303 = load i32 addrspace(1)* %302, align 4
  %304 = sext i32 %303 to i64
  %305 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %304, i32 1, i64 2
  %306 = load float addrspace(1)* %305, align 4
  %307 = fadd float %301, %306
  %308 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %289, i32 0, i64 3
  %309 = load i32 addrspace(1)* %308, align 4
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pInVB, i64 %310, i32 1, i64 2
  %312 = load float addrspace(1)* %311, align 4
  %313 = fadd float %307, %312
  %314 = fdiv float %313, 1.000000e+001
  %315 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 1, i64 2
  store float %314, float addrspace(1)* %315, align 4
  %316 = getelementptr inbounds %struct._Vertex addrspace(1)* %m_pOutVB, i64 %104, i32 0
  store float 4.000000e+000, float addrspace(1)* %316, align 4
  br label %bb.nph

bb.nph:                                           ; preds = %128, %113
  %317 = load i32 addrspace(1)* %10, align 4
  %318 = sext i32 %39 to i64
  %319 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 1, i64 0
  store i32 %317, i32 addrspace(1)* %319, align 4
  %320 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 1, i64 1
  store i32 %103, i32 addrspace(1)* %320, align 4
  %321 = load i32 addrspace(1)* %14, align 4
  %322 = add nsw i32 %39, 1
  %323 = sext i32 %322 to i64
  %324 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %323, i32 1, i64 1
  store i32 %321, i32 addrspace(1)* %324, align 4
  %325 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %323, i32 1, i64 0
  store i32 %103, i32 addrspace(1)* %325, align 4
  %326 = add nsw i32 %39, 2
  %327 = sext i32 %326 to i64
  %328 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %327, i32 1, i64 0
  store i32 %103, i32 addrspace(1)* %328, align 4
  %329 = add nsw i32 %39, 3
  %330 = sext i32 %329 to i64
  %331 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %330, i32 1, i64 0
  store i32 %103, i32 addrspace(1)* %331, align 4
  %332 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %327, i32 1, i64 1
  store i32 -2, i32 addrspace(1)* %332, align 4
  %333 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %330, i32 1, i64 1
  store i32 -2, i32 addrspace(1)* %333, align 4
  %334 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %318, i32 0, i64 1
  %tmp29 = zext i32 %326 to i64
  %tmp33 = zext i32 %39 to i64
  %tmp34 = shl i64 %tmp33, 32
  %tmp50 = zext i32 %322 to i64
  %tmp51 = shl i64 %tmp50, 32
  br label %335

; <label>:335                                     ; preds = %.critedge, %bb.nph
  %indvar32 = phi i64 [ 0, %bb.nph ], [ %indvar.next33, %.critedge ]
  %tmp30 = add i64 %tmp29, %indvar32
  %tmp57 = trunc i64 %tmp30 to i32
  %tmp32 = shl i64 %indvar32, 32
  %sext87 = add i64 %tmp34, %tmp32
  %scevgep43 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp55, i32 0, i64 %indvar32
  %tmp47 = mul i64 %indvar32, -4294967296
  %sext77 = add i64 %tmp51, %tmp47
  %336 = ashr i64 %sext87, 30
  %337 = add i64 %336, %indvar32
  %scevgep50 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 0, i32 0, i64 %337
  %338 = ashr i64 %sext77, 30
  %339 = add i64 %338, %indvar32
  %scevgep53 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 0, i32 0, i64 %339
  %340 = load i32 addrspace(1)* %scevgep43, align 4
  %341 = icmp eq i32 %340, -1
  br i1 %341, label %428, label %342

; <label>:342                                     ; preds = %335
  %343 = sext i32 %340 to i64
  %344 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %343
  %345 = load i32 addrspace(1)* %344, align 4
  %346 = icmp eq i32 %345, 1
  br i1 %346, label %351, label %347

; <label>:347                                     ; preds = %342
  %348 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %343
  %349 = load i32 addrspace(1)* %348, align 4
  %350 = icmp eq i32 %349, 1
  br i1 %350, label %351, label %364

; <label>:351                                     ; preds = %347, %342
  %352 = load i32 addrspace(1)* %97, align 4
  %353 = load i32 addrspace(1)* %99, align 4
  %354 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %343
  %355 = load i32 addrspace(1)* %354, align 4
  %356 = mul i32 %355, 3
  %357 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %343
  %358 = load i32 addrspace(1)* %357, align 4
  %359 = shl i32 %358, 2
  %360 = sub i32 %m_faces, %352
  %361 = sub i32 %360, %353
  %362 = add nsw i32 %361, %356
  %363 = add nsw i32 %362, %359
  br label %NewFaceIndex.exit3

; <label>:364                                     ; preds = %347
  %365 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %343
  %366 = load i32 addrspace(1)* %365, align 4
  %367 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %343
  %368 = load i32 addrspace(1)* %367, align 4
  %369 = sub i32 %340, %366
  %370 = sub i32 %369, %368
  br label %NewFaceIndex.exit3

NewFaceIndex.exit3:                               ; preds = %364, %351
  %371 = phi i32 [ %370, %364 ], [ %363, %351 ]
  %372 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %343
  %373 = add nsw i32 %371, 1
  %374 = sext i32 %373 to i64
  %375 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %374, i32 0, i64 0
  %376 = sext i32 %371 to i64
  %377 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %376, i32 0, i64 1
  %378 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %374, i32 0, i64 3
  %379 = sext i32 %tmp57 to i64
  %380 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %379, i32 0, i64 0
  %381 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %379, i32 0, i64 1
  %382 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %376, i32 0, i64 2
  %383 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %379, i32 1, i64 1
  %384 = add nsw i32 %371, 2
  %385 = sext i32 %384 to i64
  %386 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %385, i32 0, i64 1
  %387 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %376, i32 0, i64 3
  %tmp22 = zext i32 %371 to i64
  br label %.outer

.outer:                                           ; preds = %.outer.backedge, %NewFaceIndex.exit3
  %indvar.ph = phi i64 [ 0, %NewFaceIndex.exit3 ], [ %tmp28.us, %.outer.backedge ]
  %found.1.ph = phi i1 [ true, %NewFaceIndex.exit3 ], [ false, %.outer.backedge ]
  br i1 %found.1.ph, label %.outer.split.us, label %.critedge.loopexit65

.outer.split.us:                                  ; preds = %.outer
  %tmp19 = add i64 %indvar.ph, 1
  %tmp23 = add i64 %tmp22, %indvar.ph
  br label %388

; <label>:388                                     ; preds = %.backedge.us, %.outer.split.us
  %indvar = phi i64 [ %indvar.next, %.backedge.us ], [ 0, %.outer.split.us ]
  %tmp17 = add i64 %indvar.ph, %indvar
  %v1.0.us = trunc i64 %tmp17 to i32
  %tmp28.us = add i64 %tmp19, %indvar
  %tmp24 = add i64 %tmp23, %indvar
  %tmp27.us = trunc i64 %tmp24 to i32
  %389 = icmp slt i32 %v1.0.us, 4
  br i1 %389, label %393, label %.critedge.loopexit

; <label>:390                                     ; preds = %393
  %391 = load i32 addrspace(1)* %14, align 4
  %392 = icmp eq i32 %395, %391
  br i1 %392, label %401, label %.backedge.us

; <label>:393                                     ; preds = %388
  %tmp29.us = trunc i64 %tmp28.us to i32
  %scevgep.us = getelementptr %struct._Face addrspace(1)* %m_pInFB, i64 %343, i32 0, i64 %tmp17
  %394 = srem i32 %tmp29.us, 4
  %395 = load i32 addrspace(1)* %scevgep.us, align 4
  %396 = load i32 addrspace(1)* %10, align 4
  %397 = icmp eq i32 %395, %396
  br i1 %397, label %401, label %390

; <label>:398                                     ; preds = %401
  %399 = load i32 addrspace(1)* %14, align 4
  %400 = icmp eq i32 %404, %399
  br i1 %400, label %._crit_edge80, label %.backedge.us

; <label>:401                                     ; preds = %393, %390
  %402 = sext i32 %394 to i64
  %403 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %343, i32 0, i64 %402
  %404 = load i32 addrspace(1)* %403, align 4
  %405 = icmp eq i32 %404, %396
  br i1 %405, label %._crit_edge80, label %398

.backedge.us:                                     ; preds = %398, %390
  %indvar.next = add i64 %indvar, 1
  br label %388

._crit_edge80:                                    ; preds = %398, %401
  %406 = load i32 addrspace(1)* %372, align 4
  %407 = icmp eq i32 %406, 0
  br i1 %407, label %415, label %408

; <label>:408                                     ; preds = %._crit_edge80
  %409 = sext i32 %tmp27.us to i64
  %410 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %409, i32 0, i64 1
  store i32 %103, i32 addrspace(1)* %410, align 4
  %411 = add nsw i32 %394, %371
  %412 = sext i32 %411 to i64
  %413 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %412, i32 0, i64 3
  store i32 %103, i32 addrspace(1)* %413, align 4
  store i32 %tmp27.us, i32 addrspace(1)* %scevgep50, align 4
  store i32 %411, i32 addrspace(1)* %scevgep53, align 4
  store i32 %tmp27.us, i32 addrspace(1)* %380, align 4
  store i32 %411, i32 addrspace(1)* %381, align 4
  %414 = load i32 addrspace(1)* %382, align 4
  store i32 %414, i32 addrspace(1)* %383, align 4
  br label %.outer.backedge

; <label>:415                                     ; preds = %._crit_edge80
  %416 = sext i32 %396 to i64
  %417 = getelementptr inbounds i32 addrspace(1)* %dActive, i64 %416
  %418 = load i32 addrspace(1)* %417, align 4
  %419 = load i32 addrspace(1)* %14, align 4
  %420 = icmp ne i32 %418, 0
  %421 = select i1 %420, i32 %419, i32 %396
  %422 = load i32 addrspace(1)* %375, align 4
  %423 = icmp eq i32 %421, %422
  br i1 %423, label %424, label %426

; <label>:424                                     ; preds = %415
  store i32 %103, i32 addrspace(1)* %377, align 4
  store i32 %103, i32 addrspace(1)* %378, align 4
  store i32 %371, i32 addrspace(1)* %scevgep50, align 4
  store i32 %373, i32 addrspace(1)* %scevgep53, align 4
  store i32 %371, i32 addrspace(1)* %380, align 4
  store i32 %373, i32 addrspace(1)* %381, align 4
  %425 = load i32 addrspace(1)* %382, align 4
  store i32 %425, i32 addrspace(1)* %383, align 4
  br label %.outer.backedge

.outer.backedge:                                  ; preds = %424, %426, %408
  br label %.outer

; <label>:426                                     ; preds = %415
  store i32 %103, i32 addrspace(1)* %386, align 4
  store i32 %103, i32 addrspace(1)* %387, align 4
  store i32 %384, i32 addrspace(1)* %scevgep50, align 4
  store i32 %371, i32 addrspace(1)* %scevgep53, align 4
  store i32 %384, i32 addrspace(1)* %380, align 4
  store i32 %371, i32 addrspace(1)* %381, align 4
  %427 = load i32 addrspace(1)* %382, align 4
  store i32 %427, i32 addrspace(1)* %383, align 4
  br label %.outer.backedge

; <label>:428                                     ; preds = %335
  %tmp48 = ashr i64 %sext87, 32
  %429 = sext i32 %tmp57 to i64
  %430 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %429, i32 0, i64 0
  %431 = bitcast i32 addrspace(1)* %430 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %431, i8 -1, i64 16, i32 1, i1 false)
  store i32 -1, i32 addrspace(1)* %334, align 4
  %432 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %tmp48, i32 0, i64 1
  store i32 -1, i32 addrspace(1)* %432, align 4
  br label %.critedge

.critedge.loopexit:                               ; preds = %388
  br label %.critedge

.critedge.loopexit65:                             ; preds = %.outer
  br label %.critedge

.critedge:                                        ; preds = %.critedge.loopexit65, %.critedge.loopexit, %428
  %indvar.next33 = add i64 %indvar32, 1
  %exitcond27 = icmp eq i64 %indvar.next33, 2
  br i1 %exitcond27, label %.loopexit.loopexit, label %335

bb.nph24:                                         ; preds = %46
  %433 = sext i32 %39 to i64
  %434 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 1, i64 0
  store i32 %40, i32 addrspace(1)* %434, align 4
  %435 = load i32 addrspace(1)* %14, align 4
  %436 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 1, i64 1
  store i32 %435, i32 addrspace(1)* %436, align 4
  %437 = sext i32 %m_faces to i64
  %438 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %437
  %439 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %437
  %440 = sext i32 %m_edges to i64
  %441 = getelementptr inbounds i32 addrspace(1)* %m_pEdgeScanned, i64 %440
  %tmp59 = add i64 %1, %4
  %sext64 = shl i64 %tmp59, 32
  %tmp62 = ashr i64 %sext64, 32
  %scevgep75 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 0, i64 0
  %scevgep73 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp62, i32 0, i64 0
  %442 = load i32 addrspace(1)* %scevgep73, align 4
  %443 = icmp eq i32 %442, -1
  br i1 %443, label %507, label %444

; <label>:444                                     ; preds = %bb.nph24
  %445 = load i32 addrspace(1)* %10, align 4
  %446 = sext i32 %445 to i64
  %447 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %446
  %448 = load i32 addrspace(1)* %447, align 4
  %449 = load i32 addrspace(1)* %14, align 4
  %450 = sext i32 %442 to i64
  %451 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %450
  %452 = load i32 addrspace(1)* %451, align 4
  %453 = icmp eq i32 %452, 1
  br i1 %453, label %458, label %454

; <label>:454                                     ; preds = %444
  %455 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %450
  %456 = load i32 addrspace(1)* %455, align 4
  %457 = icmp eq i32 %456, 1
  br i1 %457, label %458, label %471

; <label>:458                                     ; preds = %454, %444
  %459 = load i32 addrspace(1)* %438, align 4
  %460 = load i32 addrspace(1)* %439, align 4
  %461 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %450
  %462 = load i32 addrspace(1)* %461, align 4
  %463 = mul i32 %462, 3
  %464 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %450
  %465 = load i32 addrspace(1)* %464, align 4
  %466 = shl i32 %465, 2
  %467 = sub i32 %m_faces, %459
  %468 = sub i32 %467, %460
  %469 = add nsw i32 %468, %463
  %470 = add nsw i32 %469, %466
  br label %NewFaceIndex.exit

; <label>:471                                     ; preds = %454
  %472 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %450
  %473 = load i32 addrspace(1)* %472, align 4
  %474 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %450
  %475 = load i32 addrspace(1)* %474, align 4
  %476 = sub i32 %442, %473
  %477 = sub i32 %476, %475
  br label %NewFaceIndex.exit

NewFaceIndex.exit:                                ; preds = %471, %458
  %478 = phi i32 [ %477, %471 ], [ %470, %458 ]
  %479 = icmp eq i32 %452, 0
  br i1 %479, label %506, label %480

; <label>:480                                     ; preds = %NewFaceIndex.exit
  %481 = icmp ne i32 %448, 0
  %482 = select i1 %481, i32 %445, i32 %449
  %483 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %450, i32 0, i64 1
  %484 = load i32 addrspace(1)* %483, align 4
  %485 = icmp eq i32 %484, %482
  %486 = add nsw i32 %478, 2
  %.pre82 = add nsw i32 %478, 1
  %storemerge = select i1 %485, i32 %.pre82, i32 %486
  store i32 %storemerge, i32 addrspace(1)* %scevgep75, align 4
  %487 = load i32 addrspace(1)* %441, align 4
  %488 = mul i32 %487, 3
  %489 = load i32 addrspace(1)* %scevgep73, align 4
  %490 = sext i32 %489 to i64
  %491 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %490
  %492 = load i32 addrspace(1)* %491, align 4
  %493 = add nsw i32 %488, %m_edges
  %494 = add nsw i32 %493, %492
  %495 = sext i32 %494 to i64
  %496 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %495, i32 0, i64 0
  store i32 %.pre82, i32 addrspace(1)* %496, align 4
  %497 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %495, i32 0, i64 1
  store i32 %486, i32 addrspace(1)* %497, align 4
  %498 = sext i32 %.pre82 to i64
  %499 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %498, i32 0, i64 1
  %500 = load i32 addrspace(1)* %499, align 4
  %501 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %495, i32 1, i64 0
  store i32 %500, i32 addrspace(1)* %501, align 4
  %502 = sext i32 %478 to i64
  %503 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %502, i32 0, i64 2
  %504 = load i32 addrspace(1)* %503, align 4
  %505 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %495, i32 1, i64 1
  store i32 %504, i32 addrspace(1)* %505, align 4
  br label %508

; <label>:506                                     ; preds = %NewFaceIndex.exit
  store i32 %478, i32 addrspace(1)* %scevgep75, align 4
  br label %508

; <label>:507                                     ; preds = %bb.nph24
  store i32 -1, i32 addrspace(1)* %scevgep75, align 4
  br label %508

; <label>:508                                     ; preds = %507, %506, %480
  %scevgep75.1 = getelementptr %struct._Edge addrspace(1)* %m_pOutEB, i64 %433, i32 0, i64 1
  %scevgep73.1 = getelementptr %struct._Edge addrspace(1)* %m_pInEB, i64 %tmp62, i32 0, i64 1
  %509 = load i32 addrspace(1)* %scevgep73.1, align 4
  %510 = icmp eq i32 %509, -1
  br i1 %510, label %584, label %562

.loopexit.loopexit:                               ; preds = %.critedge
  br label %.loopexit

.loopexit:                                        ; preds = %.loopexit.loopexit, %521, %583, %584
  %511 = sext i32 %39 to i64
  %512 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 0
  %513 = load i32 addrspace(1)* %512, align 4
  %514 = icmp slt i32 %513, 0
  br i1 %514, label %._crit_edge86, label %515

; <label>:515                                     ; preds = %.loopexit
  %516 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 1
  %517 = load i32 addrspace(1)* %516, align 4
  %518 = icmp slt i32 %517, 0
  br i1 %518, label %._crit_edge86, label %520

._crit_edge86:                                    ; preds = %.loopexit, %515
  store i32 0, i32 addrspace(1)* %512, align 4
  %519 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %511, i32 1, i64 1
  store i32 0, i32 addrspace(1)* %519, align 4
  br label %UnifiedReturnBlock

; <label>:520                                     ; preds = %515, %0
  br label %UnifiedReturnBlock

; <label>:521                                     ; preds = %NewFaceIndex.exit.1
  %522 = icmp ne i32 %566, 0
  %523 = select i1 %522, i32 %563, i32 %567
  %524 = getelementptr inbounds %struct._Face addrspace(1)* %m_pInFB, i64 %568, i32 0, i64 1
  %525 = load i32 addrspace(1)* %524, align 4
  %526 = icmp eq i32 %525, %523
  %527 = add nsw i32 %547, 2
  %.pre82.1 = add nsw i32 %547, 1
  %storemerge.1 = select i1 %526, i32 %.pre82.1, i32 %527
  store i32 %storemerge.1, i32 addrspace(1)* %scevgep75.1, align 4
  %528 = load i32 addrspace(1)* %441, align 4
  %529 = mul i32 %528, 3
  %530 = load i32 addrspace(1)* %scevgep73.1, align 4
  %531 = sext i32 %530 to i64
  %532 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %531
  %533 = load i32 addrspace(1)* %532, align 4
  %534 = add nsw i32 %529, %m_edges
  %535 = add nsw i32 %534, %533
  %536 = sext i32 %535 to i64
  %537 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %536, i32 0, i64 0
  store i32 %.pre82.1, i32 addrspace(1)* %537, align 4
  %538 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %536, i32 0, i64 1
  store i32 %527, i32 addrspace(1)* %538, align 4
  %539 = sext i32 %.pre82.1 to i64
  %540 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %539, i32 0, i64 1
  %541 = load i32 addrspace(1)* %540, align 4
  %542 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %536, i32 1, i64 0
  store i32 %541, i32 addrspace(1)* %542, align 4
  %543 = sext i32 %547 to i64
  %544 = getelementptr inbounds %struct._Face addrspace(1)* %m_pOutFB, i64 %543, i32 0, i64 2
  %545 = load i32 addrspace(1)* %544, align 4
  %546 = getelementptr inbounds %struct._Edge addrspace(1)* %m_pOutEB, i64 %536, i32 1, i64 1
  store i32 %545, i32 addrspace(1)* %546, align 4
  br label %.loopexit

NewFaceIndex.exit.1:                              ; preds = %576, %549
  %547 = phi i32 [ %582, %576 ], [ %561, %549 ]
  %548 = icmp eq i32 %570, 0
  br i1 %548, label %583, label %521

; <label>:549                                     ; preds = %572, %562
  %550 = load i32 addrspace(1)* %438, align 4
  %551 = load i32 addrspace(1)* %439, align 4
  %552 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %568
  %553 = load i32 addrspace(1)* %552, align 4
  %554 = mul i32 %553, 3
  %555 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %568
  %556 = load i32 addrspace(1)* %555, align 4
  %557 = shl i32 %556, 2
  %558 = sub i32 %m_faces, %550
  %559 = sub i32 %558, %551
  %560 = add nsw i32 %559, %554
  %561 = add nsw i32 %560, %557
  br label %NewFaceIndex.exit.1

; <label>:562                                     ; preds = %508
  %563 = load i32 addrspace(1)* %10, align 4
  %564 = sext i32 %563 to i64
  %565 = getelementptr inbounds i32 addrspace(1)* %dPotentiallyActive, i64 %564
  %566 = load i32 addrspace(1)* %565, align 4
  %567 = load i32 addrspace(1)* %14, align 4
  %568 = sext i32 %509 to i64
  %569 = getelementptr inbounds i32 addrspace(1)* %dFace3, i64 %568
  %570 = load i32 addrspace(1)* %569, align 4
  %571 = icmp eq i32 %570, 1
  br i1 %571, label %549, label %572

; <label>:572                                     ; preds = %562
  %573 = getelementptr inbounds i32 addrspace(1)* %dFace4, i64 %568
  %574 = load i32 addrspace(1)* %573, align 4
  %575 = icmp eq i32 %574, 1
  br i1 %575, label %549, label %576

; <label>:576                                     ; preds = %572
  %577 = getelementptr inbounds i32 addrspace(1)* %m_pFace3Scanned, i64 %568
  %578 = load i32 addrspace(1)* %577, align 4
  %579 = getelementptr inbounds i32 addrspace(1)* %m_pFace4Scanned, i64 %568
  %580 = load i32 addrspace(1)* %579, align 4
  %581 = sub i32 %509, %578
  %582 = sub i32 %581, %580
  br label %NewFaceIndex.exit.1

; <label>:583                                     ; preds = %NewFaceIndex.exit.1
  store i32 %547, i32 addrspace(1)* %scevgep75.1, align 4
  br label %.loopexit

; <label>:584                                     ; preds = %508
  store i32 -1, i32 addrspace(1)* %scevgep75.1, align 4
  br label %.loopexit

UnifiedReturnBlock:                               ; preds = %520, %._crit_edge86, %42
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind
