//===--- CompletionInstance.h ---------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IDE_COMPLETIONINSTANCE_H
#define SWIFT_IDE_COMPLETIONINSTANCE_H

#include "swift/Frontend/Frontend.h"
#include "swift/IDE/CancellableResult.h"
#include "swift/IDE/ConformingMethodList.h"
#include "swift/IDE/TypeContextInfo.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace swift {

class CompilerInstance;
class CompilerInvocation;
class DiagnosticConsumer;

namespace ide {

/// Copy a memory buffer inserting '\0' at the position of \c origBuf.
std::unique_ptr<llvm::MemoryBuffer>
makeCodeCompletionMemoryBuffer(const llvm::MemoryBuffer *origBuf,
                               unsigned &Offset,
                               llvm::StringRef bufferIdentifier);

/// The result returned via the callback from the perform*Operation methods.
struct CompletionInstanceResult {
  /// The compiler instance that is prepared for the second pass.
  CompilerInstance &CI;
  /// Whether an AST was reused.
  bool DidReuseAST;
  /// Whether the CompletionInstance found a code completion token in the source
  /// file. If this is \c false, the user will most likely want to return empty
  /// results.
  bool DidFindCodeCompletionToken;
};

/// The results returned from \c CompletionInstance::typeContextInfo.
struct TypeContextInfoResult {
  /// The actual results. If empty, no results were found.
  ArrayRef<TypeContextInfoItem> Results;
  /// Whether an AST was reused to produce the results.
  bool DidReuseAST;
};

/// The results returned from \c CompletionInstance::conformingMethodList.
struct ConformingMethodListResults {
  /// The actual results. If \c nullptr, no results were found.
  const ConformingMethodListResult *Result;
  /// Whether an AST was reused for the completion.
  bool DidReuseAST;
};

/// Manages \c CompilerInstance for completion like operations.
class CompletionInstance {
  struct Options {
    unsigned MaxASTReuseCount = 100;
    unsigned DependencyCheckIntervalSecond = 5;
  } Opts;

  std::mutex mtx;

  std::unique_ptr<CompilerInstance> CachedCI;
  llvm::hash_code CachedArgHash;
  llvm::sys::TimePoint<> DependencyCheckedTimestamp;
  llvm::StringMap<llvm::hash_code> InMemoryDependencyHash;
  unsigned CachedReuseCount = 0;
  std::atomic<bool> CachedCIShouldBeInvalidated;

  void cacheCompilerInstance(std::unique_ptr<CompilerInstance> CI,
                             llvm::hash_code ArgsHash);

  bool shouldCheckDependencies() const;

  /// Calls \p Callback with cached \c CompilerInstance if it's usable for the
  /// specified completion request.
  /// Returns \c true if performing the cached operation was possible. Returns
  /// \c false if the compiler argument has changed, primary file is not the
  /// same, the \c Offset is not in function bodies, or the interface hash of
  /// the file has changed.
  /// \p Callback will be called if and only if this function returns \c true.
  bool performCachedOperationIfPossible(
      llvm::hash_code ArgsHash,
      llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem,
      llvm::MemoryBuffer *completionBuffer, unsigned int Offset,
      DiagnosticConsumer *DiagC,
      llvm::function_ref<void(CancellableResult<CompletionInstanceResult>)>
          Callback);

  /// Calls \p Callback with new \c CompilerInstance for the completion
  /// request. The \c CompilerInstace passed to the callback already performed
  /// the first pass.
  /// Returns \c false if it fails to setup the \c CompilerInstance.
  void performNewOperation(
      llvm::Optional<llvm::hash_code> ArgsHash,
      swift::CompilerInvocation &Invocation,
      llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem,
      llvm::MemoryBuffer *completionBuffer, unsigned int Offset,
      DiagnosticConsumer *DiagC,
      llvm::function_ref<void(CancellableResult<CompletionInstanceResult>)>
          Callback);

public:
  CompletionInstance() : CachedCIShouldBeInvalidated(false) {}

  // Mark the cached compiler instance "should be invalidated". In the next
  // completion, new compiler instance will be used. (Thread safe.)
  void markCachedCompilerInstanceShouldBeInvalidated();

  // Update options with \c NewOpts. (Thread safe.)
  void setOptions(Options NewOpts);

  /// Calls \p Callback with a \c CompilerInstance which is prepared for the
  /// second pass. \p Callback is resposible to perform the second pass on it.
  /// The \c CompilerInstance may be reused from the previous completions,
  /// and may be cached for the next completion.
  /// In case of failure or cancellation, the callback receives the
  /// corresponding failed or cancelled result.
  ///
  /// NOTE: \p Args is only used for checking the equaity of the invocation.
  /// Since this function assumes that it is already normalized, exact the same
  /// arguments including their order is considered as the same invocation.
  void performOperation(
      swift::CompilerInvocation &Invocation, llvm::ArrayRef<const char *> Args,
      llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem,
      llvm::MemoryBuffer *completionBuffer, unsigned int Offset,
      DiagnosticConsumer *DiagC,
      llvm::function_ref<void(CancellableResult<CompletionInstanceResult>)>
          Callback);

  void typeContextInfo(
      swift::CompilerInvocation &Invocation, llvm::ArrayRef<const char *> Args,
      llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem,
      llvm::MemoryBuffer *completionBuffer, unsigned int Offset,
      DiagnosticConsumer *DiagC,
      llvm::function_ref<void(CancellableResult<TypeContextInfoResult>)>
          Callback);

  void conformingMethodList(
      swift::CompilerInvocation &Invocation, llvm::ArrayRef<const char *> Args,
      llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> FileSystem,
      llvm::MemoryBuffer *completionBuffer, unsigned int Offset,
      DiagnosticConsumer *DiagC, ArrayRef<const char *> ExpectedTypeNames,
      llvm::function_ref<void(CancellableResult<ConformingMethodListResults>)>
          Callback);
};

} // namespace ide
} // namespace swift

#endif // SWIFT_IDE_COMPLETIONINSTANCE_H
