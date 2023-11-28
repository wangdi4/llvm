; RUN: opt -passes='recursive-function-memoize,print<inline-report>' -function-memoization-cache-size=37 -disable-output -inline-report=0xf847 < %s 2>&1 | FileCheck --check-prefix=CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,recursive-function-memoize,inlinereportemitter' -function-memoization-cache-size=37 -disable-output -inline-report=0xf8c6 < %s 2>&1 | FileCheck --check-prefix=CHECK-MD %s

; Test the inline report for recursive function memoization. This test is
; derived from intel_recursive_memoize.ll.

; CHECK-CL: COMPILE FUNC: _Z3fibi.get_cache_id
; CHECK-CL: COMPILE FUNC: _Z3fibi.get_cache_entry_ptr
; CHECK-CL: _Z3fibi.get_cache_id
; CHECK-CL: COMPILE FUNC: _Z3fibi.cache_update
; CHECK-CL: _Z3fibi.get_cache_entry_ptr
; CHECK-CL: COMPILE FUNC: _Z3fibi.cache_init
; CHECK-CL: COMPILE FUNC: _Z3fibi.cached
; CHECK-CL: _Z3fibi.get_cache_entry_ptr
; CHECK-CL: _Z3fibi.cached
; CHECK-CL: _Z3fibi.cached
; CHECK-CL: _Z3fibi.cache_update
; CHECK-CL: COMPILE FUNC: _Z3fibi
; CHECK-CL: DELETE: _Z3fibi
; CHECK_CL: DELETE: _Z3fibi
; CBECK-CL: _Z3fibi.cache_init
; CHECK-CL: _Z3fibi.cached

; CHECK-MD: COMPILE FUNC: _Z3fibi
; CHECK-MD: DELETE: _Z3fibi
; CHECK-MD: DELETE: _Z3fibi
; CHECK-MD: _Z3fibi.cache_init
; CHECK-MD: _Z3fibi.cached
; CHECK-MD: COMPILE FUNC: _Z3fibi.get_cache_id
; CHECK-MD: COMPILE FUNC: _Z3fibi.get_cache_entry_ptr
; CHECK-MD: _Z3fibi.get_cache_id
; CHECK-MD: COMPILE FUNC: _Z3fibi.cache_update
; CHECK-MD: _Z3fibi.get_cache_entry_ptr
; CHECK-MD: COMPILE FUNC: _Z3fibi.cache_init
; CHECK-MD: COMPILE FUNC: _Z3fibi.cached
; CHECK-MD: _Z3fibi.get_cache_entry_ptr
; CHECK-MD: COMPILE FUNC: _Z3fibi.cached
; CHECK-MD: _Z3fibi.cached
; CHECK-MD: _Z3fibi.cached
; CHECK-MD: _Z3fibi.cache_update

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @_Z3fibi(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp = icmp slt i32 %n, 2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %sub = sub nuw nsw i32 %n, 1
  %call = call noundef i32 @_Z3fibi(i32 noundef %sub)
  %sub1 = sub nuw nsw i32 %n, 2
  %call2 = call noundef i32 @_Z3fibi(i32 noundef %sub1)
  %add = add nsw i32 %call, %call2
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %n, %if.then ], [ %add, %if.end ]
  ret i32 %retval.0
}

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
