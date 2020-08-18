; REQUIRES: asserts
; RUN: opt < %s -debug-only=jump-threading -jump-threading -S 2>&1 | FileCheck %s
; CHECK: JT: Too many DT updates

; Jump threading connects the "a1 =" blocks at the top, directly to
; the "a0 =" blocks at the bottom. This is done incrementally over many
; iterations, resulting in thousands of CFG updates.
; Check that JT aborts when the number of updates exceeds the size of the
; whole function by 10X.
;
; int a0 = 0; int a1 = 0;
; int main() {
; if (a0 == 0)
;  a1 = 1;
; else if (a0 == 1)
;  a1 = 2;
; else if (a0 == 2)
;  a1 = 3;
; ...
; else
;  a1 = 26;
; if (a1 == 1)
;  a0 = 1;
; else if (a1 == 2)
;  a0 = 2;
; else if (a1 == 3)
;  a0 = 3;
; ....
; else
;  a0 = 25;
; return a0 + a1;
; }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a0 = dso_local local_unnamed_addr global i32 0, align 4
@a1 = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i32, i32* @a0, align 4, !tbaa !2
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 1, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else:                                          ; preds = %entry
  %cmp1 = icmp eq i32 %0, 1
  br i1 %cmp1, label %if.then2, label %if.else3

if.then2:                                         ; preds = %if.else
  store i32 2, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else3:                                         ; preds = %if.else
  %cmp4 = icmp eq i32 %0, 2
  br i1 %cmp4, label %if.then5, label %if.else6

if.then5:                                         ; preds = %if.else3
  store i32 3, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else6:                                         ; preds = %if.else3
  %cmp7 = icmp eq i32 %0, 3
  br i1 %cmp7, label %if.then8, label %if.else9

if.then8:                                         ; preds = %if.else6
  store i32 4, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else9:                                         ; preds = %if.else6
  %cmp10 = icmp eq i32 %0, 4
  br i1 %cmp10, label %if.then11, label %if.else12

if.then11:                                        ; preds = %if.else9
  store i32 5, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else12:                                        ; preds = %if.else9
  %cmp13 = icmp eq i32 %0, 5
  br i1 %cmp13, label %if.then14, label %if.else15

if.then14:                                        ; preds = %if.else12
  store i32 6, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else15:                                        ; preds = %if.else12
  %cmp16 = icmp eq i32 %0, 6
  br i1 %cmp16, label %if.then17, label %if.else18

if.then17:                                        ; preds = %if.else15
  store i32 7, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else18:                                        ; preds = %if.else15
  %cmp19 = icmp eq i32 %0, 7
  br i1 %cmp19, label %if.then20, label %if.else21

if.then20:                                        ; preds = %if.else18
  store i32 8, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else21:                                        ; preds = %if.else18
  %cmp22 = icmp eq i32 %0, 8
  br i1 %cmp22, label %if.then23, label %if.else24

if.then23:                                        ; preds = %if.else21
  store i32 9, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else24:                                        ; preds = %if.else21
  %cmp25 = icmp eq i32 %0, 9
  br i1 %cmp25, label %if.then26, label %if.else27

if.then26:                                        ; preds = %if.else24
  store i32 10, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else27:                                        ; preds = %if.else24
  %cmp28 = icmp eq i32 %0, 10
  br i1 %cmp28, label %if.then29, label %if.else30

if.then29:                                        ; preds = %if.else27
  store i32 11, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else30:                                        ; preds = %if.else27
  %cmp31 = icmp eq i32 %0, 11
  br i1 %cmp31, label %if.then32, label %if.else33

if.then32:                                        ; preds = %if.else30
  store i32 12, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else33:                                        ; preds = %if.else30
  %cmp34 = icmp eq i32 %0, 12
  br i1 %cmp34, label %if.then35, label %if.else36

if.then35:                                        ; preds = %if.else33
  store i32 13, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else36:                                        ; preds = %if.else33
  %cmp37 = icmp eq i32 %0, 13
  br i1 %cmp37, label %if.then38, label %if.else39

if.then38:                                        ; preds = %if.else36
  store i32 14, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else39:                                        ; preds = %if.else36
  %cmp40 = icmp eq i32 %0, 14
  br i1 %cmp40, label %if.then41, label %if.else42

if.then41:                                        ; preds = %if.else39
  store i32 15, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else42:                                        ; preds = %if.else39
  %cmp43 = icmp eq i32 %0, 15
  br i1 %cmp43, label %if.then44, label %if.else45

