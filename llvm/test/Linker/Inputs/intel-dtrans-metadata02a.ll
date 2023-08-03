; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; Input file for test intel-drans-metadata02.ll

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%struct.network = type { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, double, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i64 }
%struct.node = type { i64, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i32, i32 }
%struct.arc = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

@stderr = external dso_local local_unnamed_addr global ptr, align 8, !intel_dtrans_type !0
@.str.4 = private unnamed_addr constant [23 x i8] c"12345678901234567890: \00", align 1

define dso_local i64 @test(ptr nocapture readonly "intel_dtrans_func_index"="1" %net) local_unnamed_addr !intel.dtrans.func.type !22 {
entry:
  %t10 = load ptr, ptr @stderr, align 8
  %t11 = tail call i64 @fwrite(ptr getelementptr inbounds ([23 x i8], ptr @.str.4, i64 0, i64 0), i64 22, i64 1, ptr %t10)
  ret i64 0
}

declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr

!intel.dtrans.types = !{!1, !8, !10, !12, !19, !20, !21}

!0 = !{%struct._IO_FILE zeroinitializer, i32 1}
!1 = !{!"S", %struct.network zeroinitializer, i32 33, !2, !2, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !5, !4, !6, !6, !7, !7, !7, !7, !7, !4, !4, !4, !4, !4}
!2 = !{!"A", i32 200, !3}
!3 = !{i8 0, i32 0}
!4 = !{i64 0, i32 0}
!5 = !{double 0.000000e+00, i32 0}
!6 = !{%struct.node zeroinitializer, i32 1}
!7 = !{%struct.arc zeroinitializer, i32 1}
!8 = !{!"S", %struct.node zeroinitializer, i32 14, !4, !9, !6, !6, !6, !6, !7, !7, !7, !7, !4, !4, !9, !9}
!9 = !{i32 0, i32 0}
!10 = !{!"S", %struct.arc zeroinitializer, i32 9, !9, !4, !6, !6, !11, !7, !7, !4, !4}
!11 = !{i16 0, i32 0}
!12 = !{!"S", %struct._IO_FILE zeroinitializer, i32 29, !9, !13, !13, !13, !13, !13, !13, !13, !13, !13, !13, !13, !14, !0, !9, !9, !4, !11, !3, !15, !13, !4, !16, !17, !0, !13, !4, !9, !18}
!13 = !{i8 0, i32 1}
!14 = !{%struct._IO_marker zeroinitializer, i32 1}
!15 = !{!"A", i32 1, !3}
!16 = !{%struct._IO_codecvt zeroinitializer, i32 1}
!17 = !{%struct._IO_wide_data zeroinitializer, i32 1}
!18 = !{!"A", i32 20, !3}
!19 = !{!"S", %struct._IO_marker zeroinitializer, i32 -1}
!20 = !{!"S", %struct._IO_codecvt zeroinitializer, i32 -1}
!21 = !{!"S", %struct._IO_wide_data zeroinitializer, i32 -1}
!22 = distinct !{!23}
!23 = !{%struct.network zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS
