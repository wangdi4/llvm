; RUN: opt < %s -passes='recursive-function-memoize' -function-memoization-cache-size=37 -S | FileCheck %s

;
; The test checks recursive function memoization in the case when
; argument type and return type differ. Such case caused a regression
; in compiler post testing and is required to be caught at earlier stages of
; development cycle.
;

define dso_local i64 @sub(i32 %n)  {
entry:
  %call = call i64 @sub(i32 noundef poison)
  %call5 = call i64 @sub(i32 noundef poison)
  unreachable
}
; CHECK-LABEL: define dso_local i64 @sub
; CHECK-SAME: (i32 [[N:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = alloca [[STRUCT_CACHE:%.*]], i32 37, align 8
; CHECK-NEXT:    call void @sub.cache_init(ptr [[TMP0]])
; CHECK-NEXT:    [[RES:%.*]] = call i64 @sub.cached(i32 [[N]], ptr [[TMP0]])
; CHECK-NEXT:    ret i64 [[RES]]
;
;
; CHECK-LABEL: define private i32 @sub.get_cache_id
; CHECK-SAME: (i32 [[KEY:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[IDX:%.*]] = urem i32 [[KEY]], 37
; CHECK-NEXT:    ret i32 [[IDX]]
;
;
; CHECK-LABEL: define private ptr @sub.get_cache_entry_ptr
; CHECK-SAME: (i32 [[KEY:%.*]], ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[IDX:%.*]] = call i32 @sub.get_cache_id(i32 [[KEY]])
; CHECK-NEXT:    [[IDX_64:%.*]] = zext i32 [[IDX]] to i64
; CHECK-NEXT:    [[CACHE_ENTRY:%.*]] = getelementptr [[STRUCT_CACHE:%.*]], ptr [[CACHE]], i64 [[IDX_64]]
; CHECK-NEXT:    ret ptr [[CACHE_ENTRY]]
;
;
; CHECK-LABEL: define private void @sub.cache_update
; CHECK-SAME: (i32 [[KEY:%.*]], i64 [[VALUE:%.*]], ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ENTRY_PTR:%.*]] = call ptr @sub.get_cache_entry_ptr(i32 [[KEY]], ptr [[CACHE]])
; CHECK-NEXT:    [[KEY_PTR:%.*]] = getelementptr [[STRUCT_CACHE:%.*]], ptr [[ENTRY_PTR]], i32 0, i32 0
; CHECK-NEXT:    store i32 [[KEY]], ptr [[KEY_PTR]], align 4
; CHECK-NEXT:    [[VALUE_PTR:%.*]] = getelementptr [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 1
; CHECK-NEXT:    store i64 [[VALUE]], ptr [[VALUE_PTR]], align 4
; CHECK-NEXT:    [[ENGAGED_PTR:%.*]] = getelementptr [[STRUCT_CACHE]], ptr [[ENTRY_PTR]], i32 0, i32 2
; CHECK-NEXT:    store i1 true, ptr [[ENGAGED_PTR]], align 1
; CHECK-NEXT:    ret void
;
;
; CHECK-LABEL: define private void @sub.cache_init
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
; CHECK-LABEL: define private i64 @sub.cached
; CHECK-SAME: (i32 [[N:%.*]], ptr [[CACHE:%.*]]) {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ENTRY_PTR:%.*]] = call ptr @sub.get_cache_entry_ptr(i32 [[N]], ptr [[CACHE]])
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
; CHECK-NEXT:    [[CACHED_VAL:%.*]] = load i64, ptr [[VAL_PTR]], align 4
; CHECK-NEXT:    ret i64 [[CACHED_VAL]]
; CHECK:       calc.val:
; CHECK-NEXT:    br label [[ENTRY_SUB:%.*]]
; CHECK:       entry.sub:
; CHECK-NEXT:    [[CALL_CACHED:%.*]] = call i64 @sub.cached(i32 poison, ptr [[CACHE]])
; CHECK-NEXT:    [[CALL5_CACHED:%.*]] = call i64 @sub.cached(i32 poison, ptr [[CACHE]])
; CHECK-NEXT:    unreachable
;
