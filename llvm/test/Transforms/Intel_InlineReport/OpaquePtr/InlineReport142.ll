; RUN: opt -opaque-pointers -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that when bitcasts are removed, a call to test_main becomes a normal
; direct call.

; CHECK-LABEL: COMPILE FUNC: thlib_main
; CHECK: EXTERN: test_main
; CHECK: EXTERN: test_main

; ModuleID = 'slop.ll'
source_filename = "slop.ll"

@the_tcdef_ptr = internal unnamed_addr global ptr null, align 8

declare i32 @test_main(ptr nocapture noundef writeonly, i32, ptr nocapture readnone)

define internal fastcc i32 @thlib_main(i32 noundef %arg, ptr noundef %arg1) unnamed_addr {
bb:
  %i = alloca ptr, align 8
  %i2 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i2)
  %i3 = alloca ptr, align 8
  %i4 = icmp ugt i16 0, 4
  br i1 %i4, label %bb16, label %bb5

bb5:                                              ; preds = %bb
  %i6 = bitcast ptr %i to ptr
  %i7 = call i32 @test_main(ptr noundef nonnull %i6, i32 noundef %arg, ptr noundef %arg1)
  %i8 = call i32 @test_main(ptr noundef nonnull %i, i32 noundef %arg, ptr noundef %arg1)
  %i9 = load ptr, ptr %i, align 8
  %i10 = alloca ptr, align 8
  %i11 = load i16, ptr null, align 8
  %i12 = icmp ugt i16 %i11, 2
  br i1 %i12, label %bb16, label %bb13

bb13:                                             ; preds = %bb5
  %i14 = icmp eq i32 %i8, 0
  br i1 %i14, label %bb15, label %bb16

bb15:                                             ; preds = %bb13
  store ptr %i9, ptr @the_tcdef_ptr, align 8
  br label %bb16

bb16:                                             ; preds = %bb15, %bb13, %bb5, %bb
  %i17 = phi i32 [ 3, %bb ], [ 4, %bb5 ], [ 0, %bb15 ], [ %i8, %bb13 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i2)
  ret i32 %i17
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
