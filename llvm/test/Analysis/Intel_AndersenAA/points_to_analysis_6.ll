; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test for a bug (CQ377744) in Andersens analysis that caused crash
; during processing of IndirectCalls when formals don't match with
; arguments.

%struct.data1 = type { i32, %struct.os_h, i32 }
%struct.os_h = type { i32 (%struct.sys_data_s*, %struct.msg_s*)*, i32 (%struct.sys_data_s*, i32, %struct.msg_s*, i8*, ...)* }
%struct.sys_data_s = type opaque
%struct.msg_s = type opaque

@data = common global %struct.data1 zeroinitializer, align 8
@s1 = external global %struct.sys_data_s*, align 8
@m1 = external global %struct.msg_s*, align 8

; Function Attrs: nounwind uwtable
define i32 @sim_log(%struct.sys_data_s* %sys, i32 %logtype, %struct.msg_s* %msg, i8* %format, ...)  {
entry:
  tail call void @isim_log(%struct.sys_data_s* %sys, i32 %logtype, %struct.msg_s* %msg, i8* %format) 
  ret i32 0
}

declare void @isim_log(%struct.sys_data_s*, i32, %struct.msg_s*, i8*) 

; Function Attrs: nounwind uwtable
define i32 @ipmi_start_timer(%struct.sys_data_s* %timer, %struct.msg_s* %timeout)  {
entry:
  %call = tail call i32 @bar(%struct.sys_data_s* %timer, %struct.msg_s* %timeout) 
  ret i32 %call
}

declare i32 @bar(%struct.sys_data_s*, %struct.msg_s*) 

; Function Attrs: nounwind uwtable
define void @hoo() {
entry:
  store i32 (%struct.sys_data_s*, %struct.msg_s*)* @ipmi_start_timer, i32 (%struct.sys_data_s*, %struct.msg_s*)** getelementptr inbounds (%struct.data1, %struct.data1* @data, i64 0, i32 1, i32 0), align 8
  store i32 (%struct.sys_data_s*, i32, %struct.msg_s*, i8*, ...)* @sim_log, i32 (%struct.sys_data_s*, i32, %struct.msg_s*, i8*, ...)** getelementptr inbounds (%struct.data1, %struct.data1* @data, i64 0, i32 1, i32 1), align 8
  ret void
}

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  %0 = load i32 (%struct.sys_data_s*, %struct.msg_s*)*, i32 (%struct.sys_data_s*, %struct.msg_s*)** getelementptr inbounds (%struct.data1, %struct.data1* @data, i64 0, i32 1, i32 0), align 8
  %1 = load %struct.sys_data_s*, %struct.sys_data_s** @s1, align 8
  %2 = load %struct.msg_s*, %struct.msg_s** @m1, align 8
  %call = tail call i32 %0(%struct.sys_data_s* %1, %struct.msg_s* %2)
  ret void
}
