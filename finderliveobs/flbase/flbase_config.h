

#ifndef FLBASE_FLBASE_CONFIG_H_
#define FLBASE_FLBASE_CONFIG_H_

#include <string>

class FLBaseConfig {
 public:
  static FLBaseConfig& GetInstance() {
    static FLBaseConfig instance;
    return instance;
  }

  FLBaseConfig();

  FLBaseConfig(const FLBaseConfig&) = delete;
  FLBaseConfig& operator=(const FLBaseConfig&) = delete;

  ~FLBaseConfig();

  void SetBaseConfigPath(const std::string& base_config_path);
  std::string GetBaseConfigPath();

 private:
  std::string base_config_path_;
};

#endif  // FLBASE_FLBASE_CONFIG_H_
