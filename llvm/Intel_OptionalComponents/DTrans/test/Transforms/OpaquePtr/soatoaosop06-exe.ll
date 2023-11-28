; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-soatoaosop                                \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -dtrans-soatoaosop-size-heuristic=false                   \
; RUN:       | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-soatoaosop                                \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                                       \
; RUN:          -dtrans-soatoaosop-size-heuristic=false                   \
; RUN:       | %lli
; REQUIRES: system-linux


; -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays,dtrans-soatoaosop-struct

; Checks that transformation happens and executable test passes.
;    // icx -std=c++11 m5.cc -O0 -g -S -emit-llvm
;    // Attributess cleanup.
;    // Simplified:
;    //     opt -S -instnamer -mem2reg -function-attrs
; extern "C" {
; extern void *malloc(int) noexcept;
; extern void free(void *) noexcept;
; }
;  struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; template <typename S> struct Arr {
;   bool flag;
;   unsigned capacity;
;   S *base;
;   unsigned size;
;   struct Mem* mem;
;   S &get(int i) {
;     if (i >= size)
;       throw;
;     return base[i];
;   }
;   void set(int i, S val) {
;     if (i >= size)
;       throw;
;     base[i] = val;
;   }
;   Arr(unsigned c = 2, struct Mem *mem = 0)
;     : flag(false), capacity(c), size(0), base(0), mem(mem) {
;     base = (S*)malloc(capacity * sizeof(S));
;     memset(base, 0, capacity * sizeof(S));
;   }
;   void realloc(int inc) {
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;     unsigned int minNewMax = (unsigned int)((double)size * 1.25);
;     if (newMax < minNewMax)
;         newMax = minNewMax;
;     S *newList = (S *) malloc (newMax * sizeof(S));
;     for (unsigned int index = 0; index < size; index++)
;        newList[index] = base[index];
;
;     free(base); //delete [] fElemList;
;     base = newList;
;     capacity = newMax;
;   }
;   void add(const S &e) {
;     realloc(1);
;     base[size] = e;
;     ++size;
;   }
;   Arr(const Arr &A) :
;     flag(A.flag), capacity(A.capacity), size(A.size), base(0), mem(A.mem) {
;     base = (S*)malloc(capacity * sizeof(S));
;     memset(base, 0, capacity * sizeof(S));
;     for (unsigned int index = 0; index < size; index++)
;       base[index] = A.base[index];
;   }
;   ~Arr() { free(base); }
; };
;
; class F {
; public:
;   Arr<int *> *f1;
;   Arr<float *> *f2;
;   struct Mem* mem;
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
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-DAG: %__SOADT_class.F = type { ptr, i64, ptr }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i8, i32, ptr, i32, ptr }
; CHECK-DAG: %__SOADT_EL_class.F = type { ptr, ptr }

%class.F = type { ptr, ptr, ptr }
%struct.Arr = type { i8, i32, ptr, i32, ptr }
%struct.Arr.0 = type { i8, i32, ptr, i32, ptr }
%struct.Mem = type { ptr }

@v1 = global i32 20, align 4, !dbg !0
@v2 = global i32 30, align 4, !dbg !13
@v3 = global float 3.500000e+00, align 4, !dbg !15
@v4 = global float 7.500000e+00, align 4, !dbg !17

; Function Attrs: nounwind memory(read)
define zeroext i1 @"check1(F*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %f) #0 !dbg !37 !intel.dtrans.func.type !137 {
entry:
  call void @llvm.dbg.value(metadata ptr %f, metadata !139, metadata !DIExpression()), !dbg !140
  %call = call i32 @"F::get1(int)"(ptr %f, i32 0), !dbg !141
  %tmp1 = load i32, ptr @v2, align 4, !dbg !143
  %cmp = icmp ne i32 %call, %tmp1, !dbg !144
  br i1 %cmp, label %if.then, label %if.end, !dbg !145

if.then:                                          ; preds = %entry
  br label %return, !dbg !146

if.end:                                           ; preds = %entry
  %call1 = call float @"F::get2(int)"(ptr %f, i32 0), !dbg !147
  %tmp3 = load float, ptr @v4, align 4, !dbg !149
  %cmp2 = fcmp une float %call1, %tmp3, !dbg !150
  br i1 %cmp2, label %if.then3, label %if.end4, !dbg !151

if.then3:                                         ; preds = %if.end
  br label %return, !dbg !152

if.end4:                                          ; preds = %if.end
  %call5 = call i32 @"F::get1(int)"(ptr %f, i32 1), !dbg !153
  %tmp5 = load i32, ptr @v2, align 4, !dbg !155
  %cmp6 = icmp ne i32 %call5, %tmp5, !dbg !156
  br i1 %cmp6, label %if.then7, label %if.end8, !dbg !157

if.then7:                                         ; preds = %if.end4
  br label %return, !dbg !158

if.end8:                                          ; preds = %if.end4
  %call9 = call float @"F::get2(int)"(ptr %f, i32 1), !dbg !159
  %tmp7 = load float, ptr @v4, align 4, !dbg !161
  %cmp10 = fcmp une float %call9, %tmp7, !dbg !162
  br i1 %cmp10, label %if.then11, label %if.end12, !dbg !163

if.then11:                                        ; preds = %if.end8
  br label %return, !dbg !164

if.end12:                                         ; preds = %if.end8
  br label %return, !dbg !165

return:                                           ; preds = %if.end12, %if.then11, %if.then7, %if.then3, %if.then
  %retval.0 = phi i1 [ false, %if.then ], [ false, %if.then3 ], [ false, %if.then7 ], [ false, %if.then11 ], [ true, %if.end12 ], !dbg !166
  ret i1 %retval.0, !dbg !167
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind memory(read)
define i32 @"F::get1(int)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i) #0 align 2 !dbg !168 !intel.dtrans.func.type !169 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !170, metadata !DIExpression()), !dbg !171
  call void @llvm.dbg.value(metadata i32 %i, metadata !172, metadata !DIExpression()), !dbg !173
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !174
  %tmp = load ptr, ptr %f1, align 8, !dbg !174
  %call = call dereferenceable(8) ptr @"Arr<int*>::get(int)"(ptr %tmp, i32 %i), !dbg !175
  %tmp2 = load ptr, ptr %call, align 8, !dbg !175
  %tmp3 = load i32, ptr %tmp2, align 4, !dbg !176
  ret i32 %tmp3, !dbg !177
}

; Function Attrs: nounwind memory(read)
define float @"F::get2(int)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i) #0 align 2 !dbg !178 !intel.dtrans.func.type !179 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !180, metadata !DIExpression()), !dbg !181
  call void @llvm.dbg.value(metadata i32 %i, metadata !182, metadata !DIExpression()), !dbg !183
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !184
  %tmp = load ptr, ptr %f2, align 8, !dbg !184
  %call = call dereferenceable(8) ptr @"Arr<float*>::get(int)"(ptr %tmp, i32 %i), !dbg !185
  %tmp2 = load ptr, ptr %call, align 8, !dbg !185
  %tmp3 = load float, ptr %tmp2, align 4, !dbg !186
  ret float %tmp3, !dbg !187
}

