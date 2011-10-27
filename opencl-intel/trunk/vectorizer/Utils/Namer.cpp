#include "Namer.h"
#include <sstream>
namespace intel {

template <class T>
inline std::string toString (const T& elem) {
  std::stringstream ss;
  ss << elem;
  return ss.str();
}

void Namer::nameBB(BasicBlock *BB) {
    // This is debug code to rename basic blocks.
    static unsigned int letter = 0;
    std::string name = std::string("BB") + toString(letter);
    BB->setName(name);
}

bool Namer::runOnFunction(Function &F) {

    // For each Basic Block
    for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe ; ++bb) {
        nameBB(bb);
    }
    return true;
}


} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
void* createNamer() {
     return new intel::Namer();
}
}

char intel::Namer::ID = 0;
static RegisterPass<intel::Namer> CLINamerX("name", "Name BBs in function in a consecutive order");

