; RUN: opt < %s -passes='recursive-function-memoize' -function-memoization-cache-size=37 -S | FileCheck %s

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

; CHECK [[STRUCT_CACHE:%.*]] = type { i32, i32, i1 }
; CHECK-LABEL: define dso_local noundef i32 @_Z3fibi
; CHECK-SAME: (i32 noundef [[N:%.*]]) local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = alloca [[STRUCT_CACHE:%.*]], i32 37, align 8
; CHECK-NEXT:    call void @_Z3fibi.cache_init(ptr [[TMP0]])
; CHECK-NEXT:    [[RES:%.*]] = call i32 @_Z3fibi.cached(i32 [[N]], ptr [[TMP0]])
; CHECK-NEXT:    ret i32 [[RES]]
;
;
; CHECK-LABEL: define private i32 @_Z3fibi.get_cache_id
; CHECK-SAME: (i32 [[KEY:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[IDX:%.*]] = urem i32 [[KEY]], 37
; CHECK-NEXT:    ret i32 [[IDX]]
;
;
; CHECK-LABEL: define private ptr @_Z3fibi.get_cache_entry_ptr
; CHECK-SAME: (i32 [[KEY:%.*]], ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[IDX:%.*]] = call i32 @_Z3fibi.get_cache_id(i32 [[KEY]])
; CHECK-NEXT:    [[IDX_64:%.*]] = zext i32 [[IDX]] to i64
; CHECK-NEXT:    [[CACHE_ENTRY:%.*]] = getelementptr [[STRUCT_CACHE:%.*]], ptr [[CACHE]], i64 [[IDX_64]]
; CHECK-NEXT:    ret ptr [[CACHE_ENTRY]]
;
;
; CHECK-LABEL: define private void @_Z3fibi.cache_update
; CHECK-SAME: (i32 [[KEY:%.*]], i32 [[VALUE:%.*]], ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ENTRY_PTR:%.*]] = call ptr @_Z3fibi.get_cache_entry_ptr(i32 [[KEY]], ptr [[CACHE]])
; CHECK-NEXT:    [[KEY_PTR:%.*]] = getelementptr [[STRUCT_CACHE:%.*]], ptr [[ENTRY_PTR]], i32 0, i32 0
; CHECK-NEXT:    store i32 [[KEY]], ptr [[KEY_PTR]], align 4
; CHECK-NEXT:    [[VALUE_PTR:%.*]] = getelementptr [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 1
; CHECK-NEXT:    store i32 [[VALUE]], ptr [[VALUE_PTR]], align 4
; CHECK-NEXT:    [[ENGAGED_PTR:%.*]] = getelementptr [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 2
; CHECK-NEXT:    store i1 true, ptr [[ENGAGED_PTR]], align 1
; CHECK-NEXT:    ret void
;
;
; CHECK-LABEL: define private void @_Z3fibi.cache_init
; CHECK-SAME: (ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[LOOP_COND:%.*]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
; CHECK:       loop.cond:
; CHECK-NEXT:    [[LOOP_I:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[LOOP_I_NEXT:%.*]], [[LOOP_BODY:%.*]] ]
; CHECK-NEXT:    [[CMP:%.*]] = icmp ult i64 [[LOOP_I]], 37
; CHECK-NEXT:    br i1 [[CMP]], label [[LOOP_BODY]], label [[EXIT:%.*]]
; CHECK:       loop.body:
; CHECK-NEXT:    [[ENGAGED_PTR:%.*]] = getelementptr [[STRUCT_CACHE:%.*]], ptr [[CACHE]], i64 [[LOOP_I]], i32 2
; CHECK-NEXT:    store i1 false, ptr [[ENGAGED_PTR]], align 1
; CHECK-NEXT:    [[LOOP_I_NEXT]] = add i64 [[LOOP_I]], 1
; CHECK-NEXT:    br label [[LOOP_COND]]
;
;
; CHECK-LABEL: define private noundef i32 @_Z3fibi.cached
; CHECK-SAME: (i32 noundef [[N:%.*]], ptr [[CACHE:%.*]]) local_unnamed_addr #[[ATTR0]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ENTRY_PTR:%.*]] = call ptr @_Z3fibi.get_cache_entry_ptr(i32 [[N]], ptr [[CACHE]])
; CHECK-NEXT:    [[ENGAGED_PTR:%.*]] = getelementptr inbounds [[STRUCT_CACHE:%.*]], ptr [[ENTRY_PTR]], i32 0, i32 2
; CHECK-NEXT:    [[ENGAGED:%.*]] = load i1, ptr [[ENGAGED_PTR]], align 1
; CHECK-NEXT:    [[IS_ENGAGED:%.*]] = icmp eq i1 [[ENGAGED]], true
; CHECK-NEXT:    br i1 [[IS_ENGAGED]], label [[CHECK_2:%.*]], label [[CALC_VAL:%.*]]
; CHECK:       check.2:
; CHECK-NEXT:    [[KEY_PTR:%.*]] = getelementptr inbounds [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 0
; CHECK-NEXT:    [[KEY_VAL:%.*]] = load i32, ptr [[KEY_PTR]], align 4
; CHECK-NEXT:    [[CACHE_ENTRY_FOUND:%.*]] = icmp eq i32 [[KEY_VAL]], [[N]]
; CHECK-NEXT:    br i1 [[CACHE_ENTRY_FOUND]], label [[GET_CACHE_VAL:%.*]], label [[CALC_VAL]]
; CHECK:       get.cache.val:
; CHECK-NEXT:    [[VAL_PTR:%.*]] = getelementptr inbounds [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 1
; CHECK-NEXT:    [[CACHED_VAL:%.*]] = load i32, ptr [[VAL_PTR]], align 4
; CHECK-NEXT:    ret i32 [[CACHED_VAL]]
; CHECK:       calc.val:
; CHECK-NEXT:    br label [[ENTRY__Z3FIBI:%.*]]
; CHECK:       entry._Z3fibi:
; CHECK-NEXT:    [[CMP:%.*]] = icmp slt i32 [[N]], 2
; CHECK-NEXT:    br i1 [[CMP]], label [[IF_THEN:%.*]], label [[IF_END:%.*]]
; CHECK:       if.then:
; CHECK-NEXT:    br label [[RETURN:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    [[SUB:%.*]] = sub nuw nsw i32 [[N]], 1
; CHECK-NEXT:    [[CALL_CACHED:%.*]] = call i32 @_Z3fibi.cached(i32 [[SUB]], ptr [[CACHE]])
; CHECK-NEXT:    [[SUB1:%.*]] = sub nuw nsw i32 [[N]], 2
; CHECK-NEXT:    [[CALL2_CACHED:%.*]] = call i32 @_Z3fibi.cached(i32 [[SUB1]], ptr [[CACHE]])
; CHECK-NEXT:    [[ADD:%.*]] = add nsw i32 [[CALL_CACHED]], [[CALL2_CACHED]]
; CHECK-NEXT:    br label [[RETURN]]
; CHECK:       return:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ [[N]], [[IF_THEN]] ], [ [[ADD]], [[IF_END]] ]
; CHECK-NEXT:    call void @_Z3fibi.cache_update(i32 [[N]], i32 [[RETVAL_0]], ptr [[CACHE]])
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
