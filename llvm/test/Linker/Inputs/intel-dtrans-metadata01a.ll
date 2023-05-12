; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
%struct._sublex_info = type { i8, i16, ptr, ptr }
%struct.sv = type { ptr, i32, i32, %union.anon }
%struct.op = type { ptr, ptr, ptr, i64, i16, i8, i8 }
%union.anon = type { ptr }

define "intel_dtrans_func_index"="1" ptr @test(ptr "intel_dtrans_func_index"="2" returned %in) !intel.dtrans.func.type !12 {
    %op.addr = getelementptr %struct._sublex_info, ptr %in, i64 0, i32 2
    %op = load ptr, ptr %op.addr
    %r = call ptr @test02(ptr %op)
    ret ptr %in
}

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @test02(ptr "intel_dtrans_func_index"="2")


!1 = !{i8 0, i32 0}  ; i8
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.op zeroinitializer, i32 1}  ; %struct.op*
!4 = !{%struct.sv zeroinitializer, i32 1}  ; %struct.sv*
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{i32 0, i32 0}  ; i32
!7 = !{%union.anon zeroinitializer, i32 0}  ; %union.anon
!8 = !{!9, i32 1}  ; {}*
!9 = !{!"L", i32 0}  ; {}
!10 = !{i64 0, i32 0}  ; i64
!11 = !{%struct._sublex_info zeroinitializer, i32 1}  ; %struct._sublex_info*
!12 = distinct !{!11, !11}
!13 = distinct !{!3, !3}
!14 = !{!"S", %struct._sublex_info zeroinitializer, i32 4, !1, !2, !3, !4} ; { i8, i16, %struct.op*, %struct.sv* }
!15 = !{!"S", %struct.sv zeroinitializer, i32 4, !5, !6, !6, !7} ; { i8*, i32, i32, %union.anon }
!16 = !{!"S", %struct.op zeroinitializer, i32 7, !3, !3, !8, !10, !2, !1, !1} ; { %struct.op*, %struct.op*, {}*, i64, i16, i8, i8 }
!17 = !{!"S", %union.anon zeroinitializer, i32 1, !5} ; { i8* }

!intel.dtrans.types = !{!14, !15, !16, !17}

; end INTEL_FEATURE_SW_DTRANS