if.then44:                                        ; preds = %if.else42
  store i32 16, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else45:                                        ; preds = %if.else42
  %cmp46 = icmp eq i32 %0, 16
  br i1 %cmp46, label %if.then47, label %if.else48

if.then47:                                        ; preds = %if.else45
  store i32 17, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else48:                                        ; preds = %if.else45
  %cmp49 = icmp eq i32 %0, 17
  br i1 %cmp49, label %if.then50, label %if.else51

if.then50:                                        ; preds = %if.else48
  store i32 18, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else51:                                        ; preds = %if.else48
  %cmp52 = icmp eq i32 %0, 18
  br i1 %cmp52, label %if.then53, label %if.else54

if.then53:                                        ; preds = %if.else51
  store i32 19, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else54:                                        ; preds = %if.else51
  %cmp55 = icmp eq i32 %0, 19
  br i1 %cmp55, label %if.then56, label %if.else57

if.then56:                                        ; preds = %if.else54
  store i32 20, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else57:                                        ; preds = %if.else54
  %cmp58 = icmp eq i32 %0, 20
  br i1 %cmp58, label %if.then59, label %if.else60

if.then59:                                        ; preds = %if.else57
  store i32 21, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else60:                                        ; preds = %if.else57
  %cmp61 = icmp eq i32 %0, 21
  br i1 %cmp61, label %if.then62, label %if.else63

if.then62:                                        ; preds = %if.else60
  store i32 22, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else63:                                        ; preds = %if.else60
  %cmp64 = icmp eq i32 %0, 22
  br i1 %cmp64, label %if.then65, label %if.else66

if.then65:                                        ; preds = %if.else63
  store i32 23, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else66:                                        ; preds = %if.else63
  %cmp67 = icmp eq i32 %0, 23
  br i1 %cmp67, label %if.then68, label %if.else69

if.then68:                                        ; preds = %if.else66
  store i32 24, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else69:                                        ; preds = %if.else66
  %cmp70 = icmp eq i32 %0, 24
  br i1 %cmp70, label %if.then71, label %if.else72

if.then71:                                        ; preds = %if.else69
  store i32 25, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.else72:                                        ; preds = %if.else69
  store i32 26, i32* @a1, align 4, !tbaa !2
  br label %if.end96

if.end96:                                         ; preds = %if.then2, %if.then8, %if.then14, %if.then20, %if.then26, %if.then32, %if.then38, %if.then44, %if.then50, %if.then56, %if.then62, %if.then68, %if.else72, %if.then71, %if.then65, %if.then59, %if.then53, %if.then47, %if.then41, %if.then35, %if.then29, %if.then23, %if.then17, %if.then11, %if.then5, %if.then
  %1 = load i32, i32* @a1, align 4, !tbaa !2
  %cmp97 = icmp eq i32 %1, 1
  br i1 %cmp97, label %if.then98, label %if.else99

if.then98:                                        ; preds = %if.end96
  store i32 1, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else99:                                        ; preds = %if.end96
  %cmp100 = icmp eq i32 %1, 2
  br i1 %cmp100, label %if.then101, label %if.else102

if.then101:                                       ; preds = %if.else99
  store i32 2, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else102:                                       ; preds = %if.else99
  %cmp103 = icmp eq i32 %1, 3
  br i1 %cmp103, label %if.then104, label %if.else105

if.then104:                                       ; preds = %if.else102
  store i32 3, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else105:                                       ; preds = %if.else102
  %cmp106 = icmp eq i32 %1, 4
  br i1 %cmp106, label %if.then107, label %if.else108

if.then107:                                       ; preds = %if.else105
  store i32 4, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else108:                                       ; preds = %if.else105
  %cmp109 = icmp eq i32 %1, 5
  br i1 %cmp109, label %if.then110, label %if.else111

if.then110:                                       ; preds = %if.else108
  store i32 5, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else111:                                       ; preds = %if.else108
  %cmp112 = icmp eq i32 %1, 6
  br i1 %cmp112, label %if.then113, label %if.else114

if.then113:                                       ; preds = %if.else111
  store i32 6, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else114:                                       ; preds = %if.else111
  %cmp115 = icmp eq i32 %1, 7
  br i1 %cmp115, label %if.then116, label %if.else117

if.then116:                                       ; preds = %if.else114
  store i32 7, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else117:                                       ; preds = %if.else114
  %cmp118 = icmp eq i32 %1, 8
  br i1 %cmp118, label %if.then119, label %if.else120

