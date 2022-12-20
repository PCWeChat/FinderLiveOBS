

#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32
#include <thread>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/task_scheduler/task_scheduler.h"

#include "mmcrashpad/client/mmcrashpad_client.h"

#include "flmojo/flmojo_service.h"
#include "flobs/flobs_task_executor.h"

#include "base/at_exit.h"
#include "base/bind.h"

// debug macro
// #define ATTATCH_DEBUG

class InitFLMojo {
 public:
  InitFLMojo() {
#ifdef _WIN32
    InitializeFLMojo(0, nullptr);
#endif  // _WIN32
  }

  InitFLMojo(const InitFLMojo&) = delete;
  InitFLMojo& operator=(const InitFLMojo&) = delete;

  ~InitFLMojo() { ShutdownFLMojo(); }
};

void BaseCheckHandler(const char* file,
                      int line,
                      const base::StringPiece message,
                      const base::StringPiece stack_trace) {}

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,
                      int nCmdShow) {
#if defined(ATTATCH_DEBUG) && defined(_DEBUG)
  while (!::IsDebuggerPresent()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
#endif  // defined(ATTATCH_DEBUG) && defined(_DEBUG)

#ifdef _WIN32
  SetErrorMode(SEM_FAILCRITICALERRORS);
#endif  // _WIN32

  MMCrashpadInitialization(false, nullptr, 0, nullptr, 0);

  base::CommandLine::Init(0, nullptr);
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  InitFLMojo flmojo;

  base::AtExitManager at_exit_manager;
  logging::ScopedLogAssertHandler scoped_assert_handler(
      base::BindRepeating(BaseCheckHandler));

  base::MessageLoop main_loop(base::MessageLoop::TYPE_UI);
  FLOBSTaskExecutor::InitializeUIThread();

  FLMojoService::GetInstance().Start();

  {
    // thread pool
    constexpr int kBackgroundMaxThreads = 1;
    constexpr int kBackgroundBlockingMaxThreads = 1;
    const int kForegroundMaxThreads = 1;
    const int kForegroundBlockingMaxThreads = 1;

    constexpr base::TimeDelta kSuggestedReclaimTime =
        base::TimeDelta::FromSeconds(30);

    base::TaskScheduler::Create("FLOBSThreadPool");
    base::TaskScheduler::GetInstance()->Start(
        {{kBackgroundMaxThreads, kSuggestedReclaimTime},
         {kBackgroundBlockingMaxThreads, kSuggestedReclaimTime},
         {kForegroundMaxThreads, kSuggestedReclaimTime},
         {kForegroundBlockingMaxThreads, kSuggestedReclaimTime}});
  }

  base::RunLoop run_loop;
  run_loop.Run();

  return 0;
}