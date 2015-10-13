; RUN: llc < %s -disable-simplify-libcalls | FileCheck %s
; RUN: llc < %s -O0 -disable-simplify-libcalls | FileCheck %s 

; This test checks that llc does not make calls to libirc mem* functions
; when invoked with option -disable-simplify-libcalls as would be the case
; when, for example, compiling for a freestanding target environment.

%struct.anon = type { [1048576 x i32] }

@dst = common global %struct.anon zeroinitializer, align 4
@src = common global %struct.anon zeroinitializer, align 4

; CHECK: FOO
; CHECK-NOT: _intel_fast_memcpy

; Function Attrs: nounwind uwtable
define void @FOO() #0 {
entry:
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast (%struct.anon* @dst to i8*), i8* bitcast (%struct.anon* @src to i8*), i64 4194304, i32 4, i1 false), !tbaa.struct !1
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

; CHECK: MOO
; CHECK-NOT: _intel_fast_memcpy

; Function Attrs: nounwind uwtable
define void @MOO(i8* noalias nocapture readonly %S, i8* noalias nocapture %D, i32 %N) #0 {
entry:
  %conv = zext i32 %N to i64
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %D, i8* %S, i64 %conv, i32 1, i1 false)
  ret void
}


; CHECK: COO
; CHECK-NOT: _intel_fast_memset

; Function Attrs: nounwind uwtable
define void @COO(i8* noalias nocapture %D, i8 signext %c, i32 %N) #0 {
entry:
  %conv1 = zext i32 %N to i64
  tail call void @llvm.memset.p0i8.i64(i8* %D, i8 %c, i64 %conv1, i32 1, i1 false)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1256)"}
!1 = !{i64 0, i64 4194304, !2}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
