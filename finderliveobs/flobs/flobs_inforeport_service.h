

#ifndef FLOBS_FLOBS_INFOREPORT_SERVICE_H_
#define FLOBS_FLOBS_INFOREPORT_SERVICE_H_

#include <stdint.h>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <utility>

class FLOBSInfoReportService {
 public:
  static FLOBSInfoReportService& GetInstance() {
    static FLOBSInfoReportService instance;
    return instance;
  }

  FLOBSInfoReportService();

  FLOBSInfoReportService(const FLOBSInfoReportService&) = delete;
  FLOBSInfoReportService& operator=(const FLOBSInfoReportService&) = delete;

  ~FLOBSInfoReportService();

  void __Init();
  void __Cleanup();

 private:
  void __RunLoop();
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::shared_ptr<std::thread> thread_ = nullptr;
  std::atomic<bool> is_working_ = false;
};

#endif  // FLOBS_FLOBS_INFOREPORT_SERVICE_H_
