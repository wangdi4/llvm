; It checks that the call to myprintf has been inlined and removed by
; dead static function elimination.  It then checks that the function
; main still exists, and that the call to @llvm.va_arg_pack has been
; removed and that the function main returns 0.

; ModuleID = 'cq415203b.cpp'
source_filename = "cq415203b.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe801 < %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-CL,CHECK
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe880 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe880 -S | opt -passes='inlinereportemitter' -inline-report=0xe880 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD,CHECK

; CHECK-MD: -> INLINE: {{.*}}myprintf{{.*}}
; CHECK-MD:DEAD STATIC FUNC: {{.*}}myprintf{{.*}}
; CHECK: define i32 @main()
; CHECK-NOT: call i32 @llvm.va_arg_pack
; CHECK: ret i32 0
; CHECK-CL:DEAD STATIC FUNC: {{.*}}myprintf{{.*}}
; CHECK-CL: -> INLINE: {{.*}}myprintf{{.*}}

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@_ZL4xcox = internal global i32 234, align 4
@.str.1 = private unnamed_addr constant [11 x i8] c"myprintf: \00", align 1

; Function Attrs: norecurse uwtable
define i32 @main() #0 {
entry:
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %1 = load i32, i32* @_ZL4xcox, align 4
  %call = call i32 (%struct._IO_FILE*, i8*, ...) @_ZL8myprintfP8_IO_FILEPKcz(%struct._IO_FILE* %0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1)
  ret i32 0
}

define internal i32 @_ZL8myprintfP8_IO_FILEPKcz(%struct._IO_FILE* %f, i8* %format, ...) #1 {
entry:
  %retval = alloca i32, align 4
  %f.addr = alloca %struct._IO_FILE*, align 8
  %format.addr = alloca i8*, align 8
  %r = alloca i32, align 4
  %s = alloca i32, align 4
  store %struct._IO_FILE* %f, %struct._IO_FILE** %f.addr, align 8
  store i8* %format, i8** %format.addr, align 8
  %0 = load %struct._IO_FILE*, %struct._IO_FILE** %f.addr, align 8
  %call = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %0, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i32 0, i32 0))
  store i32 %call, i32* %r, align 4
  %1 = load i32, i32* %r, align 4
  %cmp = icmp slt i32 %1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load i32, i32* %r, align 4
  store i32 %2, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %3 = load %struct._IO_FILE*, %struct._IO_FILE** %f.addr, align 8
  %4 = load i8*, i8** %format.addr, align 8
  %5 = call i32 @llvm.va_arg_pack()
  %call1 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %3, i8* %4, i32 %5)
  store i32 %call1, i32* %s, align 4
  %6 = load i32, i32* %s, align 4
  %cmp2 = icmp slt i32 %6, 0
  br i1 %cmp2, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  %7 = load i32, i32* %s, align 4
  store i32 %7, i32* %retval, align 4
  br label %return

if.end4:                                          ; preds = %if.end
  %8 = load i32, i32* %r, align 4
  %9 = load i32, i32* %s, align 4
  %add = add nsw i32 %8, %9
  store i32 %add, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.end4, %if.then3, %if.then
  %10 = load i32, i32* %retval, align 4
  ret i32 %10
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) #2

declare i32 @llvm.va_arg_pack() #3

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20447) (llvm/branches/ltoprof 20619)"}
