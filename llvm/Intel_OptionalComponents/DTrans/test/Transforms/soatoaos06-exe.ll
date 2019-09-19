; RUN: opt < %s -S -whole-program-assume -dtrans-soatoaos                                       \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                   \
; RUN:       | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -dtrans-soatoaos                                       \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                   \
; RUN:       | %lli
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-soatoaos                                \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                   \
; RUN:       | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-soatoaos                                \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false                   \
; RUN:       | %lli
; REQUIRES: x86_64-linux

; -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays,dtrans-soatoaos-struct

; Checks that transformation happens and executable test passes.
;    // icx -std=c++11 m5.cc -O0 -g -S -emit-llvm
;    // Attributess cleanup.
;    // Simplified:
;    //     opt -S -instnamer -mem2reg -functionattrs
; extern "C" {
; extern void *malloc(int) noexcept;
; extern void free(void *) noexcept;
; }
;
; template <typename S> struct Arr {
;   int capacilty;
;   S *base;
;   int size;
;   S &get(int i) {
;     if (capacilty > 1)
;       return base[5 * i];
;     return base[i];
;   }
;   void set(int i, S val) {
;     if (capacilty > 1)
;       base[5*i] = val;
;     else
;       base[i] = val;
;   }
;   Arr(int c = 1) : capacilty(c), size(0), base(nullptr) {
;     base = (S *)malloc(capacilty * sizeof(S));
;   }
;   void realloc(int inc) {
;     if (size + inc <= capacilty)
;       return;
;
;     capacilty = size + inc;
;     S *new_base = (S *)malloc(5 * capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i) {
;       new_base[5 * i] = base[i];
;     }
;     free(base);
;     base = new_base;
;   }
;   void add(const S &e) {
;     realloc(1);
;
;     if (capacilty > 1)
;       base[5 * size] = e;
;     else
;       base[size] = e;
;
;     ++size;
;   }
;   Arr(const Arr &A) {
;     capacilty = A.capacilty;
;     size = A.size;
;     if (capacilty > 1)
;       base = (S *)malloc(5 * capacilty * sizeof(S));
;     else
;       base = (S *)malloc(capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       if (capacilty > 1)
;         base[5 * i] = A.base[5 * i];
;       else
;         base[i] = A.base[i];
;   }
;   ~Arr() { free(base); }
; };
;
; class F {
; public:
;   Arr<int *> *f1;
;   Arr<float *> *f2;
;   F(const F &f) {
;     f1 = new Arr<int *>(*f.f1);
;     f2 = new Arr<float *>(*f.f2);
;   }
;   void put(int *a, float *b) {
;     f1->add(a);
;     f2->add(b);
;   }
;   void set1(int i, int *a) {
;     f1->set(i, a);
;   }
;   void set2(int i, float *b) {
;     f2->set(i, b);
;   }
;   int get1(int i) {
;     return *(f1->get(i));
;   }
;   float get2(int i) {
;     return *(f2->get(i));
;   }
;   F() {
;     f1 = new Arr<int *>();
;     f2 = new Arr<float *>();
;   }
;   ~F() {
;     delete f1;
;     delete f2;
;   }
; };
;
; int v1 = 20;
; int v2 = 30;
; float v3 = 3.5;
; float v4 = 7.5;
;
; bool check1(F *f) {
;   if (f->get1(0) != v2)
;     return false;
;   if (f->get2(0) != v4)
;     return false;
;   if (f->get1(1) != v2)
;     return false;
;   if (f->get2(1) != v4)
;     return false;
;   return true;
; }
;
; int main() {
;   F *f = new F();
;   f->put(&v1, &v3);
;   if (f->get1(0) != v1)
;     return -1;
;   if (f->get2(0) != v3)
;     return -1;
;   // force realloc
;   f->put(&v2, &v4);
;   if (f->get1(0) != v1)
;     return -1;
;   if (f->get2(0) != v3)
;     return -1;
;   if (f->get1(1) != v2)
;     return -1;
;   if (f->get2(1) != v4)
;     return -1;
;
;   f->set1(0, &v2);
;   f->set2(0, &v4);
;   if (!check1(f))
;     return -1;
;
;   F *f1 = new F(*f);
;   if (!check1(f1))
;     return -1;
;
;   delete f;
;   delete f1;
;   return 0;
; }
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], float**, i32, [4 x i8] }>
; CHECK-DAG: %__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
; CHECK-DAG: %__SOADT_EL_class.F = type { i32*, float* }

@v1 = global i32 20, align 4, !dbg !0
@v2 = global i32 30, align 4, !dbg !13
@v3 = global float 3.500000e+00, align 4, !dbg !15
@v4 = global float 7.500000e+00, align 4, !dbg !17

; Function Attrs: nounwind readonly
define zeroext i1 @"check1(F*)"(%class.F* nocapture readonly %f) #0 !dbg !24 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %f, metadata !124, metadata !DIExpression()), !dbg !125
  %call = call i32 @"F::get1(int)"(%class.F* %f, i32 0), !dbg !126
  %tmp1 = load i32, i32* @v2, align 4, !dbg !128
  %cmp = icmp ne i32 %call, %tmp1, !dbg !129
  br i1 %cmp, label %if.then, label %if.end, !dbg !130

if.then:                                          ; preds = %entry
  br label %return, !dbg !131

if.end:                                           ; preds = %entry
  %call1 = call float @"F::get2(int)"(%class.F* %f, i32 0), !dbg !132
  %tmp3 = load float, float* @v4, align 4, !dbg !134
  %cmp2 = fcmp une float %call1, %tmp3, !dbg !135
  br i1 %cmp2, label %if.then3, label %if.end4, !dbg !136

if.then3:                                         ; preds = %if.end
  br label %return, !dbg !137

if.end4:                                          ; preds = %if.end
  %call5 = call i32 @"F::get1(int)"(%class.F* %f, i32 1), !dbg !138
  %tmp5 = load i32, i32* @v2, align 4, !dbg !140
  %cmp6 = icmp ne i32 %call5, %tmp5, !dbg !141
  br i1 %cmp6, label %if.then7, label %if.end8, !dbg !142

if.then7:                                         ; preds = %if.end4
  br label %return, !dbg !143

if.end8:                                          ; preds = %if.end4
  %call9 = call float @"F::get2(int)"(%class.F* %f, i32 1), !dbg !144
  %tmp7 = load float, float* @v4, align 4, !dbg !146
  %cmp10 = fcmp une float %call9, %tmp7, !dbg !147
  br i1 %cmp10, label %if.then11, label %if.end12, !dbg !148

if.then11:                                        ; preds = %if.end8
  br label %return, !dbg !149

if.end12:                                         ; preds = %if.end8
  br label %return, !dbg !150

return:                                           ; preds = %if.end12, %if.then11, %if.then7, %if.then3, %if.then
  %retval.0 = phi i1 [ false, %if.then ], [ false, %if.then3 ], [ false, %if.then7 ], [ false, %if.then11 ], [ true, %if.end12 ], !dbg !151
  ret i1 %retval.0, !dbg !152
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readonly
define i32 @"F::get1(int)"(%class.F* nocapture readonly %this, i32 %i) #0 align 2 !dbg !153 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !154, metadata !DIExpression()), !dbg !155
  call void @llvm.dbg.value(metadata i32 %i, metadata !156, metadata !DIExpression()), !dbg !157
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !158
  %tmp = load %struct.Arr*, %struct.Arr** %f1, align 8, !dbg !158
  %call = call dereferenceable(8) i32** @"Arr<int*>::get(int)"(%struct.Arr* %tmp, i32 %i), !dbg !159
  %tmp2 = load i32*, i32** %call, align 8, !dbg !159
  %tmp3 = load i32, i32* %tmp2, align 4, !dbg !160
  ret i32 %tmp3, !dbg !161
}

; Function Attrs: nounwind readonly
define float @"F::get2(int)"(%class.F* nocapture readonly %this, i32 %i) #0 align 2 !dbg !162 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !163, metadata !DIExpression()), !dbg !164
  call void @llvm.dbg.value(metadata i32 %i, metadata !165, metadata !DIExpression()), !dbg !166
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !167
  %tmp = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8, !dbg !167
  %call = call dereferenceable(8) float** @"Arr<float*>::get(int)"(%struct.Arr.0* %tmp, i32 %i), !dbg !168
  %tmp2 = load float*, float** %call, align 8, !dbg !168
  %tmp3 = load float, float* %tmp2, align 4, !dbg !169
  ret float %tmp3, !dbg !170
}

define i32 @main() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !171 {
entry:
  %call = call i8* @_Znwm(i64 16), !dbg !174
  %tmp = bitcast i8* %call to %class.F*, !dbg !174
  invoke void @"F::F()"(%class.F* %tmp)
          to label %invoke.cont unwind label %lpad, !dbg !175

invoke.cont:                                      ; preds = %entry
  call void @llvm.dbg.value(metadata %class.F* %tmp, metadata !176, metadata !DIExpression()), !dbg !177
  call void @"F::put(int*, float*)"(%class.F* %tmp, i32* @v1, float* @v3), !dbg !178
  %call1 = call i32 @"F::get1(int)"(%class.F* %tmp, i32 0), !dbg !179
  %tmp3 = load i32, i32* @v1, align 4, !dbg !181
  %cmp = icmp ne i32 %call1, %tmp3, !dbg !182
  br i1 %cmp, label %if.then, label %if.end, !dbg !183

if.then:                                          ; preds = %invoke.cont
  br label %return, !dbg !184

lpad:                                             ; preds = %entry
  %tmp4 = landingpad { i8*, i32 }
          cleanup, !dbg !185
  %tmp5 = extractvalue { i8*, i32 } %tmp4, 0, !dbg !185
  %tmp6 = extractvalue { i8*, i32 } %tmp4, 1, !dbg !185
  call void @_ZdlPv(i8* %call), !dbg !174
  br label %eh.resume, !dbg !174

if.end:                                           ; preds = %invoke.cont
  %call2 = call float @"F::get2(int)"(%class.F* %tmp, i32 0), !dbg !186
  %tmp8 = load float, float* @v3, align 4, !dbg !188
  %cmp3 = fcmp une float %call2, %tmp8, !dbg !189
  br i1 %cmp3, label %if.then4, label %if.end5, !dbg !190

if.then4:                                         ; preds = %if.end
  br label %return, !dbg !191

if.end5:                                          ; preds = %if.end
  call void @"F::put(int*, float*)"(%class.F* %tmp, i32* @v2, float* @v4), !dbg !192
  %call6 = call i32 @"F::get1(int)"(%class.F* %tmp, i32 0), !dbg !193
  %tmp11 = load i32, i32* @v1, align 4, !dbg !195
  %cmp7 = icmp ne i32 %call6, %tmp11, !dbg !196
  br i1 %cmp7, label %if.then8, label %if.end9, !dbg !197

if.then8:                                         ; preds = %if.end5
  br label %return, !dbg !198

if.end9:                                          ; preds = %if.end5
  %call10 = call float @"F::get2(int)"(%class.F* %tmp, i32 0), !dbg !199
  %tmp13 = load float, float* @v3, align 4, !dbg !201
  %cmp11 = fcmp une float %call10, %tmp13, !dbg !202
  br i1 %cmp11, label %if.then12, label %if.end13, !dbg !203

if.then12:                                        ; preds = %if.end9
  br label %return, !dbg !204

if.end13:                                         ; preds = %if.end9
  %call14 = call i32 @"F::get1(int)"(%class.F* %tmp, i32 1), !dbg !205
  %tmp15 = load i32, i32* @v2, align 4, !dbg !207
  %cmp15 = icmp ne i32 %call14, %tmp15, !dbg !208
  br i1 %cmp15, label %if.then16, label %if.end17, !dbg !209

if.then16:                                        ; preds = %if.end13
  br label %return, !dbg !210

if.end17:                                         ; preds = %if.end13
  %call18 = call float @"F::get2(int)"(%class.F* %tmp, i32 1), !dbg !211
  %tmp17 = load float, float* @v4, align 4, !dbg !213
  %cmp19 = fcmp une float %call18, %tmp17, !dbg !214
  br i1 %cmp19, label %if.then20, label %if.end21, !dbg !215

if.then20:                                        ; preds = %if.end17
  br label %return, !dbg !216

if.end21:                                         ; preds = %if.end17
  call void @"F::set1(int, int*)"(%class.F* %tmp, i32 0, i32* @v2), !dbg !217
  call void @"F::set2(int, float*)"(%class.F* %tmp, i32 0, float* @v4), !dbg !218
  %call22 = call zeroext i1 @"check1(F*)"(%class.F* %tmp), !dbg !219
  br i1 %call22, label %if.end24, label %if.then23, !dbg !221

if.then23:                                        ; preds = %if.end21
  br label %return, !dbg !222

if.end24:                                         ; preds = %if.end21
  %call25 = call i8* @_Znwm(i64 16), !dbg !223
  %tmp21 = bitcast i8* %call25 to %class.F*, !dbg !223
  invoke void @"F::F(F const&)"(%class.F* %tmp21, %class.F* dereferenceable(16) %tmp)
          to label %invoke.cont27 unwind label %lpad26, !dbg !224

invoke.cont27:                                    ; preds = %if.end24
  call void @llvm.dbg.value(metadata %class.F* %tmp21, metadata !225, metadata !DIExpression()), !dbg !226
  %call28 = call zeroext i1 @"check1(F*)"(%class.F* %tmp21), !dbg !227
  br i1 %call28, label %if.end30, label %if.then29, !dbg !229

if.then29:                                        ; preds = %invoke.cont27
  br label %return, !dbg !230

lpad26:                                           ; preds = %if.end24
  %tmp24 = landingpad { i8*, i32 }
          cleanup, !dbg !185
  %tmp25 = extractvalue { i8*, i32 } %tmp24, 0, !dbg !185
  %tmp26 = extractvalue { i8*, i32 } %tmp24, 1, !dbg !185
  call void @_ZdlPv(i8* %call25), !dbg !223
  br label %eh.resume, !dbg !223

if.end30:                                         ; preds = %invoke.cont27
  %isnull = icmp eq %class.F* %tmp, null, !dbg !231
  br i1 %isnull, label %delete.end, label %delete.notnull, !dbg !231

delete.notnull:                                   ; preds = %if.end30
  call void @"F::~F()"(%class.F* %tmp), !dbg !231
  %tmp28 = bitcast %class.F* %tmp to i8*, !dbg !231
  call void @_ZdlPv(i8* %tmp28), !dbg !231
  br label %delete.end, !dbg !231

delete.end:                                       ; preds = %delete.notnull, %if.end30
  %isnull31 = icmp eq %class.F* %tmp21, null, !dbg !232
  br i1 %isnull31, label %delete.end33, label %delete.notnull32, !dbg !232

delete.notnull32:                                 ; preds = %delete.end
  call void @"F::~F()"(%class.F* %tmp21), !dbg !232
  %tmp30 = bitcast %class.F* %tmp21 to i8*, !dbg !232
  call void @_ZdlPv(i8* %tmp30), !dbg !232
  br label %delete.end33, !dbg !232

delete.end33:                                     ; preds = %delete.notnull32, %delete.end
  br label %return, !dbg !233

return:                                           ; preds = %delete.end33, %if.then29, %if.then23, %if.then20, %if.then16, %if.then12, %if.then8, %if.then4, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ -1, %if.then4 ], [ -1, %if.then8 ], [ -1, %if.then12 ], [ -1, %if.then16 ], [ -1, %if.then20 ], [ 0, %delete.end33 ], [ -1, %if.then29 ], [ -1, %if.then23 ], !dbg !234
  ret i32 %retval.0, !dbg !185

