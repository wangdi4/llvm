; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; This test is based on ind_call_conv_4.ll, with the addition of DTrans type
; metadata. This is to check that the DTrans metadata is maintained on the
; indirect call left in the fallback path of the converted IR.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A.01 = type { ptr, i32 }
%struct.A = type { ptr, i32 }

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8, !intel_dtrans_type !0

define "intel_dtrans_func_index"="1" ptr @add_fun(i32 %val, ...) !intel.dtrans.func.type !10 {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A.01, align 4
  store i32 %val, ptr %val.addr, align 4
  %0 = load i32, ptr %val.addr, align 4
  %add = add nsw i32 %0, 1
  %b = getelementptr inbounds %struct.A.01, ptr %ret, i32 0, i32 1
  store i32 %add, ptr %b, align 4
  ret ptr %ret
}

define "intel_dtrans_func_index"="1" ptr @sub_fun(i32 %val) !intel.dtrans.func.type !12 {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, ptr %val.addr, align 4
  %0 = load i32, ptr %val.addr, align 4
  %sub = add nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, ptr %ret, i32 0, i32 1
  store i32 %sub, ptr %b, align 4
  ret ptr %ret
}

define i32 @func(i32 %in_val) {
entry:
  %0 = load i32, ptr @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr @add_fun, ptr @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.end

if.else.then:                                     ; preds = %if.else
  store ptr @sub_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else.then, %if.then
  %1 = load ptr, ptr @fptr, align 8
  %call = call ptr %1(i32 %in_val), !intel_dtrans_type !1
  %out = alloca ptr, align 8, !intel_dtrans_type !2
  store ptr %call, ptr %out, align 8
  %2 = load ptr, ptr %out, align 8
  %b = getelementptr inbounds %struct.A, ptr %2, i32 0, i32 1
  %3 = load i32, ptr %b, align 4
  ret i32 %3
}

!intel.dtrans.types = !{!4, !7}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{%struct.A zeroinitializer, i32 1}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.A zeroinitializer, i32 2, !5, !3}
!5 = !{!6, i32 1}
!6 = !{!"F", i1 false, i32 0, !2}
!7 = !{!"S", %struct.A.01 zeroinitializer, i32 2, !8, !3}
!8 = !{!9, i32 1}
!9 = !{!"L", i32 0}
!10 = distinct !{!11}
!11 = !{%struct.A.01 zeroinitializer, i32 1}
!12 = distinct !{!2}

; CHECK: define i32 @func

; Check that the compare with @sub_fun was generated
; CHECK: .indconv.cmp.sub_fun:                             ; preds = %if.end
; CHECK:   %.indconv.c = icmp eq ptr %1, @sub_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.sub_fun, label %.indconv.icall.call

; Check that the call to @sub_fun was generated
; CHECK: .indconv.call.sub_fun:                            ; preds = %.indconv.cmp.sub_fun
; CHECK:   %call.indconv = call ptr @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the indirect call is preserved
; CHECK: .indconv.icall.call:                              ; preds = %.indconv.cmp.sub_fun
; CHECK:   %call.indconv1 = call ptr %1(i32 %in_val), !intel_dtrans_type !{{[0-9]+}}
; CHECK:   br label %.indconv.sink.

; CHECK: .indconv.sink.:                                   ; preds = %.indconv.icall.call, %.indconv.call.sub_fun
; CHECK:   %.indconv.ret = phi ptr [ %call.indconv, %.indconv.call.sub_fun ], [ %call.indconv1, %.indconv.icall.call ]


; end INTEL_FEATURE_SW_ADVANCED
