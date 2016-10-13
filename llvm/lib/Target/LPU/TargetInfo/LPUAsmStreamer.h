#ifndef LPU_ASM_STREAMER_H
#define LPU_ASM_STREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {


/// createLpuAsmStreamer - Create a machine code streamer which will print
/// out assembly for the LPU target, suitable for compiling with a native
/// assembler.
///
/// \param InstPrint - If given, the instruction printer to use. If not given
/// the MCInst representation will be printed.  This method takes ownership of
/// InstPrint.
///
/// \param CE - If given, a code emitter to use to show the instruction
/// encoding inline with the assembly. This method takes ownership of \p CE.
///
/// \param TAB - If given, a target asm backend to use to show the fixup
/// information in conjunction with encoding information. This method takes
/// ownership of \p TAB.
///
/// \param ShowInst - Whether to show the MCInst representation inline with
/// the assembly.
MCStreamer *createLpuAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
                                 bool isVerboseAsm, bool useDwarfDirectory,
                                 MCInstPrinter *InstPrint, MCCodeEmitter *CE,
                                 MCAsmBackend *TAB, bool ShowInst);
} // namespace llvm

#endif // LPU_ASM_STREAMER_H