define i32 @main() personality ptr @__gxx_personality_v0 !dbg !188 {
entry:
  %call = call ptr @_Znwm(i64 24), !dbg !191
  %tmp = bitcast ptr %call to ptr, !dbg !191
  invoke void @"F::F()"(ptr %tmp)
          to label %invoke.cont unwind label %lpad, !dbg !192

invoke.cont:                                      ; preds = %entry
  call void @llvm.dbg.value(metadata ptr %tmp, metadata !193, metadata !DIExpression()), !dbg !194
  call void @"F::put(int*, float*)"(ptr %tmp, ptr @v1, ptr @v3), !dbg !195
  %call1 = call i32 @"F::get1(int)"(ptr %tmp, i32 0), !dbg !196
  %tmp3 = load i32, ptr @v1, align 4, !dbg !198
  %cmp = icmp ne i32 %call1, %tmp3, !dbg !199
  br i1 %cmp, label %if.then, label %if.end, !dbg !200

if.then:                                          ; preds = %invoke.cont
  br label %return, !dbg !201

lpad:                                             ; preds = %entry
  %tmp4 = landingpad { ptr, i32 }
          cleanup, !dbg !202
  %tmp5 = extractvalue { ptr, i32 } %tmp4, 0, !dbg !202
  %tmp6 = extractvalue { ptr, i32 } %tmp4, 1, !dbg !202
  call void @_ZdlPv(ptr %call), !dbg !191
  br label %eh.resume, !dbg !191

if.end:                                           ; preds = %invoke.cont
  %call2 = call float @"F::get2(int)"(ptr %tmp, i32 0), !dbg !203
  %tmp8 = load float, ptr @v3, align 4, !dbg !205
  %cmp3 = fcmp une float %call2, %tmp8, !dbg !206
  br i1 %cmp3, label %if.then4, label %if.end5, !dbg !207

if.then4:                                         ; preds = %if.end
  br label %return, !dbg !208

if.end5:                                          ; preds = %if.end
  call void @"F::put(int*, float*)"(ptr %tmp, ptr @v2, ptr @v4), !dbg !209
  %call6 = call i32 @"F::get1(int)"(ptr %tmp, i32 0), !dbg !210
  %tmp11 = load i32, ptr @v1, align 4, !dbg !212
  %cmp7 = icmp ne i32 %call6, %tmp11, !dbg !213
  br i1 %cmp7, label %if.then8, label %if.end9, !dbg !214

if.then8:                                         ; preds = %if.end5
  br label %return, !dbg !215

if.end9:                                          ; preds = %if.end5
  %call10 = call float @"F::get2(int)"(ptr %tmp, i32 0), !dbg !216
  %tmp13 = load float, ptr @v3, align 4, !dbg !218
  %cmp11 = fcmp une float %call10, %tmp13, !dbg !219
  br i1 %cmp11, label %if.then12, label %if.end13, !dbg !220

if.then12:                                        ; preds = %if.end9
  br label %return, !dbg !221

if.end13:                                         ; preds = %if.end9
  %call14 = call i32 @"F::get1(int)"(ptr %tmp, i32 1), !dbg !222
  %tmp15 = load i32, ptr @v2, align 4, !dbg !224
  %cmp15 = icmp ne i32 %call14, %tmp15, !dbg !225
  br i1 %cmp15, label %if.then16, label %if.end17, !dbg !226

if.then16:                                        ; preds = %if.end13
  br label %return, !dbg !227

if.end17:                                         ; preds = %if.end13
  %call18 = call float @"F::get2(int)"(ptr %tmp, i32 1), !dbg !228
  %tmp17 = load float, ptr @v4, align 4, !dbg !230
  %cmp19 = fcmp une float %call18, %tmp17, !dbg !231
  br i1 %cmp19, label %if.then20, label %if.end21, !dbg !232

if.then20:                                        ; preds = %if.end17
  br label %return, !dbg !233

if.end21:                                         ; preds = %if.end17
  call void @"F::set1(int, int*)"(ptr %tmp, i32 0, ptr @v2), !dbg !234
  call void @"F::set2(int, float*)"(ptr %tmp, i32 0, ptr @v4), !dbg !235
  %call22 = call zeroext i1 @"check1(F*)"(ptr %tmp), !dbg !236
  br i1 %call22, label %if.end24, label %if.then23, !dbg !238

if.then23:                                        ; preds = %if.end21
  br label %return, !dbg !239

if.end24:                                         ; preds = %if.end21
  %call25 = call ptr @_Znwm(i64 24), !dbg !240
  %tmp21 = bitcast ptr %call25 to ptr, !dbg !240
  invoke void @"F::F(F const&)"(ptr %tmp21, ptr dereferenceable(24) %tmp)
          to label %invoke.cont27 unwind label %lpad26, !dbg !241

invoke.cont27:                                    ; preds = %if.end24
  call void @llvm.dbg.value(metadata ptr %tmp21, metadata !242, metadata !DIExpression()), !dbg !243
  %call28 = call zeroext i1 @"check1(F*)"(ptr %tmp21), !dbg !244
  br i1 %call28, label %if.end30, label %if.then29, !dbg !246

if.then29:                                        ; preds = %invoke.cont27
  br label %return, !dbg !247

lpad26:                                           ; preds = %if.end24
  %tmp24 = landingpad { ptr, i32 }
          cleanup, !dbg !202
  %tmp25 = extractvalue { ptr, i32 } %tmp24, 0, !dbg !202
  %tmp26 = extractvalue { ptr, i32 } %tmp24, 1, !dbg !202
  call void @_ZdlPv(ptr %call25), !dbg !240
  br label %eh.resume, !dbg !240

if.end30:                                         ; preds = %invoke.cont27
  %isnull = icmp eq ptr %tmp, null, !dbg !248
  br i1 %isnull, label %delete.end, label %delete.notnull, !dbg !248

delete.notnull:                                   ; preds = %if.end30
  call void @"F::~F()"(ptr %tmp), !dbg !248
  %tmp28 = bitcast ptr %tmp to ptr, !dbg !248
  call void @_ZdlPv(ptr %tmp28), !dbg !248
  br label %delete.end, !dbg !248

delete.end:                                       ; preds = %delete.notnull, %if.end30
  %isnull31 = icmp eq ptr %tmp21, null, !dbg !249
  br i1 %isnull31, label %delete.end33, label %delete.notnull32, !dbg !249

delete.notnull32:                                 ; preds = %delete.end
  call void @"F::~F()"(ptr %tmp21), !dbg !249
  %tmp30 = bitcast ptr %tmp21 to ptr, !dbg !249
  call void @_ZdlPv(ptr %tmp30), !dbg !249
  br label %delete.end33, !dbg !249

delete.end33:                                     ; preds = %delete.notnull32, %delete.end
  br label %return, !dbg !250

return:                                           ; preds = %delete.end33, %if.then29, %if.then23, %if.then20, %if.then16, %if.then12, %if.then8, %if.then4, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ -1, %if.then4 ], [ -1, %if.then8 ], [ -1, %if.then12 ], [ -1, %if.then16 ], [ -1, %if.then20 ], [ 0, %delete.end33 ], [ -1, %if.then29 ], [ -1, %if.then23 ], !dbg !251
  ret i32 %retval.0, !dbg !202

eh.resume:                                        ; preds = %lpad26, %lpad
  %exn.slot.0 = phi ptr [ %tmp25, %lpad26 ], [ %tmp5, %lpad ], !dbg !202
  %ehselector.slot.0 = phi i32 [ %tmp26, %lpad26 ], [ %tmp6, %lpad ], !dbg !202
  resume { ptr, i32 } undef, !dbg !191
}

declare !intel.dtrans.func.type !252 dso_local nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

define void @"F::F()"(ptr nocapture noundef "intel_dtrans_func_index"="1" %this) align 2 personality ptr @__gxx_personality_v0 !dbg !254 !intel.dtrans.func.type !255 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !256, metadata !DIExpression()), !dbg !257
  %call = call ptr @_Znwm(i64 32), !dbg !258
  %tmp = bitcast ptr %call to ptr, !dbg !258
  invoke void @"Arr<int*>::Arr(int, %struct.Mem*)"(ptr %tmp, i32 1, ptr null)
          to label %invoke.cont unwind label %lpad, !dbg !260

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !261
  store ptr %tmp, ptr %f1, align 8, !dbg !262
  %call2 = call ptr @_Znwm(i64 32), !dbg !263
  %tmp1 = bitcast ptr %call2 to ptr, !dbg !263
  invoke void @"Arr<float*>::Arr(int, %struct.Mem*)"(ptr %tmp1, i32 1, ptr null)
          to label %invoke.cont4 unwind label %lpad3, !dbg !264

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !265
  store ptr %tmp1, ptr %f2, align 8, !dbg !266
  ret void, !dbg !267

lpad:                                             ; preds = %entry
  %tmp2 = landingpad { ptr, i32 }
          cleanup, !dbg !268
  %tmp3 = extractvalue { ptr, i32 } %tmp2, 0, !dbg !268
  %tmp4 = extractvalue { ptr, i32 } %tmp2, 1, !dbg !268
  call void @_ZdlPv(ptr %call), !dbg !258
  br label %eh.resume, !dbg !258

lpad3:                                            ; preds = %invoke.cont
  %tmp5 = landingpad { ptr, i32 }
          cleanup, !dbg !268
  %tmp6 = extractvalue { ptr, i32 } %tmp5, 0, !dbg !268
  %tmp7 = extractvalue { ptr, i32 } %tmp5, 1, !dbg !268
  call void @_ZdlPv(ptr %call2), !dbg !263
  br label %eh.resume, !dbg !263

eh.resume:                                        ; preds = %lpad3, %lpad
  %exn.slot.0 = phi ptr [ %tmp6, %lpad3 ], [ %tmp3, %lpad ], !dbg !268
  %ehselector.slot.0 = phi i32 [ %tmp7, %lpad3 ], [ %tmp4, %lpad ], !dbg !268
  resume { ptr, i32 } undef, !dbg !258
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !269 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

define void @"F::put(int*, float*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %a, ptr "intel_dtrans_func_index"="3" %b) align 2 !dbg !270 !intel.dtrans.func.type !271 {
entry:
  %a.addr = alloca ptr, align 8, !intel_dtrans_type !272
  %b.addr = alloca ptr, align 8, !intel_dtrans_type !273
  call void @llvm.dbg.value(metadata ptr %this, metadata !274, metadata !DIExpression()), !dbg !275
  store ptr %a, ptr %a.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !276, metadata !DIExpression()), !dbg !277
  store ptr %b, ptr %b.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %b.addr, metadata !278, metadata !DIExpression()), !dbg !279
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !280
  %tmp = load ptr, ptr %f1, align 8, !dbg !280
  call void @"Arr<int*>::add(int* const&)"(ptr %tmp, ptr dereferenceable(8) %a.addr), !dbg !281
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !282
  %tmp1 = load ptr, ptr %f2, align 8, !dbg !282
  call void @"Arr<float*>::add(float* const&)"(ptr %tmp1, ptr dereferenceable(8) %b.addr), !dbg !283
  ret void, !dbg !284
}

