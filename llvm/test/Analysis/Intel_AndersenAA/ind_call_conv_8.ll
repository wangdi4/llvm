; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; This test is based on ind_call_conv_4.ll, with the addition of DTrans type
; metadata. This is to check that the DTrans metadata is maintained on the
; indirect call left in the fallback path of the converted IR.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { %struct.A* ()*, i32 }
%struct.A.01 = type { {}*, i32 }

@glob = external global i32, align 4
@fptr = internal global %struct.A* (i32)* null, align 8, !intel_dtrans_type !8

define "intel_dtrans_func_index"="1" %struct.A.01* @add_fun(i32 %val)  !intel.dtrans.func.type !10 {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A.01, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %add = add nsw i32 %0, 1
  %b = getelementptr inbounds %struct.A.01, %struct.A.01* %ret, i32 0, i32 1
  store i32 %add, i32* %b, align 4
  ret %struct.A.01* %ret
}

define "intel_dtrans_func_index"="1" %struct.A* @sub_fun(i32 %val)  !intel.dtrans.func.type !11 {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %sub = add nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, %struct.A* %ret, i32 0, i32 1
  store i32 %sub, i32* %b, align 4
  ret %struct.A* %ret
}

define "intel_dtrans_func_index"="1" %struct.A* @mult_fun(i32 %val)  !intel.dtrans.func.type !12 {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %mul = mul nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, %struct.A* %ret, i32 0, i32 1
  store i32 %mul, i32* %b, align 4
  ret %struct.A* %ret
}

define i32 @func(i32 %in_val)  {
entry:
  %0 = load i32, i32* @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store %struct.A* (i32)* bitcast(%struct.A.01* (i32)* @add_fun to %struct.A* (i32)*), %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.else.end

if.else.then:
  store %struct.A* (i32)* @sub_fun, %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.else.end:
  store %struct.A* (i32)* @mult_fun, %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load %struct.A* (i32)*, %struct.A* (i32)** @fptr, align 8
  %call = call %struct.A* %1(i32 %in_val), !intel_dtrans_type !7
  %out = alloca %struct.A*, align 8, !intel_dtrans_type !2
  store %struct.A* %call, %struct.A** %out, align 8
  %2 = load %struct.A*, %struct.A** %out, align 8
  %b = getelementptr inbounds %struct.A, %struct.A* %2, i32 0, i32 1
  %3 = load i32, i32* %b, align 4
  ret i32 %3
}

; CHECK: define i32 @func

; Check that the compare with @sub_fun was generated
; CHECK: .indconv.cmp.sub_fun:
; CHECK:   %.indconv.c = icmp eq %struct.A* (i32)* %1, @sub_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.sub_fun, label %.indconv.cmp.mult_fun

; Check that the call to @sub_fun was generated
; CHECK: .indconv.call.sub_fun:
; CHECK:   %call.indconv = call %struct.A* @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the compare to @mult_fun was generated
; CHECK: .indconv.cmp.mult_fun:
; CHECK:   %.indconv.c1 = icmp eq %struct.A* (i32)* %1, @mult_fun
; CHECK:   br i1 %.indconv.c1, label %.indconv.call.mult_fun, label %.indconv.icall.call

; Check that the call to @mult_fun was generated
; CHECK: .indconv.call.mult_fun:
; CHECK:   %call.indconv2 = call %struct.A* @mult_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the indirect call is preserved
; CHECK: .indconv.icall.call:
; CHECK:   %call.indconv3 = call %struct.A* %1(i32 %in_val), !intel_dtrans_type !{{[0-9]+}}
; CHECK:   br label %.indconv.sink.

!1 = !{!"F", i1 false, i32 0, !2}  ; %struct.A* ()
!2 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!3 = !{!1, i32 1}  ; %struct.A* ()*
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!6, i32 1}  ; {}*
!6 = !{!"L", i32 0}  ; {}
!7 = !{!"F", i1 false, i32 1, !2, !4}  ; %struct.A* (i32)
!8 = !{!7, i32 1}  ; %struct.A* (i32)*
!9 = !{%struct.A.01 zeroinitializer, i32 1}  ; %struct.A.01*
!10 = distinct !{!9}
!11 = distinct !{!2}
!12 = distinct !{!2}
!13 = !{!"S", %struct.A zeroinitializer, i32 2, !3, !4} ; { %struct.A* ()*, i32 }
!14 = !{!"S", %struct.A.01 zeroinitializer, i32 2, !5, !4} ; { {}*, i32 }

!intel.dtrans.types = !{!13, !14}

; end INTEL_FEATURE_SW_ADVANCED
