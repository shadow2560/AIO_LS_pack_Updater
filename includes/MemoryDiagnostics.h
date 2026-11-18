#ifndef MEMORY_DIAGNOSTICS_H
#define MEMORY_DIAGNOSTICS_H

#include "DebugConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MEMORY_DIAGNOSTICS
void MemoryDiagnosticsLogMemory(const char *stage);
void MemoryDiagnosticsLogAllocator(const char *stage);
#endif

#ifdef __cplusplus
}
#endif

#endif