eh.resume:                                        ; preds = %lpad26, %lpad
  %exn.slot.0 = phi i8* [ %tmp25, %lpad26 ], [ %tmp5, %lpad ], !dbg !185
  %ehselector.slot.0 = phi i32 [ %tmp26, %lpad26 ], [ %tmp6, %lpad ], !dbg !185
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0, !dbg !174
  %lpad.val34 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1, !dbg !174
  resume { i8*, i32 } %lpad.val34, !dbg !174
}

declare noalias i8* @_Znwm(i64)

define void @"F::F()"(%class.F* nocapture %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !235 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !236, metadata !DIExpression()), !dbg !237
  %call = call i8* @_Znwm(i64 24), !dbg !238
  %tmp = bitcast i8* %call to %struct.Arr*, !dbg !238
  invoke void @"Arr<int*>::Arr(int)"(%struct.Arr* %tmp, i32 1)
          to label %invoke.cont unwind label %lpad, !dbg !240

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !241
  store %struct.Arr* %tmp, %struct.Arr** %f1, align 8, !dbg !242
  %call2 = call i8* @_Znwm(i64 24), !dbg !243
  %tmp1 = bitcast i8* %call2 to %struct.Arr.0*, !dbg !243
  invoke void @"Arr<float*>::Arr(int)"(%struct.Arr.0* %tmp1, i32 1)
          to label %invoke.cont4 unwind label %lpad3, !dbg !244

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !245
  store %struct.Arr.0* %tmp1, %struct.Arr.0** %f2, align 8, !dbg !246
  ret void, !dbg !247

lpad:                                             ; preds = %entry
  %tmp2 = landingpad { i8*, i32 }
          cleanup, !dbg !248
  %tmp3 = extractvalue { i8*, i32 } %tmp2, 0, !dbg !248
  %tmp4 = extractvalue { i8*, i32 } %tmp2, 1, !dbg !248
  call void @_ZdlPv(i8* %call), !dbg !238
  br label %eh.resume, !dbg !238

lpad3:                                            ; preds = %invoke.cont
  %tmp5 = landingpad { i8*, i32 }
          cleanup, !dbg !248
  %tmp6 = extractvalue { i8*, i32 } %tmp5, 0, !dbg !248
  %tmp7 = extractvalue { i8*, i32 } %tmp5, 1, !dbg !248
  call void @_ZdlPv(i8* %call2), !dbg !243
  br label %eh.resume, !dbg !243

eh.resume:                                        ; preds = %lpad3, %lpad
  %exn.slot.0 = phi i8* [ %tmp6, %lpad3 ], [ %tmp3, %lpad ], !dbg !248
  %ehselector.slot.0 = phi i32 [ %tmp7, %lpad3 ], [ %tmp4, %lpad ], !dbg !248
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0, !dbg !238
  %lpad.val5 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1, !dbg !238
  resume { i8*, i32 } %lpad.val5, !dbg !238
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*)

define void @"F::put(int*, float*)"(%class.F* nocapture readonly %this, i32* %a, float* %b) align 2 !dbg !249 {
entry:
  %a.addr = alloca i32*, align 8
  %b.addr = alloca float*, align 8
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !250, metadata !DIExpression()), !dbg !251
  store i32* %a, i32** %a.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %a.addr, metadata !252, metadata !DIExpression()), !dbg !253
  store float* %b, float** %b.addr, align 8
  call void @llvm.dbg.declare(metadata float** %b.addr, metadata !254, metadata !DIExpression()), !dbg !255
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !256
  %tmp = load %struct.Arr*, %struct.Arr** %f1, align 8, !dbg !256
  call void @"Arr<int*>::add(int* const&)"(%struct.Arr* %tmp, i32** dereferenceable(8) %a.addr), !dbg !257
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !258
  %tmp1 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8, !dbg !258
  call void @"Arr<float*>::add(float* const&)"(%struct.Arr.0* %tmp1, float** dereferenceable(8) %b.addr), !dbg !259
  ret void, !dbg !260
}

; Function Attrs: nounwind
define void @"F::set1(int, int*)"(%class.F* nocapture readonly %this, i32 %i, i32* %a) #2 align 2 !dbg !261 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !262, metadata !DIExpression()), !dbg !263
  call void @llvm.dbg.value(metadata i32 %i, metadata !264, metadata !DIExpression()), !dbg !265
  call void @llvm.dbg.value(metadata i32* %a, metadata !266, metadata !DIExpression()), !dbg !267
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !268
  %tmp = load %struct.Arr*, %struct.Arr** %f1, align 8, !dbg !268
  call void @"Arr<int*>::set(int, int*)"(%struct.Arr* %tmp, i32 %i, i32* %a), !dbg !269
  ret void, !dbg !270
}

; Function Attrs: nounwind
define void @"F::set2(int, float*)"(%class.F* nocapture readonly %this, i32 %i, float* %b) #2 align 2 !dbg !271 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !272, metadata !DIExpression()), !dbg !273
  call void @llvm.dbg.value(metadata i32 %i, metadata !274, metadata !DIExpression()), !dbg !275
  call void @llvm.dbg.value(metadata float* %b, metadata !276, metadata !DIExpression()), !dbg !277
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !278
  %tmp = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8, !dbg !278
  call void @"Arr<float*>::set(int, float*)"(%struct.Arr.0* %tmp, i32 %i, float* %b), !dbg !279
  ret void, !dbg !280
}

define void @"F::F(F const&)"(%class.F* nocapture %this, %class.F* nocapture readonly dereferenceable(16) %f) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !281 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !282, metadata !DIExpression()), !dbg !283
  call void @llvm.dbg.value(metadata %class.F* %f, metadata !284, metadata !DIExpression()), !dbg !285
  %call = call i8* @_Znwm(i64 24), !dbg !286
  %tmp = bitcast i8* %call to %struct.Arr*, !dbg !286
  %f1 = getelementptr inbounds %class.F, %class.F* %f, i32 0, i32 0, !dbg !288
  %tmp2 = load %struct.Arr*, %struct.Arr** %f1, align 8, !dbg !288
  invoke void @"Arr<int*>::Arr(Arr<int*> const&)"(%struct.Arr* %tmp, %struct.Arr* dereferenceable(24) %tmp2)
          to label %invoke.cont unwind label %lpad, !dbg !289

invoke.cont:                                      ; preds = %entry
  %f12 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !290
  store %struct.Arr* %tmp, %struct.Arr** %f12, align 8, !dbg !291
  %call3 = call i8* @_Znwm(i64 24), !dbg !292
  %tmp3 = bitcast i8* %call3 to %struct.Arr.0*, !dbg !292
  %f2 = getelementptr inbounds %class.F, %class.F* %f, i32 0, i32 1, !dbg !293
  %tmp5 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8, !dbg !293
  invoke void @"Arr<float*>::Arr(Arr<float*> const&)"(%struct.Arr.0* %tmp3, %struct.Arr.0* dereferenceable(24) %tmp5)
          to label %invoke.cont5 unwind label %lpad4, !dbg !294

invoke.cont5:                                     ; preds = %invoke.cont
  %f26 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !295
  store %struct.Arr.0* %tmp3, %struct.Arr.0** %f26, align 8, !dbg !296
  ret void, !dbg !297

lpad:                                             ; preds = %entry
  %tmp6 = landingpad { i8*, i32 }
          cleanup, !dbg !298
  %tmp7 = extractvalue { i8*, i32 } %tmp6, 0, !dbg !298
  %tmp8 = extractvalue { i8*, i32 } %tmp6, 1, !dbg !298
  call void @_ZdlPv(i8* %call), !dbg !286
  br label %eh.resume, !dbg !286

lpad4:                                            ; preds = %invoke.cont
  %tmp9 = landingpad { i8*, i32 }
          cleanup, !dbg !298
  %tmp10 = extractvalue { i8*, i32 } %tmp9, 0, !dbg !298
  %tmp11 = extractvalue { i8*, i32 } %tmp9, 1, !dbg !298
  call void @_ZdlPv(i8* %call3), !dbg !292
  br label %eh.resume, !dbg !292

eh.resume:                                        ; preds = %lpad4, %lpad
  %exn.slot.0 = phi i8* [ %tmp10, %lpad4 ], [ %tmp7, %lpad ], !dbg !298
  %ehselector.slot.0 = phi i32 [ %tmp11, %lpad4 ], [ %tmp8, %lpad ], !dbg !298
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0, !dbg !286
  %lpad.val7 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1, !dbg !286
  resume { i8*, i32 } %lpad.val7, !dbg !286
}

define void @"F::~F()"(%class.F* nocapture readonly %this) align 2 !dbg !299 {
entry:
  call void @llvm.dbg.value(metadata %class.F* %this, metadata !300, metadata !DIExpression()), !dbg !301
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 0, !dbg !302
  %tmp = load %struct.Arr*, %struct.Arr** %f1, align 8, !dbg !302
  %isnull = icmp eq %struct.Arr* %tmp, null, !dbg !304
  br i1 %isnull, label %delete.end, label %delete.notnull, !dbg !304

delete.notnull:                                   ; preds = %entry
  call void @"Arr<int*>::~Arr()"(%struct.Arr* %tmp), !dbg !304
  %tmp1 = bitcast %struct.Arr* %tmp to i8*, !dbg !304
  call void @_ZdlPv(i8* %tmp1), !dbg !304
  br label %delete.end, !dbg !304

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1, !dbg !305
  %tmp2 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8, !dbg !305
  %isnull2 = icmp eq %struct.Arr.0* %tmp2, null, !dbg !306
  br i1 %isnull2, label %delete.end4, label %delete.notnull3, !dbg !306

delete.notnull3:                                  ; preds = %delete.end
  call void @"Arr<float*>::~Arr()"(%struct.Arr.0* %tmp2), !dbg !306
  %tmp3 = bitcast %struct.Arr.0* %tmp2 to i8*, !dbg !306
  call void @_ZdlPv(i8* %tmp3), !dbg !306
  br label %delete.end4, !dbg !306

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void, !dbg !307
}

; Function Attrs: nounwind readonly
define dereferenceable(8) i32** @"Arr<int*>::get(int)"(%struct.Arr* nocapture readonly %this, i32 %i) #0 align 2 !dbg !308 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !309, metadata !DIExpression()), !dbg !310
  call void @llvm.dbg.value(metadata i32 %i, metadata !311, metadata !DIExpression()), !dbg !312
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !313
  %tmp = load i32, i32* %capacilty, align 8, !dbg !313
  %cmp = icmp sgt i32 %tmp, 1, !dbg !315
  br i1 %cmp, label %if.then, label %if.end, !dbg !316

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !317
  %tmp1 = load i32**, i32*** %base, align 8, !dbg !317
  %mul = mul nsw i32 5, %i, !dbg !318
  %idxprom = sext i32 %mul to i64, !dbg !317
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom, !dbg !317
  br label %return, !dbg !319

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !320
  %tmp3 = load i32**, i32*** %base2, align 8, !dbg !320
  %idxprom3 = sext i32 %i to i64, !dbg !320
  %arrayidx4 = getelementptr inbounds i32*, i32** %tmp3, i64 %idxprom3, !dbg !320
  br label %return, !dbg !321

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32** [ %arrayidx, %if.then ], [ %arrayidx4, %if.end ], !dbg !310
  ret i32** %retval.0, !dbg !322
}