; Function Attrs: nounwind
define void @"F::set1(int, int*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %a) #2 align 2 !dbg !285 !intel.dtrans.func.type !286 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !287, metadata !DIExpression()), !dbg !288
  call void @llvm.dbg.value(metadata i32 %i, metadata !289, metadata !DIExpression()), !dbg !290
  call void @llvm.dbg.value(metadata ptr %a, metadata !291, metadata !DIExpression()), !dbg !292
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !293
  %tmp = load ptr, ptr %f1, align 8, !dbg !293
  call void @"Arr<int*>::set(int, int*)"(ptr %tmp, i32 %i, ptr %a), !dbg !294
  ret void, !dbg !295
}

; Function Attrs: nounwind
define void @"F::set2(int, float*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %b) #2 align 2 !dbg !296 !intel.dtrans.func.type !297 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !298, metadata !DIExpression()), !dbg !299
  call void @llvm.dbg.value(metadata i32 %i, metadata !300, metadata !DIExpression()), !dbg !301
  call void @llvm.dbg.value(metadata ptr %b, metadata !302, metadata !DIExpression()), !dbg !303
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !304
  %tmp = load ptr, ptr %f2, align 8, !dbg !304
  call void @"Arr<float*>::set(int, float*)"(ptr %tmp, i32 %i, ptr %b), !dbg !305
  ret void, !dbg !306
}

define void @"F::F(F const&)"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(24) "intel_dtrans_func_index"="2" %f) align 2 personality ptr @__gxx_personality_v0 !dbg !307 !intel.dtrans.func.type !308 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !309, metadata !DIExpression()), !dbg !310
  call void @llvm.dbg.value(metadata ptr %f, metadata !311, metadata !DIExpression()), !dbg !312
  %call = call ptr @_Znwm(i64 32), !dbg !313
  %tmp = bitcast ptr %call to ptr, !dbg !313
  %f1 = getelementptr inbounds %class.F, ptr %f, i32 0, i32 0, !dbg !315
  %tmp2 = load ptr, ptr %f1, align 8, !dbg !315
  invoke void @"Arr<int*>::Arr(Arr<int*> const&)"(ptr %tmp, ptr dereferenceable(32) %tmp2)
          to label %invoke.cont unwind label %lpad, !dbg !316

invoke.cont:                                      ; preds = %entry
  %f12 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !317
  store ptr %tmp, ptr %f12, align 8, !dbg !318
  %call3 = call ptr @_Znwm(i64 32), !dbg !319
  %tmp3 = bitcast ptr %call3 to ptr, !dbg !319
  %f2 = getelementptr inbounds %class.F, ptr %f, i32 0, i32 1, !dbg !320
  %tmp5 = load ptr, ptr %f2, align 8, !dbg !320
  invoke void @"Arr<float*>::Arr(Arr<float*> const&)"(ptr %tmp3, ptr dereferenceable(32) %tmp5)
          to label %invoke.cont5 unwind label %lpad4, !dbg !321

invoke.cont5:                                     ; preds = %invoke.cont
  %f26 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !322
  store ptr %tmp3, ptr %f26, align 8, !dbg !323
  ret void, !dbg !324

lpad:                                             ; preds = %entry
  %tmp6 = landingpad { ptr, i32 }
          cleanup, !dbg !325
  %tmp7 = extractvalue { ptr, i32 } %tmp6, 0, !dbg !325
  %tmp8 = extractvalue { ptr, i32 } %tmp6, 1, !dbg !325
  call void @_ZdlPv(ptr %call), !dbg !313
  br label %eh.resume, !dbg !313

lpad4:                                            ; preds = %invoke.cont
  %tmp9 = landingpad { ptr, i32 }
          cleanup, !dbg !325
  %tmp10 = extractvalue { ptr, i32 } %tmp9, 0, !dbg !325
  %tmp11 = extractvalue { ptr, i32 } %tmp9, 1, !dbg !325
  call void @_ZdlPv(ptr %call3), !dbg !319
  br label %eh.resume, !dbg !319

eh.resume:                                        ; preds = %lpad4, %lpad
  %exn.slot.0 = phi ptr [ %tmp10, %lpad4 ], [ %tmp7, %lpad ], !dbg !325
  %ehselector.slot.0 = phi i32 [ %tmp11, %lpad4 ], [ %tmp8, %lpad ], !dbg !325
  resume { ptr, i32 } undef, !dbg !313
}

define void @"F::~F()"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this) align 2 !dbg !326 !intel.dtrans.func.type !327 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !328, metadata !DIExpression()), !dbg !329
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 0, !dbg !330
  %tmp = load ptr, ptr %f1, align 8, !dbg !330
  %isnull = icmp eq ptr %tmp, null, !dbg !332
  br i1 %isnull, label %delete.end, label %delete.notnull, !dbg !332

delete.notnull:                                   ; preds = %entry
  call void @"Arr<int*>::~Arr()"(ptr %tmp), !dbg !332
  %tmp1 = bitcast ptr %tmp to ptr, !dbg !332
  call void @_ZdlPv(ptr %tmp1), !dbg !332
  br label %delete.end, !dbg !332

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1, !dbg !333
  %tmp2 = load ptr, ptr %f2, align 8, !dbg !333
  %isnull2 = icmp eq ptr %tmp2, null, !dbg !334
  br i1 %isnull2, label %delete.end4, label %delete.notnull3, !dbg !334

delete.notnull3:                                  ; preds = %delete.end
  call void @"Arr<float*>::~Arr()"(ptr %tmp2), !dbg !334
  %tmp3 = bitcast ptr %tmp2 to ptr, !dbg !334
  call void @_ZdlPv(ptr %tmp3), !dbg !334
  br label %delete.end4, !dbg !334

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void, !dbg !335
}

; Function Attrs: nounwind memory(read)
define dereferenceable(8) "intel_dtrans_func_index"="1" ptr @"Arr<int*>::get(int)"(ptr nocapture readonly "intel_dtrans_func_index"="2" %this, i32 %i) #0 align 2 !dbg !336 !intel.dtrans.func.type !337 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !338, metadata !DIExpression()), !dbg !339
  call void @llvm.dbg.value(metadata i32 %i, metadata !340, metadata !DIExpression()), !dbg !341
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3, !dbg !342
  %tmp = load i32, ptr %size, align 8, !dbg !342
  %cmp = icmp ugt i32 %tmp, %i, !dbg !344
  br i1 %cmp, label %if.end, label %if.then, !dbg !345

if.then:                                          ; preds = %entry
  call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom3
  ret ptr %arrayidx4
}

; Function Attrs: nounwind memory(read)
define dereferenceable(8) "intel_dtrans_func_index"="1" ptr @"Arr<float*>::get(int)"(ptr nocapture readonly "intel_dtrans_func_index"="2" %this, i32 %i) #0 align 2 !dbg !346 !intel.dtrans.func.type !347 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !348, metadata !DIExpression()), !dbg !349
  call void @llvm.dbg.value(metadata i32 %i, metadata !350, metadata !DIExpression()), !dbg !351
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3, !dbg !352
  %tmp = load i32, ptr %size, align 8, !dbg !352
  %cmp = icmp ugt i32 %tmp, %i, !dbg !354
  br i1 %cmp, label %if.end, label %if.then, !dbg !355

if.then:                                          ; preds = %entry
  call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom3
  ret ptr %arrayidx4
}

define void @"Arr<int*>::Arr(int, %struct.Mem*)"(ptr nocapture writeonly "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !dbg !356 !intel.dtrans.func.type !357 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !358, metadata !DIExpression()), !dbg !359
  call void @llvm.dbg.value(metadata i32 %c, metadata !360, metadata !DIExpression()), !dbg !361
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0, !dbg !362
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  store i32 0, ptr %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define void @"Arr<float*>::Arr(int, %struct.Mem*)"(ptr nocapture writeonly "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !dbg !363 !intel.dtrans.func.type !364 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !365, metadata !DIExpression()), !dbg !366
  call void @llvm.dbg.value(metadata i32 %c, metadata !367, metadata !DIExpression()), !dbg !368
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0, !dbg !369
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  store i32 0, ptr %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !370 dso_local "intel_dtrans_func_index"="1" ptr @malloc(i64) #3

declare void @__cxa_rethrow()

define void @"Arr<int*>::add(int* const&)"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(8) "intel_dtrans_func_index"="2" %e) align 2 !dbg !371 !intel.dtrans.func.type !372 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !373, metadata !DIExpression()), !dbg !374
  call void @llvm.dbg.value(metadata ptr %e, metadata !375, metadata !DIExpression()), !dbg !376
  call void @"Arr<int*>::realloc(int)"(ptr %this, i32 1), !dbg !377
  %tmp2 = load ptr, ptr %e, align 8, !dbg !378
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2, !dbg !380
  %tmp3 = load ptr, ptr %base, align 8, !dbg !380
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3, !dbg !381
  %tmp4 = load i32, ptr %size, align 8, !dbg !381
  %idxprom = zext i32 %tmp4 to i64, !dbg !380
  %arrayidx = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom, !dbg !380
  store ptr %tmp2, ptr %arrayidx, align 8, !dbg !382
  %inc = add nsw i32 %tmp4, 1, !dbg !383
  store i32 %inc, ptr %size, align 8, !dbg !383
  ret void, !dbg !384
}

