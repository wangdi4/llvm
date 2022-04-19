// RUN: %clang_cc1 %s -O0 -fintel-compatibility -fhls -triple=x86_64-linux-gnu -emit-llvm -no-opaque-pointers -o - | FileCheck %s

template <typename T, int buffer, int ReadyLatency, int BitsPerSymbol,
          bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty,
          bool UsesValid>
class stream_in {
public:
  T read(bool &sop, bool &eop, __int32 &empty) {
    return *__builtin_intel_hls_instream_read((T *)0, (__int64)this, buffer, ReadyLatency,
                                              BitsPerSymbol,
                                              FirstSymbolInHighOrderBits,
                                              UsesPackets,
                                              UsesEmpty,
                                              UsesValid, &sop, &eop, &empty);
  }
  T tryRead(bool &success, bool &sop, bool &eop, __int32 &empty) {
    return *__builtin_intel_hls_instream_tryRead((T *)0, (__int64)this, buffer, ReadyLatency,
                                                 BitsPerSymbol,
                                                 FirstSymbolInHighOrderBits,
                                                 UsesPackets,
                                                 UsesEmpty,
                                                 UsesValid, &sop, &eop, &empty,
                                                 &success);
  }
  void write(const T arg, bool sop, bool eop, __int32 empty) {
    __builtin_intel_hls_instream_write(&arg, (__int64)this, buffer, ReadyLatency,
                                       BitsPerSymbol,
                                       FirstSymbolInHighOrderBits,
                                       UsesPackets,
                                       UsesEmpty,
                                       UsesValid, sop, eop, empty);
  }
  bool tryWrite(const T arg, bool sop, bool eop, __int32 empty) {
    return __builtin_intel_hls_instream_tryWrite(&arg, (__int64)this, buffer, ReadyLatency,
                                                 BitsPerSymbol,
                                                 FirstSymbolInHighOrderBits,
                                                 UsesPackets,
                                                 UsesEmpty,
                                                 UsesValid, sop, eop, empty);
  }
};

template <typename T, int buffer, int ReadyLatency, int BitsPerSymbol,
          bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty,
          bool UsesReady>
class stream_out {
public:
  T read(bool &sop, bool &eop, __int32 &empty) {
    return *__builtin_intel_hls_outstream_read((T *)0, (__int64)this, buffer, ReadyLatency,
                                               BitsPerSymbol,
                                               FirstSymbolInHighOrderBits,
                                               UsesPackets, UsesEmpty, UsesReady,
                                               &sop, &eop, &empty);
  }
  T tryRead(bool &success, bool &sop, bool &eop, __int32 &empty) {
    return *__builtin_intel_hls_outstream_tryRead((T *)0, (__int64)this, buffer, ReadyLatency,
                                                  BitsPerSymbol,
                                                  FirstSymbolInHighOrderBits,
                                                  UsesPackets,
                                                  UsesEmpty, UsesReady, &sop, &eop, &empty, &success);
  }
  void write(const T arg, bool sop, bool eop, __int32 empty) {
    __builtin_intel_hls_outstream_write(&arg, (__int64)this, buffer, ReadyLatency,
                                        BitsPerSymbol,
                                        FirstSymbolInHighOrderBits,
                                        UsesPackets, UsesEmpty, UsesReady, sop, eop, empty);
  }
  bool tryWrite(const T arg, bool sop, bool eop, __int32 empty) {
    return __builtin_intel_hls_outstream_tryWrite(&arg, (__int64)this, buffer, ReadyLatency,
                                                  BitsPerSymbol,
                                                  FirstSymbolInHighOrderBits,
                                                  UsesPackets, UsesEmpty, UsesReady, sop, eop, empty);
  }
};

struct Foo {
  char a, b, c, d;
};

void TestStreams() {
  stream_in<Foo, 3, 5, 4, true, true, false, false> StrIn;
  stream_out<Foo, 3, 5, 4, false, false, true, true> StrOut;
  Foo f;
  bool b;
  int i;

  f = StrIn.read(b, b, i);
  bool s;
  f = StrIn.tryRead(s, b, b, i);
  StrIn.write(f, b, b, i);
  StrIn.tryWrite(f, b, b, i);

  f = StrOut.read(b, b, i);
  f = StrOut.tryRead(s, b, b, i);
  StrOut.write(f, b, b, i);
  s = StrOut.tryWrite(f, b, b, i);
}
void TestStreamsBool() {
  stream_in<bool, 3, 5, 4, true, true, false, false> StrIn;
  stream_out<bool, 3, 5, 4, false, false, true, true> StrOut;
  bool f;
  bool b;
  int i;

  f = StrIn.read(b, b, i);
  bool s;
  f = StrIn.tryRead(s, b, b, i);
  StrIn.write(f, b, b, i);
  StrIn.tryWrite(f, b, b, i);

  f = StrOut.read(b, b, i);
  f = StrOut.tryRead(s, b, b, i);
  StrOut.write(f, b, b, i);
  s = StrOut.tryWrite(f, b, b, i);
}
// CHECK: declare { %struct.Foo*, i8, i8, i32 } @llvm.intel.hls.instream.read.p0s_struct.Foos(i64, i32, i32, i32, i1, i1, i1, i1)
// CHECK: declare { %struct.Foo*, i8, i8, i8, i32 } @llvm.intel.hls.instream.tryRead.p0s_struct.Foos(i64, i32, i32, i32, i1, i1, i1, i1)
//
// CHECK: declare void @llvm.intel.hls.instream.write.p0s_struct.Foos(%struct.Foo*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
// CHECK: declare i1 @llvm.intel.hls.instream.tryWrite.p0s_struct.Foos(%struct.Foo*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
//
// CHECK: declare { %struct.Foo*, i8, i8, i32 } @llvm.intel.hls.outstream.read.p0s_struct.Foos(i64, i32, i32, i32, i1, i1, i1, i1)
// CHECK: declare { %struct.Foo*, i8, i8, i8, i32 } @llvm.intel.hls.outstream.tryRead.p0s_struct.Foos(i64, i32, i32, i32, i1, i1, i1, i1)
//
// CHECK: declare void @llvm.intel.hls.outstream.write.p0s_struct.Foos(%struct.Foo*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
// CHECK: declare i1 @llvm.intel.hls.outstream.tryWrite.p0s_struct.Foos(%struct.Foo*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)

// CHECK: declare { i1*, i8, i8, i32 } @llvm.intel.hls.instream.read.p0i1(i64, i32, i32, i32, i1, i1, i1, i1)
// CHECK: declare { i1*, i8, i8, i8, i32 } @llvm.intel.hls.instream.tryRead.p0i1(i64, i32, i32, i32, i1, i1, i1, i1)
//
// CHECK: declare void @llvm.intel.hls.instream.write.p0i1(i1*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
// CHECK: declare i1 @llvm.intel.hls.instream.tryWrite.p0i1(i1*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
//
// CHECK: declare { i1*, i8, i8, i32 } @llvm.intel.hls.outstream.read.p0i1(i64, i32, i32, i32, i1, i1, i1, i1)
// CHECK: declare { i1*, i8, i8, i8, i32 } @llvm.intel.hls.outstream.tryRead.p0i1(i64, i32, i32, i32, i1, i1, i1, i1)
//
// CHECK: declare void @llvm.intel.hls.outstream.write.p0i1(i1*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
// CHECK: declare i1 @llvm.intel.hls.outstream.tryWrite.p0i1(i1*, i64, i32, i32, i32, i1, i1, i1, i1, i1, i1, i32)
