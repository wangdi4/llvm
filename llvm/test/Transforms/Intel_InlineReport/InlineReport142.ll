; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that when bitcasts are removed, a call to test_main becomes a normal
; direct call.

; CHECK-LABEL: COMPILE FUNC: thlib_main
; CHECK: EXTERN: test_main
; CHECK: EXTERN: test_main

%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i64, i32 (i64, i32, i8**)*, {}*, i32 (i32, i8**)*, void ()* }
%struct.TCDef.7 = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i64, i32 (i64, i32, i8**)*, {}*, i32 (i32, i8**)*, void ()* }
%struct.version_number = type { i8, i8, i8, i8 }

@the_tcdef_ptr = internal unnamed_addr global %struct.TCDef.7* null, align 8

declare i32 @test_main(%struct.TCDef** nocapture noundef writeonly %0, i32 %1, i8** nocapture readnone %2)

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1)

declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1)

define internal fastcc i32 @thlib_main(i32 noundef %0, i8** noundef %1) unnamed_addr #26 {
  %3 = alloca %struct.TCDef.7*, align 8
  %4 = bitcast %struct.TCDef.7** %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #30
  %5 = alloca %struct.TCDef.7*, align 8
  %6 = icmp ugt i16 0, 4
  br i1 %6, label %18, label %7

7:                                                ; preds = %2
  %8 = bitcast %struct.TCDef.7** %3 to %struct.TCDef**
  %9 = call i32 @test_main(%struct.TCDef** noundef nonnull %8, i32 noundef %0, i8** noundef %1) #30
  %10 = call i32 bitcast (i32 (%struct.TCDef**, i32, i8**)* @test_main to i32 (%struct.TCDef.7**, i32, i8**)*)(%struct.TCDef.7** noundef nonnull %3, i32 noundef %0, i8** noundef %1) #30
  %11 = load %struct.TCDef.7*, %struct.TCDef.7** %3, align 8
  %12 = alloca %struct.TCDef.7*, align 8
  %13 = load i16, i16* null, align 8
  %14 = icmp ugt i16 %13, 2
  br i1 %14, label %18, label %15

15:                                               ; preds = %7
  %16 = icmp eq i32 %10, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %15
  store %struct.TCDef.7* %11, %struct.TCDef.7** @the_tcdef_ptr, align 8
  br label %18

18:                                               ; preds = %17, %15, %7, %2
  %19 = phi i32 [ 3, %2 ], [ 4, %7 ], [ 0, %17 ], [ %10, %15 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #30
  ret i32 %19
}

