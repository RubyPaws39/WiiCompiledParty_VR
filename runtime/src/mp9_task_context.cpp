#if defined(MKW_GAME_MP9)
#include "mp9_task_context.h"
#include "abi_bridge.h"
#include "host_context.h"
#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <unordered_map>

// Use the user's translated implementation for the exact custom register
// layout. Do not pass a nonlocal restore through an ABI preservation guard.
extern "C" void func_8020C7DC(CpuContext*);

namespace {
struct TaskContext {
    HostContext::Handle host = nullptr;
    uint32_t savedStack = 0;
    CpuContext* cpu = nullptr;
    bool ownsHost = false;
    std::array<PPC_FPR, 18> fprs{};
    uint32_t fpscr = 0;
};
thread_local std::unordered_map<uint32_t, std::unique_ptr<TaskContext>> contexts;

void EnterTask(void* argument) {
    auto* task = static_cast<TaskContext*>(argument);
    InvokeIndirectCpu(task->cpu->lr, task->cpu);
    throw std::runtime_error("MP9 task returned without restoring its caller context");
}
}

void Mp9RememberTaskContext(CpuContext* cpu) {
    auto& task = contexts[cpu->gpr[3]];
    if (!task) task = std::make_unique<TaskContext>();
    const auto current = HostContext::Current();
    if (!current) throw std::runtime_error("MP9 context save outside host scheduler");
    if (task->ownsHost && task->host != current) {
        // A new object can reuse a released guest address. Its initial save
        // supersedes the suspended task previously associated with that slot.
        HostContext::Destroy(task->host);
        task->ownsHost = false;
    }
    task->host = current;
    task->savedStack = cpu->gpr[1];
    task->cpu = cpu;
    // Keep the native representation exactly. The guest helper stores both
    // lfd and psq views, while this translator uses one union for scalar and
    // packed-pair values. Its psq_l would overwrite a restored double bias.
    std::copy_n(cpu->fpr + 14, task->fprs.size(), task->fprs.begin());
    task->fpscr = cpu->fpscr;
}

void Mp9RestoreTaskContext(CpuContext* cpu) {
    const uint32_t slot = cpu->gpr[3];
    const auto found = contexts.find(slot);
    if (found == contexts.end()) throw std::runtime_error("MP9 restore of unsaved task context");
    TaskContext& task = *found->second;
    func_8020C7DC(cpu);
    std::copy(task.fprs.begin(), task.fprs.end(), cpu->fpr + 14);
    cpu->fpscr = task.fpscr;
    MkwApplyHostNiMode(cpu->fpscr);
    if (task.savedStack != cpu->gpr[1]) {
        // The game seeds a new task by replacing the saved LR and SP. Start
        // that entry on a distinct host stack; subsequent saves keep it alive.
        if (task.ownsHost) throw std::runtime_error("MP9 task stack changed while suspended");
        task.host = HostContext::Create(1024 * 1024, EnterTask, &task);
        if (!task.host) throw std::runtime_error("MP9 task host stack allocation failed");
        task.ownsHost = true;
        task.savedStack = cpu->gpr[1];
    }
    if (HostContext::IsCurrent(task.host)) {
        throw std::runtime_error("MP9 task restore targets active host stack");
    }
    HostContext::Switch(task.host);
    // A later restore switched back with the guest register file already
    // loaded. Generated code must branch to that saved LR, not fall through.
}
#endif
