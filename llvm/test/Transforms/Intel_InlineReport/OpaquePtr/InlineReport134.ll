; RUN: opt -opaque-pointers -inline -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S -inline-report=0xf847 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S -inline-report=0xf847 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xf8c6 < %s -S | opt -inline -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S -inline-report=0xf8c6 | opt -inlinereportemitter -inline-report=0xf8c6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8c6 < %s -S | opt -inline -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S -inline-report=0xf8c6 | opt -passes='inlinereportemitter' -inline-report=0xef8c6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; CHECK-CL-NOT: call {{.*}} @IsLocaleTreeInstantiated
; CHECK-CL-NOT: call {{.*}} @AcquireLocaleSplayTree
; CHECK-CL-NOT: call {{.*}} @LoadLocaleCache
; CHECK-CL: DEAD STATIC FUNC: AcquireLocaleSplayTree
; CHECK-CL: DEAD STATIC FUNC: IsLocaleTreeInstantiated
; CHECK-CL: DEAD STATIC FUNC: LoadLocaleCache
; CHECK: COMPILE FUNC: GetLocaleInfo_
; CHECK: INLINE: IsLocaleTreeInstantiated
; CHECK: INLINE: AcquireLocaleSplayTree
; CHECK: INLINE: LoadLocaleCache
; CHECK-MD: DEAD STATIC FUNC: IsLocaleTreeInstantiated
; CHECK-MD: DEAD STATIC FUNC: AcquireLocaleSplayTree
; CHECK-MD: DEAD STATIC FUNC: LoadLocaleCache
; CHECK-MD-NOT: call {{.*}} @IsLocaleTreeInstantiated
; CHECK-MD-NOT: call {{.*}} @AcquireLocaleSplayTree
; CHECK-MD-NOT: call {{.*}} @LoadLocaleCache

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS14_ExceptionInfo._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct._ZTS11_LocaleInfo._LocaleInfo = type { ptr, ptr, ptr, i32, ptr, ptr, i64 }
%struct._ZTS13SemaphoreInfo.SemaphoreInfo = type opaque
%struct._ZTS14_SplayTreeInfo._SplayTreeInfo = type opaque

@.str = private unnamed_addr constant [2 x i8] c"*\00", align 1
@locale_cache = internal unnamed_addr global ptr null, align 8, !intel_dtrans_type !0
@.str.1 = private unnamed_addr constant [11 x i8] c"locale.xml\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"magick/sm1.c\00", align 1
@.str.3 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str.4 = private unnamed_addr constant [23 x i8] c"MemoryAllocationFailed\00", align 1
@.str.5 = private unnamed_addr constant [5 x i8] c"`%s'\00", align 1
@.str.6 = private unnamed_addr constant [9 x i8] c"built-in\00", align 1
@.str.7 = private unnamed_addr constant [143 x i8] c"<?xml version=\221.0\22?><localemap>  <locale name=\22C\22>    <Exception>     <Message name=\22\22>     </Message>    </Exception>  </locale></localemap>\00", align 1

