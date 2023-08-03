; CMPLRLLVM-9305: Verify that all vector/aggregate types are treated
; conservatively. Later, they will be supported after more testing if
; needed.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-constraints -disable-output 2>&1 | FileCheck %s

; Check constraints are generated conservatively for vector/aggregate
; operations.
;
; CHECK: Constraints Dump
; CHECK: bar:gep = <universal> (Copy)
; CHECK: bar:aload = <universal> (Copy)
; CHECK: *bar:bc1 = <universal> (Store)
; CHECK: bar:aload2 = <universal> (Copy)
; CHECK: bar:insertv1 = <universal> (Copy)
; CHECK: bar:insertv2 = <universal> (Copy)
; CHECK: bar:shufflev = <universal> (Copy)
; CHECK: bar:inserte1 = <universal> (Copy)
; CHECK: bar:extracte1 = <universal> (Copy)
; CHECK: bar:add1 = <universal> (Copy)
; CHECK: bar:call1 = <universal> (Copy)
; CHECK: *bar:shufflev = <universal> (Store)
; CHECK: *bar:aload2 = <universal> (Store)
; CHECK: bar:extractv1 = <universal> (Copy)
; CHECK: bar:extractv2 = <universal> (Copy)
; CHECK: bar:retval = <universal> (Copy)
; CHECK: Final Constraints Dump

@g = external global i8, align 1

define { ptr, ptr } @bar() {
bb:
  %gep = getelementptr i8, ptr @g, <2 x i64> zeroinitializer
  %alloc1 = call noalias ptr @malloc(i64 16)
  %bc1 = bitcast ptr %alloc1 to ptr
  %aload = load { ptr, ptr }, ptr %bc1, align 8
  store { ptr, ptr } %aload, ptr %bc1, align 8
  %alloc2 = call noalias ptr @malloc(i64 8)
  %bc2 = bitcast ptr %alloc2 to ptr
  %alloc3 = call noalias ptr @malloc(i64 8)
  %bc3 = bitcast ptr %alloc2 to ptr
  %aload2 = load [2 x i32], ptr %bc3, align 4
  %insertv1 = insertvalue { ptr, ptr } %aload, ptr %bc2, 0
  %insertv2 = insertvalue { ptr, ptr } %aload, ptr %bc2, 1
  %shufflev = shufflevector <2 x i32> undef, <2 x i32> undef, <2 x i32> zeroinitializer
  %inserte1 = insertelement <2 x i32> undef, i32 undef, i32 1
  %extracte1 = extractelement <2 x i32> %shufflev, i32 1
  %add1 = add <2 x i32> %shufflev, %inserte1
  %call1 = call { ptr, ptr } @foo(<2 x i32> %shufflev, [2 x i32] %aload2)
  %extractv1 = extractvalue { ptr, ptr } %call1, 0
  %extractv2 = extractvalue { ptr, ptr } %call1, 1
  ret { ptr, ptr } %insertv2
}

declare noalias ptr @malloc(i64)

declare { ptr, ptr } @foo(<2 x i32>, [2 x i32])