; Function Attrs: nounwind readonly
define dereferenceable(8) float** @"Arr<float*>::get(int)"(%struct.Arr.0* nocapture readonly %this, i32 %i) #0 align 2 !dbg !323 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !324, metadata !DIExpression()), !dbg !325
  call void @llvm.dbg.value(metadata i32 %i, metadata !326, metadata !DIExpression()), !dbg !327
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !328
  %tmp = load i32, i32* %capacilty, align 8, !dbg !328
  %cmp = icmp sgt i32 %tmp, 1, !dbg !330
  br i1 %cmp, label %if.then, label %if.end, !dbg !331

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !332
  %tmp1 = load float**, float*** %base, align 8, !dbg !332
  %mul = mul nsw i32 5, %i, !dbg !333
  %idxprom = sext i32 %mul to i64, !dbg !332
  %arrayidx = getelementptr inbounds float*, float** %tmp1, i64 %idxprom, !dbg !332
  br label %return, !dbg !334

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !335
  %tmp3 = load float**, float*** %base2, align 8, !dbg !335
  %idxprom3 = sext i32 %i to i64, !dbg !335
  %arrayidx4 = getelementptr inbounds float*, float** %tmp3, i64 %idxprom3, !dbg !335
  br label %return, !dbg !336

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi float** [ %arrayidx, %if.then ], [ %arrayidx4, %if.end ], !dbg !325
  ret float** %retval.0, !dbg !337
}

define void @"Arr<int*>::Arr(int)"(%struct.Arr* nocapture %this, i32 %c) align 2 !dbg !338 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !339, metadata !DIExpression()), !dbg !340
  call void @llvm.dbg.value(metadata i32 %c, metadata !341, metadata !DIExpression()), !dbg !342
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !343
  store i32 %c, i32* %capacilty, align 8, !dbg !343
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !344
  store i32** null, i32*** %base, align 8, !dbg !344
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !345
  store i32 0, i32* %size, align 8, !dbg !345
  %capacilty2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !346
  %tmp1 = load i32, i32* %capacilty2, align 8, !dbg !346
  %conv = sext i32 %tmp1 to i64, !dbg !346
  %mul = mul i64 %conv, 8, !dbg !348
  %conv3 = trunc i64 %mul to i32, !dbg !346
  %call = call i8* @malloc(i32 %conv3), !dbg !349
  %tmp2 = bitcast i8* %call to i32**, !dbg !350
  %base4 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !351
  store i32** %tmp2, i32*** %base4, align 8, !dbg !352
  ret void, !dbg !353
}

define void @"Arr<float*>::Arr(int)"(%struct.Arr.0* nocapture %this, i32 %c) align 2 !dbg !354 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !355, metadata !DIExpression()), !dbg !356
  call void @llvm.dbg.value(metadata i32 %c, metadata !357, metadata !DIExpression()), !dbg !358
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !359
  store i32 %c, i32* %capacilty, align 8, !dbg !359
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !360
  store float** null, float*** %base, align 8, !dbg !360
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !361
  store i32 0, i32* %size, align 8, !dbg !361
  %capacilty2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !362
  %tmp1 = load i32, i32* %capacilty2, align 8, !dbg !362
  %conv = sext i32 %tmp1 to i64, !dbg !362
  %mul = mul i64 %conv, 8, !dbg !364
  %conv3 = trunc i64 %mul to i32, !dbg !362
  %call = call i8* @malloc(i32 %conv3), !dbg !365
  %tmp2 = bitcast i8* %call to float**, !dbg !366
  %base4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !367
  store float** %tmp2, float*** %base4, align 8, !dbg !368
  ret void, !dbg !369
}

declare i8* @malloc(i32)

define void @"Arr<int*>::add(int* const&)"(%struct.Arr* nocapture %this, i32** nocapture readonly dereferenceable(8) %e) align 2 !dbg !370 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !371, metadata !DIExpression()), !dbg !372
  call void @llvm.dbg.value(metadata i32** %e, metadata !373, metadata !DIExpression()), !dbg !374
  call void @"Arr<int*>::realloc(int)"(%struct.Arr* %this, i32 1), !dbg !375
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !376
  %tmp = load i32, i32* %capacilty, align 8, !dbg !376
  %cmp = icmp sgt i32 %tmp, 1, !dbg !378
  br i1 %cmp, label %if.then, label %if.else, !dbg !379

if.then:                                          ; preds = %entry
  %tmp2 = load i32*, i32** %e, align 8, !dbg !380
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !381
  %tmp3 = load i32**, i32*** %base, align 8, !dbg !381
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !382
  %tmp4 = load i32, i32* %size, align 8, !dbg !382
  %mul = mul nsw i32 5, %tmp4, !dbg !383
  %idxprom = sext i32 %mul to i64, !dbg !381
  %arrayidx = getelementptr inbounds i32*, i32** %tmp3, i64 %idxprom, !dbg !381
  store i32* %tmp2, i32** %arrayidx, align 8, !dbg !384
  br label %if.end, !dbg !381

if.else:                                          ; preds = %entry
  %tmp6 = load i32*, i32** %e, align 8, !dbg !385
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !386
  %tmp7 = load i32**, i32*** %base2, align 8, !dbg !386
  %size3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !387
  %tmp8 = load i32, i32* %size3, align 8, !dbg !387
  %idxprom4 = sext i32 %tmp8 to i64, !dbg !386
  %arrayidx5 = getelementptr inbounds i32*, i32** %tmp7, i64 %idxprom4, !dbg !386
  store i32* %tmp6, i32** %arrayidx5, align 8, !dbg !388
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size6 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !389
  %tmp9 = load i32, i32* %size6, align 8, !dbg !390
  %inc = add nsw i32 %tmp9, 1, !dbg !390
  store i32 %inc, i32* %size6, align 8, !dbg !390
  ret void, !dbg !391
}

define void @"Arr<float*>::add(float* const&)"(%struct.Arr.0* nocapture %this, float** nocapture readonly dereferenceable(8) %e) align 2 !dbg !392 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !393, metadata !DIExpression()), !dbg !394
  call void @llvm.dbg.value(metadata float** %e, metadata !395, metadata !DIExpression()), !dbg !396
  call void @"Arr<float*>::realloc(int)"(%struct.Arr.0* %this, i32 1), !dbg !397
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !398
  %tmp = load i32, i32* %capacilty, align 8, !dbg !398
  %cmp = icmp sgt i32 %tmp, 1, !dbg !400
  br i1 %cmp, label %if.then, label %if.else, !dbg !401

if.then:                                          ; preds = %entry
  %tmp2 = load float*, float** %e, align 8, !dbg !402
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !403
  %tmp3 = load float**, float*** %base, align 8, !dbg !403
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !404
  %tmp4 = load i32, i32* %size, align 8, !dbg !404
  %mul = mul nsw i32 5, %tmp4, !dbg !405
  %idxprom = sext i32 %mul to i64, !dbg !403
  %arrayidx = getelementptr inbounds float*, float** %tmp3, i64 %idxprom, !dbg !403
  store float* %tmp2, float** %arrayidx, align 8, !dbg !406
  br label %if.end, !dbg !403

if.else:                                          ; preds = %entry
  %tmp6 = load float*, float** %e, align 8, !dbg !407
  %base2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !408
  %tmp7 = load float**, float*** %base2, align 8, !dbg !408
  %size3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !409
  %tmp8 = load i32, i32* %size3, align 8, !dbg !409
  %idxprom4 = sext i32 %tmp8 to i64, !dbg !408
  %arrayidx5 = getelementptr inbounds float*, float** %tmp7, i64 %idxprom4, !dbg !408
  store float* %tmp6, float** %arrayidx5, align 8, !dbg !410
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !411
  %tmp9 = load i32, i32* %size6, align 8, !dbg !412
  %inc = add nsw i32 %tmp9, 1, !dbg !412
  store i32 %inc, i32* %size6, align 8, !dbg !412
  ret void, !dbg !413
}

define void @"Arr<int*>::realloc(int)"(%struct.Arr* nocapture %this, i32 %inc) align 2 !dbg !414 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !415, metadata !DIExpression()), !dbg !416
  call void @llvm.dbg.value(metadata i32 %inc, metadata !417, metadata !DIExpression()), !dbg !418
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !419
  %tmp = load i32, i32* %size, align 8, !dbg !419
  %add = add nsw i32 %tmp, %inc, !dbg !421
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !422
  %tmp2 = load i32, i32* %capacilty, align 8, !dbg !422
  %cmp = icmp sle i32 %add, %tmp2, !dbg !423
  br i1 %cmp, label %if.then, label %if.end, !dbg !424

if.then:                                          ; preds = %entry
  br label %return, !dbg !425

if.end:                                           ; preds = %entry
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !426
  %tmp3 = load i32, i32* %size2, align 8, !dbg !426
  %add3 = add nsw i32 %tmp3, %inc, !dbg !427
  %capacilty4 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !428
  store i32 %add3, i32* %capacilty4, align 8, !dbg !429
  %capacilty5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !430
  %tmp5 = load i32, i32* %capacilty5, align 8, !dbg !430
  %mul = mul nsw i32 5, %tmp5, !dbg !431
  %conv = sext i32 %mul to i64, !dbg !432
  %mul6 = mul i64 %conv, 8, !dbg !433
  %conv7 = trunc i64 %mul6 to i32, !dbg !432
  %call = call i8* @malloc(i32 %conv7), !dbg !434
  %tmp6 = bitcast i8* %call to i32**, !dbg !435
  call void @llvm.dbg.value(metadata i32** %tmp6, metadata !436, metadata !DIExpression()), !dbg !437
  call void @llvm.dbg.value(metadata i32 0, metadata !438, metadata !DIExpression()), !dbg !440
  br label %for.cond, !dbg !441

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc13, %for.inc ], !dbg !442
  call void @llvm.dbg.value(metadata i32 %i.0, metadata !438, metadata !DIExpression()), !dbg !440
  %size8 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !443
  %tmp8 = load i32, i32* %size8, align 8, !dbg !443
  %cmp9 = icmp slt i32 %i.0, %tmp8, !dbg !445
  br i1 %cmp9, label %for.body, label %for.end, !dbg !446

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !447
  %tmp9 = load i32**, i32*** %base, align 8, !dbg !447
  %idxprom = sext i32 %i.0 to i64, !dbg !447
  %arrayidx = getelementptr inbounds i32*, i32** %tmp9, i64 %idxprom, !dbg !447
  %tmp11 = load i32*, i32** %arrayidx, align 8, !dbg !447
  %mul10 = mul nsw i32 5, %i.0, !dbg !449
  %idxprom11 = sext i32 %mul10 to i64, !dbg !450
  %arrayidx12 = getelementptr inbounds i32*, i32** %tmp6, i64 %idxprom11, !dbg !450
  store i32* %tmp11, i32** %arrayidx12, align 8, !dbg !451
  br label %for.inc, !dbg !452

for.inc:                                          ; preds = %for.body
  %inc13 = add nsw i32 %i.0, 1, !dbg !453
  call void @llvm.dbg.value(metadata i32 %inc13, metadata !438, metadata !DIExpression()), !dbg !440
  br label %for.cond, !dbg !454, !llvm.loop !455

for.end:                                          ; preds = %for.cond
  %base14 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !457
  %tmp15 = load i32**, i32*** %base14, align 8, !dbg !457
  %tmp16 = bitcast i32** %tmp15 to i8*, !dbg !457
  call void @free(i8* %tmp16), !dbg !458
  %base15 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !459
  store i32** %tmp6, i32*** %base15, align 8, !dbg !460
  br label %return, !dbg !461

return:                                           ; preds = %for.end, %if.then
  ret void, !dbg !461
}

declare void @free(i8*)

define void @"Arr<float*>::realloc(int)"(%struct.Arr.0* nocapture %this, i32 %inc) align 2 !dbg !462 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !463, metadata !DIExpression()), !dbg !464
  call void @llvm.dbg.value(metadata i32 %inc, metadata !465, metadata !DIExpression()), !dbg !466
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !467
  %tmp = load i32, i32* %size, align 8, !dbg !467
  %add = add nsw i32 %tmp, %inc, !dbg !469
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !470
  %tmp2 = load i32, i32* %capacilty, align 8, !dbg !470
  %cmp = icmp sle i32 %add, %tmp2, !dbg !471
  br i1 %cmp, label %if.then, label %if.end, !dbg !472

if.then:                                          ; preds = %entry
  br label %return, !dbg !473

if.end:                                           ; preds = %entry
  %size2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !474
  %tmp3 = load i32, i32* %size2, align 8, !dbg !474
  %add3 = add nsw i32 %tmp3, %inc, !dbg !475
  %capacilty4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !476
  store i32 %add3, i32* %capacilty4, align 8, !dbg !477
  %capacilty5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !478
  %tmp5 = load i32, i32* %capacilty5, align 8, !dbg !478
  %mul = mul nsw i32 5, %tmp5, !dbg !479
  %conv = sext i32 %mul to i64, !dbg !480
  %mul6 = mul i64 %conv, 8, !dbg !481
  %conv7 = trunc i64 %mul6 to i32, !dbg !480
  %call = call i8* @malloc(i32 %conv7), !dbg !482
  %tmp6 = bitcast i8* %call to float**, !dbg !483
  call void @llvm.dbg.value(metadata float** %tmp6, metadata !484, metadata !DIExpression()), !dbg !485
  call void @llvm.dbg.value(metadata i32 0, metadata !486, metadata !DIExpression()), !dbg !488
  br label %for.cond, !dbg !489

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc13, %for.inc ], !dbg !490
  call void @llvm.dbg.value(metadata i32 %i.0, metadata !486, metadata !DIExpression()), !dbg !488
  %size8 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !491
  %tmp8 = load i32, i32* %size8, align 8, !dbg !491
  %cmp9 = icmp slt i32 %i.0, %tmp8, !dbg !493
  br i1 %cmp9, label %for.body, label %for.end, !dbg !494

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !495
  %tmp9 = load float**, float*** %base, align 8, !dbg !495
  %idxprom = sext i32 %i.0 to i64, !dbg !495
  %arrayidx = getelementptr inbounds float*, float** %tmp9, i64 %idxprom, !dbg !495
  %tmp11 = load float*, float** %arrayidx, align 8, !dbg !495
  %mul10 = mul nsw i32 5, %i.0, !dbg !497
  %idxprom11 = sext i32 %mul10 to i64, !dbg !498
  %arrayidx12 = getelementptr inbounds float*, float** %tmp6, i64 %idxprom11, !dbg !498
  store float* %tmp11, float** %arrayidx12, align 8, !dbg !499
  br label %for.inc, !dbg !500