; Function Attrs: nounwind uwtable
define dso_local "intel_dtrans_func_index"="1" ptr @GetLocaleInfo_(ptr noundef "intel_dtrans_func_index"="2" %tag, ptr noundef "intel_dtrans_func_index"="3" %exception) local_unnamed_addr !intel.dtrans.func.type !16 {
entry:
  %call = call fastcc i32 @IsLocaleTreeInstantiated()
  %cmp = icmp eq i32 %call, 0
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq ptr %tag, null
  br i1 %cmp1, label %if.then4, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.end
  %call2 = call i32 @LocaleCompare(ptr noundef nonnull %tag, ptr noundef nonnull @.str)
  %cmp3 = icmp eq i32 %call2, 0
  br i1 %cmp3, label %if.then4, label %cleanup

if.then4:                                         ; preds = %lor.lhs.false, %if.end
  %0 = load ptr, ptr @locale_cache, align 8, !tbaa !18
  %call5 = call ptr @GetNextValueInSplayTree(ptr noundef %0)
  br label %cleanup

cleanup:                                          ; preds = %lor.lhs.false, %entry, %if.then4
  %retval.0 = phi ptr [ %call5, %if.then4 ], [ null, %entry ], [ undef, %lor.lhs.false ]
  ret ptr %retval.0
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: nounwind uwtable
define internal fastcc i32 @IsLocaleTreeInstantiated() unnamed_addr !intel.dtrans.func.type !22 {
entry:
  %0 = load ptr, ptr @locale_cache, align 8, !tbaa !18
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call fastcc ptr @AcquireLocaleSplayTree()
  store ptr %call, ptr @locale_cache, align 8, !tbaa !18
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %1 = load ptr, ptr @locale_cache, align 8, !tbaa !18
  %cmp1.not = icmp ne ptr %1, null
  %cond = zext i1 %cmp1.not to i32
  ret i32 %cond
}

declare !intel.dtrans.func.type !23 dso_local i32 @LocaleCompare(ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2") local_unnamed_addr

declare !intel.dtrans.func.type !24 dso_local "intel_dtrans_func_index"="1" ptr @GetNextValueInSplayTree(ptr noundef "intel_dtrans_func_index"="2") local_unnamed_addr

; Function Attrs: nounwind uwtable
define internal fastcc "intel_dtrans_func_index"="1" ptr @AcquireLocaleSplayTree() unnamed_addr !intel.dtrans.func.type !25 {
entry:
  %exception1 = alloca %struct._ZTS14_ExceptionInfo._ExceptionInfo, align 8
  %call = call ptr @NewSplayTree(ptr noundef nonnull @CompareSplayTreeString, ptr noundef null, ptr noundef nonnull @DestroyLocaleNode)
  %cmp = icmp eq ptr %call, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %exception1)
  call void @GetExceptionInfo(ptr noundef nonnull %exception1)
  %call2 = call ptr @__errno_location()
  %0 = load i32, ptr %call2, align 4, !tbaa !26
  %call3 = call ptr @GetExceptionMessage(i32 noundef %0)
  %call4 = call i32 (ptr, ptr, ptr, i64, i32, ptr, ptr, ...) @ThrowMagickException(ptr noundef nonnull %exception1, ptr noundef nonnull @.str.2, ptr noundef nonnull @.str.3, i64 noundef 82, i32 noundef 700, ptr noundef nonnull @.str.4, ptr noundef nonnull @.str.5, ptr noundef %call3)
  %call5 = call ptr @DestroyString(ptr noundef %call3)
  call void @CatchException(ptr noundef nonnull %exception1)
  %call6 = call ptr @DestroyExceptionInfo(ptr noundef nonnull %exception1)
  call void @MagickCoreTerminus()
  call void @_exit(i32 noundef 1)
  unreachable

if.end:                                           ; preds = %entry
  call fastcc void @LoadLocaleCache()
  ret ptr %call
}

declare !intel.dtrans.func.type !28 dso_local "intel_dtrans_func_index"="1" ptr @NewSplayTree(ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", ptr noundef "intel_dtrans_func_index"="4") local_unnamed_addr

declare !intel.dtrans.func.type !33 dso_local i32 @CompareSplayTreeString(ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2")

declare !intel.dtrans.func.type !34 dso_local "intel_dtrans_func_index"="1" ptr @DestroyLocaleNode(ptr noundef "intel_dtrans_func_index"="2")

declare !intel.dtrans.func.type !35 dso_local void @GetExceptionInfo(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr

declare !intel.dtrans.func.type !36 dso_local "intel_dtrans_func_index"="1" ptr @GetExceptionMessage(i32 noundef) local_unnamed_addr

; Function Attrs: mustprogress nofree nosync nounwind readnone willreturn
declare !intel.dtrans.func.type !37 dso_local "intel_dtrans_func_index"="1" ptr @__errno_location() local_unnamed_addr

declare !intel.dtrans.func.type !39 dso_local i32 @ThrowMagickException(ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2", ptr noundef "intel_dtrans_func_index"="3", i64 noundef, i32 noundef, ptr noundef "intel_dtrans_func_index"="4", ptr noundef "intel_dtrans_func_index"="5", ...) local_unnamed_addr

declare !intel.dtrans.func.type !40 dso_local "intel_dtrans_func_index"="1" ptr @DestroyString(ptr noundef "intel_dtrans_func_index"="2") local_unnamed_addr

declare !intel.dtrans.func.type !41 dso_local void @CatchException(ptr noundef "intel_dtrans_func_index"="1") local_unnamed_addr

declare !intel.dtrans.func.type !42 dso_local "intel_dtrans_func_index"="1" ptr @DestroyExceptionInfo(ptr noundef "intel_dtrans_func_index"="2") local_unnamed_addr

declare dso_local void @MagickCoreTerminus() local_unnamed_addr

; Function Attrs: nofree noreturn
declare dso_local void @_exit(i32 noundef) local_unnamed_addr

; Function Attrs: nounwind uwtable
define internal fastcc void @LoadLocaleCache() unnamed_addr !intel.dtrans.func.type !43 {
entry:
  %call = call ptr @SetFatalErrorHandler(ptr noundef nonnull @LocaleFatalErrorHandler)
  %call1 = call ptr @SetFatalErrorHandler(ptr noundef %call)
  ret void
}

declare !intel.dtrans.func.type !44 dso_local "intel_dtrans_func_index"="1" ptr @SetFatalErrorHandler(ptr noundef "intel_dtrans_func_index"="2") local_unnamed_addr

declare !intel.dtrans.func.type !48 dso_local void @LocaleFatalErrorHandler(i32 noundef, ptr noundef "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2")

!llvm.module.flags = !{!1, !2, !3, !4, !5}
!intel.dtrans.types = !{!6, !11, !13, !14}
!llvm.ident = !{!15}

!0 = !{%struct._ZTS14_SplayTreeInfo._SplayTreeInfo zeroinitializer, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 1, !"ThinLTO", i32 0}
!5 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!6 = !{!"S", %struct._ZTS11_LocaleInfo._LocaleInfo zeroinitializer, i32 7, !7, !7, !7, !8, !9, !9, !10}
!7 = !{i8 0, i32 1}
!8 = !{i32 0, i32 0}
!9 = !{%struct._ZTS11_LocaleInfo._LocaleInfo zeroinitializer, i32 1}
!10 = !{i64 0, i32 0}
!11 = !{!"S", %struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 8, !8, !8, !7, !7, !7, !8, !12, !10}
!12 = !{%struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 1}
!13 = !{!"S", %struct._ZTS14_SplayTreeInfo._SplayTreeInfo zeroinitializer, i32 -1}
!14 = !{!"S", %struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 -1}
!15 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!16 = distinct !{!9, !7, !17}
!17 = !{%struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 1}
!18 = !{!19, !19, i64 0}
!19 = !{!"pointer@_ZTSP14_SplayTreeInfo", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = distinct !{!17}
!23 = distinct !{!7, !7}
!24 = distinct !{!7, !0}
!25 = distinct !{!0, !7, !7, !17}
!26 = !{!27, !27, i64 0}
!27 = !{!"int", !20, i64 0}
!28 = distinct !{!0, !29, !31, !31}
!29 = !{!30, i32 1}
!30 = !{!"F", i1 false, i32 2, !8, !7, !7}
!31 = !{!32, i32 1}
!32 = !{!"F", i1 false, i32 1, !7, !7}
!33 = distinct !{!7, !7}
!34 = distinct !{!7, !7}
!35 = distinct !{!17}
!36 = distinct !{!7}
!37 = distinct !{!38}
!38 = !{i32 0, i32 1}
!39 = distinct !{!17, !7, !7, !7, !7}
!40 = distinct !{!7, !7}
!41 = distinct !{!17}
!42 = distinct !{!17, !17}
!43 = distinct !{!0, !7, !7, !7, !17}
!44 = distinct !{!45, !45}
!45 = !{!46, i32 1}
!46 = !{!"F", i1 false, i32 3, !47, !8, !7, !7}
!47 = !{!"void", i32 0}
!48 = distinct !{!7, !7}
