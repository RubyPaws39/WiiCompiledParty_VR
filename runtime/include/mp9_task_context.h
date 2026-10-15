#pragma once

struct CpuContext;

// MP9's game task contexts are not SDK OSContext objects. The translator
// lowers their save/load boundaries to these host stack handoffs.
void Mp9RememberTaskContext(CpuContext* cpu);
void Mp9RestoreTaskContext(CpuContext* cpu);
