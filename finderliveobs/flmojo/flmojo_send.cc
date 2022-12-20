

#include "flmojo/flmojo_send.h"
#include "flmojo/flmojo_service.h"

#include "flmojo/flmojo_writeinfo_wrapper.h"

#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"
#include "flproto/proto/flphonecasting.pb.h"
#include "flproto/proto/flpreview.pb.h"

#include "flobs/flobs_info.h"

#include "flobs/flobs_private_def.h"
#include "flobs/flobs_public_def.h"

//
namespace {

bool SendPushStringMessageByMojoPipe(uint32_t request_id,
                                     const std::string& request_msg) {
  FLMojoService::GetInstance().SendPushStringMessage(request_id, request_msg);
  return true;
}

bool SendPushProtobufMessageByMojoPipe(uint32_t request_id,
                                       const void* protobuf_message) {
  FLMojoService::GetInstance().SendPushProtobufMessage(request_id,
                                                       protobuf_message);
  return true;
}

bool SendPullReqStringMessageByMojoPipe(uint32_t request_id,
                                        const std::string& request_msg) {
  FLMojoService::GetInstance().SendPullReqStringMessage(request_id,
                                                        request_msg);
  return true;
}

flremoting::protocol::ObsItemMessage ConstructObsItemMsg(
    const OBSSourceBaseInfo& base_info,
    const std::string& source_data) {
  flremoting::protocol::ObsItemMessage obs_item_msg;
  obs_item_msg.set_item_visible(base_info.visible);
  obs_item_msg.set_item_id(base_info.item_id);
  obs_item_msg.set_source_type(base_info.source_type);
  obs_item_msg.set_source_name(base_info.source_name);
  obs_item_msg.set_source_width(base_info.width);
  obs_item_msg.set_source_height(base_info.height);
  obs_item_msg.set_item_lock_status(base_info.lock_status);
  obs_item_msg.set_source_data(source_data);

  return obs_item_msg;
}

}  // namespace

void SetObsPropertiesInfo(void* obs_properties_info,
                          int info_type,
                          const ProperyInfo& info) {
  flremoting::protocol::ObsPropertiesInfo* obs_properties_info_ptr =
      static_cast<flremoting::protocol::ObsPropertiesInfo*>(
          obs_properties_info);
  switch (info_type) {
    case flremoting::protocol::OBS_PROPERTY_INVALID:
      break;
    case flremoting::protocol::OBS_PROPERTY_BOOL:
      obs_properties_info_ptr->set_info_bool_value(info.bval);
      break;
    case flremoting::protocol::OBS_PROPERTY_INT:
      obs_properties_info_ptr->set_info_int_value(info.ival);
      break;
    case flremoting::protocol::OBS_PROPERTY_FLOAT:
      obs_properties_info_ptr->set_info_double_value(info.dval);
      break;
    case flremoting::protocol::OBS_PROPERTY_TEXT:
      obs_properties_info_ptr->set_info_string_value(info.tval);
      break;
    case flremoting::protocol::OBS_PROPERTY_PATH:
      obs_properties_info_ptr->set_info_string_value(info.tval);
      break;
    case flremoting::protocol::OBS_PROPERTY_LIST:
      break;
    case flremoting::protocol::OBS_PROPERTY_COLOR:
      obs_properties_info_ptr->set_info_int_value(info.ival);
      break;
    case flremoting::protocol::OBS_PROPERTY_BUTTON:
      break;
    case flremoting::protocol::OBS_PROPERTY_FONT:
      break;
    case flremoting::protocol::OBS_PROPERTY_EDITABLE_LIST:
      break;
    case flremoting::protocol::OBS_PROPERTY_FRAME_RATE:
      break;
    case flremoting::protocol::OBS_PROPERTY_GROUP:
      break;
    case flremoting::protocol::OBS_PROPERTY_COLOR_ALPHA:
      break;
    default:
      break;
  }

  for (auto& dval : info.vecdval) {
    auto info_double_value = obs_properties_info_ptr->add_info_double_values();
    info_double_value->set_name(dval.first);
    info_double_value->set_value(dval.second);

    obs_properties_info_ptr->set_info_double_value(info.dval);
  }

  for (auto& ival : info.vecival) {
    auto info_int64_value = obs_properties_info_ptr->add_info_int64_values();
    info_int64_value->set_name(ival.first);
    info_int64_value->set_value(ival.second);

    obs_properties_info_ptr->set_info_int_value(info.ival);
  }

  for (auto& tval : info.vectval) {
    auto info_string_value = obs_properties_info_ptr->add_info_string_values();
    info_string_value->set_name(tval.first);
    info_string_value->set_value(tval.second);

    obs_properties_info_ptr->set_info_string_value(info.tval);
  }
}