for.inc:                                          ; preds = %for.body
  %inc13 = add nsw i32 %i.0, 1, !dbg !501
  call void @llvm.dbg.value(metadata i32 %inc13, metadata !486, metadata !DIExpression()), !dbg !488
  br label %for.cond, !dbg !502, !llvm.loop !503

for.end:                                          ; preds = %for.cond
  %base14 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !505
  %tmp15 = load float**, float*** %base14, align 8, !dbg !505
  %tmp16 = bitcast float** %tmp15 to i8*, !dbg !505
  call void @free(i8* %tmp16), !dbg !506
  %base15 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !507
  store float** %tmp6, float*** %base15, align 8, !dbg !508
  br label %return, !dbg !509

return:                                           ; preds = %for.end, %if.then
  ret void, !dbg !509
}

; Function Attrs: nounwind
define void @"Arr<int*>::set(int, int*)"(%struct.Arr* nocapture readonly %this, i32 %i, i32* %val) #2 align 2 !dbg !510 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !511, metadata !DIExpression()), !dbg !512
  call void @llvm.dbg.value(metadata i32 %i, metadata !513, metadata !DIExpression()), !dbg !514
  call void @llvm.dbg.value(metadata i32* %val, metadata !515, metadata !DIExpression()), !dbg !516
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !517
  %tmp = load i32, i32* %capacilty, align 8, !dbg !517
  %cmp = icmp sgt i32 %tmp, 1, !dbg !519
  br i1 %cmp, label %if.then, label %if.else, !dbg !520

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !521
  %tmp2 = load i32**, i32*** %base, align 8, !dbg !521
  %mul = mul nsw i32 5, %i, !dbg !522
  %idxprom = sext i32 %mul to i64, !dbg !521
  %arrayidx = getelementptr inbounds i32*, i32** %tmp2, i64 %idxprom, !dbg !521
  store i32* %val, i32** %arrayidx, align 8, !dbg !523
  br label %if.end, !dbg !521

if.else:                                          ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !524
  %tmp5 = load i32**, i32*** %base2, align 8, !dbg !524
  %idxprom3 = sext i32 %i to i64, !dbg !524
  %arrayidx4 = getelementptr inbounds i32*, i32** %tmp5, i64 %idxprom3, !dbg !524
  store i32* %val, i32** %arrayidx4, align 8, !dbg !525
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void, !dbg !526
}

; Function Attrs: nounwind
define void @"Arr<float*>::set(int, float*)"(%struct.Arr.0* nocapture readonly %this, i32 %i, float* %val) #2 align 2 !dbg !527 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !528, metadata !DIExpression()), !dbg !529
  call void @llvm.dbg.value(metadata i32 %i, metadata !530, metadata !DIExpression()), !dbg !531
  call void @llvm.dbg.value(metadata float* %val, metadata !532, metadata !DIExpression()), !dbg !533
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !534
  %tmp = load i32, i32* %capacilty, align 8, !dbg !534
  %cmp = icmp sgt i32 %tmp, 1, !dbg !536
  br i1 %cmp, label %if.then, label %if.else, !dbg !537

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !538
  %tmp2 = load float**, float*** %base, align 8, !dbg !538
  %mul = mul nsw i32 5, %i, !dbg !539
  %idxprom = sext i32 %mul to i64, !dbg !538
  %arrayidx = getelementptr inbounds float*, float** %tmp2, i64 %idxprom, !dbg !538
  store float* %val, float** %arrayidx, align 8, !dbg !540
  br label %if.end, !dbg !538

if.else:                                          ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !541
  %tmp5 = load float**, float*** %base2, align 8, !dbg !541
  %idxprom3 = sext i32 %i to i64, !dbg !541
  %arrayidx4 = getelementptr inbounds float*, float** %tmp5, i64 %idxprom3, !dbg !541
  store float* %val, float** %arrayidx4, align 8, !dbg !542
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void, !dbg !543
}

define void @"Arr<int*>::Arr(Arr<int*> const&)"(%struct.Arr* nocapture %this, %struct.Arr* nocapture readonly dereferenceable(24) %A) align 2 !dbg !544 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !545, metadata !DIExpression()), !dbg !546
  call void @llvm.dbg.value(metadata %struct.Arr* %A, metadata !547, metadata !DIExpression()), !dbg !548
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 0, !dbg !549
  %tmp1 = load i32, i32* %capacilty, align 8, !dbg !549
  %capacilty2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !551
  store i32 %tmp1, i32* %capacilty2, align 8, !dbg !552
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 3, !dbg !553
  %tmp3 = load i32, i32* %size, align 8, !dbg !553
  %size3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !554
  store i32 %tmp3, i32* %size3, align 8, !dbg !555
  %capacilty4 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !556
  %tmp4 = load i32, i32* %capacilty4, align 8, !dbg !556
  %cmp = icmp sgt i32 %tmp4, 1, !dbg !558
  br i1 %cmp, label %if.then, label %if.else, !dbg !559

if.then:                                          ; preds = %entry
  %capacilty5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !560
  %tmp5 = load i32, i32* %capacilty5, align 8, !dbg !560
  %mul = mul nsw i32 5, %tmp5, !dbg !561
  %conv = sext i32 %mul to i64, !dbg !562
  %mul6 = mul i64 %conv, 8, !dbg !563
  %conv7 = trunc i64 %mul6 to i32, !dbg !562
  %call = call i8* @malloc(i32 %conv7), !dbg !564
  %tmp6 = bitcast i8* %call to i32**, !dbg !565
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !566
  store i32** %tmp6, i32*** %base, align 8, !dbg !567
  br label %if.end, !dbg !566

if.else:                                          ; preds = %entry
  %capacilty8 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !568
  %tmp7 = load i32, i32* %capacilty8, align 8, !dbg !568
  %conv9 = sext i32 %tmp7 to i64, !dbg !568
  %mul10 = mul i64 %conv9, 8, !dbg !569
  %conv11 = trunc i64 %mul10 to i32, !dbg !568
  %call12 = call i8* @malloc(i32 %conv11), !dbg !570
  %tmp8 = bitcast i8* %call12 to i32**, !dbg !571
  %base13 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !572
  store i32** %tmp8, i32*** %base13, align 8, !dbg !573
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.dbg.value(metadata i32 0, metadata !574, metadata !DIExpression()), !dbg !576
  br label %for.cond, !dbg !577

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc, %for.inc ], !dbg !578
  call void @llvm.dbg.value(metadata i32 %i.0, metadata !574, metadata !DIExpression()), !dbg !576
  %size14 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3, !dbg !579
  %tmp10 = load i32, i32* %size14, align 8, !dbg !579
  %cmp15 = icmp slt i32 %i.0, %tmp10, !dbg !581
  br i1 %cmp15, label %for.body, label %for.end, !dbg !582

for.body:                                         ; preds = %for.cond
  %capacilty16 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0, !dbg !583
  %tmp11 = load i32, i32* %capacilty16, align 8, !dbg !583
  %cmp17 = icmp sgt i32 %tmp11, 1, !dbg !585
  br i1 %cmp17, label %if.then18, label %if.else25, !dbg !586

if.then18:                                        ; preds = %for.body
  %base19 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 2, !dbg !587
  %tmp13 = load i32**, i32*** %base19, align 8, !dbg !587
  %mul20 = mul nsw i32 5, %i.0, !dbg !588
  %idxprom = sext i32 %mul20 to i64, !dbg !589
  %arrayidx = getelementptr inbounds i32*, i32** %tmp13, i64 %idxprom, !dbg !589
  %tmp15 = load i32*, i32** %arrayidx, align 8, !dbg !589
  %base21 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !590
  %tmp16 = load i32**, i32*** %base21, align 8, !dbg !590
  %mul22 = mul nsw i32 5, %i.0, !dbg !591
  %idxprom23 = sext i32 %mul22 to i64, !dbg !590
  %arrayidx24 = getelementptr inbounds i32*, i32** %tmp16, i64 %idxprom23, !dbg !590
  store i32* %tmp15, i32** %arrayidx24, align 8, !dbg !592
  br label %if.end32, !dbg !590

if.else25:                                        ; preds = %for.body
  %base26 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 2, !dbg !593
  %tmp19 = load i32**, i32*** %base26, align 8, !dbg !593
  %idxprom27 = sext i32 %i.0 to i64, !dbg !594
  %arrayidx28 = getelementptr inbounds i32*, i32** %tmp19, i64 %idxprom27, !dbg !594
  %tmp21 = load i32*, i32** %arrayidx28, align 8, !dbg !594
  %base29 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !595
  %tmp22 = load i32**, i32*** %base29, align 8, !dbg !595
  %idxprom30 = sext i32 %i.0 to i64, !dbg !595
  %arrayidx31 = getelementptr inbounds i32*, i32** %tmp22, i64 %idxprom30, !dbg !595
  store i32* %tmp21, i32** %arrayidx31, align 8, !dbg !596
  br label %if.end32

if.end32:                                         ; preds = %if.else25, %if.then18
  br label %for.inc, !dbg !597

for.inc:                                          ; preds = %if.end32
  %inc = add nsw i32 %i.0, 1, !dbg !598
  call void @llvm.dbg.value(metadata i32 %inc, metadata !574, metadata !DIExpression()), !dbg !576
  br label %for.cond, !dbg !599, !llvm.loop !600

for.end:                                          ; preds = %for.cond
  ret void, !dbg !602
}

define void @"Arr<float*>::Arr(Arr<float*> const&)"(%struct.Arr.0* nocapture %this, %struct.Arr.0* nocapture readonly dereferenceable(24) %A) align 2 !dbg !603 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !604, metadata !DIExpression()), !dbg !605
  call void @llvm.dbg.value(metadata %struct.Arr.0* %A, metadata !606, metadata !DIExpression()), !dbg !607
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 0, !dbg !608
  %tmp1 = load i32, i32* %capacilty, align 8, !dbg !608
  %capacilty2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !610
  store i32 %tmp1, i32* %capacilty2, align 8, !dbg !611
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 3, !dbg !612
  %tmp3 = load i32, i32* %size, align 8, !dbg !612
  %size3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !613
  store i32 %tmp3, i32* %size3, align 8, !dbg !614
  %capacilty4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !615
  %tmp4 = load i32, i32* %capacilty4, align 8, !dbg !615
  %cmp = icmp sgt i32 %tmp4, 1, !dbg !617
  br i1 %cmp, label %if.then, label %if.else, !dbg !618

if.then:                                          ; preds = %entry
  %capacilty5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !619
  %tmp5 = load i32, i32* %capacilty5, align 8, !dbg !619
  %mul = mul nsw i32 5, %tmp5, !dbg !620
  %conv = sext i32 %mul to i64, !dbg !621
  %mul6 = mul i64 %conv, 8, !dbg !622
  %conv7 = trunc i64 %mul6 to i32, !dbg !621
  %call = call i8* @malloc(i32 %conv7), !dbg !623
  %tmp6 = bitcast i8* %call to float**, !dbg !624
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !625
  store float** %tmp6, float*** %base, align 8, !dbg !626
  br label %if.end, !dbg !625

if.else:                                          ; preds = %entry
  %capacilty8 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !627
  %tmp7 = load i32, i32* %capacilty8, align 8, !dbg !627
  %conv9 = sext i32 %tmp7 to i64, !dbg !627
  %mul10 = mul i64 %conv9, 8, !dbg !628
  %conv11 = trunc i64 %mul10 to i32, !dbg !627
  %call12 = call i8* @malloc(i32 %conv11), !dbg !629
  %tmp8 = bitcast i8* %call12 to float**, !dbg !630
  %base13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !631
  store float** %tmp8, float*** %base13, align 8, !dbg !632
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.dbg.value(metadata i32 0, metadata !633, metadata !DIExpression()), !dbg !635
  br label %for.cond, !dbg !636

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc, %for.inc ], !dbg !637
  call void @llvm.dbg.value(metadata i32 %i.0, metadata !633, metadata !DIExpression()), !dbg !635
  %size14 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3, !dbg !638
  %tmp10 = load i32, i32* %size14, align 8, !dbg !638
  %cmp15 = icmp slt i32 %i.0, %tmp10, !dbg !640
  br i1 %cmp15, label %for.body, label %for.end, !dbg !641

for.body:                                         ; preds = %for.cond
  %capacilty16 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0, !dbg !642
  %tmp11 = load i32, i32* %capacilty16, align 8, !dbg !642
  %cmp17 = icmp sgt i32 %tmp11, 1, !dbg !644
  br i1 %cmp17, label %if.then18, label %if.else25, !dbg !645

