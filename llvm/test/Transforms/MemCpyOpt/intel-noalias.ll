; RUN: opt -S -passes=memcpyopt %s | FileCheck %s

; memcpyopt will remove the memcpy and "forward" %1 to %2. This should
; invalidate the noalias on the following call. The noalias must be removed
; because the call will contain a direct reference to %1, aliasing the store.

; CHECK-NOT: call{{.*}}noalias

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

%class.zMatrix44 = type { [16 x float] }

declare fastcc void @"?zCheckMatrix@@YAXPEADVzMatrix44@@1@Z"(ptr, ptr noalias nocapture readonly, ptr)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

define i32 @main() {
  %1 = alloca [0 x [0 x %class.zMatrix44]], i32 0, align 4
  %2 = alloca %class.zMatrix44, align 4
  %.sub = getelementptr [0 x [0 x %class.zMatrix44]], ptr %1, i64 0, i64 0
  store float 0.000000e+00, ptr %1, align 4, !alias.scope !0
  call void @llvm.memcpy.p0.p0.i64(ptr %2, ptr %1, i64 64, i1 false)
  call fastcc void @"?zCheckMatrix@@YAXPEADVzMatrix44@@1@Z"(ptr null, ptr %2, ptr null), !noalias !0
  ret i32 0
}

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!0 = !{!1}
!1 = distinct !{!1, !2, !"?zInverse@@YA?AVzMatrix44@@V1@@Z: argument 0"}
!2 = distinct !{!2, !"?zInverse@@YA?AVzMatrix44@@V1@@Z"}
