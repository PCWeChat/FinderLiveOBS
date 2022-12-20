

#include "flobs/flobs_task_executor.h"
#include "base/threading/thread_task_runner_handle.h"

// leak it
FLOBSTaskExecutor* g_orc_task_executor = nullptr;

void FLOBSTaskExecutor::InitializeUIThread() {
  g_orc_task_executor = new FLOBSTaskExecutor();
  g_orc_task_executor->task_runner_ = base::ThreadTaskRunnerHandle::Get();
}

scoped_refptr<base::SingleThreadTaskRunner>
FLOBSTaskExecutor::GetUIThreadTaskRunner() {
  return g_orc_task_executor->task_runner_;
}