if.then119:                                       ; preds = %if.else117
  store i32 8, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else120:                                       ; preds = %if.else117
  %cmp121 = icmp eq i32 %1, 9
  br i1 %cmp121, label %if.then122, label %if.else123

if.then122:                                       ; preds = %if.else120
  store i32 9, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else123:                                       ; preds = %if.else120
  %cmp124 = icmp eq i32 %1, 10
  br i1 %cmp124, label %if.then125, label %if.else126

if.then125:                                       ; preds = %if.else123
  store i32 10, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else126:                                       ; preds = %if.else123
  %cmp127 = icmp eq i32 %1, 11
  br i1 %cmp127, label %if.then128, label %if.else129

if.then128:                                       ; preds = %if.else126
  store i32 11, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else129:                                       ; preds = %if.else126
  %cmp130 = icmp eq i32 %1, 12
  br i1 %cmp130, label %if.then131, label %if.else132

if.then131:                                       ; preds = %if.else129
  store i32 12, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else132:                                       ; preds = %if.else129
  %cmp133 = icmp eq i32 %1, 13
  br i1 %cmp133, label %if.then134, label %if.else135

if.then134:                                       ; preds = %if.else132
  store i32 13, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else135:                                       ; preds = %if.else132
  %cmp136 = icmp eq i32 %1, 14
  br i1 %cmp136, label %if.then137, label %if.else138

if.then137:                                       ; preds = %if.else135
  store i32 14, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else138:                                       ; preds = %if.else135
  %cmp139 = icmp eq i32 %1, 15
  br i1 %cmp139, label %if.then140, label %if.else141

if.then140:                                       ; preds = %if.else138
  store i32 15, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else141:                                       ; preds = %if.else138
  %cmp142 = icmp eq i32 %1, 16
  br i1 %cmp142, label %if.then143, label %if.else144

if.then143:                                       ; preds = %if.else141
  store i32 16, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else144:                                       ; preds = %if.else141
  %cmp145 = icmp eq i32 %1, 17
  br i1 %cmp145, label %if.then146, label %if.else147

if.then146:                                       ; preds = %if.else144
  store i32 17, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else147:                                       ; preds = %if.else144
  %cmp148 = icmp eq i32 %1, 18
  br i1 %cmp148, label %if.then149, label %if.else150

if.then149:                                       ; preds = %if.else147
  store i32 18, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else150:                                       ; preds = %if.else147
  %cmp151 = icmp eq i32 %1, 19
  br i1 %cmp151, label %if.then152, label %if.else153

if.then152:                                       ; preds = %if.else150
  store i32 19, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else153:                                       ; preds = %if.else150
  %cmp154 = icmp eq i32 %1, 20
  br i1 %cmp154, label %if.then155, label %if.else156

if.then155:                                       ; preds = %if.else153
  store i32 20, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else156:                                       ; preds = %if.else153
  %cmp157 = icmp eq i32 %1, 21
  br i1 %cmp157, label %if.then158, label %if.else159

if.then158:                                       ; preds = %if.else156
  store i32 21, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else159:                                       ; preds = %if.else156
  %cmp160 = icmp eq i32 %1, 22
  br i1 %cmp160, label %if.then161, label %if.else162

if.then161:                                       ; preds = %if.else159
  store i32 22, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else162:                                       ; preds = %if.else159
  %cmp163 = icmp eq i32 %1, 23
  br i1 %cmp163, label %if.then164, label %if.else165

if.then164:                                       ; preds = %if.else162
  store i32 23, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else165:                                       ; preds = %if.else162
  %cmp166 = icmp eq i32 %1, 24
  br i1 %cmp166, label %if.then167, label %if.else168

if.then167:                                       ; preds = %if.else165
  store i32 24, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.else168:                                       ; preds = %if.else165
  store i32 25, i32* @a0, align 4, !tbaa !2
  br label %if.end192

if.end192:                                        ; preds = %if.then101, %if.then107, %if.then113, %if.then119, %if.then125, %if.then131, %if.then137, %if.then143, %if.then149, %if.then155, %if.then161, %if.then167, %if.else168, %if.then164, %if.then158, %if.then152, %if.then146, %if.then140, %if.then134, %if.then128, %if.then122, %if.then116, %if.then110, %if.then104, %if.then98
  %2 = load i32, i32* @a0, align 4, !tbaa !2
  %3 = load i32, i32* @a1, align 4, !tbaa !2
  %add = add nsw i32 %2, %3
  ret i32 %add
}


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
