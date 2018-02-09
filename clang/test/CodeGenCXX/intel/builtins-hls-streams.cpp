// RUN: %clang_cc1 %s -O0 -fintel-compatibility -fhls -triple=x86_64-linux-gnu -emit-llvm -o - | FileCheck %s

template <typename T, int ReadyLatency, int BitsPerSymbol, bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty, bool UsesValid>
class stream_in {
public:
  T read() {
    return *__builtin_intel_hls_instream_read((T *)0, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesValid);
  }
  T tryRead(bool *success) {
    return *__builtin_intel_hls_instream_tryRead((T *)0, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesValid, success);
  }
  void write(const T *arg) {
    __builtin_intel_hls_instream_write(arg, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesValid);
  }
  bool tryWrite(const T *arg) {
    return __builtin_intel_hls_instream_tryWrite(arg, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesValid);
  }
};

template <typename T, int ReadyLatency, int BitsPerSymbol, bool FirstSymbolInHighOrderBits, bool UsesPackets, bool UsesEmpty, bool UsesReady>
class stream_out {
public:
  T read() {
    return *__builtin_intel_hls_outstream_read((T *)0, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesReady);
  }
  T tryRead(bool *success) {
    return *__builtin_intel_hls_outstream_tryRead((T *)0, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesReady, success);
  }
  void write(const T *arg) {
    __builtin_intel_hls_outstream_write(arg, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesReady);
  }
  bool tryWrite(const T *arg) {
    return __builtin_intel_hls_outstream_tryWrite(arg, (__int64)this, ReadyLatency, BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty, UsesReady);
  }
};

struct Foo {
  char a, b, c, d;
};

void TestStreams() {
  stream_in<Foo, 5, 4, true, true, false, false> StrIn;
  stream_out<Foo, 5, 4, false, false, true, true> StrOut;
  Foo f;

  f = StrIn.read();
  bool s;
  f = StrIn.tryRead(&s);
  StrIn.write(&f);
  StrIn.tryWrite(&f);

  f = StrOut.read();
  f = StrOut.tryRead(&s);
  StrOut.write(&f);
  s = StrOut.tryWrite(&f);
}
// CHECK: declare %struct.Foo* @llvm.intel.hls.instream.read.s_struct.Foos(i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare { %struct.Foo*, i8 } @llvm.intel.hls.instream.tryRead.s_struct.Foos(i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare void @llvm.intel.hls.instream.write.s_struct.Foos(%struct.Foo*, i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare i1 @llvm.intel.hls.instream.tryWrite.s_struct.Foos(%struct.Foo*, i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare %struct.Foo* @llvm.intel.hls.outstream.read.s_struct.Foos(i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare { %struct.Foo*, i8 } @llvm.intel.hls.outstream.tryRead.s_struct.Foos(i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare void @llvm.intel.hls.outstream.write.s_struct.Foos(%struct.Foo*, i64, i32, i32, i1, i1, i1, i1)
// CHECK: declare i1 @llvm.intel.hls.outstream.tryWrite.s_struct.Foos(%struct.Foo*, i64, i32, i32, i1, i1, i1, i1)
