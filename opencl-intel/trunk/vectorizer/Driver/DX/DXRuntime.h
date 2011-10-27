#ifndef __DXRuntime_H_
#define __DXRuntime_H_

#include "RuntimeServices.h"
#include "Functions.h"

namespace intel {

extern VFH::hashEntry DXEntryDB[];

/// @brief Runtime services for DX
///  @Author Sion Berkowitz, Nadav Rotem
class DXRuntime : public RuntimeServices {
public:

  /// @brief Constructor
  DXRuntime(const Module *runtimeModule,
            unsigned packetWidth):
    m_runtimeModule(runtimeModule),
    m_packetizationWidth(packetWidth),
    m_vfh(DXEntryDB) {}

  /// @brief Destructor
  ~DXRuntime() {}

  /// @brief Find a function in the runtime's built-in functions
  /// @param Name Function name to look for
  virtual Function *findInRuntimeModule(StringRef Name) const;

  /// @brief Search for a builtin function (used by scalarizer abd packetizer)
  /// @param inp_name Function name to look for
  virtual funcEntry findBuiltinFunction(std::string &inp_name) const;

  /// @brief DX is not ordered. WIAnalysis is not needed
  ///  since everything is assumed to be random.
  /// @return True
  virtual bool orderedWI() const;

  /// @brief Check if specified Instruction is an ID generator
  /// @param inst The instruction
  /// @param err Returns TRUE, if unable to determine ID generation
  /// @param dim Dimention of TIDGenerator
  virtual bool isTIDGenerator(const Instruction *inst, bool *err, unsigned* dim) const;

  /// @brief Check the desired packetization width
  /// @return vector width for packetizing the function
  virtual unsigned getPacketizationWidth() const;

  /// @brief Sets the desired packetization width  
  /// @param width vector width for packetizing the function
  virtual void setPacketizationWidth(unsigned width);

  /// @brief Check if function is a synchronization built-in
  /// @param inp_name Function name to look for
  virtual bool isSyncFunc(const std::string &func_name) const;

  /// @brief Check if function is a known uniform function such as get_group_size
  /// @param inp_name Function name to look for
  virtual bool isKnownUniformFunc(std::string &func_name) const;

  /// @brief returns true if the function has no side effects 
  ///  this means it can be safely vectorized regardless if it is being masked 
  /// @param func_name Function name to check
  /// @return true if function has no side effects
  virtual bool hasNoSideEffect(std::string &func_name) const;

  /// @brief returns true if the function is a masked version that support 
  ///  i1 vector as first parameter
  /// @param func_name Function name to check
  /// @return true if function is masked version
  virtual bool isMaskedFunctionCall(std::string &func_name) const;

private:
  DXRuntime(); // Do not implement

  /// @brief Fills the OCL functions DB with DX buildins
  /// @param s VFH storage
  void initDB(hashEntry* entry);

  /// @brief Pointer to runtime module
  /// (module with implementation of built-in functions)
  const Module *m_runtimeModule;

  /// @brief Hold the requested packetization width
  /// (currently same one for all funcs)
  unsigned m_packetizationWidth;

  /// @brief Pointer to OpenCL wrappers hash object
  VFH m_vfh;
};

} // Namespace

#endif // __DXRuntime_H_
