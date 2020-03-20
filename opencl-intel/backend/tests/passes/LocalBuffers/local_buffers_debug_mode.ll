; RUN: %oclopt -add-implicit-args -local-buffers-debug -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; map used local variables to local buffer with correct alignment

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4
@bar.localInt4 = internal addrspace(3) global <4 x i32> zeroinitializer, align 16
@bar.localLong16 = internal addrspace(3) global <16 x i64> zeroinitializer, align 128

define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat) {
entry:
  %dummyInt = load i32, i32 addrspace(3)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %ApInt
  store i32 %dummyInt, i32 addrspace(1)* %BpInt

  %dummyChar = load i8, i8 addrspace(3)* @foo.localChar, align 1
  store i8 %dummyChar, i8 addrspace(1)* %pChar

  %dummyFloat = load float, float addrspace(3)* @foo.localFloat, align 4
  store float %dummyFloat, float addrspace(1)* %pFloat

  ret void
}

define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16) {
entry:
  %dummyInt4 = load <4 x i32>, <4 x i32> addrspace(3)* @bar.localInt4, align 16
  store <4 x i32> %dummyInt4, <4 x i32> addrspace(1)* %pInt4
  
  %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
  
  ret void
}



; CHECK:        define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; CHECK:        entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR0]] to i32 addrspace(3)*
; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; CHECK-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR2]] to i8 addrspace(3)*
; CHECK-NEXT:   [[VAR4:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 256
; CHECK-NEXT:   [[VAR5:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR4]] to float addrspace(3)*

; CHECK:        %dummyInt = load i32, i32 addrspace(3)* [[VAR1]], align 4
; CHECK-NEXT:   call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 4 bitcast (i32 addrspace(3)* @foo.localInt to i8 addrspace(3)*), i8 addrspace(3)* align 4 %0, i64 4, i1 false)

; CHECK:        %dummyChar = load i8, i8 addrspace(3)* [[VAR3]], align 1
; CHECK-NEXT:   call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 1 @foo.localChar, i8 addrspace(3)* align 1 [[VAR2]], i64 1, i1 false)

; CHECK:        %dummyFloat = load float, float addrspace(3)* [[VAR5]], align 4
; CHECK-NEXT:   call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 4 bitcast (float addrspace(3)* @foo.localFloat to i8 addrspace(3)*), i8 addrspace(3)* align 4 %4, i64 4, i1 false)

; CHECK:        ret void


; CHECK:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16,
; CHECK:        entry:
; CHECK-NEXT:   [[VAR10:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT:   [[VAR11:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR10]] to <4 x i32> addrspace(3)*
; CHECK-NEXT:   [[VAR12:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; CHECK-NEXT:   [[VAR13:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR12]] to <16 x i64> addrspace(3)*

; CHECK:        %dummyInt4 = load <4 x i32>, <4 x i32> addrspace(3)* [[VAR11]], align 16
; CHECK-NEXT:   call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 16 bitcast (<4 x i32> addrspace(3)* @bar.localInt4 to i8 addrspace(3)*), i8 addrspace(3)* align 16 %0, i64 16, i1 false)

; CHECK:        %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* [[VAR13]], align 128
; CHECK-NEXT:   call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 128 bitcast (<16 x i64> addrspace(3)* @bar.localLong16 to i8 addrspace(3)*), i8 addrspace(3)* align 128 %2, i64 128, i1 false)

; CHECK:        ret void