define void @"Arr<float*>::add(float* const&)"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(8) "intel_dtrans_func_index"="2" %e) align 2 !dbg !385 !intel.dtrans.func.type !386 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !387, metadata !DIExpression()), !dbg !388
  call void @llvm.dbg.value(metadata ptr %e, metadata !389, metadata !DIExpression()), !dbg !390
  call void @"Arr<float*>::realloc(int)"(ptr %this, i32 1), !dbg !391
  %tmp2 = load ptr, ptr %e, align 8, !dbg !392
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2, !dbg !394
  %tmp3 = load ptr, ptr %base, align 8, !dbg !394
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3, !dbg !395
  %tmp4 = load i32, ptr %size, align 8, !dbg !395
  %idxprom = zext i32 %tmp4 to i64, !dbg !394
  %arrayidx = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom, !dbg !394
  store ptr %tmp2, ptr %arrayidx, align 8, !dbg !396
  %inc = add nsw i32 %tmp4, 1, !dbg !397
  store i32 %inc, ptr %size, align 8, !dbg !397
  ret void, !dbg !398
}

define void @"Arr<int*>::realloc(int)"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) align 2 !dbg !399 !intel.dtrans.func.type !400 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !401, metadata !DIExpression()), !dbg !402
  call void @llvm.dbg.value(metadata i32 %inc, metadata !403, metadata !DIExpression()), !dbg !404
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3, !dbg !405
  %tmp = load i32, ptr %size, align 8, !dbg !405
  %add = add nsw i32 %tmp, 1, !dbg !407
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1, !dbg !408
  %tmp2 = load i32, ptr %capacity, align 8, !dbg !408
  %cmp = icmp ugt i32 %add, %tmp2, !dbg !409
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias ptr @malloc(i64 %mul8)
  %i2 = bitcast ptr %call to ptr
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i4 = bitcast ptr %base14 to ptr
  %i5 = load ptr, ptr %i4, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %i4, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = load ptr, ptr %ptridx, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  store ptr %i6, ptr %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void, !dbg !410
}

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !411 dso_local void @free(ptr "intel_dtrans_func_index"="1") #4

define void @"Arr<float*>::realloc(int)"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) align 2 !dbg !412 !intel.dtrans.func.type !413 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !414, metadata !DIExpression()), !dbg !415
  call void @llvm.dbg.value(metadata i32 %inc, metadata !416, metadata !DIExpression()), !dbg !417
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3, !dbg !418
  %tmp = load i32, ptr %size, align 8, !dbg !418
  %add = add nsw i32 %tmp, 1, !dbg !420
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1, !dbg !421
  %tmp2 = load i32, ptr %capacity, align 8, !dbg !421
  %cmp = icmp ugt i32 %add, %tmp2, !dbg !422
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias ptr @malloc(i64 %mul8)
  %i2 = bitcast ptr %call to ptr
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i4 = bitcast ptr %base14 to ptr
  %i5 = load ptr, ptr %i4, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %i4, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = load ptr, ptr %ptridx, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  store ptr %i6, ptr %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void, !dbg !423
}

; Function Attrs: nounwind
define void @"Arr<int*>::set(int, int*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) #2 align 2 !dbg !424 !intel.dtrans.func.type !425 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !426, metadata !DIExpression()), !dbg !427
  call void @llvm.dbg.value(metadata i32 %i, metadata !428, metadata !DIExpression()), !dbg !429
  call void @llvm.dbg.value(metadata ptr %val, metadata !430, metadata !DIExpression()), !dbg !431
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3, !dbg !432
  %tmp = load i32, ptr %size, align 8, !dbg !432
  %cmp = icmp ugt i32 %tmp, %i, !dbg !434
  br i1 %cmp, label %if.end, label %if.then, !dbg !435

if.then:                                          ; preds = %entry
  call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2, !dbg !436
  %tmp5 = load ptr, ptr %base2, align 8, !dbg !436
  %idxprom3 = zext i32 %i to i64, !dbg !436
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp5, i64 %idxprom3, !dbg !436
  store ptr %val, ptr %arrayidx4, align 8, !dbg !437
  ret void, !dbg !438
}

; Function Attrs: nounwind
define void @"Arr<float*>::set(int, float*)"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) #2 align 2 !dbg !439 !intel.dtrans.func.type !440 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !441, metadata !DIExpression()), !dbg !442
  call void @llvm.dbg.value(metadata i32 %i, metadata !443, metadata !DIExpression()), !dbg !444
  call void @llvm.dbg.value(metadata ptr %val, metadata !445, metadata !DIExpression()), !dbg !446
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3, !dbg !447
  %tmp = load i32, ptr %size, align 8, !dbg !447
  %cmp = icmp ugt i32 %tmp, %i, !dbg !449
  br i1 %cmp, label %if.end, label %if.then, !dbg !450

if.then:                                          ; preds = %entry
  call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2, !dbg !451
  %tmp5 = load ptr, ptr %base2, align 8, !dbg !451
  %idxprom3 = zext i32 %i to i64, !dbg !451
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp5, i64 %idxprom3, !dbg !451
  store ptr %val, ptr %arrayidx4, align 8, !dbg !452
  ret void, !dbg !453
}

define void @"Arr<int*>::Arr(Arr<int*> const&)"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(32) "intel_dtrans_func_index"="2" %A) align 2 !dbg !454 !intel.dtrans.func.type !455 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !456, metadata !DIExpression()), !dbg !457
  call void @llvm.dbg.value(metadata ptr %A, metadata !458, metadata !DIExpression()), !dbg !459
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0, !dbg !460
  %flag2 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 3
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 4
  %i3 = load ptr, ptr %mem5, align 8
  store ptr %i3, ptr %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i4 = bitcast ptr %base to ptr
  store ptr %call, ptr %i4, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 2
  %i5 = load ptr, ptr %base13, align 8
  %i6 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void, !dbg !462

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load ptr, ptr %ptridx, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i6, i64 %indvars.iv
  store ptr %i7, ptr %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @"Arr<float*>::Arr(Arr<float*> const&)"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(32) "intel_dtrans_func_index"="2" %A) align 2 !dbg !463 !intel.dtrans.func.type !464 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !465, metadata !DIExpression()), !dbg !466
  call void @llvm.dbg.value(metadata ptr %A, metadata !467, metadata !DIExpression()), !dbg !468
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0, !dbg !469
  %flag2 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 3
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %mem = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 4
  %i3 = load ptr, ptr %mem5, align 8
  store ptr %i3, ptr %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i4 = bitcast ptr %base to ptr
  store ptr %call, ptr %i4, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 2
  %i5 = load ptr, ptr %base13, align 8
  %i6 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void, !dbg !471

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load ptr, ptr %ptridx, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i6, i64 %indvars.iv
  store ptr %i7, ptr %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @"Arr<int*>::~Arr()"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this) align 2 !dbg !472 !intel.dtrans.func.type !473 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !474, metadata !DIExpression()), !dbg !475
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2, !dbg !476
  %tmp = load ptr, ptr %base, align 8, !dbg !476
  %tmp1 = bitcast ptr %tmp to ptr, !dbg !476
  call void @free(ptr %tmp1), !dbg !478
  ret void, !dbg !479
}

define void @"Arr<float*>::~Arr()"(ptr nocapture readonly "intel_dtrans_func_index"="1" %this) align 2 !dbg !480 !intel.dtrans.func.type !481 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !482, metadata !DIExpression()), !dbg !483
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2, !dbg !484
  %tmp = load ptr, ptr %base, align 8, !dbg !484
  %tmp1 = bitcast ptr %tmp to ptr, !dbg !484
  call void @free(ptr %tmp1), !dbg !486
  ret void, !dbg !487
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

