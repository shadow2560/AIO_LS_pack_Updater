#ifndef MEMORY_DIAGNOSTICS_H
#define MEMORY_DIAGNOSTICS_H

/* Set to 0 to disable the temporary Daybreak/eSpeak memory diagnostics. */
#define MEMORY_DIAGNOSTICS 0

#if MEMORY_DIAGNOSTICS
void MemoryDiagnosticsLogMemory(const char *stage);
void MemoryDiagnosticsLogAllocator(const char *stage);
#endif

#endif
