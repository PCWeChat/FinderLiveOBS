

#ifndef FLOBS_FLOBS_TASK_EXECUTOR_H_
#define FLOBS_FLOBS_TASK_EXECUTOR_H_

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"

class FLOBSTaskExecutor {
 public:
  FLOBSTaskExecutor() = default;

  FLOBSTaskExecutor(const FLOBSTaskExecutor&) = delete;
  FLOBSTaskExecutor& operator=(const FLOBSTaskExecutor&) = delete;

  ~FLOBSTaskExecutor() = default;

  static void InitializeUIThread();

  static scoped_refptr<base::SingleThreadTaskRunner> GetUIThreadTaskRunner();

 private:
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
};

#endif  // FLOBS_FLOBS_TASK_EXECUTOR_H_