if.then18:                                        ; preds = %for.body
  %base19 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 2, !dbg !646
  %tmp13 = load float**, float*** %base19, align 8, !dbg !646
  %mul20 = mul nsw i32 5, %i.0, !dbg !647
  %idxprom = sext i32 %mul20 to i64, !dbg !648
  %arrayidx = getelementptr inbounds float*, float** %tmp13, i64 %idxprom, !dbg !648
  %tmp15 = load float*, float** %arrayidx, align 8, !dbg !648
  %base21 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !649
  %tmp16 = load float**, float*** %base21, align 8, !dbg !649
  %mul22 = mul nsw i32 5, %i.0, !dbg !650
  %idxprom23 = sext i32 %mul22 to i64, !dbg !649
  %arrayidx24 = getelementptr inbounds float*, float** %tmp16, i64 %idxprom23, !dbg !649
  store float* %tmp15, float** %arrayidx24, align 8, !dbg !651
  br label %if.end32, !dbg !649

if.else25:                                        ; preds = %for.body
  %base26 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 2, !dbg !652
  %tmp19 = load float**, float*** %base26, align 8, !dbg !652
  %idxprom27 = sext i32 %i.0 to i64, !dbg !653
  %arrayidx28 = getelementptr inbounds float*, float** %tmp19, i64 %idxprom27, !dbg !653
  %tmp21 = load float*, float** %arrayidx28, align 8, !dbg !653
  %base29 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !654
  %tmp22 = load float**, float*** %base29, align 8, !dbg !654
  %idxprom30 = sext i32 %i.0 to i64, !dbg !654
  %arrayidx31 = getelementptr inbounds float*, float** %tmp22, i64 %idxprom30, !dbg !654
  store float* %tmp21, float** %arrayidx31, align 8, !dbg !655
  br label %if.end32

if.end32:                                         ; preds = %if.else25, %if.then18
  br label %for.inc, !dbg !656

for.inc:                                          ; preds = %if.end32
  %inc = add nsw i32 %i.0, 1, !dbg !657
  call void @llvm.dbg.value(metadata i32 %inc, metadata !633, metadata !DIExpression()), !dbg !635
  br label %for.cond, !dbg !658, !llvm.loop !659

for.end:                                          ; preds = %for.cond
  ret void, !dbg !661
}

define void @"Arr<int*>::~Arr()"(%struct.Arr* nocapture readonly %this) align 2 !dbg !662 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !663, metadata !DIExpression()), !dbg !664
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2, !dbg !665
  %tmp = load i32**, i32*** %base, align 8, !dbg !665
  %tmp1 = bitcast i32** %tmp to i8*, !dbg !665
  call void @free(i8* %tmp1), !dbg !667
  ret void, !dbg !668
}

