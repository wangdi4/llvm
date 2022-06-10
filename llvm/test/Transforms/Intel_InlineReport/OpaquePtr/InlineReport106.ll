; RUN: opt -opaque-pointers < %s -call-tree-clone -force-print-inline-report-after-call-tree-cloning -inline-report=0xe807 -call-tree-clone-do-mv=false -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK
; RUN: opt -opaque-pointers < %s -passes='module(call-tree-clone)' -force-print-inline-report-after-call-tree-cloning -inline-report=0xe807 -call-tree-clone-do-mv=false -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -call-tree-clone -inline-report=0xe886 -call-tree-clone-do-mv=false -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='module(call-tree-clone)' -inline-report=0xe886 -call-tree-clone-do-mv=false -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-MD

; Check that call tree clones appear in the inlining report.

; Classic inline report output: 

; CHECK-LABEL: COMPILE FUNC: foo{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo {{.*}}Newly created callsite
; CHECK: foo {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|_.6.5.11{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: gee{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: bar {{.*}}Newly created callsite
; CHECK: bar {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|_.5.6{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo {{.*}}Newly created callsite
; CHECK: foo|_.6.5.11 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|9.7.13.2{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|15.9.5.14{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|7.8.6{{ *$}}
; CHECK: printf3{{.*}}Newly created callsite
; CHECK: foo|9.7.13.2 {{.*}}Newly created callsite
; CHECK: foo|15.9.5.14 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: main{{ *$}}
; CHECK: printf0 {{.*}}Newly created callsite
; CHECK: gee|2.4.5 {{.*}}Newly created callsite
; CHECK: gee|1.2.3 {{.*}}Newly created callsite
; CHECK: gee|2.4.5 {{.*}}Newly created callsite
; CHECK: gee|3.1.0 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: gee|2.4.5{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: bar|_.5.6 {{.*}}Newly created callsite
; CHECK: bar|7.8.6 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|_.4.5.7{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|_.3.4{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo {{.*}}Newly created callsite
; CHECK: foo|_.4.5.7 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|9.7.10.2{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|15.9.5.11{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|7.8.3{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo|9.7.10.2 {{.*}}Newly created callsite
; CHECK: foo|15.9.5.11 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: gee|1.2.3{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: bar|_.3.4 {{.*}}Newly created callsite
; CHECK: bar|7.8.3 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|_.3.5.3{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|_.2.1{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo {{.*}}Newly created callsite
; CHECK: foo|_.3.5.3 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|9.7.11.2{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: foo|15.9.5.12{{ *$}}
; CHECK: printf4 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite
; CHECK: printf1 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: bar|7.8.4{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: foo|9.7.11.2 {{.*}}Newly created callsite
; CHECK: foo|15.9.5.12 {{.*}}Newly created callsite

; CHECK-LABEL: COMPILE FUNC: gee|3.1.0{{ *$}}
; CHECK: printf3 {{.*}}Newly created callsite
; CHECK: bar|_.2.1 {{.*}}Newly created callsite
; CHECK: bar|7.8.4 {{.*}}Newly created callsite

; Metadata inline report output

; CHECK-MD-LABEL: COMPILE FUNC: foo{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo {{.*}}Not tested for inlining
; CHECK-MD: foo {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: gee{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: bar {{.*}}Not tested for inlining
; CHECK-MD: bar {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: main{{ *$}}
; CHECK-MD: EXTERN: printf0{{ *$}}
; CHECK-MD: gee|2.4.5 {{.*}}Not tested for inlining
; CHECK-MD: gee|1.2.3 {{.*}}Not tested for inlining
; CHECK-MD: gee|2.4.5 {{.*}}Not tested for inlining
; CHECK-MD: gee|3.1.0 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|_.6.5.11{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|_.5.6{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo {{.*}}Not tested for inlining
; CHECK-MD: foo|_.6.5.11 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|9.7.13.2{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: foo|15.9.5.14{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|7.8.6{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo|9.7.13.2 {{.*}}Not tested for inlining
; CHECK-MD: foo|15.9.5.14 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: gee|2.4.5{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: bar|_.5.6 {{.*}}Not tested for inlining
; CHECK-MD: bar|7.8.6 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|_.4.5.7{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|_.3.4{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo {{.*}}Not tested for inlining
; CHECK-MD: foo|_.4.5.7 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|9.7.10.2{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: foo|15.9.5.11{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|7.8.3{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo|9.7.10.2 {{.*}}Not tested for inlining
; CHECK-MD: foo|15.9.5.11 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: gee|1.2.3{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: bar|_.3.4 {{.*}}Not tested for inlining
; CHECK-MD: bar|7.8.3 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|_.3.5.3{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|_.2.1{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo {{.*}}Not tested for inlining
; CHECK-MD: foo|_.3.5.3{{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: foo|9.7.11.2{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: foo|15.9.5.12{{ *$}}
; CHECK-MD: EXTERN: printf4{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}
; CHECK-MD: EXTERN: printf1{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: bar|7.8.4{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: foo|9.7.11.2 {{.*}}Not tested for inlining
; CHECK-MD: foo|15.9.5.12 {{.*}}Not tested for inlining

; CHECK-MD-LABEL: COMPILE FUNC: gee|3.1.0{{ *$}}
; CHECK-MD: EXTERN: printf3{{ *$}}
; CHECK-MD: bar|_.2.1 {{.*}}Not tested for inlining
; CHECK-MD: bar|7.8.4 {{.*}}Not tested for inlining

source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [27 x i8] c"      foo(%d, %d, %d, %d)\0A\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"        loop1:%d\0A\00", align 1
@.str.2 = private unnamed_addr constant [18 x i8] c"        loop2:%d\0A\00", align 1
@.str.3 = private unnamed_addr constant [21 x i8] c"    bar(%d, %d, %d)\0A\00", align 1
@.str.4 = private unnamed_addr constant [19 x i8] c"  gee(%d, %d, %d)\0A\00", align 1
@glob = common dso_local global i32 0, align 4
@.str.5 = private unnamed_addr constant [6 x i8] c"main\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define internal void @foo(i32, i32, i32, i32) #0 {
  tail call void @printf4(ptr getelementptr inbounds ([27 x i8], ptr @.str, i64 0, i64 0), i32 %0, i32 %1, i32 %2, i32 %3) #3
  %5 = add i32 %1, %0
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %14, label %7

; <label>:7:                                      ; preds = %14, %4
  %8 = add nsw i32 %2, %1
  %9 = add nsw i32 %8, %3
  %10 = icmp sgt i32 %9, 0
  br i1 %10, label %11, label %22

; <label>:11:                                     ; preds = %7
  %12 = add i32 %3, %2
  %13 = add i32 %12, %1
  br label %18

; <label>:14:                                     ; preds = %14, %4
  %15 = phi i32 [ %16, %14 ], [ 0, %4 ]
  tail call void @printf1(ptr getelementptr inbounds ([18 x i8], ptr @.str.1, i64 0, i64 0), i32 %15) #3
  %16 = add nuw nsw i32 %15, 1
  %17 = icmp eq i32 %16, %5
  br i1 %17, label %7, label %14

; <label>:18:                                     ; preds = %18, %11
  %19 = phi i32 [ %20, %18 ], [ 0, %11 ]
  tail call void @printf1(ptr getelementptr inbounds ([18 x i8], ptr @.str.2, i64 0, i64 0), i32 %19) #3
  %20 = add nuw nsw i32 %19, 1
  %21 = icmp eq i32 %20, %13
  br i1 %21, label %22, label %18

; <label>:22:                                     ; preds = %18, %7
  ret void
}

declare dso_local void @printf4(ptr, i32, i32, i32, i32) local_unnamed_addr #1

declare dso_local void @printf1(ptr, i32) local_unnamed_addr #1

; Function Attrs: noinline nounwind uwtable
define internal void @bar(i32, i32, i32) #0 {
  tail call void @printf3(ptr getelementptr inbounds ([21 x i8], ptr @.str.3, i64 0, i64 0), i32 %0, i32 %1, i32 %2) #3
  %4 = add nsw i32 %1, 1
  %5 = add nsw i32 %2, %0
  tail call void @foo(i32 %4, i32 %0, i32 %5, i32 2)
  %6 = add nsw i32 %1, %0
  %7 = add nsw i32 %2, %1
  tail call void @foo(i32 %6, i32 %4, i32 5, i32 %7)
  ret void
}

declare dso_local void @printf3(ptr, i32, i32, i32) local_unnamed_addr #1

; Function Attrs: noinline nounwind uwtable
define internal void @gee(i32, i32, i32) #0 {
  tail call void @printf3(ptr getelementptr inbounds ([19 x i8], ptr @.str.4, i64 0, i64 0), i32 %0, i32 %1, i32 %2) #3
  %4 = load i32, ptr @glob, align 4, !tbaa !2
  %5 = add nsw i32 %1, 1
  %6 = add nsw i32 %2, 1
  tail call void @bar(i32 %4, i32 %5, i32 %6)
  %7 = add nsw i32 %1, %0
  tail call void @bar(i32 7, i32 8, i32 %7)
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, ptr nocapture readnone) #2 {
  store i32 %0, ptr @glob, align 4, !tbaa !2
  tail call void @printf0(ptr getelementptr inbounds ([6 x i8], ptr @.str.5, i64 0, i64 0)) #3
  tail call void @gee(i32 2, i32 4, i32 5)
  tail call void @gee(i32 1, i32 2, i32 3)
  tail call void @gee(i32 2, i32 4, i32 5)
  tail call void @gee(i32 3, i32 1, i32 0)
  ret i32 0
}

declare dso_local void @printf0(ptr) local_unnamed_addr #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

;!llvm.ident = !{!0}
!llvm.module.flags = !{!1}

;!0 = !{!"clang version 7.0.0"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; Original source a.c is below.
; Compile: icx -flto a.c

; //#include <stdio.h>
; //#define printf0 printf
; //#define printf1 printf
; //#define printf3 printf
; //#define printf4 printf
; extern void printf0(const char*);
; extern void printf1(const char*, int);
; extern void printf3(const char*, int, int, int);
; extern void printf4(const char*, int, int, int, int);
; extern int glob;
; 
; int glob;
; 
; #define NOI __declspec(noinline)
; 
; NOI void foo(int p1, int p2, int p3, int p4) {
;   int i;
;   printf4("      foo(%d, %d, %d, %d)\n", p1, p2, p3, p4);
;   for (i = 0; i < p1+p2; i++)    { printf1("        loop1:%d\n", i); }
;   for (i = 0; i < p2+p3+p4; i++) { printf1("        loop2:%d\n", i); }
; }
; 
; NOI void bar(int x, int y, int z) {
;   printf3("    bar(%d, %d, %d)\n", x, y, z);
;   foo(y+1, x,   x+z, 2);
;   foo(x+y, y+1, 5,   y+z);
; }
; 
; NOI void gee(int a, int b, int c) {
;   printf3("  gee(%d, %d, %d)\n", a, b, c);
;   bar(glob, b+1, c+1);
;   bar(7,    8,   a+b);
; }
; 
; int main(int argc, char** argv) {
;   glob = argc;
;   printf0("main\n");
;   gee(2, 4, 5);
;   gee(1, 2, 3);
;   gee(2, 4, 5);
;   gee(3, 1, 0);
; }