attributes #0 = { nounwind memory(read) }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nounwind }
attributes #3 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #4 = { allockind("free") "alloc-family"="malloc" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!19, !20, !21}
!llvm.dbg.intel.emit_class_debug_always = !{!22}
!llvm.ident = !{!23}
!intel.dtrans.types = !{!24, !28, !32, !35}

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
!24 = !{!"S", %class.F zeroinitializer, i32 3, !25, !26, !27}
!25 = !{%struct.Arr zeroinitializer, i32 1}
!26 = !{%struct.Arr.0 zeroinitializer, i32 1}
!27 = !{%struct.Mem zeroinitializer, i32 1}
!28 = !{!"S", %struct.Arr zeroinitializer, i32 5, !29, !30, !31, !30, !27}
!29 = !{i8 0, i32 0}
!30 = !{i32 0, i32 0}
!31 = !{i32 0, i32 2}
!32 = !{!"S", %struct.Mem zeroinitializer, i32 1, !33}
!33 = !{!34, i32 2}
!34 = !{!"F", i1 true, i32 0, !30}
!35 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !29, !30, !36, !30, !27}
!36 = !{float 0.000000e+00, i32 2}
!37 = distinct !DISubprogram(name: "check1", linkageName: "check1(F*)", scope: !3, file: !3, line: 101, type: !38, scopeLine: 101, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!38 = !DISubroutineType(types: !39)
!39 = !{!40, !41}
!40 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!42 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "F", file: !3, line: 62, size: 128, flags: DIFlagTypePassByReference, elements: !43, identifier: "typeinfo name for F")
!43 = !{!44, !78, !112, !118, !121, !124, !127, !130, !133, !136}
!44 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !42, file: !3, line: 64, baseType: !45, size: 64, flags: DIFlagPublic)
!45 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64)
!46 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Arr<int *>", file: !3, line: 6, size: 192, flags: DIFlagTypePassByReference, elements: !47, templateParams: !76, identifier: "typeinfo name for Arr<int*>")
!47 = !{!48, !49, !50, !51, !56, !59, !62, !63, !68, !73}
!48 = !DIDerivedType(tag: DW_TAG_member, name: "capacity", scope: !46, file: !3, line: 7, baseType: !8, size: 32)
!49 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !46, file: !3, line: 8, baseType: !6, size: 64, offset: 64)
!50 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !46, file: !3, line: 9, baseType: !8, size: 32, offset: 128)
!51 = !DISubprogram(name: "get", linkageName: "Arr<int*>::get(int)", scope: !46, file: !3, line: 10, type: !52, scopeLine: 10, flags: DIFlagPrototyped, spFlags: 0)
!52 = !DISubroutineType(types: !53)
!53 = !{!54, !55, !8}
!54 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !7, size: 64)
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!56 = !DISubprogram(name: "set", linkageName: "Arr<int*>::set(int, int*)", scope: !46, file: !3, line: 15, type: !57, scopeLine: 15, flags: DIFlagPrototyped, spFlags: 0)
!57 = !DISubroutineType(types: !58)
!58 = !{null, !55, !8, !7}
!59 = !DISubprogram(name: "Arr", scope: !46, file: !3, line: 21, type: !60, scopeLine: 21, flags: DIFlagPrototyped, spFlags: 0)
!60 = !DISubroutineType(types: !61)
!61 = !{null, !55, !8}
!62 = !DISubprogram(name: "realloc", linkageName: "Arr<int*>::realloc(int)", scope: !46, file: !3, line: 24, type: !60, scopeLine: 24, flags: DIFlagPrototyped, spFlags: 0)
!63 = !DISubprogram(name: "add", linkageName: "Arr<int*>::add(int* const&)", scope: !46, file: !3, line: 36, type: !64, scopeLine: 36, flags: DIFlagPrototyped, spFlags: 0)
!64 = !DISubroutineType(types: !65)
!65 = !{null, !55, !66}
!66 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !67, size: 64)
!67 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!68 = !DISubprogram(name: "Arr", scope: !46, file: !3, line: 46, type: !69, scopeLine: 46, flags: DIFlagPrototyped, spFlags: 0)
!69 = !DISubroutineType(types: !70)
!70 = !{null, !55, !71}
!71 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !72, size: 64)
!72 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !46)
!73 = !DISubprogram(name: "~Arr", scope: !46, file: !3, line: 59, type: !74, scopeLine: 59, flags: DIFlagPrototyped, spFlags: 0)
!74 = !DISubroutineType(types: !75)
!75 = !{null, !55}
!76 = !{!77}
!77 = !DITemplateTypeParameter(name: "S", type: !7)
!78 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !42, file: !3, line: 65, baseType: !79, size: 64, offset: 64, flags: DIFlagPublic)
!79 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !80, size: 64)
!80 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Arr<float *>", file: !3, line: 6, size: 192, flags: DIFlagTypePassByReference, elements: !81, templateParams: !110, identifier: "typeinfo name for Arr<float*>")
!81 = !{!82, !83, !84, !85, !90, !93, !96, !97, !102, !107}
!82 = !DIDerivedType(tag: DW_TAG_member, name: "capacity", scope: !80, file: !3, line: 7, baseType: !8, size: 32)
!83 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !80, file: !3, line: 8, baseType: !9, size: 64, offset: 64)
!84 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !80, file: !3, line: 9, baseType: !8, size: 32, offset: 128)
!85 = !DISubprogram(name: "get", linkageName: "Arr<float*>::get(int)", scope: !80, file: !3, line: 10, type: !86, scopeLine: 10, flags: DIFlagPrototyped, spFlags: 0)
!86 = !DISubroutineType(types: !87)
!87 = !{!88, !89, !8}
!88 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !10, size: 64)
!89 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !80, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!90 = !DISubprogram(name: "set", linkageName: "Arr<float*>::set(int, float*)", scope: !80, file: !3, line: 15, type: !91, scopeLine: 15, flags: DIFlagPrototyped, spFlags: 0)
!91 = !DISubroutineType(types: !92)
!92 = !{null, !89, !8, !10}
!93 = !DISubprogram(name: "Arr", scope: !80, file: !3, line: 21, type: !94, scopeLine: 21, flags: DIFlagPrototyped, spFlags: 0)
!94 = !DISubroutineType(types: !95)
!95 = !{null, !89, !8}
!96 = !DISubprogram(name: "realloc", linkageName: "Arr<float*>::realloc(int)", scope: !80, file: !3, line: 24, type: !94, scopeLine: 24, flags: DIFlagPrototyped, spFlags: 0)
!97 = !DISubprogram(name: "add", linkageName: "Arr<float*>::add(float* const&)", scope: !80, file: !3, line: 36, type: !98, scopeLine: 36, flags: DIFlagPrototyped, spFlags: 0)
!98 = !DISubroutineType(types: !99)
!99 = !{null, !89, !100}
!100 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !101, size: 64)
!101 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !10)
!102 = !DISubprogram(name: "Arr", scope: !80, file: !3, line: 46, type: !103, scopeLine: 46, flags: DIFlagPrototyped, spFlags: 0)
!103 = !DISubroutineType(types: !104)
!104 = !{null, !89, !105}
!105 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !106, size: 64)
!106 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !80)
!107 = !DISubprogram(name: "~Arr", scope: !80, file: !3, line: 59, type: !108, scopeLine: 59, flags: DIFlagPrototyped, spFlags: 0)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !89}
!110 = !{!111}
!111 = !DITemplateTypeParameter(name: "S", type: !10)
!112 = !DISubprogram(name: "F", scope: !42, file: !3, line: 66, type: !113, scopeLine: 66, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!113 = !DISubroutineType(types: !114)
!114 = !{null, !115, !116}
!115 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!116 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !117, size: 64)
!117 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !42)
!118 = !DISubprogram(name: "put", linkageName: "F::put(int*, float*)", scope: !42, file: !3, line: 70, type: !119, scopeLine: 70, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!119 = !DISubroutineType(types: !120)
!120 = !{null, !115, !7, !10}
!121 = !DISubprogram(name: "set1", linkageName: "F::set1(int, int*)", scope: !42, file: !3, line: 74, type: !122, scopeLine: 74, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!122 = !DISubroutineType(types: !123)
!123 = !{null, !115, !8, !7}
!124 = !DISubprogram(name: "set2", linkageName: "F::set2(int, float*)", scope: !42, file: !3, line: 77, type: !125, scopeLine: 77, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!125 = !DISubroutineType(types: !126)
!126 = !{null, !115, !8, !10}
!127 = !DISubprogram(name: "get1", linkageName: "F::get1(int)", scope: !42, file: !3, line: 80, type: !128, scopeLine: 80, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!128 = !DISubroutineType(types: !129)
!129 = !{!8, !115, !8}
!130 = !DISubprogram(name: "get2", linkageName: "F::get2(int)", scope: !42, file: !3, line: 83, type: !131, scopeLine: 83, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!131 = !DISubroutineType(types: !132)
!132 = !{!11, !115, !8}
!133 = !DISubprogram(name: "F", scope: !42, file: !3, line: 86, type: !134, scopeLine: 86, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!134 = !DISubroutineType(types: !135)
!135 = !{null, !115}
!136 = !DISubprogram(name: "~F", scope: !42, file: !3, line: 90, type: !134, scopeLine: 90, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!137 = distinct !{!138}
!138 = !{%class.F zeroinitializer, i32 1}
!139 = !DILocalVariable(name: "f", arg: 1, scope: !37, file: !3, line: 101, type: !41)
!140 = !DILocation(line: 101, column: 16, scope: !37)
!141 = !DILocation(line: 102, column: 10, scope: !142)
!142 = distinct !DILexicalBlock(scope: !37, file: !3, line: 102, column: 7)
!143 = !DILocation(line: 102, column: 21, scope: !142)
!144 = !DILocation(line: 102, column: 18, scope: !142)
!145 = !DILocation(line: 102, column: 7, scope: !37)
!146 = !DILocation(line: 103, column: 5, scope: !142)
!147 = !DILocation(line: 104, column: 10, scope: !148)
!148 = distinct !DILexicalBlock(scope: !37, file: !3, line: 104, column: 7)
!149 = !DILocation(line: 104, column: 21, scope: !148)
!150 = !DILocation(line: 104, column: 18, scope: !148)
!151 = !DILocation(line: 104, column: 7, scope: !37)
!152 = !DILocation(line: 105, column: 5, scope: !148)
!153 = !DILocation(line: 106, column: 10, scope: !154)
!154 = distinct !DILexicalBlock(scope: !37, file: !3, line: 106, column: 7)
!155 = !DILocation(line: 106, column: 21, scope: !154)
!156 = !DILocation(line: 106, column: 18, scope: !154)
!157 = !DILocation(line: 106, column: 7, scope: !37)
!158 = !DILocation(line: 107, column: 5, scope: !154)
!159 = !DILocation(line: 108, column: 10, scope: !160)
!160 = distinct !DILexicalBlock(scope: !37, file: !3, line: 108, column: 7)
!161 = !DILocation(line: 108, column: 21, scope: !160)
!162 = !DILocation(line: 108, column: 18, scope: !160)
!163 = !DILocation(line: 108, column: 7, scope: !37)
!164 = !DILocation(line: 109, column: 5, scope: !160)
!165 = !DILocation(line: 110, column: 3, scope: !37)
!166 = !DILocation(line: 0, scope: !37)
!167 = !DILocation(line: 111, column: 1, scope: !37)
!168 = distinct !DISubprogram(name: "get1", linkageName: "F::get1(int)", scope: !42, file: !3, line: 80, type: !128, scopeLine: 80, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !127, retainedNodes: !4)
!169 = distinct !{!138}
!170 = !DILocalVariable(name: "this", arg: 1, scope: !168, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!171 = !DILocation(line: 0, scope: !168)
!172 = !DILocalVariable(name: "i", arg: 2, scope: !168, file: !3, line: 80, type: !8)
!173 = !DILocation(line: 80, column: 16, scope: !168)
!174 = !DILocation(line: 81, column: 14, scope: !168)
!175 = !DILocation(line: 81, column: 18, scope: !168)
!176 = !DILocation(line: 81, column: 12, scope: !168)
!177 = !DILocation(line: 81, column: 5, scope: !168)
!178 = distinct !DISubprogram(name: "get2", linkageName: "F::get2(int)", scope: !42, file: !3, line: 83, type: !131, scopeLine: 83, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !130, retainedNodes: !4)
!179 = distinct !{!138}
!180 = !DILocalVariable(name: "this", arg: 1, scope: !178, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!181 = !DILocation(line: 0, scope: !178)
!182 = !DILocalVariable(name: "i", arg: 2, scope: !178, file: !3, line: 83, type: !8)
!183 = !DILocation(line: 83, column: 18, scope: !178)
!184 = !DILocation(line: 84, column: 14, scope: !178)
!185 = !DILocation(line: 84, column: 18, scope: !178)
!186 = !DILocation(line: 84, column: 12, scope: !178)
!187 = !DILocation(line: 84, column: 5, scope: !178)
!188 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 113, type: !189, scopeLine: 113, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!189 = !DISubroutineType(types: !190)
!190 = !{!8}
!191 = !DILocation(line: 114, column: 10, scope: !188)
!192 = !DILocation(line: 114, column: 14, scope: !188)
!193 = !DILocalVariable(name: "f", scope: !188, file: !3, line: 114, type: !41)
!194 = !DILocation(line: 114, column: 6, scope: !188)
!195 = !DILocation(line: 115, column: 6, scope: !188)
!196 = !DILocation(line: 116, column: 10, scope: !197)
!197 = distinct !DILexicalBlock(scope: !188, file: !3, line: 116, column: 7)
!198 = !DILocation(line: 116, column: 21, scope: !197)
!199 = !DILocation(line: 116, column: 18, scope: !197)
!200 = !DILocation(line: 116, column: 7, scope: !188)
!201 = !DILocation(line: 117, column: 5, scope: !197)
!202 = !DILocation(line: 143, column: 1, scope: !188)
!203 = !DILocation(line: 118, column: 10, scope: !204)
!204 = distinct !DILexicalBlock(scope: !188, file: !3, line: 118, column: 7)
!205 = !DILocation(line: 118, column: 21, scope: !204)
!206 = !DILocation(line: 118, column: 18, scope: !204)
!207 = !DILocation(line: 118, column: 7, scope: !188)
!208 = !DILocation(line: 119, column: 5, scope: !204)
!209 = !DILocation(line: 121, column: 6, scope: !188)
!210 = !DILocation(line: 122, column: 10, scope: !211)
!211 = distinct !DILexicalBlock(scope: !188, file: !3, line: 122, column: 7)
!212 = !DILocation(line: 122, column: 21, scope: !211)
!213 = !DILocation(line: 122, column: 18, scope: !211)
!214 = !DILocation(line: 122, column: 7, scope: !188)
!215 = !DILocation(line: 123, column: 5, scope: !211)
!216 = !DILocation(line: 124, column: 10, scope: !217)
!217 = distinct !DILexicalBlock(scope: !188, file: !3, line: 124, column: 7)
!218 = !DILocation(line: 124, column: 21, scope: !217)
!219 = !DILocation(line: 124, column: 18, scope: !217)
!220 = !DILocation(line: 124, column: 7, scope: !188)
!221 = !DILocation(line: 125, column: 5, scope: !217)
!222 = !DILocation(line: 126, column: 10, scope: !223)
!223 = distinct !DILexicalBlock(scope: !188, file: !3, line: 126, column: 7)
!224 = !DILocation(line: 126, column: 21, scope: !223)
!225 = !DILocation(line: 126, column: 18, scope: !223)
!226 = !DILocation(line: 126, column: 7, scope: !188)
!227 = !DILocation(line: 127, column: 5, scope: !223)
!228 = !DILocation(line: 128, column: 10, scope: !229)
!229 = distinct !DILexicalBlock(scope: !188, file: !3, line: 128, column: 7)
!230 = !DILocation(line: 128, column: 21, scope: !229)
!231 = !DILocation(line: 128, column: 18, scope: !229)
!232 = !DILocation(line: 128, column: 7, scope: !188)
!233 = !DILocation(line: 129, column: 5, scope: !229)
!234 = !DILocation(line: 131, column: 6, scope: !188)
!235 = !DILocation(line: 132, column: 6, scope: !188)
!236 = !DILocation(line: 133, column: 8, scope: !237)
!237 = distinct !DILexicalBlock(scope: !188, file: !3, line: 133, column: 7)
!238 = !DILocation(line: 133, column: 7, scope: !188)
!239 = !DILocation(line: 134, column: 5, scope: !237)
!240 = !DILocation(line: 136, column: 11, scope: !188)
!241 = !DILocation(line: 136, column: 15, scope: !188)
!242 = !DILocalVariable(name: "f1", scope: !188, file: !3, line: 136, type: !41)
!243 = !DILocation(line: 136, column: 6, scope: !188)
!244 = !DILocation(line: 137, column: 8, scope: !245)
!245 = distinct !DILexicalBlock(scope: !188, file: !3, line: 137, column: 7)
!246 = !DILocation(line: 137, column: 7, scope: !188)
!247 = !DILocation(line: 138, column: 5, scope: !245)
!248 = !DILocation(line: 140, column: 3, scope: !188)
!249 = !DILocation(line: 141, column: 3, scope: !188)
!250 = !DILocation(line: 142, column: 3, scope: !188)
!251 = !DILocation(line: 0, scope: !188)
!252 = distinct !{!253}
!253 = !{i8 0, i32 1}
!254 = distinct !DISubprogram(name: "F", linkageName: "F::F()", scope: !42, file: !3, line: 86, type: !134, scopeLine: 86, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !133, retainedNodes: !4)
!255 = distinct !{!138}
!256 = !DILocalVariable(name: "this", arg: 1, scope: !254, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!257 = !DILocation(line: 0, scope: !254)
!258 = !DILocation(line: 87, column: 10, scope: !259)
!259 = distinct !DILexicalBlock(scope: !254, file: !3, line: 86, column: 7)
!260 = !DILocation(line: 87, column: 14, scope: !259)
!261 = !DILocation(line: 87, column: 5, scope: !259)
!262 = !DILocation(line: 87, column: 8, scope: !259)
!263 = !DILocation(line: 88, column: 10, scope: !259)
!264 = !DILocation(line: 88, column: 14, scope: !259)
!265 = !DILocation(line: 88, column: 5, scope: !259)
!266 = !DILocation(line: 88, column: 8, scope: !259)
!267 = !DILocation(line: 89, column: 3, scope: !254)
!268 = !DILocation(line: 89, column: 3, scope: !259)
!269 = distinct !{!253}
!270 = distinct !DISubprogram(name: "put", linkageName: "F::put(int*, float*)", scope: !42, file: !3, line: 70, type: !119, scopeLine: 70, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !118, retainedNodes: !4)
!271 = distinct !{!138, !272, !273}
!272 = !{i32 0, i32 1}
!273 = !{float 0.000000e+00, i32 1}
!274 = !DILocalVariable(name: "this", arg: 1, scope: !270, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!275 = !DILocation(line: 0, scope: !270)
!276 = !DILocalVariable(name: "a", arg: 2, scope: !270, file: !3, line: 70, type: !7)
!277 = !DILocation(line: 70, column: 17, scope: !270)
!278 = !DILocalVariable(name: "b", arg: 3, scope: !270, file: !3, line: 70, type: !10)
!279 = !DILocation(line: 70, column: 27, scope: !270)
!280 = !DILocation(line: 71, column: 5, scope: !270)
!281 = !DILocation(line: 71, column: 9, scope: !270)
!282 = !DILocation(line: 72, column: 5, scope: !270)
!283 = !DILocation(line: 72, column: 9, scope: !270)
!284 = !DILocation(line: 73, column: 3, scope: !270)
!285 = distinct !DISubprogram(name: "set1", linkageName: "F::set1(int, int*)", scope: !42, file: !3, line: 74, type: !122, scopeLine: 74, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !121, retainedNodes: !4)
!286 = distinct !{!138, !272}
!287 = !DILocalVariable(name: "this", arg: 1, scope: !285, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!288 = !DILocation(line: 0, scope: !285)
!289 = !DILocalVariable(name: "i", arg: 2, scope: !285, file: !3, line: 74, type: !8)
!290 = !DILocation(line: 74, column: 17, scope: !285)
!291 = !DILocalVariable(name: "a", arg: 3, scope: !285, file: !3, line: 74, type: !7)
!292 = !DILocation(line: 74, column: 25, scope: !285)
!293 = !DILocation(line: 75, column: 5, scope: !285)
!294 = !DILocation(line: 75, column: 9, scope: !285)
!295 = !DILocation(line: 76, column: 3, scope: !285)
!296 = distinct !DISubprogram(name: "set2", linkageName: "F::set2(int, float*)", scope: !42, file: !3, line: 77, type: !125, scopeLine: 77, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !124, retainedNodes: !4)
!297 = distinct !{!138, !273}
!298 = !DILocalVariable(name: "this", arg: 1, scope: !296, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!299 = !DILocation(line: 0, scope: !296)
!300 = !DILocalVariable(name: "i", arg: 2, scope: !296, file: !3, line: 77, type: !8)
!301 = !DILocation(line: 77, column: 17, scope: !296)
!302 = !DILocalVariable(name: "b", arg: 3, scope: !296, file: !3, line: 77, type: !10)
!303 = !DILocation(line: 77, column: 27, scope: !296)
!304 = !DILocation(line: 78, column: 5, scope: !296)
!305 = !DILocation(line: 78, column: 9, scope: !296)
!306 = !DILocation(line: 79, column: 3, scope: !296)
!307 = distinct !DISubprogram(name: "F", linkageName: "F::F(F const&)", scope: !42, file: !3, line: 66, type: !113, scopeLine: 66, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !112, retainedNodes: !4)
!308 = distinct !{!138, !138}
!309 = !DILocalVariable(name: "this", arg: 1, scope: !307, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!310 = !DILocation(line: 0, scope: !307)
!311 = !DILocalVariable(name: "f", arg: 2, scope: !307, file: !3, line: 66, type: !116)
!312 = !DILocation(line: 66, column: 14, scope: !307)
!313 = !DILocation(line: 67, column: 10, scope: !314)
!314 = distinct !DILexicalBlock(scope: !307, file: !3, line: 66, column: 17)
!315 = !DILocation(line: 67, column: 28, scope: !314)
!316 = !DILocation(line: 67, column: 14, scope: !314)
!317 = !DILocation(line: 67, column: 5, scope: !314)
!318 = !DILocation(line: 67, column: 8, scope: !314)
!319 = !DILocation(line: 68, column: 10, scope: !314)
!320 = !DILocation(line: 68, column: 30, scope: !314)
!321 = !DILocation(line: 68, column: 14, scope: !314)
!322 = !DILocation(line: 68, column: 5, scope: !314)
!323 = !DILocation(line: 68, column: 8, scope: !314)
!324 = !DILocation(line: 69, column: 3, scope: !307)
!325 = !DILocation(line: 69, column: 3, scope: !314)
!326 = distinct !DISubprogram(name: "~F", linkageName: "F::~F()", scope: !42, file: !3, line: 90, type: !134, scopeLine: 90, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !136, retainedNodes: !4)
!327 = distinct !{!138}
!328 = !DILocalVariable(name: "this", arg: 1, scope: !326, type: !41, flags: DIFlagArtificial | DIFlagObjectPointer)
!329 = !DILocation(line: 0, scope: !326)
!330 = !DILocation(line: 91, column: 12, scope: !331)
!331 = distinct !DILexicalBlock(scope: !326, file: !3, line: 90, column: 8)
!332 = !DILocation(line: 91, column: 5, scope: !331)
!333 = !DILocation(line: 92, column: 12, scope: !331)
!334 = !DILocation(line: 92, column: 5, scope: !331)
!335 = !DILocation(line: 93, column: 3, scope: !326)
!336 = distinct !DISubprogram(name: "get", linkageName: "Arr<int*>::get(int)", scope: !46, file: !3, line: 10, type: !52, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !51, retainedNodes: !4)
!337 = distinct !{!31, !25}
!338 = !DILocalVariable(name: "this", arg: 1, scope: !336, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!339 = !DILocation(line: 0, scope: !336)
!340 = !DILocalVariable(name: "i", arg: 2, scope: !336, file: !3, line: 10, type: !8)
!341 = !DILocation(line: 10, column: 14, scope: !336)
!342 = !DILocation(line: 11, column: 9, scope: !343)
!343 = distinct !DILexicalBlock(scope: !336, file: !3, line: 11, column: 9)
!344 = !DILocation(line: 11, column: 19, scope: !343)
!345 = !DILocation(line: 11, column: 9, scope: !336)
!346 = distinct !DISubprogram(name: "get", linkageName: "Arr<float*>::get(int)", scope: !80, file: !3, line: 10, type: !86, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !85, retainedNodes: !4)
!347 = distinct !{!36, !26}
!348 = !DILocalVariable(name: "this", arg: 1, scope: !346, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!349 = !DILocation(line: 0, scope: !346)
!350 = !DILocalVariable(name: "i", arg: 2, scope: !346, file: !3, line: 10, type: !8)
!351 = !DILocation(line: 10, column: 14, scope: !346)
!352 = !DILocation(line: 11, column: 9, scope: !353)
!353 = distinct !DILexicalBlock(scope: !346, file: !3, line: 11, column: 9)
!354 = !DILocation(line: 11, column: 19, scope: !353)
!355 = !DILocation(line: 11, column: 9, scope: !346)
!356 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<int*>::Arr(int, %struct.Mem*)", scope: !46, file: !3, line: 21, type: !60, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !59, retainedNodes: !4)
!357 = distinct !{!25, !27}
!358 = !DILocalVariable(name: "this", arg: 1, scope: !356, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!359 = !DILocation(line: 0, scope: !356)
!360 = !DILocalVariable(name: "c", arg: 2, scope: !356, file: !3, line: 21, type: !8)
!361 = !DILocation(line: 21, column: 11, scope: !356)
!362 = !DILocation(line: 21, column: 20, scope: !356)
!363 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<float*>::Arr(int, %struct.Mem*)", scope: !80, file: !3, line: 21, type: !94, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !93, retainedNodes: !4)
!364 = distinct !{!26, !27}
!365 = !DILocalVariable(name: "this", arg: 1, scope: !363, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!366 = !DILocation(line: 0, scope: !363)
!367 = !DILocalVariable(name: "c", arg: 2, scope: !363, file: !3, line: 21, type: !8)
!368 = !DILocation(line: 21, column: 11, scope: !363)
!369 = !DILocation(line: 21, column: 20, scope: !363)
!370 = distinct !{!253}
!371 = distinct !DISubprogram(name: "add", linkageName: "Arr<int*>::add(int* const&)", scope: !46, file: !3, line: 36, type: !64, scopeLine: 36, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !63, retainedNodes: !4)
!372 = distinct !{!25, !31}
!373 = !DILocalVariable(name: "this", arg: 1, scope: !371, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!374 = !DILocation(line: 0, scope: !371)
!375 = !DILocalVariable(name: "e", arg: 2, scope: !371, file: !3, line: 36, type: !66)
!376 = !DILocation(line: 36, column: 21, scope: !371)
!377 = !DILocation(line: 37, column: 5, scope: !371)
!378 = !DILocation(line: 39, column: 9, scope: !379)
!379 = distinct !DILexicalBlock(scope: !371, file: !3, line: 39, column: 9)
!380 = !DILocation(line: 40, column: 7, scope: !379)
!381 = !DILocation(line: 40, column: 16, scope: !379)
!382 = !DILocation(line: 40, column: 22, scope: !379)
!383 = !DILocation(line: 44, column: 5, scope: !371)
!384 = !DILocation(line: 45, column: 3, scope: !371)
!385 = distinct !DISubprogram(name: "add", linkageName: "Arr<float*>::add(float* const&)", scope: !80, file: !3, line: 36, type: !98, scopeLine: 36, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !97, retainedNodes: !4)
!386 = distinct !{!26, !36}
!387 = !DILocalVariable(name: "this", arg: 1, scope: !385, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!388 = !DILocation(line: 0, scope: !385)
!389 = !DILocalVariable(name: "e", arg: 2, scope: !385, file: !3, line: 36, type: !100)
!390 = !DILocation(line: 36, column: 21, scope: !385)
!391 = !DILocation(line: 37, column: 5, scope: !385)
!392 = !DILocation(line: 39, column: 9, scope: !393)
!393 = distinct !DILexicalBlock(scope: !385, file: !3, line: 39, column: 9)
!394 = !DILocation(line: 40, column: 7, scope: !393)
!395 = !DILocation(line: 40, column: 16, scope: !393)
!396 = !DILocation(line: 40, column: 22, scope: !393)
!397 = !DILocation(line: 44, column: 5, scope: !385)
!398 = !DILocation(line: 45, column: 3, scope: !385)
!399 = distinct !DISubprogram(name: "realloc", linkageName: "Arr<int*>::realloc(int)", scope: !46, file: !3, line: 24, type: !60, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !62, retainedNodes: !4)
!400 = distinct !{!25}
!401 = !DILocalVariable(name: "this", arg: 1, scope: !399, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!402 = !DILocation(line: 0, scope: !399)
!403 = !DILocalVariable(name: "inc", arg: 2, scope: !399, file: !3, line: 24, type: !8)
!404 = !DILocation(line: 24, column: 20, scope: !399)
!405 = !DILocation(line: 25, column: 9, scope: !406)
!406 = distinct !DILexicalBlock(scope: !399, file: !3, line: 25, column: 9)
!407 = !DILocation(line: 25, column: 14, scope: !406)
!408 = !DILocation(line: 25, column: 23, scope: !406)
!409 = !DILocation(line: 25, column: 20, scope: !406)
!410 = !DILocation(line: 35, column: 3, scope: !399)
!411 = distinct !{!253}
!412 = distinct !DISubprogram(name: "realloc", linkageName: "Arr<float*>::realloc(int)", scope: !80, file: !3, line: 24, type: !94, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !96, retainedNodes: !4)
!413 = distinct !{!26}
!414 = !DILocalVariable(name: "this", arg: 1, scope: !412, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!415 = !DILocation(line: 0, scope: !412)
!416 = !DILocalVariable(name: "inc", arg: 2, scope: !412, file: !3, line: 24, type: !8)
!417 = !DILocation(line: 24, column: 20, scope: !412)
!418 = !DILocation(line: 25, column: 9, scope: !419)
!419 = distinct !DILexicalBlock(scope: !412, file: !3, line: 25, column: 9)
!420 = !DILocation(line: 25, column: 14, scope: !419)
!421 = !DILocation(line: 25, column: 23, scope: !419)
!422 = !DILocation(line: 25, column: 20, scope: !419)
!423 = !DILocation(line: 35, column: 3, scope: !412)
!424 = distinct !DISubprogram(name: "set", linkageName: "Arr<int*>::set(int, int*)", scope: !46, file: !3, line: 15, type: !57, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !56, retainedNodes: !4)
!425 = distinct !{!25, !272}
!426 = !DILocalVariable(name: "this", arg: 1, scope: !424, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!427 = !DILocation(line: 0, scope: !424)
!428 = !DILocalVariable(name: "i", arg: 2, scope: !424, file: !3, line: 15, type: !8)
!429 = !DILocation(line: 15, column: 16, scope: !424)
!430 = !DILocalVariable(name: "val", arg: 3, scope: !424, file: !3, line: 15, type: !7)
!431 = !DILocation(line: 15, column: 21, scope: !424)
!432 = !DILocation(line: 16, column: 9, scope: !433)
!433 = distinct !DILexicalBlock(scope: !424, file: !3, line: 16, column: 9)
!434 = !DILocation(line: 16, column: 19, scope: !433)
!435 = !DILocation(line: 16, column: 9, scope: !424)
!436 = !DILocation(line: 19, column: 7, scope: !433)
!437 = !DILocation(line: 19, column: 15, scope: !433)
!438 = !DILocation(line: 20, column: 3, scope: !424)
!439 = distinct !DISubprogram(name: "set", linkageName: "Arr<float*>::set(int, float*)", scope: !80, file: !3, line: 15, type: !91, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !90, retainedNodes: !4)
!440 = distinct !{!26, !273}
!441 = !DILocalVariable(name: "this", arg: 1, scope: !439, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!442 = !DILocation(line: 0, scope: !439)
!443 = !DILocalVariable(name: "i", arg: 2, scope: !439, file: !3, line: 15, type: !8)
!444 = !DILocation(line: 15, column: 16, scope: !439)
!445 = !DILocalVariable(name: "val", arg: 3, scope: !439, file: !3, line: 15, type: !10)
!446 = !DILocation(line: 15, column: 21, scope: !439)
!447 = !DILocation(line: 16, column: 9, scope: !448)
!448 = distinct !DILexicalBlock(scope: !439, file: !3, line: 16, column: 9)
!449 = !DILocation(line: 16, column: 19, scope: !448)
!450 = !DILocation(line: 16, column: 9, scope: !439)
!451 = !DILocation(line: 19, column: 7, scope: !448)
!452 = !DILocation(line: 19, column: 15, scope: !448)
!453 = !DILocation(line: 20, column: 3, scope: !439)
!454 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<int*>::Arr(Arr<int*> const&)", scope: !46, file: !3, line: 46, type: !69, scopeLine: 46, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !68, retainedNodes: !4)
!455 = distinct !{!25, !25}
!456 = !DILocalVariable(name: "this", arg: 1, scope: !454, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!457 = !DILocation(line: 0, scope: !454)
!458 = !DILocalVariable(name: "A", arg: 2, scope: !454, file: !3, line: 46, type: !71)
!459 = !DILocation(line: 46, column: 18, scope: !454)
!460 = !DILocation(line: 47, column: 19, scope: !461)
!461 = distinct !DILexicalBlock(scope: !454, file: !3, line: 46, column: 21)
!462 = !DILocation(line: 58, column: 3, scope: !454)
!463 = distinct !DISubprogram(name: "Arr", linkageName: "Arr<float*>::Arr(Arr<float*> const&)", scope: !80, file: !3, line: 46, type: !103, scopeLine: 46, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !102, retainedNodes: !4)
!464 = distinct !{!26, !26}
!465 = !DILocalVariable(name: "this", arg: 1, scope: !463, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!466 = !DILocation(line: 0, scope: !463)
!467 = !DILocalVariable(name: "A", arg: 2, scope: !463, file: !3, line: 46, type: !105)
!468 = !DILocation(line: 46, column: 18, scope: !463)
!469 = !DILocation(line: 47, column: 19, scope: !470)
!470 = distinct !DILexicalBlock(scope: !463, file: !3, line: 46, column: 21)
!471 = !DILocation(line: 58, column: 3, scope: !463)
!472 = distinct !DISubprogram(name: "~Arr", linkageName: "Arr<int*>::~Arr()", scope: !46, file: !3, line: 59, type: !74, scopeLine: 59, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !73, retainedNodes: !4)
!473 = distinct !{!25}
!474 = !DILocalVariable(name: "this", arg: 1, scope: !472, type: !45, flags: DIFlagArtificial | DIFlagObjectPointer)
!475 = !DILocation(line: 0, scope: !472)
!476 = !DILocation(line: 59, column: 17, scope: !477)
!477 = distinct !DILexicalBlock(scope: !472, file: !3, line: 59, column: 10)
!478 = !DILocation(line: 59, column: 12, scope: !477)
!479 = !DILocation(line: 59, column: 24, scope: !472)
!480 = distinct !DISubprogram(name: "~Arr", linkageName: "Arr<float*>::~Arr()", scope: !80, file: !3, line: 59, type: !108, scopeLine: 59, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !107, retainedNodes: !4)
!481 = distinct !{!26}
!482 = !DILocalVariable(name: "this", arg: 1, scope: !480, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!483 = !DILocation(line: 0, scope: !480)
!484 = !DILocation(line: 59, column: 17, scope: !485)
!485 = distinct !DILexicalBlock(scope: !480, file: !3, line: 59, column: 10)
!486 = !DILocation(line: 59, column: 12, scope: !485)
!487 = !DILocation(line: 59, column: 24, scope: !480)
