; Test taking the global variables used for code coverage information
; and writing it into a temporary file that uses a test data format
; of the llvm-cov tool to verify the spi-emission pass for converting
; the memory data structures to a file format.

; RUN: opt -passes spi-emitter -spi-generate=true -spi-temp-filename=%t.spi -disable-output %s
; RUN: llvm-profdata merge %S/Inputs/spi-emitter2.profile -o %t.profdata
; RUN: llvm-cov report -format=xml -instr-profile=%t.profdata %t.spi | FileCheck %s

; CHECK: <PROJECT>
; CHECK:   <MODULE name="{{.*}}test.c">
; CHECK:     <FUNCTION name="foo" freq="1">
; CHECK:       <BLOCKS total="4" covered="3" coverage="75.000000%">
; CHECK:       </BLOCKS>
; CHECK:     </FUNCTION>
; CHECK:     <FUNCTION name="func" freq="0">
; CHECK:       <BLOCKS total="1" covered="0" coverage="0.000000%">
; CHECK:       </BLOCKS>
; CHECK:     </FUNCTION>
; CHECK:     <FUNCTION name="main" freq="1">
; CHECK:       <BLOCKS total="1" covered="1" coverage="100.000000%">
; CHECK:       </BLOCKS>
; CHECK:     </FUNCTION>
; CHECK:   </MODULE>
; CHECK: </PROJECT>

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$__covrec_5CF8C24CDB18BDACu = comdat any
$__covrec_65285A4A5F93F47Du = comdat any
$__covrec_DB956436E78DD5FAu = comdat any
$__profc_foo = comdat nodeduplicate
$__profc_func = comdat nodeduplicate
$__profc_main = comdat nodeduplicate

@gVar = dso_local global i32 0, align 4
@__covrec_5CF8C24CDB18BDACu = linkonce_odr hidden constant <{ i64, i32, i64, i64, [56 x i8] }> <{ i64 6699318081062747564, i32 56, i64 172590168, i64 -6265844179253399062, [56 x i8] c"\01\01\01\01\05\08\01\03\14\06\02\01\01\07\00\0C \05\02\00\07\00\0C\05\00\0D\00\8E\80\80\80\08\05\00\0E\02\04\02\02\04\02\83\80\80\80\08\10\01\01\00\01\02\01\03\00\10" }>, section "__llvm_covfun", comdat, align 8
@__covrec_65285A4A5F93F47Du = linkonce_odr hidden constant <{ i64, i32, i64, i64, [9 x i8] }> <{ i64 7289175272376759421, i32 9, i64 0, i64 -6265844179253399062, [9 x i8] c"\01\01\00\01\01\0B\0D\02\02" }>, section "__llvm_covfun", comdat, align 8
@__covrec_DB956436E78DD5FAu = linkonce_odr hidden constant <{ i64, i32, i64, i64, [9 x i8] }> <{ i64 -2624081020897602054, i32 9, i64 24, i64 -6265844179253399062, [9 x i8] c"\01\01\00\01\01\0F\22\03\02" }>, section "__llvm_covfun", comdat, align 8
@__llvm_coverage_mapping = private constant { { i32, i32, i32, i32 }, [58 x i8] } { { i32, i32, i32, i32 } { i32 0, i32 58, i32 0, i32 5 }, [58 x i8] c"\027\00//localdisk2/cmchruls/workspaces/xmain-cov/test1\06test.c" }, section "__llvm_covmap", align 8
@__profc_foo = private global [2 x i64] zeroinitializer, section "__llvm_prf_cnts", comdat, align 8
@__profd_foo = private global { i64, i64, i64, ptr, ptr, i32, [3 x i16] } { i64 6699318081062747564, i64 172590168, i64 sub (i64 ptrtoint (ptr @__profc_foo to i64), i64 ptrtoint (ptr @__profd_foo to i64)), ptr null, ptr null, i32 2, [3 x i16] zeroinitializer }, section "__llvm_prf_data", comdat($__profc_foo), align 8
@__profc_func = private global [1 x i64] zeroinitializer, section "__llvm_prf_cnts", comdat, align 8
@__profd_func = private global { i64, i64, i64, ptr, ptr, i32, [3 x i16] } { i64 7289175272376759421, i64 0, i64 sub (i64 ptrtoint (ptr @__profc_func to i64), i64 ptrtoint (ptr @__profd_func to i64)), ptr null, ptr null, i32 1, [3 x i16] zeroinitializer }, section "__llvm_prf_data", comdat($__profc_func), align 8
@__profc_main = private global [1 x i64] zeroinitializer, section "__llvm_prf_cnts", comdat, align 8
@__profd_main = private global { i64, i64, i64, ptr, ptr, i32, [3 x i16] } { i64 -2624081020897602054, i64 24, i64 sub (i64 ptrtoint (ptr @__profc_main to i64), i64 ptrtoint (ptr @__profd_main to i64)), ptr null, ptr null, i32 1, [3 x i16] zeroinitializer }, section "__llvm_prf_data", comdat($__profc_main), align 8
@__llvm_prf_nm = private constant [15 x i8] c"\0D\00foo\01func\01main", section "__llvm_prf_names", align 1
@llvm.compiler.used = appending global [3 x ptr] [ptr @__profd_foo, ptr @__profd_func, ptr @__profd_main], section "llvm.metadata"
@llvm.used = appending global [5 x ptr] [ptr @__covrec_5CF8C24CDB18BDACu, ptr @__covrec_65285A4A5F93F47Du, ptr @__covrec_DB956436E78DD5FAu, ptr @__llvm_coverage_mapping, ptr @__llvm_prf_nm], section "llvm.metadata"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 noundef %count) #0 {
entry:
  %retval = alloca i32, align 4
  %count.addr = alloca i32, align 4
  store i32 %count, ptr %count.addr, align 4
  %pgocount = load i64, ptr @__profc_foo, align 8
  %0 = add i64 %pgocount, 1
  store i64 %0, ptr @__profc_foo, align 8
  %1 = load i32, ptr %count.addr, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %pgocount1 = load i64, ptr getelementptr inbounds ([2 x i64], ptr @__profc_foo, i32 0, i32 1), align 8
  %2 = add i64 %pgocount1, 1
  store i64 %2, ptr getelementptr inbounds ([2 x i64], ptr @__profc_foo, i32 0, i32 1), align 8
  %3 = load i32, ptr @gVar, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr @gVar, align 4
  store i32 %inc, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %4 = load i32, ptr @gVar, align 4
  %dec = add nsw i32 %4, -1
  store i32 %dec, ptr @gVar, align 4
  store i32 %dec, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %5 = load i32, ptr %retval, align 4
  ret i32 %5
}

; Function Attrs: nounwind
declare void @llvm.instrprof.increment(ptr, i64, i32, i32) #1

; Function Attrs: nounwind uwtable
define dso_local void @func() #0 {
entry:
  %pgocount = load i64, ptr @__profc_func, align 8
  %0 = add i64 %pgocount, 1
  store i64 %0, ptr @__profc_func, align 8
  store i32 10, ptr @gVar, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 noundef %argc, ptr noundef %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  store i32 %argc, ptr %argc.addr, align 4
  store ptr %argv, ptr %argv.addr, align 8
  %pgocount = load i64, ptr @__profc_main, align 8
  %0 = add i64 %pgocount, 1
  store i64 %0, ptr @__profc_main, align 8
  %1 = load i32, ptr %argc.addr, align 4
  %call = call i32 @foo(i32 noundef %1)
  %2 = load i32, ptr @gVar, align 4
  ret i32 %2
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"EnableValueProfiling", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}