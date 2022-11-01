; It checks indirectcallconv that replaces indirect call "%1(100)"
; with direct call to "malloc" by eliminating other possible targets
; like "calloc" and "free" due to signature mismatches.

; RUN: opt < %s -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' -S 2>&1 | FileCheck %s

; CHECK: %call = call i8* @malloc(i64 100)
; CHECK-NOT: %call = call i8* %1(i64 100)

%struct.A = type { i8* (i64)*, void (i8*)*, i8* (i64, i64)* }

@APtr = internal global %struct.A* null, align 8
@A1 = internal global %struct.A { i8* (i64)* @malloc, void (i8*)* @free, i8* (i64, i64)* @calloc }, align 8

; Function Attrs: noinline nounwind uwtable
define i8* @getmem(i32 %i) {
entry:
  %i.addr = alloca i32, align 4
  %p = alloca i8*, align 8
  store i32 %i, i32* %i.addr, align 4
  %0 = load %struct.A*, %struct.A** @APtr, align 8
  %m = getelementptr inbounds %struct.A, %struct.A* %0, i32 0, i32 0
  %1 = load i8* (i64)*, i8* (i64)** %m, align 8
  %call = call i8* %1(i64 100)
  store i8* %call, i8** %p, align 8
  %2 = load i8*, i8** %p, align 8
  ret i8* %2
}

; Function Attrs: noinline nounwind uwtable
define void @ptrassign() {
entry:
  store %struct.A* @A1, %struct.A** @APtr, align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64)

; Function Attrs: nounwind
declare void @free(i8*)

; Function Attrs: nounwind
declare noalias i8* @calloc(i64, i64)
