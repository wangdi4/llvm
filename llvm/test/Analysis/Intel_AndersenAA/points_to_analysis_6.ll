; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test for a bug (CQ377744) in Andersens analysis that caused crash
; during processing of IndirectCalls when formals don't match with
; arguments.

%struct.data1 = type { i32, %struct.os_h, i32 }
%struct.os_h = type { ptr, ptr }

@data = common global %struct.data1 zeroinitializer, align 8
@s1 = external global ptr, align 8
@m1 = external global ptr, align 8

define i32 @sim_log(ptr %sys, i32 %logtype, ptr %msg, ptr %format, ...) {
entry:
  tail call void @isim_log(ptr %sys, i32 %logtype, ptr %msg, ptr %format)
  ret i32 0
}

declare void @isim_log(ptr, i32, ptr, ptr)

define i32 @ipmi_start_timer(ptr %timer, ptr %timeout) {
entry:
  %call = tail call i32 @bar(ptr %timer, ptr %timeout)
  ret i32 %call
}

declare i32 @bar(ptr, ptr)

define void @hoo() {
entry:
  store ptr @ipmi_start_timer, ptr getelementptr inbounds (%struct.data1, ptr @data, i64 0, i32 1, i32 0), align 8
  store ptr @sim_log, ptr getelementptr inbounds (%struct.data1, ptr @data, i64 0, i32 1, i32 1), align 8
  ret void
}

define void @foo() {
entry:
  %i = load ptr, ptr getelementptr inbounds (%struct.data1, ptr @data, i64 0, i32 1, i32 0), align 8
  %i1 = load ptr, ptr @s1, align 8
  %i2 = load ptr, ptr @m1, align 8
  %call = tail call i32 %i(ptr %i1, ptr %i2)
  ret void
}
