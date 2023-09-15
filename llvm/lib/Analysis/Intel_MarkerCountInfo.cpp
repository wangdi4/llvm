#if INTEL_FEATURE_MARKERCOUNT
//===- Intel_MarkerCountInfo.cpp - Marker Count Parser --------------------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the APIs that are used to parse the marker count for
// functions in from a JSON file/string.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_MarkerCountInfo.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;

void MarkerCount::parseMarkerCountString(std::map<std::string, unsigned> &Map,
                                         unsigned MarkerCountKind,
                                         StringRef Str, StringRef Root) {

  auto emitError = [&](StringRef Msg) {
    Twine Err =
        Msg + (Root.empty() ? Twine(":\n") + Str : Twine(": ") + Root) + "\n";
    report_fatal_error(Err, false /*no crash dump*/);
  };

  Expected<json::Value> Value = json::parse(Str);

  if (!Value) {
    std::string Msg = std::string("Override marker count ") +
                      (Root.empty() ? "string" : "file") +
                      " is not a valid JSON";
    emitError(Msg);
  }

  auto Array = Value->getAsArray();
  if (!Array)
    emitError("Expected a top-level array");

  auto emitFormatError = [&](const json::Value &Value) {
    errs() << Value << "\n";
    emitError("Expected an array of {name:<function_name>, "
              "function:<never|me|be>, loop:<never|me|be>, "
              "comment:<comment>}. name must exits, at least one of function "
              "and loop exist and comment is optional");
  };

  for (const auto &Value : *Array) {
    const auto *Obj = Value.getAsObject();
    if (!Obj)
      emitFormatError(Value);

    auto Name = Obj->getString("name");
    auto Function = Obj->getString("function");
    auto Loop = Obj->getString("loop");
    if (!Name || (!Function && !Loop))
      emitFormatError(Value);

    unsigned FunctionMarker =
        Function ? StringSwitch<MarkerCount::Flag>(Function.value())
                       .Case("never", MarkerCount::Function_Never)
                       .Case("me", MarkerCount::Function_ME)
                       .Case("be", MarkerCount::Function_BE)
                       .Default(MarkerCount::Unknown)
                 : MarkerCountKind & MarkerCount::Function;

    if (FunctionMarker == MarkerCount::Unknown)
      emitFormatError(Value);

    unsigned LoopMarker = Loop ? StringSwitch<MarkerCount::Flag>(Loop.value())
                                     .Case("never", MarkerCount::Loop_Never)
                                     .Case("me", MarkerCount::Loop_ME)
                                     .Case("be", MarkerCount::Loop_BE)
                                     .Default(MarkerCount::Unknown)
                               : MarkerCountKind & MarkerCount::Loop;

    if (LoopMarker == MarkerCount::Unknown)
      emitFormatError(Value);

    unsigned MCK = FunctionMarker | LoopMarker;
    std::string FunctionName = Name.value().str();
    if (Map.count(FunctionName))
      emitError("Duplicate object for function " + FunctionName);

    Map.insert({FunctionName, MCK});
  }
}

void MarkerCount::parseMarkerCountFile(std::map<std::string, unsigned> &Map,
                                       unsigned MarkerCountKind,
                                       StringRef File) {
  if (File.empty())
    return;

  ErrorOr<std::unique_ptr<MemoryBuffer>> Buffer =
      llvm::MemoryBuffer::getFile(File, true);

  if (!Buffer)
    report_fatal_error(Twine("Error opening marker count file: ") + File + "\n",
                       false /*no crash dump*/);

  parseMarkerCountString(Map, MarkerCountKind, Buffer.get()->getBuffer(), File);
}
#endif // INTEL_FEATURE_MARKERCOUNT
