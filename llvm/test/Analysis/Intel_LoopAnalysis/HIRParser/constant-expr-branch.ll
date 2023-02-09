; RUN: opt -opaque-pointers=0 < %s -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Check that we are able to handle constant expressions in the branch condition.

; CHECK DO i1

; CHECK: if (bitcast ([0 x i64]* @global to %struct.barney*) >=u inttoptr (i64 -24 to %struct.barney*))

%struct.quux = type { %struct.wobble }
%struct.wobble = type { i8* }
%struct.barney = type { %struct.hoge }
%struct.hoge = type { i64, i64, i32 }

@global = external global [0 x i64], align 8

define void @widget(i64 %arg, %struct.quux* %arg1, i64 %tmp6) {
bb:
  br label %bb2

bb2:                                              ; preds = %bb17, %bb
  %tmp = phi %struct.quux* [ %tmp18, %bb17 ], [ %arg1, %bb ]
  %tmp3 = phi i64 [ %tmp19, %bb17 ], [ %arg, %bb ]
  %tmp4 = icmp ne %struct.quux* %tmp, null
  %tmp7 = icmp ugt i64 %tmp6, 7
  %tmp8 = ptrtoint %struct.quux* %tmp to i64
  %tmp9 = and i64 %tmp8, 7
  %tmp10 = icmp eq i64 %tmp9, 0
  %tmp11 = and i1 %tmp4, %tmp7
  %tmp12 = and i1 %tmp10, %tmp11
  br i1 %tmp12, label %bb13, label %bb21

bb13:                                             ; preds = %bb2
  br i1 icmp ult (%struct.barney* bitcast ([0 x i64]* @global to %struct.barney*), %struct.barney* inttoptr (i64 -24 to %struct.barney*)), label %bb14, label %bb21

bb14:                                             ; preds = %bb13
  %tmp15 = getelementptr inbounds %struct.quux, %struct.quux* %tmp, i64 0, i32 0, i32 0
  store i8* bitcast (i64* getelementptr inbounds ([0 x i64], [0 x i64]* @global, i64 0, i64 3) to i8*), i8** %tmp15, align 8
  %tmp16 = icmp ult %struct.quux* %tmp, inttoptr (i64 -8 to %struct.quux*)
  br i1 %tmp16, label %bb17, label %bb21

bb17:                                             ; preds = %bb14
  %tmp18 = getelementptr inbounds %struct.quux, %struct.quux* %tmp, i64 1
  %tmp19 = add i64 %tmp3, -1
  %tmp20 = icmp eq i64 %tmp19, 0
  br i1 %tmp20, label %bb21, label %bb2

bb21:                                             ; preds = %bb17, %bb14, %bb13, %bb2
  ret void
}

