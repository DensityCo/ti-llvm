//===-- C6000TargetMachine.cpp - Define TargetMachine for C6000 -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "C6000.h"
#include "C6000TargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeC6000Target() {
  // Register the target.
  RegisterTargetMachine<C6000TargetMachine> X(TheC6000Target);
}

static std::string computeDataLayout() {
  // TODO:  for now, assume little endian
  std::string Ret = "e-m:e";

  // 32bit pointers.
    Ret += "-p:32:32";

  // Alignments for 64 bit integers.
  Ret += "-i64:64";

  // Alignments for 128 bit and above vectors is 64
  Ret += "-v128:64:128";

  // 32 bit ints
  // 64 bit aligned stack
  Ret += "-n32-S64";

  return Ret;
}

/// C6000TargetMachine ctor
///
C6000TargetMachine::C6000TargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    DL(computeDataLayout())
{
}

//===----------------------------------------------------------------------===//
// C6000 Analysis Pass Setup
//===----------------------------------------------------------------------===//

void C6000TargetMachine::addAnalysisPasses(PassManagerBase &PM) {
  // Add first the target-independent BasicTTI pass, then our C6000 pass. This
  // allows the C6000 pass to delegate to the target independent layer when
  // appropriate.
  PM.add(createBasicTargetTransformInfoPass(this));
  PM.add(createC6000TargetTransformInfoPass(this));
}

namespace {
/// C6000 Code Generator Pass Configuration Options.
class C6000PassConfig : public TargetPassConfig {
public:
  C6000PassConfig(C6000TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  C6000TargetMachine &getC6000TargetMachine() const {
    return getTM<C6000TargetMachine>();
  }
};
} // namespace

TargetPassConfig *C6000TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new C6000PassConfig(this, PM);
}

