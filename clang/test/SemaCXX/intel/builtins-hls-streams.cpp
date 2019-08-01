// RUN: %clang_cc1 -fhls -fsyntax-only -verify %s

struct Foo {
  char a, b, c, d;
};
// Quick sanity check to validate the normal implementations.
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
  s = StrIn.tryWrite(f, b, b, i);

  f = StrOut.read(b, b, i);
  f = StrOut.tryRead(s, b, b, i);
  StrOut.write(f, b, b, i);
  s = StrOut.tryWrite(f, b, b, i);
}

void TestArgCounts() {
  // expected-error@+1 {{too few arguments to function call, expected 12, have 11}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 4, 0, true, true, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{too few arguments to function call, expected 13, have 11}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 1, 99, 4, true, true, true, true, nullptr, nullptr, nullptr);

  // expected-error@+1 {{too many arguments to function call, expected 12, have 13}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 4, 0, true, true, false, true, 5, nullptr, nullptr, nullptr);
  bool b;
  // expected-error@+1 {{too many arguments to function call, expected 13, have 14}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 1, 99, 4, 0, true, true, true, true, &b, 5, nullptr, nullptr, nullptr);
}

struct F;
int temp;

void ArgValues() {
  Foo f;
  bool b;
  int i;
  // Type Arg:
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read(f, 1, 99, 0, 4, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read((F *)0, 1, 99, 0, 4, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a complete object type}}
  __builtin_intel_hls_instream_read((void *)0, 1, 99, 0, 4, true, true, false, true, nullptr, nullptr, nullptr);

  // BufferID Arg:
  // expected-error@+1 {{HLS builtin parameter must be an integer}}
  __builtin_intel_hls_instream_read((Foo *)0, "str", 1, 0, 4, true, true, false, true, nullptr, nullptr, nullptr);

  // Buffer Arg:
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant}}
  __builtin_intel_hls_instream_read(&f, 99, -1, 0, 4, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{argument to '__builtin_intel_hls_instream_read' must be a constant integer}}
  __builtin_intel_hls_instream_read(&f, 99, "str", 0, 4, true, true, false, true, nullptr, nullptr, nullptr);

  // ReadyLatency Arg:
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, -1, 4, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{HLS builtin parameter must be a non-negative integer constant, 0 or -1}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, -2, 4, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{argument to '__builtin_intel_hls_instream_read' must be a constant integer}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, temp, 4, true, true, false, true, nullptr, nullptr, nullptr);

  // BitsPerSymbol Arg:
  // expected-error@+1 {{HLS builtin parameter must be a factor of the type size}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 0, -1, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{argument to '__builtin_intel_hls_instream_read' must be a constant integer}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 0, temp, true, true, false, true, nullptr, nullptr, nullptr);
  // expected-error@+1 {{HLS builtin parameter must be a factor of the type size}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 0, 3, true, true, false, true, nullptr, nullptr, nullptr);

  // Bool Args:
  // expected-error@+1 {{HLS builtin parameter must be a boolean value}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 3, 1, 5, true, false, true, nullptr, nullptr, nullptr);

  // sop/eop/empty
  // OK
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 3, 1, true, true, false, true, &b, &b, &i);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a boolean}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 3, 1, true, true, false, true, 5, nullptr, nullptr);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to an integer}}
  __builtin_intel_hls_instream_read((Foo *)0, 1, 99, 3, 1, true, true, false, true, &b, nullptr, 3);

  __builtin_intel_hls_instream_write((Foo *)0, 1, 99, 3, 1, true, true, false, true, b, b, i);
  // expected-error@+1 {{HLS builtin parameter must be a boolean}}
  __builtin_intel_hls_instream_write((Foo *)0, 1, 99, 3, 1, true, true, false, true, &b, nullptr, i);
  // expected-error@+1 {{HLS builtin parameter must be an integer}}
  __builtin_intel_hls_instream_write((Foo *)0, 1, 99, 3, 1, true, true, false, true, b, true, "str");

  // Success:
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a boolean}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 1, 99, 3, 1, false, true, false, true, nullptr, nullptr, nullptr, false);
  // expected-error@+1 {{HLS builtin parameter must be a pointer to a boolean}}
  __builtin_intel_hls_instream_tryRead((Foo *)0, 1, 99, 3, 1, false, true, false, true, nullptr, nullptr, nullptr, &temp);
}
