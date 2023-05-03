; It checks indirectcallconv that replaces indirect call "%1(100)"
; with direct call to "malloc" by eliminating other possible targets
; like "calloc" and "free" due to signature mismatches.

; RUN: opt < %s -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' -S 2>&1 | FileCheck %s

; CHECK: %call = call ptr @malloc(i64 100)
; CHECK-NOT: %call = call ptr %1(i64 100)

%struct.A = type { ptr, ptr, ptr }

@APtr = internal global ptr null, align 8
@A1 = internal global %struct.A { ptr @malloc, ptr @free, ptr @calloc }, align 8

define ptr @getmem(i32 %i) {
entry:
  %i.addr = alloca i32, align 4
  %p = alloca ptr, align 8
  store i32 %i, ptr %i.addr, align 4
  %0 = load ptr, ptr @APtr, align 8
  %m = getelementptr inbounds %struct.A, ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %m, align 8
  %call = call ptr %1(i64 100)
  store ptr %call, ptr %p, align 8
  %2 = load ptr, ptr %p, align 8
  ret ptr %2
}

define void @ptrassign() {
entry:
  store ptr @A1, ptr @APtr, align 8
  ret void
}

declare noalias ptr @malloc(i64)

declare void @free(ptr)

declare noalias ptr @calloc(i64, i64)
