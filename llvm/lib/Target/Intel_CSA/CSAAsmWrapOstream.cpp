//===- CSAAsmWrapOstream.cpp - Assembly-wrapping ostream --------*- C++ -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a raw_ostream adapter which supports transparently
// string-wrapping assembly to make it survive offload.
//
//===----------------------------------------------------------------------===//

#include "CSAAsmWrapOstream.h"

#include <llvm/MC/MCStreamer.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>

using namespace llvm;

namespace {

// The delimiter character used to start and end wrapped asm strings, which
// should be valid as long as it isn't a character that is allowed to show up in
// assembly output. Because MCStreamer really likes adding newlines to "raw"
// text output, '\n's to either side of this delimiter will also be ignored.
constexpr char DELIMITER = '\xff';

/// A stream adapter type which adds quotes and does string escaping when
/// requested.
class asm_wrapping_raw_ostream final : public raw_ostream {

  // The original stream that this stream wraps.
  raw_ostream *WrappedStream;

  // Whether string escaping mode is currently enabled.
  bool Escaping = false;

  // Whether a newline present at the end of an earlier buffer might still need
  // to be written out.
  bool CarryingNewline = false;

  // Whether the next newline encountered should be ignored.
  bool SkipNewline = false;

  // The virtual function that implements most of the logic.
  void write_impl(const char *, size_t) override;

  // Just use the wrapped stream position.
  uint64_t current_pos() const override { return WrappedStream->tell(); }

public:
  asm_wrapping_raw_ostream(raw_ostream &);
  ~asm_wrapping_raw_ostream() override;
};

/// A dummy base class that holds an asm_wrapping_raw_ostream member - this is
/// necessary to ensure that it is initialized before the other base.
struct asm_wrapping_raw_ostream_holder {
  asm_wrapping_raw_ostream TheWrappingOStream;
  asm_wrapping_raw_ostream_holder(raw_ostream &StreamIn)
      : TheWrappingOStream{StreamIn} {}
};

/// A type that combines the asm wrapper adapter with formatted_raw_ostream.
class formatted_wrapping_raw_ostream final
    : private asm_wrapping_raw_ostream_holder,
      public formatted_raw_ostream {

public:
  formatted_wrapping_raw_ostream(raw_ostream &StreamIn)
      : asm_wrapping_raw_ostream_holder{StreamIn}, formatted_raw_ostream{
                                                     TheWrappingOStream} {}
};

} // namespace

asm_wrapping_raw_ostream::asm_wrapping_raw_ostream(raw_ostream &WrappedIn)
    : WrappedStream{&WrappedIn} {

  // As in formattted_raw_ostream, steal buffering from the wrapped stream.
  if (size_t BufferSize = WrappedStream->GetBufferSize()) {
    SetBufferSize(BufferSize);
  } else {
    SetUnbuffered();
  }
}

asm_wrapping_raw_ostream::~asm_wrapping_raw_ostream() {
  flush();

  // Wrapped strings should be closed before the destructor.
  assert(not Escaping);

  // Make sure to write out any lagging newlines.
  if (CarryingNewline)
    *WrappedStream << "\n";
  WrappedStream->flush();

  // Set buffering back on the wrapped stream.
  if (size_t BufferSize = GetBufferSize()) {
    WrappedStream->SetBufferSize(BufferSize);
  } else {
    WrappedStream->SetUnbuffered();
  }
}

void asm_wrapping_raw_ostream::write_impl(const char *Ptr, size_t Size) {
  using std::find;

  // Empty buffers don't need writing.
  if (not Size)
    return;

  // If the next newline should be skipped and the buffer starts with a newline,
  // skip that newline and start over at the next character.
  if (SkipNewline and Ptr[0] == '\n') {
    SkipNewline = false;
    return write_impl(Ptr + 1, Size - 1);
  }

  // Otherwise, there's a non-newline character after the delimiter and we don't
  // need to keep looking for newlines.
  SkipNewline = false;

  // Look for the next delimiter.
  const auto NextDelim = find(Ptr, Ptr + Size, DELIMITER);

  // If we're carrying a newline and the delimiter is not the first character,
  // it's safe to write the newline now.
  if (CarryingNewline and NextDelim != Ptr) {
    *WrappedStream << (Escaping ? "\\n" : "\n");
  }

  // Is there a newline at the end of the chunk? If there is, ignore it but set
  // CarryingNewline to keep track of it.
  CarryingNewline            = (NextDelim != Ptr and NextDelim[-1] == '\n');
  const char *const ChunkEnd = CarryingNewline ? NextDelim - 1 : NextDelim;

  // Output everything up to the end of the chunk.
  const size_t ChunkSize = ChunkEnd - Ptr;
  if (Escaping)
    WrappedStream->write_escaped({Ptr, ChunkSize});
  else
    WrappedStream->write(Ptr, ChunkSize);

  // If no delimiter was found, we're done here.
  if (NextDelim == Ptr + Size)
    return;

  // Otherwise, output '"', switch escaping modes, and continue.
  WrappedStream->write('"');
  Escaping        = not Escaping;
  CarryingNewline = false;
  SkipNewline     = true;
  return write_impl(NextDelim + 1, Size - ((NextDelim + 1) - Ptr));
}

void llvm::startCSAAsmString(MCStreamer &OutStreamer) {
  OutStreamer.EmitRawText(Twine(DELIMITER));
}

void llvm::endCSAAsmString(MCStreamer &OutStreamer) {
  OutStreamer.EmitRawText(Twine(DELIMITER));
}

std::unique_ptr<formatted_raw_ostream>
llvm::wrapStreamForCSAAsmWrapping(raw_ostream &ToWrap) {
  return std::make_unique<formatted_wrapping_raw_ostream>(ToWrap);
}
