//===----- OptReportSupport.cpp - Utils to support emitters -*- C++ -*------==//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of routines to support various emitters
// of loopopt and vectorizer related Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReportSupport.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/Intel_OptReport/Diag.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#ifdef INTEL_ENABLE_PROTO_BIN_OPTRPT
#include "opt_report_proto.pb.h"
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <sstream>
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT

#define DEBUG_TYPE "opt-report-support-utils"

#ifdef INTEL_ENABLE_PROTO_BIN_OPTRPT
static llvm::cl::opt<bool> EnableProtobufBinOptReport(
    "enable-protobuf-opt-report", llvm::cl::Hidden, llvm::cl::init(false),
    llvm::cl::desc("Enable experimental feature: Protobuf-based binary "
                   "opt-report embedded in generated object files."));
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT

namespace llvm {
namespace OptReportSupport {
static int getMDNodeAsInt(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getSExtValue();
}

#ifdef INTEL_ENABLE_PROTO_BIN_OPTRPT
static unsigned getMDNodeAsUnsigned(const ConstantAsMetadata *CM) {
  uint64_t V = cast<ConstantInt>(CM->getValue())->getValue().getZExtValue();
  assert(V <= UINT32_MAX && "Value doesn't fit in unsigned range.");
  return V;
}

// Helper to pretty-print Protobuf object that encodes a single loop's
// opt-report.
static void printProtoLoopOptReport(
    const opt_report_proto::BinOptReport::LoopOptReport &LOR) {
  LLVM_DEBUG(dbgs() << "\n=== Loop Begin ===\n");
  LLVM_DEBUG(dbgs() << "Anchor ID: " << LOR.anchor_id() << "\n");
  LLVM_DEBUG(dbgs() << "Number of remarks: " << LOR.remarks_size() << "\n");
  for (auto I = 0; I < LOR.remarks_size(); ++I) {
    const opt_report_proto::BinOptReport::Remark &R = LOR.remarks(I);
    LLVM_DEBUG(
        dbgs() << "  Property: "
               << opt_report_proto::BinOptReport::Property_Name(R.prop_id()));
    LLVM_DEBUG(dbgs() << ", Remark ID: " << R.remark_id());
    LLVM_DEBUG(dbgs() << ", Remark Args: ");
    for (auto J = 0; J < R.args_size(); ++J) {
      const opt_report_proto::BinOptReport::Arg &Arg = R.args(J);
      if (Arg.has_str_arg())
        LLVM_DEBUG(dbgs() << Arg.str_arg().value() << " ");
      else if (Arg.has_int32_arg())
        LLVM_DEBUG(dbgs() << Arg.int32_arg().value() << " ");
    }
    LLVM_DEBUG(dbgs() << "\n");
  }
  LLVM_DEBUG(dbgs() << "==== Loop End ====\n");
}

// Helper to pretty-print Protobuf object that encodes overall binary
// opt-report.
static void printProtoBinOptReport(opt_report_proto::BinOptReport &BOR) {
  LLVM_DEBUG(dbgs() << "--- Start Protobuf Binary OptReport Printer ---\n");
  LLVM_DEBUG(dbgs() << "Version: " << BOR.major_version() << "."
                    << BOR.minor_version() << "\n");
  LLVM_DEBUG(dbgs() << "Property Message Map:\n");
  for (auto &MapIt : BOR.property_msg_map()) {
    LLVM_DEBUG(
        dbgs() << "  "
               << opt_report_proto::BinOptReport::Property_Name(MapIt.first)
               << " --> " << MapIt.second << "\n");
    (void)MapIt;
  }
  LLVM_DEBUG(dbgs() << "Number of reports: " << BOR.opt_reports_size() << "\n");
  for (auto I = 0; I < BOR.opt_reports_size(); ++I) {
    const opt_report_proto::BinOptReport::LoopOptReport &LOR =
        BOR.opt_reports(I);
    printProtoLoopOptReport(LOR);
  }

  LLVM_DEBUG(dbgs() << "--- End Protobuf Binary OptReport Printer ---\n\n");
}
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT

// TODO (vzakhari 02/11/2019): this is an experimental implementation
//       of binary opt-report representation for tech.preview release.
//       I guess this implementation should actually belong to LoopOpt
//       and Vectorizer opt-report support libraries, so that here we just
//       query this libraries for the releavant parts of the stream.
std::string formatBinaryStream(OptReport OR) {
  BitVector LoopBits(64);
  BitVector VecBits(64);

  uint32_t MvVersion = 0;
  uint32_t UnrollFactor = 1;

  // Return 0 stream, if there is no opt-report attached.
  if (!OR)
    return "";

  for (const OptRemark Remark : OR.origin()) {
    const MDString *R = cast<MDString>(Remark.getOperand(1));
    std::string OriginString = std::string(R->getString());

    if (OriginString == "Remainder loop for vectorization") {
      VecBits.set(2);
      LLVM_DEBUG(dbgs() << "VecBits: bits 2-2 set to 1\n");
    } else if (OriginString == "Multiversioned loop") {
      // We only support DD multiversioning right now.
      LoopBits.set(17);
      LLVM_DEBUG(dbgs() << "LoopBits: bits 17-20 set to 1\n");
      // Loop with "Multiversioned loop" origin is the "optimized"
      // version #1, the default version is #0.
      // If we later meet "The loop has been multiversioned" for this
      // loop, this would mean version #1 was multiversioned again,
      // thus, it would be version #2.  There is currently no way
      // to recognize version #3.
      MvVersion++;
    }
  }

  for (const OptRemark Remark : OR.remarks()) {
    const auto *R = cast<MDString>(Remark.getOperand(1));
    std::string FormatString = std::string(R->getString());

    if (FormatString == "LOOP WAS VECTORIZED") {
      // TODO (vzakhari 02/10/2019): bits 47-49 must specify main vectorization
      //       type, which does not seem to exist in the opt-report now.
      //       Default is 8-bit integer type.
      VecBits.set(0);
      LLVM_DEBUG(dbgs() << "VecBits: bits 0-0 set to 1\n");
    } else if (FormatString == "vectorization support: vector length %s") {
      const MDString *SM = nullptr;
      if (Remark.getNumOperands() >= 3)
        SM = dyn_cast<MDString>(Remark.getOperand(2));
      else
        llvm_unreachable("Missing 'vector length' operand.");

      assert(SM && "Expected string argument");
      // Use 1 vectorlength for release builds in case of incorrect
      // argument specification.
      uint32_t VecLen = SM ? std::stoi(std::string(SM->getString())) : 1u;
      uint32_t Log2VecLen = std::min(Log2_32(VecLen), 15u);
      uint32_t Mask[] { 0, Log2VecLen << 3 };
      VecBits.setBitsInMask(Mask);
      LLVM_DEBUG(dbgs() << "VecBits: bits 35-38 set to " << Log2VecLen << "\n");
    } else if (FormatString == "The loop has been multiversioned") {
      // We only support DD multiversioning right now.
      LoopBits.set(17);
      LLVM_DEBUG(dbgs() << "LoopBits: bits 17-20 set to 1\n");
      // This loop has "Multiversioned loop" origin, and it was
      // multiversioned again - version #2.
      if (MvVersion != 0)
        MvVersion++;
    } else if (FormatString == "LLorg: Loop has been completely unrolled" ||
               FormatString == "Loop completely unrolled") {
      // Set UnrollFactor to zero, so that all following "unroll by factor"
      // opt-reports are ignored.  UnrollFactor equal to zero means complete
      // unroll.
      UnrollFactor = 0;
    } else if (FormatString == "Loop has been unrolled by %d factor" ||
               FormatString == "LLorg: Loop has been unrolled by %d factor") {
      const ConstantAsMetadata *CM = nullptr;
      if (Remark.getNumOperands() >= 3)
        CM = dyn_cast<ConstantAsMetadata>(Remark.getOperand(2));
      else
        llvm_unreachable("Missing 'unroll factor' operand.");

      assert(CM && "Expected constant in argument");
      // In case of invalid argument use 1 for release build.
      int UF = CM ? getMDNodeAsInt(CM) : 1;
      // Treat invalid 0 unroll factor as 1.
      UF = std::max(UF, 1);
      UnrollFactor *= UF;
    }
  }

  if (MvVersion != 0) {
    MvVersion = std::min(MvVersion, 3u);
    uint32_t Mask[] { MvVersion << 21, 0 };
    LoopBits.setBitsInMask(Mask);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 21-22 set to " << MvVersion << "\n");
  }

  if (UnrollFactor > 1) {
    // Set unroll factor.  Maximum value is 31.
    UnrollFactor = (UnrollFactor > 31) ? 0 : UnrollFactor;
    uint32_t Mask[] { 0 , UnrollFactor << 4 };
    LoopBits.setBitsInMask(Mask);
    LLVM_DEBUG(dbgs() <<
               "LoopBits: bits 36-40 set to " << UnrollFactor << "\n");
    // Set unroll type to UNROLLED (2).
    LoopBits.set(35);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 34-35 set to 2\n");
  } else if (UnrollFactor == 0) {
    // Set unroll type to COMPLETELY_UNROLLED (1).
    LoopBits.set(34);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 34-35 set to 1\n");
  }

  // Form a byte stream, where 64 loop bits are followed by 64 vector bits.
  std::string Stream;
  for (int Pos = 0; Pos < 64; Pos += 8) {
    uint8_t Byte = 0;
    for (int SubPos = 0; SubPos < 8; ++SubPos) {
      Byte |= (LoopBits.test(Pos + SubPos) << SubPos);
    }
    Stream.push_back(Byte);
  }
  for (int Pos = 0; Pos < 64; Pos += 8) {
    uint8_t Byte = 0;
    for (int SubPos = 0; SubPos < 8; ++SubPos) {
      Byte |= (VecBits.test(Pos + SubPos) << SubPos);
    }
    Stream.push_back(Byte);
  }

  assert(Stream.length() == 16 &&
         "Opt-report binary stream must be 16 bytes long.");

  LLVM_DEBUG(
      dbgs() << "Opt-report binary stream:\n";
      for (int Pos = 0; Pos < 16; ++Pos) {
        (dbgs() << "0x").write_hex(Stream[Pos]) << " ";
        if (((Pos + 1) % 4) == 0 && Pos != 15)
          dbgs() << "| ";
      }
      dbgs() << "\n";);

  return Stream;
}

bool isProtobufBinOptReportEnabled() {
#ifdef INTEL_ENABLE_PROTO_BIN_OPTRPT
  return EnableProtobufBinOptReport;
#else
  return false;
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT
}

#ifdef INTEL_ENABLE_PROTO_BIN_OPTRPT
// Central map to sync text opt-report diagnostic remarks with binary opt-report
// properties. The Key is unique RemarkID used to identify diagnostic remark and
// Value is the binary opt-report property that it corresponds to. Size of this
// map is expected to match the number of properties defined by binary
// opt-report Protobuf schema (check the enum Property in
// opt_report_proto.proto).
static const DenseMap<unsigned, opt_report_proto::BinOptReport::Property>
    DiagPropertyMap = {
        {15300, opt_report_proto::BinOptReport::C_LOOP_VECTORIZED},
        {15305, opt_report_proto::BinOptReport::C_LOOP_VEC_VL},
        {25508, opt_report_proto::BinOptReport::C_LOOP_COMPLETE_UNROLL},
        {25436, opt_report_proto::BinOptReport::C_LOOP_COMPLETE_UNROLL_FACTOR},
        {25439, opt_report_proto::BinOptReport::C_LOOP_UNROLL_WITH_REMAINDER},
        {25438,
         opt_report_proto::BinOptReport::C_LOOP_UNROLL_WITHOUT_REMAINDER},
        {25540, opt_report_proto::BinOptReport::C_LOOP_UNROLL_AND_JAM},
        {25491, opt_report_proto::BinOptReport::C_LOOP_REMAINDER},
};
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT

std::string generateProtobufBinOptReport(OptRptAnchorMapTy &OptRptAnchorMap,
                                         unsigned MajorVer, unsigned MinorVer) {
  if (!isProtobufBinOptReportEnabled())
    llvm_unreachable(
        "Cannot generate binary opt-report if Protobuf is not enabled.");

#ifndef INTEL_ENABLE_PROTO_BIN_OPTRPT
  return "";
#else
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // TODO: If we need compile-time static_assert then use a list approach
  // (similar to OptReportDiag::Diags class from Diag.h) instead of map.
  assert(DiagPropertyMap.size() ==
             opt_report_proto::BinOptReport::Property_MAX &&
         "Mismatch in text opt-report diagnostics and binary opt-report "
         "properties.");

  // Return empty stream if there are no opt-reports.
  if (OptRptAnchorMap.empty())
    return "";

  opt_report_proto::BinOptReport BOR;

  // Set major and minor versions of binary opt-report.
  BOR.set_major_version(MajorVer);
  BOR.set_minor_version(MinorVer);

  // Set used to collect unique remark IDs emitted in current list of
  // opt-reports.
  DenseSet<unsigned> EmittedRemarkIDs;

  // Translate text opt-reports to binary opt-report format.
  for (StringRef AnchorID : OptRptAnchorMap.keys()) {
    OptReport OR = OptRptAnchorMap[AnchorID];

    if (!OR)
      continue;

    opt_report_proto::BinOptReport::LoopOptReport *LOR = BOR.add_opt_reports();

    // Set anchoring info for the loop that this optreport corresponds to.
    LOR->set_anchor_id(AnchorID.str());

    auto ProcessRemark = [&LOR, &EmittedRemarkIDs](const OptRemark Remark) {
      // TODO: Promote interface to LoopOptRemark::getRemarkID.
      unsigned RemarkID =
          mdconst::extract<ConstantInt>(Remark.getOperand(0))->getZExtValue();

      // Ignore remark if it doesn't have an equivalent property in binary
      // opt-report.
      auto DiagPropMapIt = DiagPropertyMap.find(RemarkID);
      if (DiagPropMapIt == DiagPropertyMap.end())
        return;

      opt_report_proto::BinOptReport::Property PropID = (*DiagPropMapIt).second;
      opt_report_proto::BinOptReport::Remark *BinRemark = LOR->add_remarks();
      BinRemark->set_prop_id(PropID);
      BinRemark->set_remark_id(RemarkID);
      EmittedRemarkIDs.insert(RemarkID);

      for (unsigned Op = 2; Op < Remark.getNumOperands(); ++Op) {
        // Create and add remark argument based on its type.
        if (auto *RemarkOp = dyn_cast<MDString>(Remark.getOperand(Op))) {
          std::string ArgString = std::string(RemarkOp->getString());
          opt_report_proto::BinOptReport::Arg *RemarkArg =
              BinRemark->add_args();
          auto *StrArg = RemarkArg->mutable_str_arg();
          StrArg->set_value(ArgString);
        }

        if (auto *RemarkOp =
                mdconst::dyn_extract<ConstantInt>(Remark.getOperand(Op))) {
          unsigned ArgInt = RemarkOp->getZExtValue();
          opt_report_proto::BinOptReport::Arg *RemarkArg =
              BinRemark->add_args();
          auto *IntArg = RemarkArg->mutable_int32_arg();
          IntArg->set_value(ArgInt);
        }
      }
    };

    // Translate origin remarks to properties.
    for (const OptRemark Remark : OR.origin())
      ProcessRemark(Remark);
    // Translate diagnostic remarks to properties.
    for (const OptRemark Remark : OR.remarks())
      ProcessRemark(Remark);
  }

  // Populate Property->RemarkMsg map.
  auto *PropMsgMap = BOR.mutable_property_msg_map();
  for (unsigned RemarkID : EmittedRemarkIDs) {
    auto DiagPropMapIt = DiagPropertyMap.find(RemarkID);
    assert(DiagPropMapIt != DiagPropertyMap.end() &&
           "Unknown remark ID emitted.");
    int32_t PropID = DiagPropMapIt->second;
    assert(opt_report_proto::BinOptReport::Property_IsValid(PropID) &&
           "Invalid binary opt-report property.");
    (*PropMsgMap)[PropID] = OptReportDiag::getMsg(RemarkID);
  }

  LLVM_DEBUG(dbgs() << "[ProtoBOR] BinOptReport before encoding:\n";
             printProtoBinOptReport(BOR));

  std::string BinOptRptStream;

  int UncompressedBytes = BOR.ByteSizeLong();
  LLVM_DEBUG(dbgs() << "Uncompressed protobuf msg size: " << UncompressedBytes
                    << " bytes\n");
  (void)UncompressedBytes;
  google::protobuf::io::StringOutputStream SOS(&BinOptRptStream);
  google::protobuf::io::GzipOutputStream::Options GzipOptions;
  GzipOptions.format = google::protobuf::io::GzipOutputStream::ZLIB;
  GzipOptions.compression_level = 9;
  google::protobuf::io::GzipOutputStream GOS(&SOS, GzipOptions);

  if (BOR.SerializeToZeroCopyStream(&GOS)) {
    GOS.Close();
    if (GOS.ZlibErrorCode() > 0) {
      auto CompressedBytes = SOS.ByteCount();
      LLVM_DEBUG(dbgs() << "Compressed protobuf msg size: " << CompressedBytes
                        << " bytes\n");
      (void)CompressedBytes;
    } else {
      std::string ZlibErrorMsg(GOS.ZlibErrorMessage());
      report_fatal_error(Twine("Failed to compress protobuf message (zlib error:" +
                         ZlibErrorMsg + ")"));
    }
  } else {
    report_fatal_error("Failed to serialize protobuf message.");
  }

  assert(!BinOptRptStream.empty() && "Binary opt-report stream is empty.");
  LLVM_DEBUG(
      dbgs() << "Opt-report binary stream serialized via protobuf (size: "
             << BinOptRptStream.length() << "):\n"
             << BinOptRptStream << "\n");

  return BinOptRptStream;
#endif // INTEL_ENABLE_PROTO_BIN_OPTRPT
}

} // namespace OptReportSupport
} // namespace llvm
