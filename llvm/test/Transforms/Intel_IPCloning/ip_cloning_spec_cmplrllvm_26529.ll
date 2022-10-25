; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='module(ip-cloning)' -ip-specialization-cloning -debug-only=ipcloning -S 2>&1 | FileCheck %s

; CMPLRLLVM-26529: Ensure that 3 unique clones are created, due to three
; unique values for the "special" address of stack variable "constants".

; CHECK: define{{.*}}convolutionalEncode.1
; CHECK: define{{.*}}convolutionalEncode.2
; CHECK: define{{.*}}convolutionalEncode.3

@__const.t_run_test.CM_ONE = private unnamed_addr constant [3 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@__const.t_run_test.CM_THREE = private unnamed_addr constant [5 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\00\01", [2 x i8] c"\01\00", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@t_buf = internal unnamed_addr global i8* null, align 8
@input_buf = internal global [512 x i8] zeroinitializer, align 1

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #1

declare dso_local noalias noundef i8* @malloc(i64 %0) local_unnamed_addr #13

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3)

declare dso_local i64 @strtol(i8* readonly %0, i8** nocapture %1, i32 %2) local_unnamed_addr

define dso_local i32 @main(i32 %0, i8** %1) #0 {
  %t1 = call i32 @t_run_test(i64 0, i32 1, i8** null)
  ret i32 %t1
}

; Function Attrs: nounwind uwtable
define internal i32 @t_run_test(i64 %0, i32 %t1, i8** nocapture readonly %t2) #0 {
t3:
  %t5 = alloca [3 x [2 x i8]], align 1
  %t6 = alloca i64, align 8
  %t7 = alloca [5 x [2 x i8]], align 1
  %t9 = getelementptr inbounds [3 x [2 x i8]], [3 x [2 x i8]]* %t5, i64 0, i64 0, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 1 dereferenceable(6) %t9, i8* nonnull align 1 dereferenceable(6) getelementptr inbounds ([3 x [2 x i8]], [3 x [2 x i8]]* @__const.t_run_test.CM_ONE, i64 0, i64 0, i64 0), i64 6, i1 false)
  %t10 = bitcast i64* %t6 to i8*
  store i64 72340172821299457, i64* %t6, align 8
  %t11 = getelementptr inbounds [5 x [2 x i8]], [5 x [2 x i8]]* %t7, i64 0, i64 0, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 1 dereferenceable(10) %t11, i8* nonnull align 1 dereferenceable(10) getelementptr inbounds ([5 x [2 x i8]], [5 x [2 x i8]]* @__const.t_run_test.CM_THREE, i64 0, i64 0, i64 0), i64 10, i1 false)
  %t12 = call noalias dereferenceable_or_null(1056) i8* @malloc(i64 1056)
  store i8* %t12, i8** @t_buf, align 8
  %t13 = icmp eq i8* %t12, null
  br i1 %t13, label %t14, label %t16

t14:                                               ; preds = %t3
  %t15 = load i8*, i8** @t_buf, align 8
  br label %t16

t16:                                               ; preds = %t14, %t3
  %t17 = phi i8* [ %t15, %t14 ], [ %t12, %t3 ]
  %t18 = getelementptr inbounds i8, i8* %t17, i64 512
  %t19 = bitcast i8* %t18 to [2 x i8]*
  %t20 = icmp slt i32 %t1, 2
  br i1 %t20, label %t21, label %t23

t21:                                               ; preds = %t16
  %t22 = add i1 %t20, %t20
  br label %t39

t23:                                               ; preds = %t16
  %t24 = icmp eq i32 %t1, 2
  br i1 %t24, label %t39, label %t25

t25:                                               ; preds = %t23
  %t26 = getelementptr inbounds i8*, i8** %t2, i64 2
  %t27 = load i8*, i8** %t26, align 8
  %t28 = tail call i64 @strtol(i8* nocapture nonnull %t27, i8** null, i32 10) #26
  %t29 = trunc i64 %t28 to i16
  %t30 = icmp eq i16 %t29, 0
  br i1 %t30, label %t39, label %t31

t31:                                               ; preds = %t25
  %t32 = trunc i64 %t28 to i32
  %t33 = shl i32 %t32, 16
  %t34 = ashr exact i32 %t33, 16
  switch i32 %t34, label %t43 [
    i32 1, label %t35
    i32 2, label %t39
    i32 3, label %t41
  ]

t35:                                               ; preds = %t31
  %t36 = getelementptr inbounds [3 x [2 x i8]], [3 x [2 x i8]]* %t5, i64 0, i64 0
  br label %t43

t39:                                               ; preds = %t25, %t23, %t21, %t31
  %t40 = bitcast i64* %t6 to [2 x i8]*
  br label %t43

t41:                                               ; preds = %t31
  %t42 = getelementptr inbounds [5 x [2 x i8]], [5 x [2 x i8]]* %t7, i64 0, i64 0
  br label %t43

t43:                                               ; preds = %t41, %t39, %t35, %t31
  %t44 = phi [2 x i8]* [ %t19, %t31 ], [ %t42, %t41 ], [ %t40, %t39 ], [ %t36, %t35 ]
  %t45 = phi i1 [ true, %t31 ], [ false, %t41 ], [ false, %t39 ], [ false, %t35 ]
  %t46 = phi i16 [ 0, %t31 ], [ 2, %t41 ], [ 2, %t39 ], [ 2, %t35 ]
  %t47 = phi i16 [ 0, %t31 ], [ 5, %t41 ], [ 4, %t39 ], [ 3, %t35 ]
  call void @convolutionalEncode(i8* getelementptr inbounds ([512 x i8], [512 x i8]* @input_buf, i64 0, i64 0), i16 signext 512, i16 signext %t46, i16 signext %t47, [2 x i8]* %t44, i8* %t17) #26
  ret i32 0
}

define internal void @convolutionalEncode(i8* nocapture readonly %0, i16 signext %1, i16 signext %2, i16 signext %3, [2 x i8]* nocapture readonly %4, i8* nocapture %5) #5 {
  %arrayidx44 = getelementptr inbounds [2 x i8], [2 x i8]* %4, i64 0, i64 0
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