void SetObsPropertiesData(void* obs_properties_data,
                          const std::vector<ProperyInfo>& infos) {
  flremoting::protocol::ObsPropertiesData* obs_properties_data_ptr =
      static_cast<flremoting::protocol::ObsPropertiesData*>(
          obs_properties_data);
  for (auto& info : infos) {
    auto obs_properties_info =
        obs_properties_data_ptr->add_obs_properties_infos();
    obs_properties_info->set_info_name(info.pname);
    auto info_type =
        static_cast<flremoting::protocol::ObsPropertyType>(info.ptype);
    obs_properties_info->set_info_type(info_type);
    SetObsPropertiesInfo(obs_properties_info, info_type, info);
  }
}

//
void SendObsSceneItemInfoMessage(const std::string& message) {
  SendPushStringMessageByMojoPipe(
      flremoting::protocol::MsgType::OBS_SCENEITEM_INFO, message);
}

void SendPreviewCanvasMessage(int base_cx,
                              int base_cy,
                              int res_type,
                              int res_cx,
                              int res_cy,
                              bool is_portrait) {
  flremoting::protocol::PreviewCanvasMessage preview_canvas_msg;
  preview_canvas_msg.mutable_base()->set_cx(base_cx);
  preview_canvas_msg.mutable_base()->set_cy(base_cy);
  preview_canvas_msg.set_res_type(res_type);
  preview_canvas_msg.mutable_res()->set_cx(res_cx);
  preview_canvas_msg.mutable_res()->set_cy(res_cy);
  preview_canvas_msg.set_is_portrait(is_portrait);

  SendPushProtobufMessageByMojoPipe(
      flremoting::protocol::MsgType::PREVIEWWINDOWSZINFO, &preview_canvas_msg);
}

void SendObsInfoMessage(const OBSSourceBaseInfo& base_info,
                        const std::string& source_data) {
  auto obs_item_msg = ConstructObsItemMsg(base_info, source_data);

  SendPushProtobufMessageByMojoPipe(flremoting::protocol::MsgType::OBSITEMINFO,
                                    &obs_item_msg);
}

void SendItemEditMsg(const OBSSourceBaseInfo& base_info,
                     const std::string& source_data) {
  auto obs_item_msg = ConstructObsItemMsg(base_info, source_data);

  SendPushProtobufMessageByMojoPipe(flremoting::protocol::MsgType::OBSITEMEDIT,
                                    &obs_item_msg);
}

void SendDeviceMessage(const std::string& message) {
  SendPushStringMessageByMojoPipe(flremoting::protocol::MsgType::OBSDEVICEINFO,
                                  message);
}

std::string GenInputVolumeMessage(int value) {
  flremoting::protocol::InputVolumeData input_volume_data_msg;
  input_volume_data_msg.set_value(value);

  std::string send_msg;
  input_volume_data_msg.SerializeToString(&send_msg);
  return send_msg;
}

void SendInputVolumeMessage(const std::string& message) {
  SendPushStringMessageByMojoPipe(
      flremoting::protocol::MsgType::INPUTVOLUMEINFO, message);
}

std::string GenOutputVolumeMessage(int value) {
  flremoting::protocol::OutputVolumeData output_volume_data_msg;
  output_volume_data_msg.set_value(value);

  std::string send_msg;
  output_volume_data_msg.SerializeToString(&send_msg);
  return send_msg;
}

void SendOutputVolumeMessage(const std::string& message) {
  SendPushStringMessageByMojoPipe(
      flremoting::protocol::MsgType::OUTPUTVOLUMEINFO, message);
}

void SendShowMouseMenuMessage(int64_t id, int x, int y) {
  flremoting::protocol::PreviewMouseMenuMessage preview_mousemenu_msg;
  preview_mousemenu_msg.set_id(id);
  preview_mousemenu_msg.set_x(x);
  preview_mousemenu_msg.set_y(y);

  SendPushProtobufMessageByMojoPipe(
      flremoting::protocol::MsgType::MOUSEMENUEVENT, &preview_mousemenu_msg);
}

void SendPhoneCastingInfo(uint32_t status,
                          int method,
                          std::string& ip,
                          uint32_t port) {
  flremoting::protocol::PhoneCastingMessage phone_casting_msg;
  phone_casting_msg.set_status(status);
  phone_casting_msg.set_method(method);
  phone_casting_msg.set_ip(ip);
  phone_casting_msg.set_port(port);

  SendPushProtobufMessageByMojoPipe(
      flremoting::protocol::MsgType::PHONECASTINGINFO, &phone_casting_msg);
}