define void @"Arr<float*>::~Arr()"(%struct.Arr.0* nocapture readonly %this) align 2 !dbg !669 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr.0* %this, metadata !670, metadata !DIExpression()), !dbg !671
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2, !dbg !672
  %tmp = load float**, float*** %base, align 8, !dbg !672
  %tmp1 = bitcast float** %tmp to i8*, !dbg !672
  call void @free(i8* %tmp1), !dbg !674
  ret void, !dbg !675
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!19, !20, !21}
!llvm.dbg.intel.emit_class_debug_always = !{!22}
!llvm.ident = !{!23}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "v1", scope: !2, file: !3, line: 96, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang version 8.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !5, globals: !12, nameTableKind: None)
!3 = !DIFile(filename: "test.cc", directory: "llvm")
!4 = !{}
!5 = !{!6, !9}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!12 = !{!0, !13, !15, !17}
!13 = !DIGlobalVariableExpression(var: !14, expr: !DIExpression())
!14 = distinct !DIGlobalVariable(name: "v2", scope: !2, file: !3, line: 97, type: !8, isLocal: false, isDefinition: true)
!15 = !DIGlobalVariableExpression(var: !16, expr: !DIExpression())
!16 = distinct !DIGlobalVariable(name: "v3", scope: !2, file: !3, line: 98, type: !11, isLocal: false, isDefinition: true)
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(name: "v4", scope: !2, file: !3, line: 99, type: !11, isLocal: false, isDefinition: true)
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"wchar_size", i32 4}
!22 = !{!"true"}
!23 = !{!"clang version 8.0.0"}
!24 = distinct !DISubprogram(name: "check1", linkageName: "check1(F*)", scope: !3, file: !3, line: 101, type: !25, isLocal: false, isDefinition: true, scopeLine: 101, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!25 = !DISubroutineType(types: !26)
!26 = !{!27, !28}
!27 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!29 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "F", file: !3, line: 62, size: 128, flags: DIFlagTypePassByReference, elements: !30, identifier: "typeinfo name for F")
!30 = !{!31, !65, !99, !105, !108, !111, !114, !117, !120, !123}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !29, file: !3, line: 64, baseType: !32, size: 64, flags: DIFlagPublic)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!33 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Arr<int *>", file: !3, line: 6, size: 192, flags: DIFlagTypePassByReference, elements: !34, templateParams: !63, identifier: "typeinfo name for Arr<int*>")
!34 = !{!35, !36, !37, !38, !43, !46, !49, !50, !55, !60}
!35 = !DIDerivedType(tag: DW_TAG_member, name: "capacilty", scope: !33, file: !3, line: 7, baseType: !8, size: 32)
!36 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !33, file: !3, line: 8, baseType: !6, size: 64, offset: 64)
!37 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !33, file: !3, line: 9, baseType: !8, size: 32, offset: 128)
!38 = !DISubprogram(name: "get", linkageName: "Arr<int*>::get(int)", scope: !33, file: !3, line: 10, type: !39, isLocal: false, isDefinition: false, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false)
!39 = !DISubroutineType(types: !40)
!40 = !{!41, !42, !8}
!41 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !7, size: 64)
!42 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!43 = !DISubprogram(name: "set", linkageName: "Arr<int*>::set(int, int*)", scope: !33, file: !3, line: 15, type: !44, isLocal: false, isDefinition: false, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false)
!44 = !DISubroutineType(types: !45)
!45 = !{null, !42, !8, !7}
!46 = !DISubprogram(name: "Arr", scope: !33, file: !3, line: 21, type: !47, isLocal: false, isDefinition: false, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false)
!47 = !DISubroutineType(types: !48)
!48 = !{null, !42, !8}
!49 = !DISubprogram(name: "realloc", linkageName: "Arr<int*>::realloc(int)", scope: !33, file: !3, line: 24, type: !47, isLocal: false, isDefinition: false, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false)
!50 = !DISubprogram(name: "add", linkageName: "Arr<int*>::add(int* const&)", scope: !33, file: !3, line: 36, type: !51, isLocal: false, isDefinition: false, scopeLine: 36, flags: DIFlagPrototyped, isOptimized: false)
!51 = !DISubroutineType(types: !52)
!52 = !{null, !42, !53}
!53 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !54, size: 64)
!54 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!55 = !DISubprogram(name: "Arr", scope: !33, file: !3, line: 46, type: !56, isLocal: false, isDefinition: false, scopeLine: 46, flags: DIFlagPrototyped, isOptimized: false)
!56 = !DISubroutineType(types: !57)
!57 = !{null, !42, !58}
!58 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !59, size: 64)
!59 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !33)
!60 = !DISubprogram(name: "~Arr", scope: !33, file: !3, line: 59, type: !61, isLocal: false, isDefinition: false, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false)
!61 = !DISubroutineType(types: !62)
!62 = !{null, !42}
!63 = !{!64}
!64 = !DITemplateTypeParameter(name: "S", type: !7)
!65 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !29, file: !3, line: 65, baseType: !66, size: 64, offset: 64, flags: DIFlagPublic)
!66 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !67, size: 64)
!67 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Arr<float *>", file: !3, line: 6, size: 192, flags: DIFlagTypePassByReference, elements: !68, templateParams: !97, identifier: "typeinfo name for Arr<float*>")
!68 = !{!69, !70, !71, !72, !77, !80, !83, !84, !89, !94}
!69 = !DIDerivedType(tag: DW_TAG_member, name: "capacilty", scope: !67, file: !3, line: 7, baseType: !8, size: 32)
!70 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !67, file: !3, line: 8, baseType: !9, size: 64, offset: 64)
!71 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !67, file: !3, line: 9, baseType: !8, size: 32, offset: 128)
!72 = !DISubprogram(name: "get", linkageName: "Arr<float*>::get(int)", scope: !67, file: !3, line: 10, type: !73, isLocal: false, isDefinition: false, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false)
!73 = !DISubroutineType(types: !74)
!74 = !{!75, !76, !8}
!75 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !10, size: 64)
!76 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !67, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!77 = !DISubprogram(name: "set", linkageName: "Arr<float*>::set(int, float*)", scope: !67, file: !3, line: 15, type: !78, isLocal: false, isDefinition: false, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false)
!78 = !DISubroutineType(types: !79)
!79 = !{null, !76, !8, !10}
!80 = !DISubprogram(name: "Arr", scope: !67, file: !3, line: 21, type: !81, isLocal: false, isDefinition: false, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false)
!81 = !DISubroutineType(types: !82)
!82 = !{null, !76, !8}
!83 = !DISubprogram(name: "realloc", linkageName: "Arr<float*>::realloc(int)", scope: !67, file: !3, line: 24, type: !81, isLocal: false, isDefinition: false, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false)
!84 = !DISubprogram(name: "add", linkageName: "Arr<float*>::add(float* const&)", scope: !67, file: !3, line: 36, type: !85, isLocal: false, isDefinition: false, scopeLine: 36, flags: DIFlagPrototyped, isOptimized: false)
!85 = !DISubroutineType(types: !86)
!86 = !{null, !76, !87}
!87 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !88, size: 64)
!88 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !10)
!89 = !DISubprogram(name: "Arr", scope: !67, file: !3, line: 46, type: !90, isLocal: false, isDefinition: false, scopeLine: 46, flags: DIFlagPrototyped, isOptimized: false)
!90 = !DISubroutineType(types: !91)
!91 = !{null, !76, !92}
!92 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !93, size: 64)
!93 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !67)
!94 = !DISubprogram(name: "~Arr", scope: !67, file: !3, line: 59, type: !95, isLocal: false, isDefinition: false, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false)
!95 = !DISubroutineType(types: !96)
!96 = !{null, !76}
!97 = !{!98}
!98 = !DITemplateTypeParameter(name: "S", type: !10)
!99 = !DISubprogram(name: "F", scope: !29, file: !3, line: 66, type: !100, isLocal: false, isDefinition: false, scopeLine: 66, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!100 = !DISubroutineType(types: !101)
!101 = !{null, !102, !103}
!102 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!103 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !104, size: 64)
!104 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !29)
!105 = !DISubprogram(name: "put", linkageName: "F::put(int*, float*)", scope: !29, file: !3, line: 70, type: !106, isLocal: false, isDefinition: false, scopeLine: 70, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!106 = !DISubroutineType(types: !107)
!107 = !{null, !102, !7, !10}
!108 = !DISubprogram(name: "set1", linkageName: "F::set1(int, int*)", scope: !29, file: !3, line: 74, type: !109, isLocal: false, isDefinition: false, scopeLine: 74, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!109 = !DISubroutineType(types: !110)
!110 = !{null, !102, !8, !7}
!111 = !DISubprogram(name: "set2", linkageName: "F::set2(int, float*)", scope: !29, file: !3, line: 77, type: !112, isLocal: false, isDefinition: false, scopeLine: 77, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!112 = !DISubroutineType(types: !113)
!113 = !{null, !102, !8, !10}
!114 = !DISubprogram(name: "get1", linkageName: "F::get1(int)", scope: !29, file: !3, line: 80, type: !115, isLocal: false, isDefinition: false, scopeLine: 80, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!115 = !DISubroutineType(types: !116)
!116 = !{!8, !102, !8}
!117 = !DISubprogram(name: "get2", linkageName: "F::get2(int)", scope: !29, file: !3, line: 83, type: !118, isLocal: false, isDefinition: false, scopeLine: 83, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!118 = !DISubroutineType(types: !119)
!119 = !{!11, !102, !8}
!120 = !DISubprogram(name: "F", scope: !29, file: !3, line: 86, type: !121, isLocal: false, isDefinition: false, scopeLine: 86, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!121 = !DISubroutineType(types: !122)
!122 = !{null, !102}
!123 = !DISubprogram(name: "~F", scope: !29, file: !3, line: 90, type: !121, isLocal: false, isDefinition: false, scopeLine: 90, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!124 = !DILocalVariable(name: "f", arg: 1, scope: !24, file: !3, line: 101, type: !28)
!125 = !DILocation(line: 101, column: 16, scope: !24)
!126 = !DILocation(line: 102, column: 10, scope: !127)
!127 = distinct !DILexicalBlock(scope: !24, file: !3, line: 102, column: 7)
!128 = !DILocation(line: 102, column: 21, scope: !127)
!129 = !DILocation(line: 102, column: 18, scope: !127)
!130 = !DILocation(line: 102, column: 7, scope: !24)
!131 = !DILocation(line: 103, column: 5, scope: !127)
!132 = !DILocation(line: 104, column: 10, scope: !133)
!133 = distinct !DILexicalBlock(scope: !24, file: !3, line: 104, column: 7)
!134 = !DILocation(line: 104, column: 21, scope: !133)
!135 = !DILocation(line: 104, column: 18, scope: !133)
!136 = !DILocation(line: 104, column: 7, scope: !24)
!137 = !DILocation(line: 105, column: 5, scope: !133)
!138 = !DILocation(line: 106, column: 10, scope: !139)
!139 = distinct !DILexicalBlock(scope: !24, file: !3, line: 106, column: 7)
!140 = !DILocation(line: 106, column: 21, scope: !139)
!141 = !DILocation(line: 106, column: 18, scope: !139)
!142 = !DILocation(line: 106, column: 7, scope: !24)
!143 = !DILocation(line: 107, column: 5, scope: !139)
!144 = !DILocation(line: 108, column: 10, scope: !145)
!145 = distinct !DILexicalBlock(scope: !24, file: !3, line: 108, column: 7)
!146 = !DILocation(line: 108, column: 21, scope: !145)
!147 = !DILocation(line: 108, column: 18, scope: !145)
!148 = !DILocation(line: 108, column: 7, scope: !24)
!149 = !DILocation(line: 109, column: 5, scope: !145)
!150 = !DILocation(line: 110, column: 3, scope: !24)
!151 = !DILocation(line: 0, scope: !24)
!152 = !DILocation(line: 111, column: 1, scope: !24)
!153 = distinct !DISubprogram(name: "get1", linkageName: "F::get1(int)", scope: !29, file: !3, line: 80, type: !115, isLocal: false, isDefinition: true, scopeLine: 80, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !114, retainedNodes: !4)
!154 = !DILocalVariable(name: "this", arg: 1, scope: !153, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!155 = !DILocation(line: 0, scope: !153)
!156 = !DILocalVariable(name: "i", arg: 2, scope: !153, file: !3, line: 80, type: !8)
!157 = !DILocation(line: 80, column: 16, scope: !153)
!158 = !DILocation(line: 81, column: 14, scope: !153)
!159 = !DILocation(line: 81, column: 18, scope: !153)
!160 = !DILocation(line: 81, column: 12, scope: !153)
!161 = !DILocation(line: 81, column: 5, scope: !153)
!162 = distinct !DISubprogram(name: "get2", linkageName: "F::get2(int)", scope: !29, file: !3, line: 83, type: !118, isLocal: false, isDefinition: true, scopeLine: 83, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !117, retainedNodes: !4)
!163 = !DILocalVariable(name: "this", arg: 1, scope: !162, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!164 = !DILocation(line: 0, scope: !162)
!165 = !DILocalVariable(name: "i", arg: 2, scope: !162, file: !3, line: 83, type: !8)
!166 = !DILocation(line: 83, column: 18, scope: !162)
!167 = !DILocation(line: 84, column: 14, scope: !162)
!168 = !DILocation(line: 84, column: 18, scope: !162)
!169 = !DILocation(line: 84, column: 12, scope: !162)
!170 = !DILocation(line: 84, column: 5, scope: !162)
!171 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 113, type: !172, isLocal: false, isDefinition: true, scopeLine: 113, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!172 = !DISubroutineType(types: !173)
!173 = !{!8}
!174 = !DILocation(line: 114, column: 10, scope: !171)
!175 = !DILocation(line: 114, column: 14, scope: !171)
!176 = !DILocalVariable(name: "f", scope: !171, file: !3, line: 114, type: !28)
!177 = !DILocation(line: 114, column: 6, scope: !171)
!178 = !DILocation(line: 115, column: 6, scope: !171)
!179 = !DILocation(line: 116, column: 10, scope: !180)
!180 = distinct !DILexicalBlock(scope: !171, file: !3, line: 116, column: 7)
!181 = !DILocation(line: 116, column: 21, scope: !180)
!182 = !DILocation(line: 116, column: 18, scope: !180)
!183 = !DILocation(line: 116, column: 7, scope: !171)
!184 = !DILocation(line: 117, column: 5, scope: !180)
!185 = !DILocation(line: 143, column: 1, scope: !171)
!186 = !DILocation(line: 118, column: 10, scope: !187)
!187 = distinct !DILexicalBlock(scope: !171, file: !3, line: 118, column: 7)
!188 = !DILocation(line: 118, column: 21, scope: !187)
!189 = !DILocation(line: 118, column: 18, scope: !187)
!190 = !DILocation(line: 118, column: 7, scope: !171)
!191 = !DILocation(line: 119, column: 5, scope: !187)
!192 = !DILocation(line: 121, column: 6, scope: !171)
!193 = !DILocation(line: 122, column: 10, scope: !194)
!194 = distinct !DILexicalBlock(scope: !171, file: !3, line: 122, column: 7)
!195 = !DILocation(line: 122, column: 21, scope: !194)
!196 = !DILocation(line: 122, column: 18, scope: !194)
!197 = !DILocation(line: 122, column: 7, scope: !171)
!198 = !DILocation(line: 123, column: 5, scope: !194)
!199 = !DILocation(line: 124, column: 10, scope: !200)
!200 = distinct !DILexicalBlock(scope: !171, file: !3, line: 124, column: 7)
!201 = !DILocation(line: 124, column: 21, scope: !200)
!202 = !DILocation(line: 124, column: 18, scope: !200)
!203 = !DILocation(line: 124, column: 7, scope: !171)
!204 = !DILocation(line: 125, column: 5, scope: !200)
!205 = !DILocation(line: 126, column: 10, scope: !206)
!206 = distinct !DILexicalBlock(scope: !171, file: !3, line: 126, column: 7)
!207 = !DILocation(line: 126, column: 21, scope: !206)
!208 = !DILocation(line: 126, column: 18, scope: !206)
!209 = !DILocation(line: 126, column: 7, scope: !171)
!210 = !DILocation(line: 127, column: 5, scope: !206)
!211 = !DILocation(line: 128, column: 10, scope: !212)
!212 = distinct !DILexicalBlock(scope: !171, file: !3, line: 128, column: 7)
!213 = !DILocation(line: 128, column: 21, scope: !212)
!214 = !DILocation(line: 128, column: 18, scope: !212)
!215 = !DILocation(line: 128, column: 7, scope: !171)
!216 = !DILocation(line: 129, column: 5, scope: !212)
!217 = !DILocation(line: 131, column: 6, scope: !171)
!218 = !DILocation(line: 132, column: 6, scope: !171)
!219 = !DILocation(line: 133, column: 8, scope: !220)
!220 = distinct !DILexicalBlock(scope: !171, file: !3, line: 133, column: 7)
!221 = !DILocation(line: 133, column: 7, scope: !171)
!222 = !DILocation(line: 134, column: 5, scope: !220)
!223 = !DILocation(line: 136, column: 11, scope: !171)
!224 = !DILocation(line: 136, column: 15, scope: !171)
!225 = !DILocalVariable(name: "f1", scope: !171, file: !3, line: 136, type: !28)
!226 = !DILocation(line: 136, column: 6, scope: !171)
!227 = !DILocation(line: 137, column: 8, scope: !228)
!228 = distinct !DILexicalBlock(scope: !171, file: !3, line: 137, column: 7)
!229 = !DILocation(line: 137, column: 7, scope: !171)
!230 = !DILocation(line: 138, column: 5, scope: !228)
!231 = !DILocation(line: 140, column: 3, scope: !171)
!232 = !DILocation(line: 141, column: 3, scope: !171)
!233 = !DILocation(line: 142, column: 3, scope: !171)
!234 = !DILocation(line: 0, scope: !171)
!235 = distinct !DISubprogram(name: "F", linkageName: "F::F()", scope: !29, file: !3, line: 86, type: !121, isLocal: false, isDefinition: true, scopeLine: 86, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !120, retainedNodes: !4)
!236 = !DILocalVariable(name: "this", arg: 1, scope: !235, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!237 = !DILocation(line: 0, scope: !235)
!238 = !DILocation(line: 87, column: 10, scope: !239)
!239 = distinct !DILexicalBlock(scope: !235, file: !3, line: 86, column: 7)
!240 = !DILocation(line: 87, column: 14, scope: !239)
!241 = !DILocation(line: 87, column: 5, scope: !239)
!242 = !DILocation(line: 87, column: 8, scope: !239)
!243 = !DILocation(line: 88, column: 10, scope: !239)
!244 = !DILocation(line: 88, column: 14, scope: !239)
!245 = !DILocation(line: 88, column: 5, scope: !239)
!246 = !DILocation(line: 88, column: 8, scope: !239)
!247 = !DILocation(line: 89, column: 3, scope: !235)
!248 = !DILocation(line: 89, column: 3, scope: !239)
!249 = distinct !DISubprogram(name: "put", linkageName: "F::put(int*, float*)", scope: !29, file: !3, line: 70, type: !106, isLocal: false, isDefinition: true, scopeLine: 70, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !105, retainedNodes: !4)
!250 = !DILocalVariable(name: "this", arg: 1, scope: !249, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!251 = !DILocation(line: 0, scope: !249)
!252 = !DILocalVariable(name: "a", arg: 2, scope: !249, file: !3, line: 70, type: !7)
!253 = !DILocation(line: 70, column: 17, scope: !249)
!254 = !DILocalVariable(name: "b", arg: 3, scope: !249, file: !3, line: 70, type: !10)
!255 = !DILocation(line: 70, column: 27, scope: !249)
!256 = !DILocation(line: 71, column: 5, scope: !249)
!257 = !DILocation(line: 71, column: 9, scope: !249)
!258 = !DILocation(line: 72, column: 5, scope: !249)
!259 = !DILocation(line: 72, column: 9, scope: !249)
!260 = !DILocation(line: 73, column: 3, scope: !249)
!261 = distinct !DISubprogram(name: "set1", linkageName: "F::set1(int, int*)", scope: !29, file: !3, line: 74, type: !109, isLocal: false, isDefinition: true, scopeLine: 74, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !108, retainedNodes: !4)
!262 = !DILocalVariable(name: "this", arg: 1, scope: !261, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!263 = !DILocation(line: 0, scope: !261)
!264 = !DILocalVariable(name: "i", arg: 2, scope: !261, file: !3, line: 74, type: !8)
!265 = !DILocation(line: 74, column: 17, scope: !261)
!266 = !DILocalVariable(name: "a", arg: 3, scope: !261, file: !3, line: 74, type: !7)
!267 = !DILocation(line: 74, column: 25, scope: !261)
!268 = !DILocation(line: 75, column: 5, scope: !261)
!269 = !DILocation(line: 75, column: 9, scope: !261)
!270 = !DILocation(line: 76, column: 3, scope: !261)
!271 = distinct !DISubprogram(name: "set2", linkageName: "F::set2(int, float*)", scope: !29, file: !3, line: 77, type: !112, isLocal: false, isDefinition: true, scopeLine: 77, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !111, retainedNodes: !4)
!272 = !DILocalVariable(name: "this", arg: 1, scope: !271, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!273 = !DILocation(line: 0, scope: !271)
!274 = !DILocalVariable(name: "i", arg: 2, scope: !271, file: !3, line: 77, type: !8)
!275 = !DILocation(line: 77, column: 17, scope: !271)
!276 = !DILocalVariable(name: "b", arg: 3, scope: !271, file: !3, line: 77, type: !10)
!277 = !DILocation(line: 77, column: 27, scope: !271)
!278 = !DILocation(line: 78, column: 5, scope: !271)
!279 = !DILocation(line: 78, column: 9, scope: !271)
!280 = !DILocation(line: 79, column: 3, scope: !271)
!281 = distinct !DISubprogram(name: "F", linkageName: "F::F(F const&)", scope: !29, file: !3, line: 66, type: !100, isLocal: false, isDefinition: true, scopeLine: 66, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !99, retainedNodes: !4)
!282 = !DILocalVariable(name: "this", arg: 1, scope: !281, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!283 = !DILocation(line: 0, scope: !281)
!284 = !DILocalVariable(name: "f", arg: 2, scope: !281, file: !3, line: 66, type: !103)
!285 = !DILocation(line: 66, column: 14, scope: !281)
!286 = !DILocation(line: 67, column: 10, scope: !287)
!287 = distinct !DILexicalBlock(scope: !281, file: !3, line: 66, column: 17)
!288 = !DILocation(line: 67, column: 28, scope: !287)
!289 = !DILocation(line: 67, column: 14, scope: !287)
!290 = !DILocation(line: 67, column: 5, scope: !287)
!291 = !DILocation(line: 67, column: 8, scope: !287)
!292 = !DILocation(line: 68, column: 10, scope: !287)
!293 = !DILocation(line: 68, column: 30, scope: !287)
!294 = !DILocation(line: 68, column: 14, scope: !287)
!295 = !DILocation(line: 68, column: 5, scope: !287)
!296 = !DILocation(line: 68, column: 8, scope: !287)
!297 = !DILocation(line: 69, column: 3, scope: !281)
!298 = !DILocation(line: 69, column: 3, scope: !287)
!299 = distinct !DISubprogram(name: "~F", linkageName: "F::~F()", scope: !29, file: !3, line: 90, type: !121, isLocal: false, isDefinition: true, scopeLine: 90, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !123, retainedNodes: !4)
!300 = !DILocalVariable(name: "this", arg: 1, scope: !299, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!301 = !DILocation(line: 0, scope: !299)
!302 = !DILocation(line: 91, column: 12, scope: !303)
!303 = distinct !DILexicalBlock(scope: !299, file: !3, line: 90, column: 8)
!304 = !DILocation(line: 91, column: 5, scope: !303)
!305 = !DILocation(line: 92, column: 12, scope: !303)
!306 = !DILocation(line: 92, column: 5, scope: !303)
!307 = !DILocation(line: 93, column: 3, scope: !299)
!308 = distinct !DISubprogram(name: "get", linkageName: "Arr<int*>::get(int)", scope: !33, file: !3, line: 10, type: !39, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !38, retainedNodes: !4)
!309 = !DILocalVariable(name: "this", arg: 1, scope: !308, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!310 = !DILocation(line: 0, scope: !308)
!311 = !DILocalVariable(name: "i", arg: 2, scope: !308, file: !3, line: 10, type: !8)
!312 = !DILocation(line: 10, column: 14, scope: !308)
!313 = !DILocation(line: 11, column: 9, scope: !314)
!314 = distinct !DILexicalBlock(scope: !308, file: !3, line: 11, column: 9)
!315 = !DILocation(line: 11, column: 19, scope: !314)
!316 = !DILocation(line: 11, column: 9, scope: !308)
!317 = !DILocation(line: 12, column: 14, scope: !314)
!318 = !DILocation(line: 12, column: 21, scope: !314)
!319 = !DILocation(line: 12, column: 7, scope: !314)
!320 = !DILocation(line: 13, column: 12, scope: !308)
!321 = !DILocation(line: 13, column: 5, scope: !308)
!322 = !DILocation(line: 14, column: 3, scope: !308)
!323 = distinct !DISubprogram(name: "get", linkageName: "Arr<float*>::get(int)", scope: !67, file: !3, line: 10, type: !73, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !72, retainedNodes: !4)
!324 = !DILocalVariable(name: "this", arg: 1, scope: !323, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!325 = !DILocation(line: 0, scope: !323)
!326 = !DILocalVariable(name: "i", arg: 2, scope: !323, file: !3, line: 10, type: !8)
!327 = !DILocation(line: 10, column: 14, scope: !323)
!328 = !DILocation(line: 11, column: 9, scope: !329)
!329 = distinct !DILexicalBlock(scope: !323, file: !3, line: 11, column: 9)
!330 = !DILocation(line: 11, column: 19, scope: !329)
!331 = !DILocation(line: 11, column: 9, scope: !323)
!332 = !DILocation(line: 12, column: 14, scope: !329)
!333 = !DILocation(line: 12, column: 21, scope: !329)
!334 = !DILocation(line: 12, column: 7, scope: !329)
!335 = !DILocation(line: 13, column: 12, scope: !323)
!336 = !DILocation(line: 13, column: 5, scope: !323)
!337 = !DILocation(line: 14, column: 3, scope: !323)
!338 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<int*>::Arr(int)", scope: !33, file: !3, line: 21, type: !47, isLocal: false, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !46, retainedNodes: !4)
!339 = !DILocalVariable(name: "this", arg: 1, scope: !338, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!340 = !DILocation(line: 0, scope: !338)
!341 = !DILocalVariable(name: "c", arg: 2, scope: !338, file: !3, line: 21, type: !8)
!342 = !DILocation(line: 21, column: 11, scope: !338)
!343 = !DILocation(line: 21, column: 20, scope: !338)
!344 = !DILocation(line: 21, column: 43, scope: !338)
!345 = !DILocation(line: 21, column: 34, scope: !338)
!346 = !DILocation(line: 22, column: 24, scope: !347)
!347 = distinct !DILexicalBlock(scope: !338, file: !3, line: 21, column: 57)
!348 = !DILocation(line: 22, column: 34, scope: !347)
!349 = !DILocation(line: 22, column: 17, scope: !347)
!350 = !DILocation(line: 22, column: 12, scope: !347)
!351 = !DILocation(line: 22, column: 5, scope: !347)
!352 = !DILocation(line: 22, column: 10, scope: !347)
!353 = !DILocation(line: 23, column: 3, scope: !338)
!354 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<float*>::Arr(int)", scope: !67, file: !3, line: 21, type: !81, isLocal: false, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !80, retainedNodes: !4)
!355 = !DILocalVariable(name: "this", arg: 1, scope: !354, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!356 = !DILocation(line: 0, scope: !354)
!357 = !DILocalVariable(name: "c", arg: 2, scope: !354, file: !3, line: 21, type: !8)
!358 = !DILocation(line: 21, column: 11, scope: !354)
!359 = !DILocation(line: 21, column: 20, scope: !354)
!360 = !DILocation(line: 21, column: 43, scope: !354)
!361 = !DILocation(line: 21, column: 34, scope: !354)
!362 = !DILocation(line: 22, column: 24, scope: !363)
!363 = distinct !DILexicalBlock(scope: !354, file: !3, line: 21, column: 57)
!364 = !DILocation(line: 22, column: 34, scope: !363)
!365 = !DILocation(line: 22, column: 17, scope: !363)
!366 = !DILocation(line: 22, column: 12, scope: !363)
!367 = !DILocation(line: 22, column: 5, scope: !363)
!368 = !DILocation(line: 22, column: 10, scope: !363)
!369 = !DILocation(line: 23, column: 3, scope: !354)
!370 = distinct !DISubprogram(name: "add", linkageName: "Arr<int*>::add(int* const&)", scope: !33, file: !3, line: 36, type: !51, isLocal: false, isDefinition: true, scopeLine: 36, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !50, retainedNodes: !4)
!371 = !DILocalVariable(name: "this", arg: 1, scope: !370, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!372 = !DILocation(line: 0, scope: !370)
!373 = !DILocalVariable(name: "e", arg: 2, scope: !370, file: !3, line: 36, type: !53)
!374 = !DILocation(line: 36, column: 21, scope: !370)
!375 = !DILocation(line: 37, column: 5, scope: !370)
!376 = !DILocation(line: 39, column: 9, scope: !377)
!377 = distinct !DILexicalBlock(scope: !370, file: !3, line: 39, column: 9)
!378 = !DILocation(line: 39, column: 19, scope: !377)
!379 = !DILocation(line: 39, column: 9, scope: !370)
!380 = !DILocation(line: 40, column: 24, scope: !377)
!381 = !DILocation(line: 40, column: 7, scope: !377)
!382 = !DILocation(line: 40, column: 16, scope: !377)
!383 = !DILocation(line: 40, column: 14, scope: !377)
!384 = !DILocation(line: 40, column: 22, scope: !377)
!385 = !DILocation(line: 42, column: 20, scope: !377)
!386 = !DILocation(line: 42, column: 7, scope: !377)
!387 = !DILocation(line: 42, column: 12, scope: !377)
!388 = !DILocation(line: 42, column: 18, scope: !377)
!389 = !DILocation(line: 44, column: 7, scope: !370)
!390 = !DILocation(line: 44, column: 5, scope: !370)
!391 = !DILocation(line: 45, column: 3, scope: !370)
!392 = distinct !DISubprogram(name: "add", linkageName: "Arr<float*>::add(float* const&)", scope: !67, file: !3, line: 36, type: !85, isLocal: false, isDefinition: true, scopeLine: 36, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !84, retainedNodes: !4)
!393 = !DILocalVariable(name: "this", arg: 1, scope: !392, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!394 = !DILocation(line: 0, scope: !392)
!395 = !DILocalVariable(name: "e", arg: 2, scope: !392, file: !3, line: 36, type: !87)
!396 = !DILocation(line: 36, column: 21, scope: !392)
!397 = !DILocation(line: 37, column: 5, scope: !392)
!398 = !DILocation(line: 39, column: 9, scope: !399)
!399 = distinct !DILexicalBlock(scope: !392, file: !3, line: 39, column: 9)
!400 = !DILocation(line: 39, column: 19, scope: !399)
!401 = !DILocation(line: 39, column: 9, scope: !392)
!402 = !DILocation(line: 40, column: 24, scope: !399)
!403 = !DILocation(line: 40, column: 7, scope: !399)
!404 = !DILocation(line: 40, column: 16, scope: !399)
!405 = !DILocation(line: 40, column: 14, scope: !399)
!406 = !DILocation(line: 40, column: 22, scope: !399)
!407 = !DILocation(line: 42, column: 20, scope: !399)
!408 = !DILocation(line: 42, column: 7, scope: !399)
!409 = !DILocation(line: 42, column: 12, scope: !399)
!410 = !DILocation(line: 42, column: 18, scope: !399)
!411 = !DILocation(line: 44, column: 7, scope: !392)
!412 = !DILocation(line: 44, column: 5, scope: !392)
!413 = !DILocation(line: 45, column: 3, scope: !392)
!414 = distinct !DISubprogram(name: "realloc", linkageName: "Arr<int*>::realloc(int)", scope: !33, file: !3, line: 24, type: !47, isLocal: false, isDefinition: true, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !49, retainedNodes: !4)
!415 = !DILocalVariable(name: "this", arg: 1, scope: !414, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!416 = !DILocation(line: 0, scope: !414)
!417 = !DILocalVariable(name: "inc", arg: 2, scope: !414, file: !3, line: 24, type: !8)
!418 = !DILocation(line: 24, column: 20, scope: !414)
!419 = !DILocation(line: 25, column: 9, scope: !420)
!420 = distinct !DILexicalBlock(scope: !414, file: !3, line: 25, column: 9)
!421 = !DILocation(line: 25, column: 14, scope: !420)
!422 = !DILocation(line: 25, column: 23, scope: !420)
!423 = !DILocation(line: 25, column: 20, scope: !420)
!424 = !DILocation(line: 25, column: 9, scope: !414)
!425 = !DILocation(line: 26, column: 7, scope: !420)
!426 = !DILocation(line: 28, column: 17, scope: !414)
!427 = !DILocation(line: 28, column: 22, scope: !414)
!428 = !DILocation(line: 28, column: 5, scope: !414)
!429 = !DILocation(line: 28, column: 15, scope: !414)
!430 = !DILocation(line: 29, column: 35, scope: !414)
!431 = !DILocation(line: 29, column: 33, scope: !414)
!432 = !DILocation(line: 29, column: 31, scope: !414)
!433 = !DILocation(line: 29, column: 45, scope: !414)
!434 = !DILocation(line: 29, column: 24, scope: !414)
!435 = !DILocation(line: 29, column: 19, scope: !414)
!436 = !DILocalVariable(name: "new_base", scope: !414, file: !3, line: 29, type: !6)
!437 = !DILocation(line: 29, column: 8, scope: !414)
!438 = !DILocalVariable(name: "i", scope: !439, file: !3, line: 30, type: !8)
!439 = distinct !DILexicalBlock(scope: !414, file: !3, line: 30, column: 5)
!440 = !DILocation(line: 30, column: 14, scope: !439)
!441 = !DILocation(line: 30, column: 10, scope: !439)
!442 = !DILocation(line: 0, scope: !439)
!443 = !DILocation(line: 30, column: 25, scope: !444)
!444 = distinct !DILexicalBlock(scope: !439, file: !3, line: 30, column: 5)
!445 = !DILocation(line: 30, column: 23, scope: !444)
!446 = !DILocation(line: 30, column: 5, scope: !439)
!447 = !DILocation(line: 31, column: 25, scope: !448)
!448 = distinct !DILexicalBlock(scope: !444, file: !3, line: 30, column: 36)
!449 = !DILocation(line: 31, column: 18, scope: !448)
!450 = !DILocation(line: 31, column: 7, scope: !448)
!451 = !DILocation(line: 31, column: 23, scope: !448)
!452 = !DILocation(line: 32, column: 5, scope: !448)
!453 = !DILocation(line: 30, column: 31, scope: !444)
!454 = !DILocation(line: 30, column: 5, scope: !444)
!455 = distinct !{!455, !446, !456}
!456 = !DILocation(line: 32, column: 5, scope: !439)
!457 = !DILocation(line: 33, column: 10, scope: !414)
!458 = !DILocation(line: 33, column: 5, scope: !414)
!459 = !DILocation(line: 34, column: 5, scope: !414)
!460 = !DILocation(line: 34, column: 10, scope: !414)
!461 = !DILocation(line: 35, column: 3, scope: !414)
!462 = distinct !DISubprogram(name: "realloc", linkageName: "Arr<float*>::realloc(int)", scope: !67, file: !3, line: 24, type: !81, isLocal: false, isDefinition: true, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !83, retainedNodes: !4)
!463 = !DILocalVariable(name: "this", arg: 1, scope: !462, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!464 = !DILocation(line: 0, scope: !462)
!465 = !DILocalVariable(name: "inc", arg: 2, scope: !462, file: !3, line: 24, type: !8)
!466 = !DILocation(line: 24, column: 20, scope: !462)
!467 = !DILocation(line: 25, column: 9, scope: !468)
!468 = distinct !DILexicalBlock(scope: !462, file: !3, line: 25, column: 9)
!469 = !DILocation(line: 25, column: 14, scope: !468)
!470 = !DILocation(line: 25, column: 23, scope: !468)
!471 = !DILocation(line: 25, column: 20, scope: !468)
!472 = !DILocation(line: 25, column: 9, scope: !462)
!473 = !DILocation(line: 26, column: 7, scope: !468)
!474 = !DILocation(line: 28, column: 17, scope: !462)
!475 = !DILocation(line: 28, column: 22, scope: !462)
!476 = !DILocation(line: 28, column: 5, scope: !462)
!477 = !DILocation(line: 28, column: 15, scope: !462)
!478 = !DILocation(line: 29, column: 35, scope: !462)
!479 = !DILocation(line: 29, column: 33, scope: !462)
!480 = !DILocation(line: 29, column: 31, scope: !462)
!481 = !DILocation(line: 29, column: 45, scope: !462)
!482 = !DILocation(line: 29, column: 24, scope: !462)
!483 = !DILocation(line: 29, column: 19, scope: !462)
!484 = !DILocalVariable(name: "new_base", scope: !462, file: !3, line: 29, type: !9)
!485 = !DILocation(line: 29, column: 8, scope: !462)
!486 = !DILocalVariable(name: "i", scope: !487, file: !3, line: 30, type: !8)
!487 = distinct !DILexicalBlock(scope: !462, file: !3, line: 30, column: 5)
!488 = !DILocation(line: 30, column: 14, scope: !487)
!489 = !DILocation(line: 30, column: 10, scope: !487)
!490 = !DILocation(line: 0, scope: !487)
!491 = !DILocation(line: 30, column: 25, scope: !492)
!492 = distinct !DILexicalBlock(scope: !487, file: !3, line: 30, column: 5)
!493 = !DILocation(line: 30, column: 23, scope: !492)
!494 = !DILocation(line: 30, column: 5, scope: !487)
!495 = !DILocation(line: 31, column: 25, scope: !496)
!496 = distinct !DILexicalBlock(scope: !492, file: !3, line: 30, column: 36)
!497 = !DILocation(line: 31, column: 18, scope: !496)
!498 = !DILocation(line: 31, column: 7, scope: !496)
!499 = !DILocation(line: 31, column: 23, scope: !496)
!500 = !DILocation(line: 32, column: 5, scope: !496)
!501 = !DILocation(line: 30, column: 31, scope: !492)
!502 = !DILocation(line: 30, column: 5, scope: !492)
!503 = distinct !{!503, !494, !504}
!504 = !DILocation(line: 32, column: 5, scope: !487)
!505 = !DILocation(line: 33, column: 10, scope: !462)
!506 = !DILocation(line: 33, column: 5, scope: !462)
!507 = !DILocation(line: 34, column: 5, scope: !462)
!508 = !DILocation(line: 34, column: 10, scope: !462)
!509 = !DILocation(line: 35, column: 3, scope: !462)
!510 = distinct !DISubprogram(name: "set", linkageName: "Arr<int*>::set(int, int*)", scope: !33, file: !3, line: 15, type: !44, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !43, retainedNodes: !4)
!511 = !DILocalVariable(name: "this", arg: 1, scope: !510, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!512 = !DILocation(line: 0, scope: !510)
!513 = !DILocalVariable(name: "i", arg: 2, scope: !510, file: !3, line: 15, type: !8)
!514 = !DILocation(line: 15, column: 16, scope: !510)
!515 = !DILocalVariable(name: "val", arg: 3, scope: !510, file: !3, line: 15, type: !7)
!516 = !DILocation(line: 15, column: 21, scope: !510)
!517 = !DILocation(line: 16, column: 9, scope: !518)
!518 = distinct !DILexicalBlock(scope: !510, file: !3, line: 16, column: 9)
!519 = !DILocation(line: 16, column: 19, scope: !518)
!520 = !DILocation(line: 16, column: 9, scope: !510)
!521 = !DILocation(line: 17, column: 7, scope: !518)
!522 = !DILocation(line: 17, column: 13, scope: !518)
!523 = !DILocation(line: 17, column: 17, scope: !518)
!524 = !DILocation(line: 19, column: 7, scope: !518)
!525 = !DILocation(line: 19, column: 15, scope: !518)
!526 = !DILocation(line: 20, column: 3, scope: !510)
!527 = distinct !DISubprogram(name: "set", linkageName: "Arr<float*>::set(int, float*)", scope: !67, file: !3, line: 15, type: !78, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !77, retainedNodes: !4)
!528 = !DILocalVariable(name: "this", arg: 1, scope: !527, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!529 = !DILocation(line: 0, scope: !527)
!530 = !DILocalVariable(name: "i", arg: 2, scope: !527, file: !3, line: 15, type: !8)
!531 = !DILocation(line: 15, column: 16, scope: !527)
!532 = !DILocalVariable(name: "val", arg: 3, scope: !527, file: !3, line: 15, type: !10)
!533 = !DILocation(line: 15, column: 21, scope: !527)
!534 = !DILocation(line: 16, column: 9, scope: !535)
!535 = distinct !DILexicalBlock(scope: !527, file: !3, line: 16, column: 9)
!536 = !DILocation(line: 16, column: 19, scope: !535)
!537 = !DILocation(line: 16, column: 9, scope: !527)
!538 = !DILocation(line: 17, column: 7, scope: !535)
!539 = !DILocation(line: 17, column: 13, scope: !535)
!540 = !DILocation(line: 17, column: 17, scope: !535)
!541 = !DILocation(line: 19, column: 7, scope: !535)
!542 = !DILocation(line: 19, column: 15, scope: !535)
!543 = !DILocation(line: 20, column: 3, scope: !527)
!544 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<int*>::Arr(Arr<int*> const&)", scope: !33, file: !3, line: 46, type: !56, isLocal: false, isDefinition: true, scopeLine: 46, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !55, retainedNodes: !4)
!545 = !DILocalVariable(name: "this", arg: 1, scope: !544, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!546 = !DILocation(line: 0, scope: !544)
!547 = !DILocalVariable(name: "A", arg: 2, scope: !544, file: !3, line: 46, type: !58)
!548 = !DILocation(line: 46, column: 18, scope: !544)
!549 = !DILocation(line: 47, column: 19, scope: !550)
!550 = distinct !DILexicalBlock(scope: !544, file: !3, line: 46, column: 21)
!551 = !DILocation(line: 47, column: 5, scope: !550)
!552 = !DILocation(line: 47, column: 15, scope: !550)
!553 = !DILocation(line: 48, column: 14, scope: !550)
!554 = !DILocation(line: 48, column: 5, scope: !550)
!555 = !DILocation(line: 48, column: 10, scope: !550)
!556 = !DILocation(line: 49, column: 9, scope: !557)
!557 = distinct !DILexicalBlock(scope: !550, file: !3, line: 49, column: 9)
!558 = !DILocation(line: 49, column: 19, scope: !557)
!559 = !DILocation(line: 49, column: 9, scope: !550)
!560 = !DILocation(line: 50, column: 30, scope: !557)
!561 = !DILocation(line: 50, column: 28, scope: !557)
!562 = !DILocation(line: 50, column: 26, scope: !557)
!563 = !DILocation(line: 50, column: 40, scope: !557)
!564 = !DILocation(line: 50, column: 19, scope: !557)
!565 = !DILocation(line: 50, column: 14, scope: !557)
!566 = !DILocation(line: 50, column: 7, scope: !557)
!567 = !DILocation(line: 50, column: 12, scope: !557)
!568 = !DILocation(line: 52, column: 26, scope: !557)
!569 = !DILocation(line: 52, column: 36, scope: !557)
!570 = !DILocation(line: 52, column: 19, scope: !557)
!571 = !DILocation(line: 52, column: 14, scope: !557)
!572 = !DILocation(line: 52, column: 7, scope: !557)
!573 = !DILocation(line: 52, column: 12, scope: !557)
!574 = !DILocalVariable(name: "i", scope: !575, file: !3, line: 53, type: !8)
!575 = distinct !DILexicalBlock(scope: !550, file: !3, line: 53, column: 5)
!576 = !DILocation(line: 53, column: 14, scope: !575)
!577 = !DILocation(line: 53, column: 10, scope: !575)
!578 = !DILocation(line: 0, scope: !575)
!579 = !DILocation(line: 53, column: 25, scope: !580)
!580 = distinct !DILexicalBlock(scope: !575, file: !3, line: 53, column: 5)
!581 = !DILocation(line: 53, column: 23, scope: !580)
!582 = !DILocation(line: 53, column: 5, scope: !575)
!583 = !DILocation(line: 54, column: 11, scope: !584)
!584 = distinct !DILexicalBlock(scope: !580, file: !3, line: 54, column: 11)
!585 = !DILocation(line: 54, column: 21, scope: !584)
!586 = !DILocation(line: 54, column: 11, scope: !580)
!587 = !DILocation(line: 55, column: 25, scope: !584)
!588 = !DILocation(line: 55, column: 32, scope: !584)
!589 = !DILocation(line: 55, column: 23, scope: !584)
!590 = !DILocation(line: 55, column: 9, scope: !584)
!591 = !DILocation(line: 55, column: 16, scope: !584)
!592 = !DILocation(line: 55, column: 21, scope: !584)
!593 = !DILocation(line: 57, column: 21, scope: !584)
!594 = !DILocation(line: 57, column: 19, scope: !584)
!595 = !DILocation(line: 57, column: 9, scope: !584)
!596 = !DILocation(line: 57, column: 17, scope: !584)
!597 = !DILocation(line: 54, column: 23, scope: !584)
!598 = !DILocation(line: 53, column: 31, scope: !580)
!599 = !DILocation(line: 53, column: 5, scope: !580)
!600 = distinct !{!600, !582, !601}
!601 = !DILocation(line: 57, column: 27, scope: !575)
!602 = !DILocation(line: 58, column: 3, scope: !544)
!603 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<float*>::Arr(Arr<float*> const&)", scope: !67, file: !3, line: 46, type: !90, isLocal: false, isDefinition: true, scopeLine: 46, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !89, retainedNodes: !4)
!604 = !DILocalVariable(name: "this", arg: 1, scope: !603, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!605 = !DILocation(line: 0, scope: !603)
!606 = !DILocalVariable(name: "A", arg: 2, scope: !603, file: !3, line: 46, type: !92)
!607 = !DILocation(line: 46, column: 18, scope: !603)
!608 = !DILocation(line: 47, column: 19, scope: !609)
!609 = distinct !DILexicalBlock(scope: !603, file: !3, line: 46, column: 21)
!610 = !DILocation(line: 47, column: 5, scope: !609)
!611 = !DILocation(line: 47, column: 15, scope: !609)
!612 = !DILocation(line: 48, column: 14, scope: !609)
!613 = !DILocation(line: 48, column: 5, scope: !609)
!614 = !DILocation(line: 48, column: 10, scope: !609)
!615 = !DILocation(line: 49, column: 9, scope: !616)
!616 = distinct !DILexicalBlock(scope: !609, file: !3, line: 49, column: 9)
!617 = !DILocation(line: 49, column: 19, scope: !616)
!618 = !DILocation(line: 49, column: 9, scope: !609)
!619 = !DILocation(line: 50, column: 30, scope: !616)
!620 = !DILocation(line: 50, column: 28, scope: !616)
!621 = !DILocation(line: 50, column: 26, scope: !616)
!622 = !DILocation(line: 50, column: 40, scope: !616)
!623 = !DILocation(line: 50, column: 19, scope: !616)
!624 = !DILocation(line: 50, column: 14, scope: !616)
!625 = !DILocation(line: 50, column: 7, scope: !616)
!626 = !DILocation(line: 50, column: 12, scope: !616)
!627 = !DILocation(line: 52, column: 26, scope: !616)
!628 = !DILocation(line: 52, column: 36, scope: !616)
!629 = !DILocation(line: 52, column: 19, scope: !616)
!630 = !DILocation(line: 52, column: 14, scope: !616)
!631 = !DILocation(line: 52, column: 7, scope: !616)
!632 = !DILocation(line: 52, column: 12, scope: !616)
!633 = !DILocalVariable(name: "i", scope: !634, file: !3, line: 53, type: !8)
!634 = distinct !DILexicalBlock(scope: !609, file: !3, line: 53, column: 5)
!635 = !DILocation(line: 53, column: 14, scope: !634)
!636 = !DILocation(line: 53, column: 10, scope: !634)
!637 = !DILocation(line: 0, scope: !634)
!638 = !DILocation(line: 53, column: 25, scope: !639)
!639 = distinct !DILexicalBlock(scope: !634, file: !3, line: 53, column: 5)
!640 = !DILocation(line: 53, column: 23, scope: !639)
!641 = !DILocation(line: 53, column: 5, scope: !634)
!642 = !DILocation(line: 54, column: 11, scope: !643)
!643 = distinct !DILexicalBlock(scope: !639, file: !3, line: 54, column: 11)
!644 = !DILocation(line: 54, column: 21, scope: !643)
!645 = !DILocation(line: 54, column: 11, scope: !639)
!646 = !DILocation(line: 55, column: 25, scope: !643)
!647 = !DILocation(line: 55, column: 32, scope: !643)
!648 = !DILocation(line: 55, column: 23, scope: !643)
!649 = !DILocation(line: 55, column: 9, scope: !643)
!650 = !DILocation(line: 55, column: 16, scope: !643)
!651 = !DILocation(line: 55, column: 21, scope: !643)
!652 = !DILocation(line: 57, column: 21, scope: !643)
!653 = !DILocation(line: 57, column: 19, scope: !643)
!654 = !DILocation(line: 57, column: 9, scope: !643)
!655 = !DILocation(line: 57, column: 17, scope: !643)
!656 = !DILocation(line: 54, column: 23, scope: !643)
!657 = !DILocation(line: 53, column: 31, scope: !639)
!658 = !DILocation(line: 53, column: 5, scope: !639)
!659 = distinct !{!659, !641, !660}
!660 = !DILocation(line: 57, column: 27, scope: !634)
!661 = !DILocation(line: 58, column: 3, scope: !603)
!662 = distinct !DISubprogram(name: "~Arr", linkageName: "Arr<int*>::~Arr()", scope: !33, file: !3, line: 59, type: !61, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !60, retainedNodes: !4)
!663 = !DILocalVariable(name: "this", arg: 1, scope: !662, type: !32, flags: DIFlagArtificial | DIFlagObjectPointer)
!664 = !DILocation(line: 0, scope: !662)
!665 = !DILocation(line: 59, column: 17, scope: !666)
!666 = distinct !DILexicalBlock(scope: !662, file: !3, line: 59, column: 10)
!667 = !DILocation(line: 59, column: 12, scope: !666)
!668 = !DILocation(line: 59, column: 24, scope: !662)
!669 = distinct !DISubprogram(name: "~Arr", linkageName: "Arr<float*>::~Arr()", scope: !67, file: !3, line: 59, type: !95, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false, unit: !2, declaration: !94, retainedNodes: !4)
!670 = !DILocalVariable(name: "this", arg: 1, scope: !669, type: !66, flags: DIFlagArtificial | DIFlagObjectPointer)
!671 = !DILocation(line: 0, scope: !669)
!672 = !DILocation(line: 59, column: 17, scope: !673)
!673 = distinct !DILexicalBlock(scope: !669, file: !3, line: 59, column: 10)
!674 = !DILocation(line: 59, column: 12, scope: !673)
!675 = !DILocation(line: 59, column: 24, scope: !669)
