

#include "flbase/flbase_config.h"

FLBaseConfig::FLBaseConfig() {}

FLBaseConfig::~FLBaseConfig() {}

void FLBaseConfig::SetBaseConfigPath(const std::string& base_config_path) {
  base_config_path_ = base_config_path;
}

std::string FLBaseConfig::GetBaseConfigPath() {
  return base_config_path_;
}
