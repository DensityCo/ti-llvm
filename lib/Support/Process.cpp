//===-- Process.cpp - Implement OS Process Concept --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"

using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

/// \brief A helper function to compute the elapsed wall-time since the program
/// started.
///
/// Note that this routine actually computes the elapsed wall time since the
/// first time it was called. However, we arrange to have it called during the
/// startup of the process to get approximately correct results.
static TimeValue getElapsedWallTime() {
#ifndef _SYS_BIOS
  static TimeValue &StartTime = *new TimeValue(TimeValue::now());
  return TimeValue::now() - StartTime;
#else
  return TimeValue ( 0,0 );
#endif
}

/// \brief A special global variable to ensure we call \c getElapsedWallTime
/// during global initialization of the program.
///
/// Note that this variable is never referenced elsewhere. Doing so could
/// create race conditions during program startup or shutdown.
static volatile TimeValue DummyTimeValue = getElapsedWallTime();

Optional<std::string> Process::FindInEnvPath(const std::string& EnvName,
                                             const std::string& FileName)
{
  Optional<std::string> FoundPath;
  Optional<std::string> OptPath = Process::GetEnv(EnvName);
  if (!OptPath.hasValue())
    return FoundPath;

  const char EnvPathSeparatorStr[] = {EnvPathSeparator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(OptPath.getValue(), Dirs, EnvPathSeparatorStr);

  for (const auto &Dir : Dirs) {
    if (Dir.empty())
      continue;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    if (fs::exists(Twine(FilePath))) {
      FoundPath = FilePath.str();
      break;
    }
  }

  return FoundPath;
}


#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG,BOLD) {\
    COLOR(FGBG, "0", BOLD),\
    COLOR(FGBG, "1", BOLD),\
    COLOR(FGBG, "2", BOLD),\
    COLOR(FGBG, "3", BOLD),\
    COLOR(FGBG, "4", BOLD),\
    COLOR(FGBG, "5", BOLD),\
    COLOR(FGBG, "6", BOLD),\
    COLOR(FGBG, "7", BOLD)\
  }

static const char colorcodes[2][2][8][10] = {
 { ALLCOLORS("3",""), ALLCOLORS("3","1;") },
 { ALLCOLORS("4",""), ALLCOLORS("4","1;") }
};
#ifdef _SYS_BIOS
#include "Bios/Process.inc"
#else

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Process.inc"
#endif
#ifdef LLVM_ON_WIN32
#include "Windows/Process.inc"
#endif
#endif
