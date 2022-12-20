

#ifndef FLMOJO_FLMOJO_SEND_H_
#define FLMOJO_FLMOJO_SEND_H_

#include <stdint.h>
#include <string>
#include <vector>

struct ProperyInfo;
struct OBSSourceBaseInfo;

void SetObsPropertiesInfo(void* obs_properties_info,
                          int info_type,
                          const ProperyInfo& info);
void SetObsPropertiesData(void* obs_properties_data,
                          const std::vector<ProperyInfo>& infos);

//
void SendObsSceneItemInfoMessage(const std::string& message);

void SendPreviewCanvasMessage(int base_cx,
                              int base_cy,
                              int res_type,
                              int res_cx,
                              int res_cy,
                              bool is_portrait);

void SendObsInfoMessage(const OBSSourceBaseInfo& base_info,
                        const std::string& source_data);

void SendItemEditMsg(const OBSSourceBaseInfo& base_info,
                     const std::string& source_data);

void SendDeviceMessage(const std::string& message);

std::string GenInputVolumeMessage(int value);
void SendInputVolumeMessage(const std::string& message);

std::string GenOutputVolumeMessage(int value);
void SendOutputVolumeMessage(const std::string& message);

void SendShowMouseMenuMessage(int64_t id, int x, int y);

void SendPhoneCastingInfo(uint32_t status,
                          int method,
                          std::string& ip,
                          uint32_t port);

#endif  // FLMOJO_FLMOJO_SEND_H_