; RUN: opt -passes="loop(loop-rotate)" -S %s | FileCheck %s

; CHECK: bb86: ; preds = %bb83, %bb78, %bb74, %bb70

; This code is roughly:

; OpenMP LOOP {
;   try {
;     invoke ... unwind catchsw-inner:
;   }
;   catchsw-inner: unwind bb86: {
;     %tmp79 = catchpad....
;     invoke ... unwind bb86:
;     ...
;   }
; } END LOOP
;
; bb83:
;   %tmp84 = cleanuppad within %tmp79 []
;   cleanupret unwind bb86:
;
; bb86:
;    cleanuppad
;
; The bb86 cleanuppad has 2 preds inside the loop (catchsw-inner and invoke)
; and one outside the loop (cleanupret). It is an exit block.
; The problem here is that all predecessors are part of the same EH region
; (linked by the handler token %tmp79). They cannot be separated, as all
; unwinds in the same EH region must have the same target.
; 
; For loop rotation, exit blocks cannot have any non-loop predecessors.
; Rotation is not possible in this case, as we cannot separate the predecessors
; of bb86 without separating the unwind destinations.
;
; 
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%struct.hoge = type { i32, i32, i32, i32, i32, i32 }

@global.32 = external constant %struct.hoge

declare dllimport void @bar.145() local_unnamed_addr

declare dllimport void @blam.125() local_unnamed_addr

declare dllimport void @blam.146() local_unnamed_addr

declare dllimport void @wobble.169() local_unnamed_addr

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #1

define dso_local void @baz.171() personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb:
  %tmp = alloca i32, align 4
  br label %bb37

bb37:                                             ; preds = %bb
  %tmp38 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"() ]
  br label %bb42

bb42:                                             ; preds = %bb37
  %tmp43 = load volatile i32, i32* undef, align 4
  %tmp44 = icmp sgt i32 %tmp43, undef
  br i1 %tmp44, label %bb109, label %bb66

bb66:                                             ; preds = %bb85, %bb42
  invoke void @wobble.169()
          to label %bb69 unwind label %bb70

bb69:                                             ; preds = %bb66
  unreachable

bb70:                                             ; preds = %bb66
  %tmp71 = catchswitch within none [label %bb72, label %bb74, label %bb78] unwind label %bb86

bb72:                                             ; preds = %bb70
  %tmp73 = catchpad within %tmp71 [%struct.hoge* @global.32, i32 1, i8** undef]
  unreachable

bb74:                                             ; preds = %bb70
  %tmp75 = catchpad within %tmp71 [%struct.hoge* @global.32, i32 8, i8** undef]
  invoke void @bar.145() [ "funclet"(token %tmp75) ]
          to label %bb77 unwind label %bb86

bb77:                                             ; preds = %bb74
  catchret from %tmp75 to label %bb85

bb78:                                             ; preds = %bb70
  %tmp79 = catchpad within %tmp71 [%struct.hoge* @global.32, i32 8, i8** undef]
  invoke void @bar.145() [ "funclet"(token %tmp79) ]
          to label %bb82 unwind label %bb86

bb82:                                             ; preds = %bb78
  unreachable

bb83:                                             ; No predecessors!
  %tmp84 = cleanuppad within %tmp79 []
  cleanupret from %tmp84 unwind label %bb86

bb85:                                             ; preds = %bb77
  br label %bb66

bb86:                                             ; preds = %bb83, %bb78, %bb74, %bb70
  %tmp87 = cleanuppad within none []
  unreachable

bb109:                                            ; preds = %bb42
  br label %bb110

bb110:                                            ; preds = %bb109
  call void @llvm.directive.region.exit(token %tmp38) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  unreachable

bb129:                                            ; No predecessors!
  %tmp130 = cleanuppad within none []
  unreachable
}

attributes #0 = { nounwind }
attributes #1 = { nofree }
