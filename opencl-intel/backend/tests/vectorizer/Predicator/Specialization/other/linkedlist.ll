; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_22246_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-linux-gnu"

%struct.LLIST = type { i32, %struct.LLIST* }

@.str = private constant [14 x i8] c"list is empty\00", align 1 ; <[14 x i8]*> [#uses=1]
@.str1 = private constant [16 x i8] c"print %p %p %d\0A\00", align 1 ; <[16 x i8]*> [#uses=1]

; CHECK: @list_add
; CHECK: footer
define %struct.LLIST* @list_add(%struct.LLIST** %p, i32 %i) nounwind {
entry:
  %0 = icmp eq %struct.LLIST** %p, null           ; <i1> [#uses=1]
  br i1 %0, label %bb4, label %bb1

bb1:                                              ; preds = %entry
  %1 = tail call noalias i8* @malloc(i64 16) nounwind ; <i8*> [#uses=4]
  %2 = icmp eq i8* %1, null                       ; <i1> [#uses=1]
  br i1 %2, label %bb4, label %bb3

bb3:                                              ; preds = %bb1
  %3 = bitcast i8* %1 to %struct.LLIST*           ; <%struct.LLIST*> [#uses=2]
  %4 = load %struct.LLIST** %p, align 8           ; <%struct.LLIST*> [#uses=1]
  %5 = getelementptr inbounds i8* %1, i64 8       ; <i8*> [#uses=1]
  %6 = bitcast i8* %5 to %struct.LLIST**          ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* %4, %struct.LLIST** %6, align 8
  store %struct.LLIST* %3, %struct.LLIST** %p, align 8
  %7 = bitcast i8* %1 to i32*                     ; <i32*> [#uses=1]
  store i32 %i, i32* %7, align 8
  ret %struct.LLIST* %3

bb4:                                              ; preds = %bb1, %entry
  ret %struct.LLIST* null
}

define %struct.LLIST** @list_search(%struct.LLIST** %n, i32 %i) nounwind readonly {
entry:
  %0 = icmp eq %struct.LLIST** %n, null           ; <i1> [#uses=1]
  br i1 %0, label %bb6, label %bb4

bb1:                                              ; preds = %bb4
  %1 = getelementptr inbounds %struct.LLIST* %5, i64 0, i32 0 ; <i32*> [#uses=1]
  %2 = load i32* %1, align 8                      ; <i32> [#uses=1]
  %3 = icmp eq i32 %2, %i                         ; <i1> [#uses=1]
  br i1 %3, label %bb6, label %bb3

bb3:                                              ; preds = %bb1
  %4 = getelementptr inbounds %struct.LLIST* %5, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  br label %bb4

bb4:                                              ; preds = %bb3, %entry
  %n_addr.0 = phi %struct.LLIST** [ %4, %bb3 ], [ %n, %entry ] ; <%struct.LLIST**> [#uses=2]
  %5 = load %struct.LLIST** %n_addr.0, align 8    ; <%struct.LLIST*> [#uses=3]
  %6 = icmp eq %struct.LLIST* %5, null            ; <i1> [#uses=1]
  br i1 %6, label %bb6, label %bb1

bb6:                                              ; preds = %bb4, %bb1, %entry
  %.0 = phi %struct.LLIST** [ null, %entry ], [ null, %bb4 ], [ %n_addr.0, %bb1 ] ; <%struct.LLIST**> [#uses=1]
  ret %struct.LLIST** %.0
}

define void @list_print(%struct.LLIST* %n) nounwind {
entry:
  %0 = icmp eq %struct.LLIST* %n, null            ; <i1> [#uses=1]
  br i1 %0, label %bb2.preheader, label %bb1

bb2.preheader:                                    ; preds = %entry
  %1 = tail call i32 @puts(i8* getelementptr inbounds ([14 x i8]* @.str, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  ret void

bb1:                                              ; preds = %bb1, %entry
  %n_addr.04 = phi %struct.LLIST* [ %7, %bb1 ], [ %n, %entry ] ; <%struct.LLIST*> [#uses=3]
  %2 = getelementptr inbounds %struct.LLIST* %n_addr.04, i64 0, i32 0 ; <i32*> [#uses=1]
  %3 = load i32* %2, align 8                      ; <i32> [#uses=1]
  %4 = getelementptr inbounds %struct.LLIST* %n_addr.04, i64 0, i32 1 ; <%struct.LLIST**> [#uses=2]
  %5 = load %struct.LLIST** %4, align 8           ; <%struct.LLIST*> [#uses=1]
  %6 = tail call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([16 x i8]* @.str1, i64 0, i64 0), %struct.LLIST* %n_addr.04, %struct.LLIST* %5, i32 %3) nounwind ; <i32> [#uses=0]
  %7 = load %struct.LLIST** %4, align 8           ; <%struct.LLIST*> [#uses=2]
  %8 = icmp eq %struct.LLIST* %7, null            ; <i1> [#uses=1]
  br i1 %8, label %return, label %bb1

return:                                           ; preds = %bb1
  ret void
}

declare i32 @puts(i8* nocapture) nounwind

declare i32 @printf(i8* nocapture, ...) nounwind

define void @list_remove(%struct.LLIST** %p) nounwind {
entry:
  %0 = icmp eq %struct.LLIST** %p, null           ; <i1> [#uses=1]
  br i1 %0, label %return, label %bb

bb:                                               ; preds = %entry
  %1 = load %struct.LLIST** %p, align 8           ; <%struct.LLIST*> [#uses=3]
  %2 = icmp eq %struct.LLIST* %1, null            ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb1

bb1:                                              ; preds = %bb
  %3 = getelementptr inbounds %struct.LLIST* %1, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %4 = load %struct.LLIST** %3, align 8           ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %4, %struct.LLIST** %p, align 8
  %5 = bitcast %struct.LLIST* %1 to i8*           ; <i8*> [#uses=1]
  tail call void @free(i8* %5) nounwind
  ret void

return:                                           ; preds = %bb, %entry
  ret void
}

declare void @free(i8* nocapture) nounwind

declare noalias i8* @malloc(i64) nounwind

define i32 @main() nounwind {
bb1.i:
  %n = alloca %struct.LLIST*, align 8             ; <%struct.LLIST**> [#uses=16]
  store %struct.LLIST* null, %struct.LLIST** %n, align 8
  %0 = call noalias i8* @malloc(i64 16) nounwind  ; <i8*> [#uses=4]
  %1 = icmp eq i8* %0, null                       ; <i1> [#uses=1]
  br i1 %1, label %bb1.i41, label %bb3.i

bb3.i:                                            ; preds = %bb1.i
  %2 = bitcast i8* %0 to %struct.LLIST*           ; <%struct.LLIST*> [#uses=2]
  %3 = getelementptr inbounds i8* %0, i64 8       ; <i8*> [#uses=1]
  %4 = bitcast i8* %3 to %struct.LLIST**          ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* null, %struct.LLIST** %4, align 8
  store %struct.LLIST* %2, %struct.LLIST** %n, align 8
  %5 = bitcast i8* %0 to i32*                     ; <i32*> [#uses=1]
  store i32 0, i32* %5, align 8
  br label %bb1.i41

bb1.i41:                                          ; preds = %bb3.i, %bb1.i
  %6 = phi %struct.LLIST* [ %2, %bb3.i ], [ null, %bb1.i ] ; <%struct.LLIST*> [#uses=2]
  %7 = call noalias i8* @malloc(i64 16) nounwind  ; <i8*> [#uses=4]
  %8 = icmp eq i8* %7, null                       ; <i1> [#uses=1]
  br i1 %8, label %bb1.i37, label %bb3.i42

bb3.i42:                                          ; preds = %bb1.i41
  %9 = bitcast i8* %7 to %struct.LLIST*           ; <%struct.LLIST*> [#uses=2]
  %10 = getelementptr inbounds i8* %7, i64 8      ; <i8*> [#uses=1]
  %11 = bitcast i8* %10 to %struct.LLIST**        ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* %6, %struct.LLIST** %11, align 8
  store %struct.LLIST* %9, %struct.LLIST** %n, align 8
  %12 = bitcast i8* %7 to i32*                    ; <i32*> [#uses=1]
  store i32 1, i32* %12, align 8
  br label %bb1.i37

bb1.i37:                                          ; preds = %bb3.i42, %bb1.i41
  %13 = phi %struct.LLIST* [ %9, %bb3.i42 ], [ %6, %bb1.i41 ] ; <%struct.LLIST*> [#uses=2]
  %14 = call noalias i8* @malloc(i64 16) nounwind ; <i8*> [#uses=4]
  %15 = icmp eq i8* %14, null                     ; <i1> [#uses=1]
  br i1 %15, label %bb1.i33, label %bb3.i38

bb3.i38:                                          ; preds = %bb1.i37
  %16 = bitcast i8* %14 to %struct.LLIST*         ; <%struct.LLIST*> [#uses=2]
  %17 = getelementptr inbounds i8* %14, i64 8     ; <i8*> [#uses=1]
  %18 = bitcast i8* %17 to %struct.LLIST**        ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* %13, %struct.LLIST** %18, align 8
  store %struct.LLIST* %16, %struct.LLIST** %n, align 8
  %19 = bitcast i8* %14 to i32*                   ; <i32*> [#uses=1]
  store i32 2, i32* %19, align 8
  br label %bb1.i33

bb1.i33:                                          ; preds = %bb3.i38, %bb1.i37
  %20 = phi %struct.LLIST* [ %16, %bb3.i38 ], [ %13, %bb1.i37 ] ; <%struct.LLIST*> [#uses=2]
  %21 = call noalias i8* @malloc(i64 16) nounwind ; <i8*> [#uses=4]
  %22 = icmp eq i8* %21, null                     ; <i1> [#uses=1]
  br i1 %22, label %bb1.i29, label %bb3.i34

bb3.i34:                                          ; preds = %bb1.i33
  %23 = bitcast i8* %21 to %struct.LLIST*         ; <%struct.LLIST*> [#uses=2]
  %24 = getelementptr inbounds i8* %21, i64 8     ; <i8*> [#uses=1]
  %25 = bitcast i8* %24 to %struct.LLIST**        ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* %20, %struct.LLIST** %25, align 8
  store %struct.LLIST* %23, %struct.LLIST** %n, align 8
  %26 = bitcast i8* %21 to i32*                   ; <i32*> [#uses=1]
  store i32 3, i32* %26, align 8
  br label %bb1.i29

bb1.i29:                                          ; preds = %bb3.i34, %bb1.i33
  %27 = phi %struct.LLIST* [ %23, %bb3.i34 ], [ %20, %bb1.i33 ] ; <%struct.LLIST*> [#uses=2]
  %28 = call noalias i8* @malloc(i64 16) nounwind ; <i8*> [#uses=4]
  %29 = icmp eq i8* %28, null                     ; <i1> [#uses=1]
  br i1 %29, label %list_add.exit32, label %bb3.i30

bb3.i30:                                          ; preds = %bb1.i29
  %30 = bitcast i8* %28 to %struct.LLIST*         ; <%struct.LLIST*> [#uses=2]
  %31 = getelementptr inbounds i8* %28, i64 8     ; <i8*> [#uses=1]
  %32 = bitcast i8* %31 to %struct.LLIST**        ; <%struct.LLIST**> [#uses=1]
  store %struct.LLIST* %27, %struct.LLIST** %32, align 8
  store %struct.LLIST* %30, %struct.LLIST** %n, align 8
  %33 = bitcast i8* %28 to i32*                   ; <i32*> [#uses=1]
  store i32 4, i32* %33, align 8
  br label %list_add.exit32

list_add.exit32:                                  ; preds = %bb3.i30, %bb1.i29
  %34 = phi %struct.LLIST* [ %27, %bb1.i29 ], [ %30, %bb3.i30 ] ; <%struct.LLIST*> [#uses=2]
  %35 = icmp eq %struct.LLIST* %34, null          ; <i1> [#uses=1]
  br i1 %35, label %bb2.preheader.i24, label %bb1.i26

bb2.preheader.i24:                                ; preds = %list_add.exit32
  %36 = call i32 @puts(i8* getelementptr inbounds ([14 x i8]* @.str, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  br label %bb.i20

bb1.i26:                                          ; preds = %bb1.i26, %list_add.exit32
  %n_addr.04.i25 = phi %struct.LLIST* [ %42, %bb1.i26 ], [ %34, %list_add.exit32 ] ; <%struct.LLIST*> [#uses=3]
  %37 = getelementptr inbounds %struct.LLIST* %n_addr.04.i25, i64 0, i32 0 ; <i32*> [#uses=1]
  %38 = load i32* %37, align 8                    ; <i32> [#uses=1]
  %39 = getelementptr inbounds %struct.LLIST* %n_addr.04.i25, i64 0, i32 1 ; <%struct.LLIST**> [#uses=2]
  %40 = load %struct.LLIST** %39, align 8         ; <%struct.LLIST*> [#uses=1]
  %41 = call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([16 x i8]* @.str1, i64 0, i64 0), %struct.LLIST* %n_addr.04.i25, %struct.LLIST* %40, i32 %38) nounwind ; <i32> [#uses=0]
  %42 = load %struct.LLIST** %39, align 8         ; <%struct.LLIST*> [#uses=2]
  %43 = icmp eq %struct.LLIST* %42, null          ; <i1> [#uses=1]
  br i1 %43, label %bb.i20, label %bb1.i26

bb.i20:                                           ; preds = %bb1.i26, %bb2.preheader.i24
  %44 = load %struct.LLIST** %n, align 8          ; <%struct.LLIST*> [#uses=4]
  %45 = icmp eq %struct.LLIST* %44, null          ; <i1> [#uses=1]
  br i1 %45, label %list_remove.exit23, label %bb1.i21

bb1.i21:                                          ; preds = %bb.i20
  %46 = getelementptr inbounds %struct.LLIST* %44, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %47 = load %struct.LLIST** %46, align 8         ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %47, %struct.LLIST** %n, align 8
  %48 = bitcast %struct.LLIST* %44 to i8*         ; <i8*> [#uses=1]
  call void @free(i8* %48) nounwind
  %.pre = load %struct.LLIST** %n, align 8        ; <%struct.LLIST*> [#uses=1]
  br label %list_remove.exit23

list_remove.exit23:                               ; preds = %bb1.i21, %bb.i20
  %49 = phi %struct.LLIST* [ %44, %bb.i20 ], [ %.pre, %bb1.i21 ] ; <%struct.LLIST*> [#uses=1]
  %50 = getelementptr inbounds %struct.LLIST* %49, i64 0, i32 1 ; <%struct.LLIST**> [#uses=3]
  %51 = icmp eq %struct.LLIST** %50, null         ; <i1> [#uses=1]
  br i1 %51, label %bb4.i15, label %bb.i16

bb.i16:                                           ; preds = %list_remove.exit23
  %52 = load %struct.LLIST** %50, align 8         ; <%struct.LLIST*> [#uses=3]
  %53 = icmp eq %struct.LLIST* %52, null          ; <i1> [#uses=1]
  br i1 %53, label %bb4.i15, label %bb1.i17

bb1.i17:                                          ; preds = %bb.i16
  %54 = getelementptr inbounds %struct.LLIST* %52, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %55 = load %struct.LLIST** %54, align 8         ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %55, %struct.LLIST** %50, align 8
  %56 = bitcast %struct.LLIST* %52 to i8*         ; <i8*> [#uses=1]
  call void @free(i8* %56) nounwind
  br label %bb4.i15

bb1.i13:                                          ; preds = %bb4.i15
  %57 = getelementptr inbounds %struct.LLIST* %61, i64 0, i32 0 ; <i32*> [#uses=1]
  %58 = load i32* %57, align 8                    ; <i32> [#uses=1]
  %59 = icmp eq i32 %58, 1                        ; <i1> [#uses=1]
  br i1 %59, label %list_search.exit, label %bb3.i14

bb3.i14:                                          ; preds = %bb1.i13
  %60 = getelementptr inbounds %struct.LLIST* %61, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  br label %bb4.i15

bb4.i15:                                          ; preds = %bb3.i14, %bb1.i17, %bb.i16, %list_remove.exit23
  %n_addr.0.i = phi %struct.LLIST** [ %60, %bb3.i14 ], [ %n, %list_remove.exit23 ], [ %n, %bb.i16 ], [ %n, %bb1.i17 ] ; <%struct.LLIST**> [#uses=3]
  %61 = load %struct.LLIST** %n_addr.0.i, align 8 ; <%struct.LLIST*> [#uses=5]
  %62 = icmp eq %struct.LLIST* %61, null          ; <i1> [#uses=1]
  br i1 %62, label %list_remove.exit12, label %bb1.i13

list_search.exit:                                 ; preds = %bb1.i13
  %63 = icmp eq %struct.LLIST** %n_addr.0.i, null ; <i1> [#uses=1]
  br i1 %63, label %list_remove.exit12, label %bb1.i10

bb1.i10:                                          ; preds = %list_search.exit
  %64 = getelementptr inbounds %struct.LLIST* %61, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %65 = load %struct.LLIST** %64, align 8         ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %65, %struct.LLIST** %n_addr.0.i, align 8
  %66 = bitcast %struct.LLIST* %61 to i8*         ; <i8*> [#uses=1]
  call void @free(i8* %66) nounwind
  br label %list_remove.exit12

list_remove.exit12:                               ; preds = %bb1.i10, %list_search.exit, %bb4.i15
  %67 = load %struct.LLIST** %n, align 8          ; <%struct.LLIST*> [#uses=3]
  %68 = getelementptr inbounds %struct.LLIST* %67, i64 0, i32 1 ; <%struct.LLIST**> [#uses=3]
  %69 = icmp eq %struct.LLIST** %68, null         ; <i1> [#uses=1]
  br i1 %69, label %bb.i, label %bb.i5

bb.i5:                                            ; preds = %list_remove.exit12
  %70 = load %struct.LLIST** %68, align 8         ; <%struct.LLIST*> [#uses=3]
  %71 = icmp eq %struct.LLIST* %70, null          ; <i1> [#uses=1]
  br i1 %71, label %bb.i, label %bb1.i6

bb1.i6:                                           ; preds = %bb.i5
  %72 = getelementptr inbounds %struct.LLIST* %70, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %73 = load %struct.LLIST** %72, align 8         ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %73, %struct.LLIST** %68, align 8
  %74 = bitcast %struct.LLIST* %70 to i8*         ; <i8*> [#uses=1]
  call void @free(i8* %74) nounwind
  %.pr.pre = load %struct.LLIST** %n, align 8     ; <%struct.LLIST*> [#uses=1]
  br label %bb.i

bb.i:                                             ; preds = %bb1.i6, %bb.i5, %list_remove.exit12
  %75 = phi %struct.LLIST* [ %67, %list_remove.exit12 ], [ %67, %bb.i5 ], [ %.pr.pre, %bb1.i6 ] ; <%struct.LLIST*> [#uses=4]
  %76 = icmp eq %struct.LLIST* %75, null          ; <i1> [#uses=1]
  br i1 %76, label %list_remove.exit, label %bb1.i3

bb1.i3:                                           ; preds = %bb.i
  %77 = getelementptr inbounds %struct.LLIST* %75, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %78 = load %struct.LLIST** %77, align 8         ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %78, %struct.LLIST** %n, align 8
  %79 = bitcast %struct.LLIST* %75 to i8*         ; <i8*> [#uses=1]
  call void @free(i8* %79) nounwind
  %.pre47 = load %struct.LLIST** %n, align 8      ; <%struct.LLIST*> [#uses=1]
  br label %list_remove.exit

list_remove.exit:                                 ; preds = %bb1.i3, %bb.i
  %80 = phi %struct.LLIST* [ %75, %bb.i ], [ %.pre47, %bb1.i3 ] ; <%struct.LLIST*> [#uses=2]
  %81 = icmp eq %struct.LLIST* %80, null          ; <i1> [#uses=1]
  br i1 %81, label %bb2.preheader.i, label %bb1.i2

bb2.preheader.i:                                  ; preds = %list_remove.exit
  %82 = call i32 @puts(i8* getelementptr inbounds ([14 x i8]* @.str, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  ret i32 0

bb1.i2:                                           ; preds = %bb1.i2, %list_remove.exit
  %n_addr.04.i = phi %struct.LLIST* [ %88, %bb1.i2 ], [ %80, %list_remove.exit ] ; <%struct.LLIST*> [#uses=3]
  %83 = getelementptr inbounds %struct.LLIST* %n_addr.04.i, i64 0, i32 0 ; <i32*> [#uses=1]
  %84 = load i32* %83, align 8                    ; <i32> [#uses=1]
  %85 = getelementptr inbounds %struct.LLIST* %n_addr.04.i, i64 0, i32 1 ; <%struct.LLIST**> [#uses=2]
  %86 = load %struct.LLIST** %85, align 8         ; <%struct.LLIST*> [#uses=1]
  %87 = call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([16 x i8]* @.str1, i64 0, i64 0), %struct.LLIST* %n_addr.04.i, %struct.LLIST* %86, i32 %84) nounwind ; <i32> [#uses=0]
  %88 = load %struct.LLIST** %85, align 8         ; <%struct.LLIST*> [#uses=2]
  %89 = icmp eq %struct.LLIST* %88, null          ; <i1> [#uses=1]
  br i1 %89, label %list_print.exit, label %bb1.i2

list_print.exit:                                  ; preds = %bb1.i2
  ret i32 0
}

