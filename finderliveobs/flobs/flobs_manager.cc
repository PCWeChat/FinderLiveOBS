
#include "flobs/flobs_manager.h"

#include <AudioClient.h>
#include <Shlwapi.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>
#include <algorithm>
#include <format>

#include "base/strings/stringprintf.h"

#include "flobs/display-helpers.hpp"
#include "flobs/flobs_audio.h"
#include "flobs/flobs_camera_beautify.h"
#include "flobs/flobs_properyinfo.h"
#include "flobs/flobs_public_def.h"
#include "flobs/flobs_quicktransition.h"
#include "flobs/flobs_video.h"
#include "flobs/flobs_video_decimator.h"

#include "third_party/obs-studio/src/libobs/util/platform.h"
#include "third_party/obs-studio/src/libobs/util/profiler.hpp"

// config
#include "flbase/flbase_config.h"

// proto
#include "flproto/proto/flmsgtype.pb.h"
#include "flproto/proto/flobsinfo.pb.h"

// not open
#include "flobs/flobs_private_def.h"

#include "flobs/flobs_ui_def.h"

#include "flobs/flobs_util.h"

#include "flobs/flobs_data_callback.h"

#include "flmojo/flmojo_log.h"
#include "flmojo/flmojo_send.h"

#include "flobs/flobs_inforeport_service.h"

//
#include "flui/ms_window_win.h"
static std::unique_ptr<MsWindowWin> g_ms_window_win;

constexpr char kFLObsDir[] = "Finderlive\\obs";
constexpr char kFLObsBasicDir[] = "Finderlive\\obs\\basic";
constexpr char kFLObsBasicScenesDir[] = "Finderlive\\obs\\basic\\scenes";
constexpr char kFLObsLogDir[] = "Finderlive\\obs\\logs";
constexpr char kFLObsProfilerDataDir[] = "Finderlive\\obs\\profiler_data";
constexpr char kFLObsProfilerDir[] = "Finderlive\\obs\\profiles";
constexpr char kFLObsCrashesDir[] = "Finderlive\\obs\\crashes";
constexpr char kFLObsPluginConfigDir[] = "Finderlive\\obs\\plugin_config";

static std::string sg_finderlive_config_empty = "";
static std::string sg_finderlive_obs_config_path = "";
static std::string sg_finderlive_obs_base_config_path = "";
static std::string sg_finderlive_obs_base_scene_config_path = "";
static std::string sg_finderlive_obs_log_config_path = "";
static std::string sg_finderlive_obs_profiler_data_config_path = "";
static std::string sg_finderlive_obs_profiles_config_path = "";
static std::string sg_finderlive_obs_crashes_config_path = "";
static std::string sg_finderlive_obs_plugin_config_config_path = "";

#define DESKTOP_AUDIO ("DesktopAudioDevice")
#define DESKTOP_AUDIO_2 ("DesktopAudioDevice2")
#define AUX_AUDIO_1 ("AuxAudioDevice1")
#define AUX_AUDIO_2 ("AuxAudioDevice2")
#define AUX_AUDIO_3 ("AuxAudioDevice3")
#define AUX_AUDIO_4 ("AuxAudioDevice4")

#define GREY_COLOR_BACKGROUND (0xFFFFFFFF)

static int kChannel_1_destopAudio = 1;
static int kChannel_2 = 2;
static int kChannel_3_Mic = 3;
static int kChannel_4 = 5;
static int kChannel_5 = 5;
static int kChannel_6 = 6;

static const double kVolumeMeterDecayFast = 23.53;

static constexpr auto kFloatWndFontSizeEnumBase = 100;

enum class FloatFontSize {
  Small = kFloatWndFontSizeEnumBase - 2,
  Standard = kFloatWndFontSizeEnumBase,
  Big = kFloatWndFontSizeEnumBase + 2
};

enum class EncodeQuality : uint32_t { baseline = 1, main, high };

#ifdef __APPLE__
#define INPUT_AUDIO_SOURCE "coreaudio_input_capture"
#define OUTPUT_AUDIO_SOURCE "coreaudio_output_capture"
#elif _WIN32
#define INPUT_AUDIO_SOURCE "wasapi_input_capture"
#define OUTPUT_AUDIO_SOURCE "wasapi_output_capture"
#else
#define INPUT_AUDIO_SOURCE "pulse_input_capture"
#define OUTPUT_AUDIO_SOURCE "pulse_output_capture"
#endif

static const std::string kDefaultInputName = "Mic/Aux";
static const std::string kDefaultOuputName = "Desktop Audio";

// static
static void LogFilter(obs_source_t*, obs_source_t* filter, void* v_val) {}

static bool LoadAudioDevice(const char* name, int channel, obs_data_t* parent) {
  OBSDataAutoRelease data = obs_data_get_obj(parent, name);
  if (!data)
    return false;

  OBSSourceAutoRelease source = obs_load_source(data);
  if (!source)
    return false;

  obs_set_output_source(channel, source);

  const char* source_name = obs_source_get_name(source);
  blog(LOG_INFO, "[Loaded global audio device]: '%s'", source_name);
  obs_source_enum_filters(source, LogFilter, (void*)(intptr_t)1);
  obs_monitoring_type monitoring_type = obs_source_get_monitoring_type(source);
  if (monitoring_type != OBS_MONITORING_TYPE_NONE) {
    const char* type = (monitoring_type == OBS_MONITORING_TYPE_MONITOR_ONLY)
                           ? "monitor only"
                           : "monitor and output";

    blog(LOG_INFO, "    - monitoring: %s", type);
  }

  return true;
}

static void InitFinderliveConfigPath() {
  std::string userpath = FLBaseConfig::GetInstance().GetBaseConfigPath();

  sg_finderlive_obs_config_path = userpath + kFLObsDir;
  sg_finderlive_obs_base_config_path = userpath + kFLObsBasicDir;
  sg_finderlive_obs_base_scene_config_path = userpath + kFLObsBasicScenesDir;
  sg_finderlive_obs_log_config_path = userpath + kFLObsLogDir;
  sg_finderlive_obs_profiler_data_config_path =
      userpath + kFLObsProfilerDataDir;
  sg_finderlive_obs_profiles_config_path = userpath + kFLObsProfilerDir;
  sg_finderlive_obs_crashes_config_path = userpath + kFLObsCrashesDir;
  sg_finderlive_obs_plugin_config_config_path =
      userpath + kFLObsPluginConfigDir;
}

static void OBSLog(int lvl, const char* msg, va_list args, void* p) {
  char stack_buf[4096] = {0};
  int result = vsnprintf(stack_buf, std::size(stack_buf), msg, args);
  std::size_t mem_length = std::size(stack_buf);

  // not process buffer-doubling
  if (result >= 0 && result < mem_length) {
    if (lvl == LOG_ERROR) {
      FLMOJOLOGW(stack_buf);
    } else if (lvl == LOG_WARNING) {
      FLMOJOLOGW(stack_buf);
    } else if (lvl == LOG_INFO) {
      FLMOJOLOGI(stack_buf);
    } else if (lvl == LOG_DEBUG) {
      FLMOJOLOGI(stack_buf);
    }
  }
}

// parse data

static void ParseObsPropertiesInfo(
    const flremoting::protocol::ObsPropertiesInfo& _obs_properties_info,
    flremoting::protocol::ObsPropertyType _info_type,
    ProperyInfo& _info) {
  switch (_info_type) {
    case flremoting::protocol::OBS_PROPERTY_INVALID:
      break;
    case flremoting::protocol::OBS_PROPERTY_BOOL:
      _info.bval = _obs_properties_info.info_bool_value();
      break;
    case flremoting::protocol::OBS_PROPERTY_INT:
      _info.ival = _obs_properties_info.info_int_value();
      break;
    case flremoting::protocol::OBS_PROPERTY_FLOAT:
      _info.dval = _obs_properties_info.info_double_value();
      break;
    case flremoting::protocol::OBS_PROPERTY_TEXT:
      _info.tval = _obs_properties_info.info_string_value();
      break;
    case flremoting::protocol::OBS_PROPERTY_PATH:
      _info.tval = _obs_properties_info.info_string_value();
      break;
    case flremoting::protocol::OBS_PROPERTY_LIST:
      break;
    case flremoting::protocol::OBS_PROPERTY_COLOR:
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

  for (auto& info_double_value : _obs_properties_info.info_double_values()) {
    _info.vecdval.emplace_back(
        std::make_pair(info_double_value.name(), info_double_value.value()));
  }

  for (auto& info_int64_value : _obs_properties_info.info_int64_values()) {
    _info.vecival.emplace_back(
        std::make_pair(info_int64_value.name(), info_int64_value.value()));
  }

  for (auto& info_string_value : _obs_properties_info.info_string_values()) {
    _info.vectval.emplace_back(
        std::make_pair(info_string_value.name(), info_string_value.value()));
  }

  if (_info.vecdval.size()) {
    _info.dval = _obs_properties_info.info_double_value();
  }

  if (_info.vecival.size()) {
    _info.ival = _obs_properties_info.info_int_value();
  }

  if (_info.vectval.size()) {
    _info.tval = _obs_properties_info.info_string_value();
  }
}

static void ParseObsPropertiesData(
    const flremoting::protocol::ObsPropertiesData& _obs_properties_data,
    std::vector<ProperyInfo>& _infos) {
  for (auto& obs_properties_info :
       _obs_properties_data.obs_properties_infos()) {
    auto info_type = obs_properties_info.info_type();
    auto& info_name = obs_properties_info.info_name();

    ProperyInfo info;
    info.pname = info_name;
    info.ptype = static_cast<obs_property_type>(info_type);
    ParseObsPropertiesInfo(obs_properties_info, info_type, info);

    _infos.emplace_back(info);
  }
}

// capture type
enum class CaptureType { None, Game, Window, Monitor };

// casting
enum class casting_status : uint32_t {
  Zero = 0,
  Init,
  Connect,
  Disconnect,
  DirChange,
  AndroidCasting
};

enum class casting_method : uint32_t {
  android_wired = 1,
  android_wireless,
  ios_wired,
  ios_wireless
};

struct casting_status_info {
  casting_status status;
  int extern_status_info;

  // android connect info
  unsigned char ip_str[MAX_PATH];
  uint32_t port;

  casting_status_info() {
    status = casting_status::Zero;
    extern_status_info = 0;
  }
};

static void PhoneCastingEventCallback(casting_status_info info) {
  std::string ip;
  uint32_t port = 0;
  switch (info.status) {
    case casting_status::Connect: {
      SendPhoneCastingInfo(static_cast<uint32_t>(info.status),
                           info.extern_status_info, ip, port);
    } break;
    case casting_status::DirChange: {
      FLOBSManager::GetInstance().OnPhoneCastingDirChange(
          info.extern_status_info);
    } break;
    case casting_status::AndroidCasting: {
      ip = reinterpret_cast<char*>(info.ip_str);
      port = info.port;
      SendPhoneCastingInfo(static_cast<uint32_t>(info.status),
                           info.extern_status_info, ip, port);
    } break;
  }
}

// temp draw
static void TempDisplayFunc(void* data, uint32_t cx, uint32_t cy) {
  if (!data) {
    return;
  }
  uint32_t sourceCX = obs_source_get_width((obs_source_t*)data);
  uint32_t sourceCY = obs_source_get_height((obs_source_t*)data);

  gs_viewport_push();
  gs_projection_push();
  gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY), -100.0f, 100.0f);
  gs_set_viewport(0, 0, cx, cy);

  obs_source_video_render((obs_source_t*)data);

  gs_projection_pop();
  gs_viewport_pop();
}

// main draw
static bool DrawSelectedItem(obs_scene_t* scene,
                             obs_sceneitem_t* item,
                             void* param) {
  if (obs_sceneitem_locked(item))
    return true;

  if (!SceneItemHasVideo(item))
    return true;

  bool hovered = false;
  if (g_ms_window_win) {
    hovered = g_ms_window_win->HasHoverItem(item);
  }

  bool selected = obs_sceneitem_selected(item);

  if (!selected && !hovered)
    return true;

  matrix4 boxTransform;
  matrix4 invBoxTransform;
  obs_sceneitem_get_box_transform(item, &boxTransform);
  matrix4_inv(&invBoxTransform, &boxTransform);

  vec3 bounds[] = {
      {{{0.f, 0.f, 0.f}}},
      {{{1.f, 0.f, 0.f}}},
      {{{0.f, 1.f, 0.f}}},
      {{{1.f, 1.f, 0.f}}},
  };

  vec4 red;
  vec4 green;
  vec4 blue;

  vec4_set(&red, 1.0f, 0.0f, 0.0f, 1.0f);
  vec4_set(&green, 0.0f, 1.0f, 0.0f, 1.0f);
  vec4_set(&blue, 0.0f, 0.5f, 1.0f, 1.0f);

  bool visible =
      std::all_of(std::begin(bounds), std::end(bounds), [&](const vec3& b) {
        vec3 pos;
        vec3_transform(&pos, &b, &boxTransform);
        vec3_transform(&pos, &pos, &invBoxTransform);
        return CloseFloat(pos.x, b.x) && CloseFloat(pos.y, b.y);
      });

  if (!visible)
    return true;

  GS_DEBUG_MARKER_BEGIN(GS_DEBUG_COLOR_DEFAULT, "DrawSelectedItem");

  matrix4 curTransform;
  vec2 boxScale;
  gs_matrix_get(&curTransform);
  obs_sceneitem_get_box_scale(item, &boxScale);
  boxScale.x *= curTransform.x.x;
  boxScale.y *= curTransform.y.y;

  obs_transform_info info;
  obs_sceneitem_get_info(item, &info);

  gs_matrix_push();
  gs_matrix_mul(&boxTransform);

  obs_sceneitem_crop crop;
  obs_sceneitem_get_crop(item, &crop);

  gs_effect_t* eff = gs_get_effect();
  gs_eparam_t* colParam = gs_effect_get_param_by_name(eff, "color");

  if (info.bounds_type == OBS_BOUNDS_NONE && crop_enabled(&crop)) {
#define DRAW_SIDE(side, x1, y1, x2, y2)                  \
  if (hovered && !selected)                              \
    gs_effect_set_vec4(colParam, &blue);                 \
  else if (crop.side > 0)                                \
    gs_effect_set_vec4(colParam, &green);                \
  DrawLine(x1, y1, x2, y2, HANDLE_RADIUS / 2, boxScale); \
  gs_effect_set_vec4(colParam, &red);

    DRAW_SIDE(left, 0.0f, 0.0f, 0.0f, 1.0f);
    DRAW_SIDE(top, 0.0f, 0.0f, 1.0f, 0.0f);
    DRAW_SIDE(right, 1.0f, 0.0f, 1.0f, 1.0f);
    DRAW_SIDE(bottom, 0.0f, 1.0f, 1.0f, 1.0f);
#undef DRAW_SIDE
  } else {
    if (!selected) {
      gs_effect_set_vec4(colParam, &blue);
      DrawRect(HANDLE_RADIUS / 2, boxScale);
    } else {
      DrawRect(HANDLE_RADIUS / 2, boxScale);
    }
  }

  gs_load_vertexbuffer(FLOBSManager::GetInstance().GetGxBox());
  gs_effect_set_vec4(colParam, &red);

  if (selected) {
    // diff obs is add logic
    float itemRot = obs_sceneitem_get_rot(item);
    DrawSquareAtPos(0.0f, 0.0f, itemRot);
    DrawSquareAtPos(0.0f, 1.0f, itemRot);
    DrawSquareAtPos(1.0f, 0.0f, itemRot);
    DrawSquareAtPos(1.0f, 1.0f, itemRot);
    DrawSquareAtPos(0.5f, 0.0f, itemRot);
    DrawSquareAtPos(0.0f, 0.5f, itemRot);
    DrawSquareAtPos(0.5f, 1.0f, itemRot);
    DrawSquareAtPos(1.0f, 0.5f, itemRot);
  }

  gs_matrix_pop();

  GS_DEBUG_MARKER_END();

  UNUSED_PARAMETER(scene);
  UNUSED_PARAMETER(param);
  return true;
}

static void DrawSceneEditing() {
  GS_DEBUG_MARKER_BEGIN(GS_DEBUG_COLOR_DEFAULT, "DrawSceneEditing");

  gs_effect_t* solid = obs_get_base_effect(OBS_EFFECT_SOLID);
  gs_technique_t* tech = gs_effect_get_technique(solid, "Solid");

  vec4 color;
  vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f);
  gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &color);

  gs_technique_begin(tech);
  gs_technique_begin_pass(tech, 0);

  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();

  if (scene) {
    auto scale = GetPreviewScale();
    gs_matrix_push();
    gs_matrix_scale3f(scale, scale, 1.0f);
    obs_scene_enum_items(scene, DrawSelectedItem, nullptr);
    gs_matrix_pop();
  }

  gs_load_vertexbuffer(nullptr);

  gs_technique_end_pass(tech);
  gs_technique_end(tech);

  GS_DEBUG_MARKER_END();
}

// source item scale
static bool MultiplySelectedItemScale(obs_scene_t* scene,
                                      obs_sceneitem_t* item,
                                      void* param) {
  vec2& mul = *reinterpret_cast<vec2*>(param);

  if (obs_sceneitem_is_group(item))
    obs_sceneitem_group_enum_items(item, MultiplySelectedItemScale, param);
  if (!obs_sceneitem_selected(item))
    return true;
  if (obs_sceneitem_locked(item))
    return true;

  vec3 tl = GetItemTL(item);

  vec2 scale;
  obs_sceneitem_get_scale(item, &scale);
  vec2_mul(&scale, &scale, &mul);
  obs_sceneitem_set_scale(item, &scale);

  obs_sceneitem_force_update_transform(item);

  SetItemTL(item, tl);

  UNUSED_PARAMETER(scene);
  return true;
}

// flobsmanager
FLOBSManager::FLOBSManager() {}

FLOBSManager::~FLOBSManager() {}

bool FLOBSManager::Start() {
  return true;
}

bool FLOBSManager::Shutdown() {
  return true;
}

void FLOBSManager::InitObsEnv(const std::string& base_config_path, HWND hwnd) {
  FLBaseConfig::GetInstance().SetBaseConfigPath(base_config_path);
  SetPreviewHWND(hwnd);
}

void FLOBSManager::InitOBSCallbacks() {
  ProfileScope("FLOBSManager::InitOBSCallbacks");

  signalHandlers.reserve(signalHandlers.size() + 7);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_create",
                              FLOBSManager::SourceCreated, this);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_remove",
                              FLOBSManager::SourceRemoved, this);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_activate",
                              FLOBSManager::SourceActivated, this);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_deactivate",
                              FLOBSManager::SourceDeactivated, this);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_audio_activate",
                              FLOBSManager::SourceAudioActivated, this);
  signalHandlers.emplace_back(obs_get_signal_handler(),
                              "source_audio_deactivate",
                              FLOBSManager::SourceAudioDeactivated, this);
  signalHandlers.emplace_back(obs_get_signal_handler(), "source_rename",
                              FLOBSManager::SourceRenamed, this);
}

void FLOBSManager::InitPrimitives() {
  ProfileScope("FLOBSManager::InitPrimitives");

  obs_enter_graphics();

  gs_render_start(true);
  gs_vertex2f(0.0f, 0.0f);
  gs_vertex2f(0.0f, 1.0f);
  gs_vertex2f(1.0f, 0.0f);
  gs_vertex2f(1.0f, 1.0f);
  box = gs_render_save();

  gs_render_start(true);
  gs_vertex2f(0.0f, 0.0f);
  gs_vertex2f(0.0f, 1.0f);
  boxLeft = gs_render_save();

  gs_render_start(true);
  gs_vertex2f(0.0f, 0.0f);
  gs_vertex2f(1.0f, 0.0f);
  boxTop = gs_render_save();

  gs_render_start(true);
  gs_vertex2f(1.0f, 0.0f);
  gs_vertex2f(1.0f, 1.0f);
  boxRight = gs_render_save();

  gs_render_start(true);
  gs_vertex2f(0.0f, 1.0f);
  gs_vertex2f(1.0f, 1.0f);
  boxBottom = gs_render_save();

  gs_render_start(true);
  for (int i = 0; i <= 360; i += (360 / 20)) {
    float pos = RAD(float(i));
    gs_vertex2f(cosf(pos), sinf(pos));
  }
  circle = gs_render_save();

  InitSafeAreas(&actionSafeMargin, &graphicsSafeMargin, &fourByThreeSafeMargin,
                &leftLine, &topLine, &rightLine);
  obs_leave_graphics();
}

bool FLOBSManager::OBSInit() {
  bool result = false;

  do {
    // here should get current utf8 path
    // obs using utf8
    char* absolute_exec_path = os_get_executable_path_ptr(nullptr);
    if (absolute_exec_path == nullptr) {
      break;
    }
    m_currentDir = absolute_exec_path;
    bfree(absolute_exec_path);

    // int config path
    InitFinderliveConfigPath();

    m_currentSceneData.scene = nullptr;
    m_currentSceneData.sceneItem.clear();

    // initlog
    base_set_log_handler(OBSLog, nullptr);

    obs_startup("zh-CN", sg_finderlive_obs_plugin_config_config_path.c_str(),
                nullptr);

    if (!InitializeBasicConfig()) {
      break;
    }

    if (!InitializeGlobalConfig()) {
      break;
    }

    if (!AddAllModulePath()) {
      break;
    }

    if (!ResetAudio()) {
      break;
    }

    if (ResetVideo() != OBS_VIDEO_SUCCESS) {
      break;
    }

    InitPrimitives();
    InitOBSCallbacks();

    obs_load_all_modules();
    obs_post_load_modules();

    if (!Load()) {
      // should uninit callback
      break;
    }

    ResetVolume();

    result = true;
    is_initialize_ = true;

    SendLoadSceneItemData();
  } while (0);

  return result;
}

bool FLOBSManager::OBSUninit() {
  bool result = false;
  if (!is_initialize_) {
    return result;
  }

  FLOBSCameraBeautify::GetInstance().CameraVideoBeautifyIsWorking(false);
  FLOBSCameraBeautify::GetInstance().CameraVideoBeautifyDestoryAll();

  if (camera_display_) {
    auto sceneitem =
        obs_scene_find_source(m_currentSceneData.scene, camera_name_.c_str());
    if (sceneitem) {
      auto source = obs_sceneitem_get_source(sceneitem);
      if (source) {
        TempDisplayCameraDestory(source);
      }
    }
  }

  LiveOutputStop();

  signalHandlers.clear();

  obs_display_remove_draw_callback(m_display, FLOBSManager::RenderMain, this);
  obs_enter_graphics();
  gs_vertexbuffer_destroy(box);
  gs_vertexbuffer_destroy(boxLeft);
  gs_vertexbuffer_destroy(boxTop);
  gs_vertexbuffer_destroy(boxRight);
  gs_vertexbuffer_destroy(boxBottom);
  gs_vertexbuffer_destroy(circle);
  obs_leave_graphics();

  lst_iodevice_.clear();

  SaveProject();

  m_display = nullptr;
  g_ms_window_win = nullptr;

  m_OBSOptions.Clear();
  m_currentSceneData.Clear();
  ClearSceneData();
  m_basicConfig.Close();
  m_globalConfig.Close();
  fadeTransition = nullptr;

  quickTransitionIdCounter = 1;
  obs_volmeter_remove_callback(obs_volmeter_, OBSVolumeLevel, this);
  obs_volmeter_destroy(obs_volmeter_);
  obs_volmeter_ = nullptr;
  base_set_log_handler(nullptr, nullptr);

  // final call it
  obs_shutdown();

  is_initialize_ = false;
  result = true;
  return result;
}

void FLOBSManager::SourceCreated(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  obs_scene_t* scene = obs_scene_from_source(source);
  if (scene) {
    thisObject->AddScene(source);
  }
}

void FLOBSManager::SourceRemoved(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  obs_scene_t* scene = obs_scene_from_source(source);
  if (scene) {
    obs_source_release(source);
  }
}

void FLOBSManager::SourceActivated(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  if (source) {
    uint32_t flags = obs_source_get_output_flags(source);
    if (OBS_SOURCE_AUDIO & flags) {
      FLOBSManager::GetInstance().ActivateAudioSource(source);
    }
  }
}

void FLOBSManager::SourceDeactivated(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  if (source) {
    uint32_t flags = obs_source_get_output_flags(source);
    if (OBS_SOURCE_AUDIO & flags) {
      FLOBSManager::GetInstance().DeactivateAudioSource(source);
    }
  }
}

void FLOBSManager::SourceAudioActivated(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  if (source) {
    FLOBSManager::GetInstance().ActivateAudioSource(source);
  }
}

void FLOBSManager::SourceAudioDeactivated(void* data, calldata_t* params) {
  FLOBSManager* thisObject = reinterpret_cast<FLOBSManager*>(data);
  if (!thisObject) {
    return;
  }
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  if (source) {
    FLOBSManager::GetInstance().DeactivateAudioSource(source);
  }
}

void FLOBSManager::SourceRenamed(void* data, calldata_t* params) {
  obs_source_t* source = (obs_source_t*)calldata_ptr(params, "source");
  const char* newName = calldata_string(params, "new_name");
  const char* prevName = calldata_string(params, "prev_name");
  FLOBSManager* thisObject = static_cast<FLOBSManager*>(data);
  thisObject->RenameSources(source, newName, prevName);
}

void FLOBSManager::RenderMain(void* data, uint32_t cx, uint32_t cy) {
  GS_DEBUG_MARKER_BEGIN(GS_DEBUG_COLOR_DEFAULT, "RenderMain");

  if (!data) {
    return;
  }

  FLOBSManager* thisObject = static_cast<FLOBSManager*>(data);
  obs_video_info ovi;

  obs_get_video_info(&ovi);

  // bool using_mojo = false;
  // thisObject->m_previewCX = int(previewScale * float(ovi.base_width));
  // thisObject->m_previewCY = int(previewScale * float(ovi.base_height));

  gs_viewport_push();
  gs_projection_push();

  obs_display_t* display = thisObject->m_display;
  uint32_t width, height;
  obs_display_size(display, &width, &height);
  float right = float(width) - thisObject->m_previewX;
  float bottom = float(height) - thisObject->m_previewY;

  gs_ortho(-thisObject->m_previewX, right, -thisObject->m_previewY, bottom,
           -100.0f, 100.0f);

  // todo
  // window->ui->preview->DrawOverflow();
  /* --------------------------------------- */

  gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height), -100.0f,
           100.0f);
  gs_set_viewport(thisObject->m_previewX, thisObject->m_previewY,
                  thisObject->m_previewCX, thisObject->m_previewCY);

  /*if (window->IsPreviewProgramMode()) {
    window->DrawBackdrop(float(ovi.base_width), float(ovi.base_height));

    OBSScene scene = window->GetCurrentScene();
    obs_source_t *source = obs_scene_get_source(scene);
    if (source) obs_source_video_render(source);
  } else {
    obs_render_main_texture_src_color_only();
  }*/
  obs_render_main_texture_src_color_only();
  gs_load_vertexbuffer(nullptr);

  /* --------------------------------------- */

  gs_ortho(-thisObject->m_previewX, right, -thisObject->m_previewY, bottom,
           -100.0f, 100.0f);
  gs_reset_viewport();

  DrawSceneEditing();
  /* --------------------------------------- */

  gs_projection_pop();
  gs_viewport_pop();

  GS_DEBUG_MARKER_END();

  UNUSED_PARAMETER(cx);
  UNUSED_PARAMETER(cy);
}

// 3333333333333333333333333
// should lock or seq
void FLOBSManager::GenDeviceInfo(AudioDeviceType _type,
                                 OBSSource _source,
                                 bool _is_update) {
  auto ptr_iodevice = std::make_unique<FLOBSIODevice>(_source, _type);
  lst_iodevice_[_type] = std::move(ptr_iodevice);
}

void FLOBSManager::GenObsDeviceInfo(AudioDeviceType _type,
                                    bool _is_update,
                                    void* _ptr_device_info) {
  flremoting::protocol::ObsDeviceMessage* ptr_device_info =
      static_cast<flremoting::protocol::ObsDeviceMessage*>(_ptr_device_info);
  auto it = lst_iodevice_.find(_type);
  if (it != lst_iodevice_.end()) {
    if (_is_update) {
      it->second->UpdateFilters();
    }
    it->second->SetFiltersValue(_ptr_device_info);
    it->second->SetDeviceSource(_ptr_device_info);
    ptr_device_info->set_id_value(it->second->GetDeviceId());
    ptr_device_info->set_monitoring_value(it->second->GetMonitoringType());
  }
}

void FLOBSManager::GenObsDeviceMessage(AudioDeviceType _type,
                                       bool _is_update,
                                       std::string* _out_msg,
                                       bool _is_activate) {
  flremoting::protocol::ObsDeviceMessage device_msg;

  GenObsDeviceInfo(_type, _is_update, &device_msg);

  if (AudioDeviceType::kInputDevice == _type) {
    device_msg.set_device_type(
        flremoting::protocol::ObsDeviceType::OBS_DEVICE_INPUT_MIC);
  } else if (AudioDeviceType::kOutputDevice == _type) {
    device_msg.set_device_type(
        flremoting::protocol::ObsDeviceType::OBS_DEVICE_OUTPUT_SPEAKER);
  }

  device_msg.set_is_activate(_is_activate);

  device_msg.SerializeToString(_out_msg);
}

void FLOBSManager::ActivateAudioSource(OBSSource _source) {
  if (SourceMixerHidden(_source)) {
    return;
  }
  if (!obs_source_audio_active(_source)) {
    return;
  }

  auto type = AudioDeviceType::KUnknownDevice;

  auto device_id = obs_source_get_id(_source);
  if (stricmp(INPUT_AUDIO_SOURCE, device_id) == 0) {
    type = AudioDeviceType::kInputDevice;
  } else if (stricmp(OUTPUT_AUDIO_SOURCE, device_id) == 0) {
    type = AudioDeviceType::kOutputDevice;
  }

  if (AudioDeviceType::KUnknownDevice == type) {
    return;
  }

  auto it = lst_iodevice_.find(type);
  if (it == lst_iodevice_.end()) {
    GenDeviceInfo(type, _source, true);
    SendIOVolumeMessage(type);
  }

  std::string send_msg;
  GenObsDeviceMessage(type, true, &send_msg, true);
  SendDeviceMessage(send_msg);
}

void FLOBSManager::DeactivateAudioSource(OBSSource _source) {
  auto type = AudioDeviceType::KUnknownDevice;

  auto device_id = obs_source_get_id(_source);
  if (stricmp(INPUT_AUDIO_SOURCE, device_id) == 0) {
    type = AudioDeviceType::kInputDevice;
  } else if (stricmp(OUTPUT_AUDIO_SOURCE, device_id) == 0) {
    type = AudioDeviceType::kOutputDevice;
  }

  auto it = lst_iodevice_.find(type);
  auto result = false;
  if (it != lst_iodevice_.end()) {
    result = it->second->IsSameSource(_source);
  }

  if (result) {
    std::string send_msg;
    GenObsDeviceMessage(type, true, &send_msg, false);
    SendDeviceMessage(send_msg);
  }
}

void FLOBSManager::AddScene(OBSSource source) {
  const char* name = obs_source_get_name(source);
  obs_scene_t* scene = obs_scene_from_source(source);
  m_currentSceneData.scene = scene;
  // QListWidgetItem *item = new QListWidgetItem(QT_UTF8(name));
  // SetOBSRef(item, OBSScene(scene));
  // ui->scenes->addItem(item);

  /*obs_hotkey_register_source(
          source, "OBSBasic.SelectScene", Str("Basic.Hotkeys.SelectScene"),
          [](void *data, obs_hotkey_id, obs_hotkey_t *, bool pressed) {
            OBSBasic *main = reinterpret_cast<OBSBasic
     *>(App()->GetMainWindow());

            auto potential_source = static_cast<obs_source_t *>(data);
            auto source = obs_source_get_ref(potential_source);
            if (source && pressed) main->SetCurrentScene(source);
            obs_source_release(source);
          },
          static_cast<obs_source_t *>(source));*/

  signal_handler_t* handler = obs_source_get_signal_handler(source);
  m_currentSceneData.sceneContainer.Clear();
  m_currentSceneData.sceneContainer.ref = scene;
  m_currentSceneData.sceneContainer.handlers.assign({
      std::make_shared<OBSSignal>(handler, "item_add",
                                  FLOBSManager::SceneItemAddCallback, this),
      std::make_shared<OBSSignal>(handler, "item_remove",
                                  FLOBSManager::SceneItemRemovedCallback, this),
      // std::make_shared<OBSSignal>(handler, "item_select",
      // OBSManager::SceneItemSelected, this),
      // std::make_shared<OBSSignal>(handler, "item_deselect",
      // OBSManager::SceneItemDeselected, this),
      std::make_shared<OBSSignal>(handler, "reorder",
                                  FLOBSManager::SceneItemReorderCallback, this)
      // std::make_shared<OBSSignal>(handler, "refresh",
      // OBSManager::SceneRefreshed, this),
  });

  // item->setData(static_cast<int>(QtDataRole::OBSSignals),
  // QVariant::fromValue(container));

  /* if the scene already has items (a duplicated scene) add them */
  auto addSceneItem = [this](obs_sceneitem_t* item) { AddSceneItem(item); };

  using addSceneItem_t = decltype(addSceneItem);

  obs_scene_enum_items(
      scene,
      [](obs_scene_t*, obs_sceneitem_t* item, void* param) {
        addSceneItem_t* func;
        func = reinterpret_cast<addSceneItem_t*>(param);
        (*func)(item);
        return true;
      },
      &addSceneItem);

  /*if (!disableSaving) {
    obs_source_t *source = obs_scene_get_source(scene);
    blog(LOG_INFO, "User added scene '%s'", obs_source_get_name(source));

    OBSProjector::UpdateMultiviewProjectors();
  }*/

  // if (api) api->on_event(OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED);
}

// void FLOBSManager::LoadSceneItem(OBSSource source) {
//   if (!source) {
//     return;
//   }
//
//   auto item = obs_scene_sceneitem_from_source(m_currentSceneData.scene,
//   source); if (!item) {
//     return;
//   }
//
//   auto item_visible = obs_sceneitem_visible(item);
//   auto item_id = obs_sceneitem_get_id(item);
//   auto source_type = obs_source_get_unversioned_id(source);
//   auto source_width = obs_source_get_width(source);
//   auto source_height = obs_source_get_height(source);
//
//   std::string source_data;
//   auto source_name = obs_source_get_name(source);
//   auto source_id = obs_source_get_id(source);
//
//   if (stricmp(source_type, kTextInputSourceId) == 0) {
//     std::string face = "Arial";
//     int32_t flags = 0;
//     std::string style = "Regular";
//     int32_t size = 256;
//     OBSData settings = obs_source_get_settings(source);
//     if (settings) {
//       OBSData objdata = obs_data_get_obj(settings, "font");
//       face = obs_data_get_string(objdata, "face");
//       flags = obs_data_get_int(objdata, "flags");
//       style = obs_data_get_string(objdata, "style");
//       size = obs_data_get_int(objdata, "size");
//       obs_data_release(settings);
//     }
//
//     static const char* kInputTextProName = "text";
//     static const char* kInputColorProName = "color";
//     static const char* kInputBkProName = "bk_color";
//
//     ProperyInfo info1, info2, info3;
//     GetInputSourcePropety(source, kInputTextProName, info1);
//     GetInputSourcePropety(source, kInputColorProName, info2);
//     GetInputSourcePropety(source, kInputBkProName, info3);
//
//     SendTextSourceData(item_visible, item_id, source_width, source_height,
//                        source_id, source_name, face, flags, style, size,
//                        {info1, info2, info3});
//   }
// }

void FLOBSManager::AddSceneItem(OBSSceneItem item) {
  obs_scene_t* scene = obs_sceneitem_get_scene(item);
  if (m_currentSceneData.scene == scene) {
    m_currentSceneData.sceneItem.push_back(item);
  }
  if (is_initialize_) {
    // 初始化加载的时候源
    // 还获取不到具体的item id
    SendSceneItemData(item);
  }
}

void FLOBSManager::RemoveSceneItem(OBSSceneItem item) {
  auto source = obs_sceneitem_get_source(item);
  const char* source_id = obs_source_get_id(source);

  if (_stricmp(source_id, "x_phone_casting") == 0) {
    OnPhoneCastingDelete();
  } else if (_stricmp(source_id, kCameraSourceId) == 0) {
    // can move to ObsItemActionType::OBS_ITEMACTION_REMOVE
    auto item_id = obs_sceneitem_get_id(item);
    FLOBSCameraBeautify::GetInstance().CameraVideoBeautifyDestory(item_id);
  }

  obs_source_remove(source);
}

void FLOBSManager::ReoderSceneItem(OBSScene scene) {
  // replace by edit
}

void FLOBSManager::RenameSources(OBSSource source,
                                 const char* name,
                                 const char* prevName) {}

void FLOBSManager::SceneItemAddCallback(void* data, calldata_t* params) {
  auto thisObject = static_cast<FLOBSManager*>(data);
  obs_sceneitem_t* item = (obs_sceneitem_t*)calldata_ptr(params, "item");
  thisObject->AddSceneItem(item);
}

void FLOBSManager::SceneItemRemovedCallback(void* data, calldata_t* params) {
  auto thisObject = static_cast<FLOBSManager*>(data);
  obs_sceneitem_t* item = (obs_sceneitem_t*)calldata_ptr(params, "item");
  thisObject->RemoveSceneItem(item);
}

void FLOBSManager::SceneItemReorderCallback(void* data, calldata_t* params) {
  auto thisObject = static_cast<FLOBSManager*>(data);
  obs_scene_t* scene = (obs_scene_t*)calldata_ptr(params, "scene");
  thisObject->ReoderSceneItem(scene);
}

bool FLOBSManager::Load() {
  defaultConfigFilePath_ =
      sg_finderlive_obs_base_scene_config_path + "\\default.json";
  OBSDataAutoRelease data = obs_data_create_from_json_file_safe(
      defaultConfigFilePath_.c_str(), "bak");
  if (!data) {
    // create default scene
    CreateDefaultScene(true);
    SaveProject();
    return true;
  }

  ClearSceneData();
  InitDefaultTransitions();

  OBSDataAutoRelease modulesObj = obs_data_get_obj(data, "modules");
  // if (api) api->on_preload(modulesObj);

  OBSDataArrayAutoRelease sceneOrder = obs_data_get_array(data, "scene_order");
  OBSDataArrayAutoRelease sources = obs_data_get_array(data, "sources");
  OBSDataArrayAutoRelease groups = obs_data_get_array(data, "groups");
  OBSDataArrayAutoRelease transitions = obs_data_get_array(data, "transitions");
  const char* sceneName = obs_data_get_string(data, "current_scene");
  const char* programSceneName =
      obs_data_get_string(data, "current_program_scene");
  const char* transitionName = obs_data_get_string(data, "current_transition");

  /*if (!opt_starting_scene.empty()) {
    programSceneName = opt_starting_scene.c_str();
    if (!IsPreviewProgramMode()) sceneName = opt_starting_scene.c_str();
  }*/

  std::int64_t newDuration = obs_data_get_int(data, "transition_duration");
  if (!newDuration)
    newDuration = 300;

  if (!transitionName)
    transitionName = obs_source_get_name(fadeTransition);

  const char* curSceneCollection =
      config_get_string(m_globalConfig, "Basic", "SceneCollection");

  obs_data_set_default_string(data, "name", curSceneCollection);

  const char* name = obs_data_get_string(data, "name");
  OBSSourceAutoRelease curScene;
  OBSSourceAutoRelease curProgramScene;
  obs_source_t* curTransition;

  if (!name || !*name)
    name = curSceneCollection;

  // create if missing default input and output
  if (!LoadAudioDevice(DESKTOP_AUDIO, kChannel_1_destopAudio, data)) {
    if (HasAudioDevices(kOutputAudioSource)) {
      ResetAudioDevice(kOutputAudioSource, "default", kDefaultOuputName.c_str(),
                       kChannel_1_destopAudio);
    }
  }
  if (!LoadAudioDevice(AUX_AUDIO_1, kChannel_3_Mic, data)) {
    if (HasAudioDevices(kInputAudioSource)) {
      ResetAudioDevice(kInputAudioSource, "default", kDefaultInputName.c_str(),
                       kChannel_3_Mic);
    }
  }

  LoadAudioDevice(DESKTOP_AUDIO_2, kChannel_2, data);
  LoadAudioDevice(AUX_AUDIO_2, kChannel_4, data);
  LoadAudioDevice(AUX_AUDIO_3, kChannel_5, data);
  LoadAudioDevice(AUX_AUDIO_4, kChannel_6, data);

  if (!sources) {
    sources = std::move(groups);
  } else {
    obs_data_array_push_back_array(sources, groups);
  }

  // private
  // 3333333333333333333333333
  // 专门为手机源作加载时的 settings 设置，保证加载时，callback 地址就传递给插件
  size_t count = obs_data_array_count(sources);
  for (int i = 0; i < count; i++) {
    OBSDataAutoRelease source_data = obs_data_array_item(sources, i);
    auto source_id = obs_data_get_string(source_data, "id");
    if (_stricmp(source_id, kPhoneCastingSourceId) == 0) {
      OBSDataAutoRelease settings = obs_data_get_obj(source_data, "settings");
      obs_data_set_int(settings, "x_phone_casting.event_callback",
                       (long long)(&PhoneCastingEventCallback));
    } else if (_stricmp(source_id, kCameraSourceId) == 0) {
      OBSDataAutoRelease settings = obs_data_get_obj(source_data, "settings");
      obs_data_set_int(settings, "VideoBeautifyCallback",
                       (long long)&OnCameraVideoBeautifyCallback);
    }
  }

  obs_load_sources(sources, nullptr, nullptr);

  if (transitions) {
    LoadTransitions(transitions);
  }
  if (sceneOrder) {
    LoadSceneListOrder(sceneOrder);
  }

  curTransition = FindTransition(transitionName);
  if (!curTransition)
    curTransition = fadeTransition;

  // ui->transitionDuration->setValue(newDuration);
  SetTransition(curTransition);

retryScene:
  curScene = obs_get_source_by_name(sceneName);
  curProgramScene = obs_get_source_by_name(programSceneName);

  /* if the starting scene command line parameter is bad at all,
   * fall back to original settings */
  /*if (!opt_starting_scene.empty() && (!curScene || !curProgramScene)) {
    sceneName = obs_data_get_string(data, "current_scene");
    programSceneName = obs_data_get_string(data, "current_program_scene");
    obs_source_release(curScene);
    obs_source_release(curProgramScene);
    opt_starting_scene.clear();
    goto retryScene;
  }*/

  if (!curProgramScene) {
    curProgramScene = std::move(curScene);
  }

  SetCurrentScene(curScene.Get(), true);
  // if (IsPreviewProgramMode()) TransitionToScene(curProgramScene, true);

  /* ------------------- */

  bool projectorSave =
      config_get_bool(m_globalConfig, "BasicWindow", "SaveProjectors");

  if (projectorSave) {
    OBSDataArrayAutoRelease savedProjectors =
        obs_data_get_array(data, "saved_projectors");

    if (savedProjectors) {
      // LoadSavedProjectors(savedProjectors);
      // OpenSavedProjectors();
      // activateWindow();
    }
  }

  /* ------------------- */

  // std::string file_base = strrchr(m_defaultConfigPath.c_str(), '/') + 1;
  // file_base.erase(file_base.size() - 5, 5);

  config_set_string(m_globalConfig, "Basic", "SceneCollection", name);
  config_set_string(m_globalConfig, "Basic", "SceneCollectionFile", "default");

  OBSDataArrayAutoRelease quickTransitionData =
      obs_data_get_array(data, "quick_transitions");
  LoadQuickTransitions(quickTransitionData);

  RefreshQuickTransitions();

  bool previewLocked = obs_data_get_bool(data, "preview_locked");
  // ui->preview->SetLocked(previewLocked);
  // ui->actionLockPreview->setChecked(previewLocked);

  /* ---------------------- */

  bool fixedScaling = obs_data_get_bool(data, "scaling_enabled");
  int scalingLevel = (int)obs_data_get_int(data, "scaling_level");
  float scrollOffX = (float)obs_data_get_double(data, "scaling_off_x");
  float scrollOffY = (float)obs_data_get_double(data, "scaling_off_y");

  if (fixedScaling) {
    // ui->preview->SetScalingLevel(scalingLevel);
    // ui->preview->SetScrollingOffset(scrollOffX, scrollOffY);
  }
  // ui->preview->SetFixedScaling(fixedScaling);
  // emit ui->preview->DisplayResized();

  /* ---------------------- */

  // if (api) api->on_load(modulesObj);

  // if (!opt_starting_scene.empty()) opt_starting_scene.clear();

  // if (opt_start_streaming) {
  //  blog(LOG_INFO, "Starting stream due to command line parameter");
  //  QMetaObject::invokeMethod(this, "StartStreaming", Qt::QueuedConnection);
  //  opt_start_streaming = false;
  //}

  // if (opt_start_recording) {
  //  blog(LOG_INFO, "Starting recording due to command line parameter");
  //  QMetaObject::invokeMethod(this, "StartRecording", Qt::QueuedConnection);
  //  opt_start_recording = false;
  //}

  // if (opt_start_replaybuffer) {
  // QMetaObject::invokeMethod(this, "StartReplayBuffer", Qt::QueuedConnection);
  // opt_start_replaybuffer = false;
  //}

  // copyStrings.clear();
  // copyFiltersString = nullptr;

  // LogScenes();
  return true;
}

bool FLOBSManager::CreateDefaultScene(bool firstStart) {
  ClearSceneData();
  InitDefaultTransitions();
  CreateDefaultQuickTransitions();
  SetTransition(fadeTransition);
  OBSSceneAutoRelease scene = obs_scene_create("default_scene");
  obs_source_t* source = obs_scene_get_source(scene);
  SetCurrentScene(source, true);
  if (firstStart) {
    CreateFirstRunSources();
  }
  return true;
}

void FLOBSManager::InitTransition(obs_source_t* transition) {
  auto onTransitionStop = [](void* data, calldata_t*) {
    FLOBSManager* window = (FLOBSManager*)data;
    // QMetaObject::invokeMethod(window, "TransitionStopped",
    // Qt::QueuedConnection);
  };

  auto onTransitionFullStop = [](void* data, calldata_t*) {
    FLOBSManager* window = (FLOBSManager*)data;
    // QMetaObject::invokeMethod(window, "TransitionFullyStopped",
    // Qt::QueuedConnection);
  };

  signal_handler_t* handler = obs_source_get_signal_handler(transition);
  signal_handler_connect(handler, "transition_video_stop", onTransitionStop,
                         this);
  signal_handler_connect(handler, "transition_stop", onTransitionFullStop,
                         this);
}

void FLOBSManager::LoadTransitions(obs_data_array_t* transitions) {
  size_t count = obs_data_array_count(transitions);

  for (size_t i = 0; i < count; i++) {
    OBSDataAutoRelease item = obs_data_array_item(transitions, i);
    const char* name = obs_data_get_string(item, "name");
    const char* id = obs_data_get_string(item, "id");
    OBSDataAutoRelease settings = obs_data_get_obj(item, "settings");

    obs_source_t* source = obs_source_create_private(id, name, settings);
    if (!obs_obj_invalid(source)) {
      InitTransition(source);
      // todo
      // ui->transitions->addItem(QT_UTF8(name),
      // QVariant::fromValue(OBSSource(source)));
      // ui->transitions->setCurrentIndex(ui->transitions->count() - 1);
      m_transitions.push_back(std::make_pair(name, source));
    }

    obs_source_release(source);
  }
}

void FLOBSManager::SetTransition(OBSSource transition) {
  OBSSourceAutoRelease oldTransition = obs_get_output_source(0);

  if (oldTransition && transition) {
    obs_transition_swap_begin(transition, oldTransition);
    // if (transition != GetCurrentTransition())
    // SetComboTransition(ui->transitions, transition);
    obs_set_output_source(0, transition);
    obs_transition_swap_end(transition, oldTransition);
  } else {
    obs_set_output_source(0, transition);
  }

  bool fixed = transition ? obs_transition_fixed(transition) : false;
  // ui->transitionDurationLabel->setVisible(!fixed);
  // ui->transitionDuration->setVisible(!fixed);

  bool configurable = obs_source_configurable(transition);
  // ui->transitionRemove->setEnabled(configurable);
  // ui->transitionProps->setEnabled(configurable);

  // if (api) api->on_event(OBS_FRONTEND_EVENT_TRANSITION_CHANGED);
}

void FLOBSManager::TransitionToScene(OBSSource source,
                                     bool force,
                                     bool quickTransition,
                                     int quickDuration,
                                     bool black,
                                     bool manual) {
  obs_scene_t* scene = obs_scene_from_source(source);
  bool usingPreviewProgram = false;
  if (!scene)
    return;

  OBSWeakSource lastProgramScene;

  OBSSourceAutoRelease transition = obs_get_output_source(0);
  if (!transition) {
    return;
  }

  float t = obs_transition_get_time(transition);
  bool stillTransitioning = t < 1.0f && t > 0.0f;

  // If actively transitioning, block new transitions from starting

  if (force) {
    obs_transition_set(transition, source);
  } /*else {
    int duration = ui->transitionDuration->value();

    //check for scene override
    OBSSource trOverride = GetOverrideTransition(source);

    if (trOverride && !overridingTransition && !quickTransition) {
          transition = trOverride;
          duration = GetOverrideTransitionDuration(source);
          OverrideTransition(trOverride);
          overridingTransition = true;
    }

    if (black && !prevFTBSource) {
          source = nullptr;
          prevFTBSource = obs_transition_get_active_source(transition);
          obs_source_release(prevFTBSource);
    } else if (black && prevFTBSource) {
          source = prevFTBSource;
          prevFTBSource = nullptr;
    } else if (!black) {
          prevFTBSource = nullptr;
    }

    if (quickTransition) duration = quickDuration;

    enum obs_transition_mode mode = manual ? OBS_TRANSITION_MODE_MANUAL :
  OBS_TRANSITION_MODE_AUTO;

    EnableTransitionWidgets(false);

    bool success = obs_transition_start(transition, mode, duration, source);

    if (!success) TransitionFullyStopped();
  }*/

  // cleanup:
  // if (usingPreviewProgram && sceneDuplicationMode) obs_scene_release(scene);
}

void FLOBSManager::CreateFirstRunSources() {
  bool hasDesktopAudio = HasAudioDevices(kOutputAudioSource);
  bool hasInputAudio = HasAudioDevices(kInputAudioSource);
  if (hasDesktopAudio) {
    ResetAudioDevice(kOutputAudioSource, "default", kDefaultOuputName.c_str(),
                     1);
  }
  if (hasInputAudio) {
    ResetAudioDevice(kInputAudioSource, "default", kDefaultInputName.c_str(),
                     3);
  }
}

void FLOBSManager::UpdateSceneSelection(OBSSource source) {
  if (source) {
    obs_scene_t* scene = obs_scene_from_source(source);
    const char* name = obs_source_get_name(source);

    if (!scene)
      return;

    // QList<QListWidgetItem *> items = ui->scenes->findItems(QT_UTF8(name),
    // Qt::MatchExactly);

    /*if (items.count()) {
      sceneChanging = true;
      ui->scenes->setCurrentItem(items.first());
      sceneChanging = false;

      OBSScene curScene = GetOBSRef<OBSScene>(ui->scenes->currentItem());
      if (api && scene != curScene)
    api->on_event(OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED);
    }*/
  }
}

void FLOBSManager::LoadSceneListOrder(obs_data_array_t* array) {
  size_t num = obs_data_array_count(array);

  for (size_t i = 0; i < num; i++) {
    OBSDataAutoRelease data = obs_data_array_item(array, i);
    const char* name = obs_data_get_string(data, "name");

    //    ReorderItemByName(ui->scenes, name, (int)i);
  }
}

obs_source_t* FLOBSManager::FindTransition(const char* name) {
  for (auto& item : m_transitions) {
    OBSSource tr = item.second;
    if (!tr)
      continue;
    const char* trName = obs_source_get_name(tr);
    if (trName && *trName && strcmp(trName, name) == 0)
      return tr;
  }
  return nullptr;
}

void FLOBSManager::AddQuickTransitionHotkey(QuickTransition* qt) {}

void FLOBSManager::RemoveQuickTransitionHotkey(QuickTransition* qt) {
  obs_hotkey_unregister(qt->hotkey);
}

OBSScene FLOBSManager::GetCurrentScene() {
  return m_currentSceneData.scene;
}

ConfigFile& FLOBSManager::GetGlobalConfig() {
  return m_globalConfig;
}

ConfigFile& FLOBSManager::GetBasicConfig() {
  return m_basicConfig;
}

void FLOBSManager::SetCurrentScene(OBSSource scene, bool force) {
  // OBSSource transition = obs_get_output_source(0);
  // obs_source_release(transition);

  // obs_transition_set(transition, scene);

  // if (!IsPreviewProgramMode()) {
  TransitionToScene(scene, force);
  //} else {
  //  OBSSource actualLastScene = OBSGetStrongRef(lastScene);
  //  if (actualLastScene != scene) {
  //    if (scene) obs_source_inc_showing(scene);
  //    if (actualLastScene) obs_source_dec_showing(actualLastScene);
  //    lastScene = OBSGetWeakRef(scene);
  //  }
  //}

  if (obs_scene_get_source(GetCurrentScene()) != scene) {
    /*for (int i = 0; i < ui->scenes->count(); i++) {
      QListWidgetItem *item = ui->scenes->item(i);
      OBSScene itemScene = GetOBSRef<OBSScene>(item);
      obs_source_t *source = obs_scene_get_source(itemScene);

      if (source == scene) {
            ui->scenes->blockSignals(true);
            ui->scenes->setCurrentItem(item);
            ui->scenes->blockSignals(false);
            if (api) api->on_event(OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED);
            break;
      }
    }*/
  }

  UpdateSceneSelection(scene);

  // if (scene) {
  //  bool userSwitched = (!force && !disableSaving);
  //  blog(LOG_INFO, "%s to scene '%s'", userSwitched ? "User switched" :
  //  "Switched",
  //       obs_source_get_name(scene));
  //}
}

void FLOBSManager::ClearSceneData() {
  ClearQuickTransitions();
  SetTransition(nullptr);
  obs_set_output_source(0, nullptr);
  obs_set_output_source(1, nullptr);
  obs_set_output_source(2, nullptr);
  obs_set_output_source(3, nullptr);
  obs_set_output_source(4, nullptr);
  obs_set_output_source(5, nullptr);

  auto cb = [](void* unused, obs_source_t* source) {
    obs_source_remove(source);
    UNUSED_PARAMETER(unused);
    return true;
  };

  obs_enum_scenes(cb, nullptr);
  obs_enum_sources(cb, nullptr);
}

void FLOBSManager::ClearQuickTransitions() {
  for (QuickTransition& qt : quickTransitions) {
    RemoveQuickTransitionHotkey(&qt);
  }
  quickTransitions.clear();
  m_transitions.clear();
}

void FLOBSManager::InitDefaultTransitions() {
  std::vector<OBSSource> transitions;
  size_t idx = 0;
  const char* id;

  /* automatically add transitions that have no configuration (things
   * such as cut/fade/etc) */
  while (obs_enum_transition_types(idx++, &id)) {
    if (!obs_is_source_configurable(id)) {
      const char* name = obs_source_get_display_name(id);

      OBSSourceAutoRelease tr = obs_source_create_private(id, name, NULL);
      InitTransition(tr);
      transitions.emplace_back(tr);

      if (strcmp(id, "fade_transition") == 0) {
        fadeTransition = tr;
      }
    }
  }

  for (OBSSource& tr : transitions) {
    // ui->transitions->addItem(QT_UTF8(obs_source_get_name(tr)),
    // QVariant::fromValue(OBSSource(tr)));
    m_transitions.push_back(
        std::make_pair(obs_source_get_name(tr), OBSSource(tr)));
  }
}

void FLOBSManager::CreateDefaultQuickTransitions() {
  quickTransitions.emplace_back(m_transitions[0].second, 300, 1);
  quickTransitions.emplace_back(m_transitions[1].second, 300, 2);
  quickTransitions.emplace_back(m_transitions[1].second, 300, 3, true);
}

void FLOBSManager::LoadQuickTransitions(obs_data_array_t* array) {
  size_t count = obs_data_array_count(array);

  quickTransitionIdCounter = 1;

  for (size_t i = 0; i < count; i++) {
    OBSDataAutoRelease data = obs_data_array_item(array, i);
    OBSDataArrayAutoRelease hotkeys = obs_data_get_array(data, "hotkeys");
    const char* name = obs_data_get_string(data, "name");
    std::int64_t duration = obs_data_get_int(data, "duration");
    std::int64_t id = obs_data_get_int(data, "id");
    bool toBlack = obs_data_get_bool(data, "fade_to_black");

    if (id) {
      obs_source_t* source = FindTransition(name);
      if (source) {
        quickTransitions.emplace_back(source, duration, id, toBlack);

        if (quickTransitionIdCounter <= id)
          quickTransitionIdCounter = id + 1;

        int idx = (int)quickTransitions.size() - 1;
        AddQuickTransitionHotkey(&quickTransitions[idx]);
        obs_hotkey_load(quickTransitions[idx].hotkey, hotkeys);
      }
    }
  }
}

void FLOBSManager::RefreshQuickTransitions() {}

bool FLOBSManager::SaveProject() {
  bool result = false;

  if (!is_initialize_) {
    return result;
  }

  OBSScene scene = GetCurrentScene();
  OBSSource curProgramScene = obs_scene_get_source(scene);

  OBSDataArrayAutoRelease sceneOrder = SaveSceneListOrder();
  OBSDataArrayAutoRelease transitions = SaveTransitions();
  OBSDataArrayAutoRelease quickTrData = SaveQuickTransitions();
  OBSDataAutoRelease saveData = GenerateSaveData(
      sceneOrder, quickTrData, 300, transitions, scene, curProgramScene);

  auto save_path = defaultConfigFilePath_.c_str();
  if (!obs_data_save_json_safe(saveData, save_path, "tmp", "bak")) {
    blog(LOG_ERROR, "Could not save scene data to %s", save_path);
  } else {
    result = true;
  }

  return result;
}

obs_data_array_t* FLOBSManager::SaveSceneListOrder() {
  obs_data_array_t* sceneOrder = obs_data_array_create();
  OBSDataAutoRelease data = obs_data_create();
  obs_data_set_string(data, "name", "default_scene");
  obs_data_array_push_back(sceneOrder, data);
  return sceneOrder;
}

obs_data_t* FLOBSManager::GenerateSaveData(
    obs_data_array_t* sceneOrder,
    obs_data_array_t* quickTransitionData,
    int transitionDuration,
    obs_data_array_t* transitions,
    OBSScene& scene,
    OBSSource& curProgramScene) {
  obs_data_t* saveData = obs_data_create();

  std::vector<OBSSource> audioSources;
  audioSources.reserve(6);

  SaveAudioDevice(DESKTOP_AUDIO, 1, saveData, audioSources);
  SaveAudioDevice(DESKTOP_AUDIO_2, 2, saveData, audioSources);
  SaveAudioDevice(AUX_AUDIO_1, 3, saveData, audioSources);
  SaveAudioDevice(AUX_AUDIO_2, 4, saveData, audioSources);
  SaveAudioDevice(AUX_AUDIO_3, 5, saveData, audioSources);
  SaveAudioDevice(AUX_AUDIO_4, 6, saveData, audioSources);

  /* -------------------------------- */
  /* save non-group sources           */

  auto FilterAudioSources = [&](obs_source_t* source) {
    if (obs_source_is_group(source))
      return false;

    return std::find(std::begin(audioSources), std::end(audioSources),
                     source) == std::end(audioSources);
  };
  using FilterAudioSources_t = decltype(FilterAudioSources);

  obs_data_array_t* sourcesArray = obs_save_sources_filtered(
      [](void* data, obs_source_t* source) {
        return (*static_cast<FilterAudioSources_t*>(data))(source);
      },
      static_cast<void*>(&FilterAudioSources));

  /* -------------------------------- */
  /* save group sources separately    */

  /* saving separately ensures they won't be loaded in older versions */
  obs_data_array_t* groupsArray = obs_save_sources_filtered(
      [](void*, obs_source_t* source) { return obs_source_is_group(source); },
      nullptr);

  /* -------------------------------- */

  OBSSourceAutoRelease transition = obs_get_output_source(0);
  obs_source_t* currentScene = obs_scene_get_source(scene);
  const char* sceneName = obs_source_get_name(currentScene);
  const char* programName = obs_source_get_name(curProgramScene);

  const char* sceneCollection =
      config_get_string(m_globalConfig, "Basic", "SceneCollection");

  obs_data_set_string(saveData, "current_scene", sceneName);
  obs_data_set_string(saveData, "current_program_scene", programName);
  obs_data_set_array(saveData, "scene_order", sceneOrder);
  obs_data_set_string(saveData, "name", sceneCollection);
  obs_data_set_array(saveData, "sources", sourcesArray);
  obs_data_set_array(saveData, "groups", groupsArray);
  obs_data_set_array(saveData, "quick_transitions", quickTransitionData);
  obs_data_set_array(saveData, "transitions", transitions);
  // obs_data_set_array(saveData, "saved_projectors", savedProjectorList);
  obs_data_array_release(sourcesArray);
  obs_data_array_release(groupsArray);

  obs_data_set_string(saveData, "current_transition",
                      obs_source_get_name(transition));
  obs_data_set_int(saveData, "transition_duration", transitionDuration);

  return saveData;
}

obs_data_array_t* FLOBSManager::SaveTransitions() {
  obs_data_array_t* transitions = obs_data_array_create();
  for (auto& item : m_transitions) {
    OBSSource tr = item.second;
    if (!obs_source_configurable(tr))
      continue;

    OBSDataAutoRelease sourceData = obs_data_create();
    OBSDataAutoRelease settings = obs_source_get_settings(tr);

    obs_data_set_string(sourceData, "name", obs_source_get_name(tr));
    obs_data_set_string(sourceData, "id", obs_obj_get_id(tr));
    obs_data_set_obj(sourceData, "settings", settings);

    obs_data_array_push_back(transitions, sourceData);
  }

  return transitions;
}

obs_data_array_t* FLOBSManager::SaveQuickTransitions() {
  obs_data_array_t* array = obs_data_array_create();

  for (QuickTransition& qt : quickTransitions) {
    OBSDataAutoRelease data = obs_data_create();
    OBSDataArrayAutoRelease hotkeys = obs_hotkey_save(qt.hotkey);

    obs_data_set_string(data, "name", obs_source_get_name(qt.source));
    obs_data_set_int(data, "duration", qt.duration);
    obs_data_set_array(data, "hotkeys", hotkeys);
    obs_data_set_int(data, "id", qt.id);
    obs_data_set_bool(data, "fade_to_black", qt.fadeToBlack);

    obs_data_array_push_back(array, data);
  }

  return array;
}

int FLOBSManager::ResetVideo() {
  ProfileScope("FLOBSManager::ResestVideo");
  int result = 0;
  struct obs_video_info ovi;

  ovi.fps_num = (uint32_t)config_get_uint(m_basicConfig, "Video", "FPSNum");
  ovi.fps_den = (uint32_t)config_get_uint(m_basicConfig, "Video", "FPSDen");

  // using remote set fpsnum
  ovi.fps_num = GetFPSNum() ? GetFPSNum() : ovi.fps_num;

  std::string graphics_module = m_currentDir + "/libobs-d3d11.dll";
  ovi.graphics_module = graphics_module.c_str();
  const char* colorFormat =
      config_get_string(m_basicConfig, "Video", "ColorFormat");
  const char* colorSpace =
      config_get_string(m_basicConfig, "Video", "ColorSpace");
  const char* colorRange =
      config_get_string(m_basicConfig, "Video", "ColorRange");

  ovi.base_width = (uint32_t)config_get_uint(m_basicConfig, "Video", "BaseCX");
  ovi.base_height = (uint32_t)config_get_uint(m_basicConfig, "Video", "BaseCY");

  if (ovi.base_width > ovi.base_height) {
    m_canvas_mode_ = kHorizontalMode;
  } else {
    m_canvas_mode_ = kVerticalMode;
  }

  canvas_size_.cx = ovi.base_width;
  canvas_size_.cy = ovi.base_height;
  ovi.output_width = ovi.base_width;
  ovi.output_height = ovi.base_height;
  ovi.output_format = GetVideoFormatFromName(colorFormat);
  ovi.colorspace =
      astrcmpi(colorSpace, "601") == 0 ? VIDEO_CS_601 : VIDEO_CS_709;
  ovi.range = astrcmpi(colorRange, "Full") == 0 ? VIDEO_RANGE_FULL
                                                : VIDEO_RANGE_PARTIAL;
  ovi.adapter =
      0;  // config_get_uint(App()->GlobalConfig(), "Video", "AdapterIdx");
  ovi.gpu_conversion = true;
  ovi.scale_type = GetScaleType(m_basicConfig);

  result = obs_reset_video(&ovi);
  if (result != OBS_VIDEO_SUCCESS) {
    if (result == OBS_VIDEO_CURRENTLY_ACTIVE) {
      blog(LOG_WARNING,
           "Tried to reset when "
           "already active");
      return result;
    }
    /* Try OpenGL if DirectX fails on windows */
    // if (astrcmpi(ovi.graphics_module, "/libobs-opengl.dll") != 0) {
    blog(LOG_WARNING,
         "Failed to initialize obs video (%d) "
         "with graphics_module='%s', retrying "
         "with graphics_module='%s'",
         result, ovi.graphics_module, "libobs-opengl.dll");
    std::string graphics_module = m_currentDir + "/libobs-opengl.dll";
    ovi.graphics_module = graphics_module.c_str();
    result = obs_reset_video(&ovi);
    //}
  } else if (result == OBS_VIDEO_SUCCESS) {
    CanvasInfo canvasInfo;
    GetBasicConfigVideoCanvas(canvasInfo);
    base_window_size_ = canvasInfo.sz;
    SendPreviewCanvasMessage(canvasInfo.sz.cx, canvasInfo.sz.cy,
                             canvasInfo.res_type, canvasInfo.real_type_num.cx,
                             canvasInfo.real_type_num.cy,
                             canvasInfo.is_portrait);
  }

  return result;
}

void FLOBSManager::SetVideoFPS() {
  obs_video_info ovi;
  obs_get_video_info(&ovi);
  auto obs_fps_num = ovi.fps_num;
  auto fps_num = GetFPSNum() ? GetFPSNum() : obs_fps_num;
  // fl not using fps_den
  // auto fps_den = GetFPSDen() ? GetFPSDen() : ovi.fps_den;
  if (ovi.fps_num != fps_num) {
    ovi.fps_num = fps_num;
    if (OBS_VIDEO_SUCCESS != obs_reset_video(&ovi)) {
      SetFPSNum(obs_fps_num);
    }
  }
}

bool FLOBSManager::ResetAudio() {
  ProfileScope("FLOBSManager::ResetAudio");
  struct obs_audio_info ai;
  ai.samples_per_sec = config_get_uint(m_basicConfig, "Audio", "SampleRate");

  const char* channelSetupStr =
      config_get_string(m_basicConfig, "Audio", "ChannelSetup");

  if (strcmp(channelSetupStr, "Mono") == 0)
    ai.speakers = SPEAKERS_MONO;
  else if (strcmp(channelSetupStr, "2.1") == 0)
    ai.speakers = SPEAKERS_2POINT1;
  else if (strcmp(channelSetupStr, "4.0") == 0)
    ai.speakers = SPEAKERS_4POINT0;
  else if (strcmp(channelSetupStr, "4.1") == 0)
    ai.speakers = SPEAKERS_4POINT1;
  else if (strcmp(channelSetupStr, "5.1") == 0)
    ai.speakers = SPEAKERS_5POINT1;
  else if (strcmp(channelSetupStr, "7.1") == 0)
    ai.speakers = SPEAKERS_7POINT1;
  else
    ai.speakers = SPEAKERS_STEREO;

  return obs_reset_audio(&ai);
}

void FLOBSManager::ResetVolume() {
  if (obs_volmeter_) {
    obs_volmeter_remove_callback(obs_volmeter_, OBSVolumeLevel, this);
    obs_volmeter_destroy(obs_volmeter_);
    obs_volmeter_ = nullptr;
  }
  obs_source_t* source = obs_get_output_source(kChannel_3_Mic);
  if (source) {
    obs_volmeter_ = obs_volmeter_create(OBS_FADER_LOG);
    obs_volmeter_add_callback(obs_volmeter_, &OBSVolumeLevel, this);
    obs_volmeter_attach_source(obs_volmeter_, source);
  }
}

int FLOBSManager::GetNowPeakVolume() {
  return now_peak_volume_;
}

void FLOBSManager::SetNowPeakVolume(int peak) {
  now_peak_volume_ = peak;
}

void FLOBSManager::OBSVolumeLevel(void* data,
                                  const float magnitude[MAX_AUDIO_CHANNELS],
                                  const float peak[MAX_AUDIO_CHANNELS],
                                  const float inputPeak[MAX_AUDIO_CHANNELS]) {
  // fl using 0/1
  auto input_peak = std::max<int>((int)inputPeak[0], (int)inputPeak[1]);
  FLOBSManager::GetInstance().SetNowPeakVolume(input_peak);
}

bool FLOBSManager::AddAllModulePath() {
  std::string bin_dir = m_currentDir + "/obs-plugins/";
  std::string data_plugins_dir = m_currentDir + "/data/obs-plugins/%module%";
  std::string data_dir = m_currentDir + "/data/libobs/";
  obs_add_module_path(bin_dir.c_str(), data_plugins_dir.c_str());
  obs_add_data_path(data_dir.c_str());
  return true;
}

bool FLOBSManager::InitializeBasicConfig() {
  ProfileScope("FLOBSManager::InitializeBasicConfig");

  std::string basic_path =
      sg_finderlive_obs_profiles_config_path + "\\basic.ini";
  int code = m_basicConfig.Open(basic_path.c_str(), CONFIG_OPEN_ALWAYS);
  if (CONFIG_SUCCESS != code) {
    return false;
  }
  return InitializeBasicConfigDefaults();
}

bool FLOBSManager::InitializeBasicConfigDefaults() {
  // todo
  config_set_default_uint(m_basicConfig, "Video", "BaseCX", BASE_WINDOW_WIDTH);
  config_set_default_uint(m_basicConfig, "Video", "BaseCY", BASE_WINDOW_HEIGHT);
  config_set_default_uint(m_basicConfig, "Video", "OutputCX",
                          BASE_WINDOW_WIDTH);
  config_set_default_uint(m_basicConfig, "Video", "OutputCY",
                          BASE_WINDOW_HEIGHT);

  config_set_default_uint(m_basicConfig, "Video", "FPSType", 0);
  config_set_default_string(m_basicConfig, "Video", "FPSCommon", "30");
  config_set_default_uint(m_basicConfig, "Video", "FPSInt", 30);
  config_set_default_uint(m_basicConfig, "Video", "FPSNum", 30);
  config_set_default_uint(m_basicConfig, "Video", "FPSDen", 1);
  config_set_default_string(m_basicConfig, "Video", "ScaleType", "bicubic");
  config_set_default_string(m_basicConfig, "Video", "ColorFormat", "I420");
  config_set_default_string(m_basicConfig, "Video", "ColorSpace", "601");
  config_set_default_string(m_basicConfig, "Video", "ColorRange", "Partial");

  config_set_default_string(m_basicConfig, "Audio", "MonitoringDeviceId",
                            "default");
  config_set_default_string(m_basicConfig, "Audio", "MonitoringDeviceName",
                            ("Basic.Settings.Advanced.Audio.MonitoringDevice"
                             ".Default"));
  config_set_default_uint(m_basicConfig, "Audio", "SampleRate", 48000);
  config_set_default_string(m_basicConfig, "Audio", "ChannelSetup", "Stereo");
  config_set_default_double(m_basicConfig, "Audio", "MeterDecayRate", 23.53);
  config_set_default_uint(m_basicConfig, "Audio", "PeakMeterType", 0);

  return true;
}

bool FLOBSManager::InitializeGlobalConfig() {
  defaultGlobalConfigPath_ = sg_finderlive_obs_config_path + "\\global.ini";
  int errorCode =
      m_globalConfig.Open(defaultGlobalConfigPath_.c_str(), CONFIG_OPEN_ALWAYS);
  if (CONFIG_SUCCESS != errorCode) {
    return false;
  }

  return InitializeGlobalConfigDefaults();
}

bool FLOBSManager::InitializeGlobalConfigDefaults() {
  config_set_default_string(m_globalConfig, "General", "Language", "en-US");
  config_set_default_uint(m_globalConfig, "General", "MaxLogs", 10);
  config_set_default_int(m_globalConfig, "General", "InfoIncrement", -1);
  config_set_default_string(m_globalConfig, "General", "ProcessPriority",
                            "Normal");
  config_set_default_bool(m_globalConfig, "General", "EnableAutoUpdates", true);

#if _WIN32
  config_set_default_string(m_globalConfig, "Video", "Renderer", "Direct3D 11");
#else
  config_set_default_string(m_globalConfig, "Video", "Renderer", "OpenGL");
#endif

  config_set_default_bool(m_globalConfig, "BasicWindow", "PreviewEnabled",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "PreviewProgramMode",
                          false);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SceneDuplicationMode",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SwapScenesMode",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SnappingEnabled",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "ScreenSnapping",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SourceSnapping",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "CenterSnapping",
                          false);
  config_set_default_double(m_globalConfig, "BasicWindow", "SnapDistance",
                            10.0);
  config_set_default_bool(m_globalConfig, "BasicWindow", "RecordWhenStreaming",
                          false);
  config_set_default_bool(m_globalConfig, "BasicWindow",
                          "KeepRecordingWhenStreamStops", false);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SysTrayEnabled",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SysTrayWhenStarted",
                          false);
  config_set_default_bool(m_globalConfig, "BasicWindow", "SaveProjectors",
                          false);
  config_set_default_bool(m_globalConfig, "BasicWindow", "ShowTransitions",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "ShowListboxToolbars",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "ShowStatusBar", true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "ShowSourceIcons",
                          true);
  config_set_default_bool(m_globalConfig, "BasicWindow", "StudioModeLabels",
                          true);

  // if (!config_get_bool(m_globalConfig, "General", "Pre21Defaults")) {
  //  config_set_default_string(m_globalConfig, "General", "CurrentTheme",
  //  DEFAULT_THEME);
  //}

  config_set_default_string(m_globalConfig, "General", "HotkeyFocusType",
                            "NeverDisableHotkeys");

  config_set_default_bool(m_globalConfig, "BasicWindow", "VerticalVolControl",
                          false);

  config_set_default_bool(m_globalConfig, "BasicWindow", "MultiviewMouseSwitch",
                          true);

  config_set_default_bool(m_globalConfig, "BasicWindow", "MultiviewDrawNames",
                          true);

  config_set_default_bool(m_globalConfig, "BasicWindow", "MultiviewDrawAreas",
                          true);

#ifdef _WIN32
  // uint32_t winver = GetWindowsVersion();

  config_set_default_bool(m_globalConfig, "Audio", "DisableAudioDucking", true);
  // config_set_default_bool(m_globalConfig, "General", "BrowserHWAccel",
  // winver > 0x601);
#endif

#ifdef __APPLE__
  config_set_default_bool(globalConfig, "Video", "DisableOSXVSync", true);
  config_set_default_bool(globalConfig, "Video", "ResetOSXVSyncOnExit", true);
#endif
  return true;
}

// bool FLOBSManager::ResetOutput() {
//   if (m_liveOutput) {
//     obs_output_stop(m_liveOutput);
//     obs_output_release(m_liveOutput);
//   }
//
//   obs_output_t* live_output =
//       obs_output_create("live_output", "simple_file_output", nullptr,
//       nullptr);
//   m_liveOutput = live_output;
//   obs_output_release(live_output);
//
//   return true;
// }

OBSDisplay FLOBSManager::CreateDisplay(HWND hwnd) {
  g_ms_window_win = std::make_unique<MsWindowWin>(hwnd);
  gs_init_data info = {};
  RECT rect;
  GetClientRect(hwnd, &rect);
  info.cx = rect.right;
  info.cy = rect.bottom;
  info.format = GS_RGBA;
  info.zsformat = GS_ZS_NONE;
  info.window.hwnd = g_ms_window_win->GetWindowHandle();
  obs_display_t* display = obs_display_create(&info, GREY_COLOR_BACKGROUND);
  if (display) {
    obs_display_add_draw_callback(display, FLOBSManager::RenderMain, this);
    struct obs_video_info ovi;
    if (obs_get_video_info(&ovi)) {
      // todo ResizePreview
    }
  }
  return display;
}

obs_display_t* FLOBSManager::CreateTempDisplay(HWND hwnd, OBSSource source) {
  obs_display_t* display = nullptr;

  if (hwnd) {
    gs_init_data info = {};
    RECT rect;
    GetClientRect(hwnd, &rect);
    info.cx = rect.right;
    info.cy = rect.bottom;
    info.format = GS_RGBA;
    info.zsformat = GS_ZS_NONE;
    info.window.hwnd = hwnd;
    display = obs_display_create(&info, GREY_COLOR_BACKGROUND);
    if (display) {
      obs_display_add_draw_callback(display, TempDisplayFunc, source);
      struct obs_video_info ovi;
      if (obs_get_video_info(&ovi)) {
        // todo ResizePreview
      }
    }
  }

  return display;
}

void FLOBSManager::ResizePreview(uint32_t cx, uint32_t cy) {
  if (m_previewCX == cx && m_previewCY == cy) {
    return;
  }

  m_previewCX = cx;
  m_previewCY = cy;
  if (m_display) {
    obs_display_resize(m_display, cx, cy);
  }
}

bool GetCanvasInfoFromeSize(SIZE sz, CanvasInfo& canvasInfo) {
  bool ret = false;
  SIZE scalc11 = {1, 1};
  SIZE scalc169 = {16, 9};
  SIZE scalc1610 = {16, 10};
  SIZE scalc43 = {4, 3};
  SIZE scalc209 = {20, 9};
  if (sz.cx != 0 && sz.cy != 0) {
    canvasInfo.sz = sz;
    canvasInfo.is_portrait = false;
    if (canvasInfo.sz.cy > canvasInfo.sz.cx) {
      canvasInfo.is_portrait = true;

      // 竖屏改变分辨率比例
      auto revertFun = [](SIZE& inout) {
        int iTmp = inout.cx;
        inout.cx = inout.cy;
        inout.cy = iTmp;
      };
      revertFun(scalc169);
      revertFun(scalc1610);
      revertFun(scalc43);
      revertFun(scalc209);
    }

    canvasInfo.res_type = CanvasInfo::CanvasResType_unknow;
    canvasInfo.real_type_num = {sz.cx, sz.cy};
    if (sz.cx * scalc169.cy == sz.cy * scalc169.cx) {
      canvasInfo.res_type = CanvasInfo::CanvasResType_16_9;
      canvasInfo.real_type_num = scalc169;
    } else if (sz.cx * scalc1610.cy == sz.cy * scalc1610.cx) {
      canvasInfo.res_type = CanvasInfo::CanvasResType_16_10;
      canvasInfo.real_type_num = scalc1610;
    } else if (sz.cx * scalc43.cy == sz.cy * scalc43.cx) {
      canvasInfo.res_type = CanvasInfo::CanvasResType_4_3;
      canvasInfo.real_type_num = scalc43;
    } else if (sz.cx * scalc209.cy == sz.cy * scalc209.cx) {
      canvasInfo.res_type = CanvasInfo::CanvasResType_20_9;
      canvasInfo.real_type_num = scalc209;
    } else if (sz.cx == sz.cy) {
      canvasInfo.res_type = CanvasInfo::CanvasResType_1_1;
      canvasInfo.real_type_num = scalc11;
    }
    ret = true;
  }
  return ret;
}

bool FLOBSManager::GetBasicConfigVideoCanvas(CanvasInfo& canvasInfo) {
  bool ret = false;
  SettingInfo info1;
  info1.secName = "Video";
  info1.proName = "BaseCX";
  info1.ptype = OBS_PROPERTY_INT;
  SettingInfo info2;
  info2.secName = "Video";
  info2.proName = "BaseCY";
  info2.ptype = OBS_PROPERTY_INT;
  info1 = FLOBSManager::GetInstance().GetBasicConfigSetting(info1);
  info2 = FLOBSManager::GetInstance().GetBasicConfigSetting(info2);
  if (info1.ival != 0 && info2.ival != 0) {
    SIZE sz = {0};
    sz.cx = info1.ival;
    sz.cy = info2.ival;
    ret = GetCanvasInfoFromeSize(sz, canvasInfo);
  }

  return ret;
}

SettingInfo FLOBSManager::GetBasicConfigSetting(SettingInfo getInfo) {
  SettingInfo outInfo;
  outInfo = getInfo;
  switch (getInfo.ptype) {
    case OBS_PROPERTY_BOOL: {
      outInfo.bval = config_get_bool(m_basicConfig, getInfo.secName.c_str(),
                                     getInfo.proName.c_str());
    } break;
    case OBS_PROPERTY_INT: {
      outInfo.ival = config_get_uint(m_basicConfig, getInfo.secName.c_str(),
                                     getInfo.proName.c_str());
    } break;
    case OBS_PROPERTY_FLOAT: {
      outInfo.dval = config_get_double(m_basicConfig, getInfo.secName.c_str(),
                                       getInfo.proName.c_str());
    } break;
    case OBS_PROPERTY_TEXT: {
      outInfo.tval = config_get_string(m_basicConfig, getInfo.secName.c_str(),
                                       getInfo.proName.c_str());
    } break;
  }
  return outInfo;
}

void FLOBSManager::removeDisplayCallBack(IOBS* cb) {
  auto it = std::find_if(m_OBSOptions.interfaces.begin(),
                         m_OBSOptions.interfaces.end(),
                         [&](IOBS* callback) { return cb == callback; });
  if (it != m_OBSOptions.interfaces.end()) {
    obs_enter_graphics();
    m_OBSOptions.interfaces.erase(it);
    obs_leave_graphics();
  }
}

bool FLOBSManager::CreatePreviewDisplay() {
  auto hwnd = GetPreviewHWND();
  if (!hwnd) {
    return false;
  }
  m_display = CreateDisplay(hwnd);

  CanvasInfo canvasInfo;
  GetBasicConfigVideoCanvas(canvasInfo);
  if (canvasInfo.sz.cx == 0 || canvasInfo.sz.cy == 0) {
    canvasInfo.sz.cx = BASE_WINDOW_WIDTH;
    canvasInfo.sz.cy = BASE_WINDOW_HEIGHT;
  }
  base_window_size_ = canvasInfo.sz;
  SendPreviewCanvasMessage(canvasInfo.sz.cx, canvasInfo.sz.cy,
                           canvasInfo.res_type, canvasInfo.real_type_num.cx,
                           canvasInfo.real_type_num.cy, canvasInfo.is_portrait);

  ResetOBSPreviewSize();
  return true;
}

bool FLOBSManager::AddSource(const char* id,
                             const char* source_name,
                             const bool visible,
                             OBSSource& new_source,
                             obs_data_t* settings,
                             obs_data_t* hotkey) {
  if (!m_currentSceneData.scene) {
    return false;
  }

  auto type_id = obs_get_latest_input_type_id(id);
  OBSSourceAutoRelease source =
      obs_source_create(type_id, source_name, settings, hotkey);
  if (!source) {
    blog(LOG_ERROR, "create type [%s] failed.", type_id);
    return false;
  }

  new_source = source;

  uint32_t flags = obs_source_get_output_flags(source);
  if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) {
    obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_ONLY);
  }

  // 如果是 audio 源，同步音量
  if (flags & OBS_SOURCE_AUDIO) {
    int volume = 0;
    if (stricmp(id, kInputAudioSource) == 0) {
      volume = GetInputVolume();
    } else {
      volume = GetOutputVolume();
    }
    obs_source_set_volume(source, static_cast<float>(volume) / 100.0);
  }

  return true;
}

void FLOBSManager::RemoveSource(obs_source_t* source, bool& success) {
  auto it = m_currentSceneData.sceneItem.begin();
  while (it != m_currentSceneData.sceneItem.end()) {
    if (obs_sceneitem_get_source(*it) == source) {
      success = true;
      obs_sceneitem_remove(*it);
      it = m_currentSceneData.sceneItem.erase(it);
    } else
      it++;
  }
}

void FLOBSManager::SetOptions(const OBSOptions& options) {
  m_OBSOptions = options;
}

gs_vertbuffer_t* FLOBSManager::GetGxBox() {
  return box;
}

void FLOBSManager::SetSceneItemRot(OBSSceneItem item, float rotCW) {
  if (item) {
    float rot = rotCW;
    vec3 tl = GetItemTL(item);

    rot += obs_sceneitem_get_rot(item);
    if (rot >= 360.0f)
      rot -= 360.0f;
    else if (rot <= -360.0f)
      rot += 360.0f;
    obs_sceneitem_set_rot(item, rot);

    obs_sceneitem_force_update_transform(item);

    SetItemTL(item, tl);
  }
}

void FLOBSManager::SetSceneItemFullScreen(OBSSceneItem item,
                                          bool isProportional) {
  obs_source_t* source = obs_sceneitem_get_source(item);
  uint32_t base_cx = obs_source_get_width(source);
  uint32_t base_cy = obs_source_get_height(source);
  if (base_cx == 0 || base_cy == 0)
    return;
  assert(base_cx != 0);
  assert(base_cy != 0);

  if (item) {
    obs_sceneitem_defer_update_begin(item);
    obs_sceneitem_set_rot(item, 0.0f);
    obs_transform_info info;
    obs_sceneitem_get_info(item, &info);

    if (isProportional) {  // 等比的情况
      info.bounds_type = OBS_BOUNDS_NONE;
      float scaleX = canvas_size_.cx / (float)base_cx;
      float scaleY = canvas_size_.cy / (float)base_cy;
      float scale = std::min<float>(scaleX, scaleY);
      vec2_set(&info.scale, scale, scale);
      float posX = 0.0f;
      float posY = 0.0f;
      if (scaleX < scaleY)  //
      {
        posY = (canvas_size_.cy - base_cy * scale) / 2.0f;
      } else {
        posX = (canvas_size_.cx - base_cx * scale) / 2.0f;
      }
      vec2_set(&info.pos, posX, posY);
    } else {  // 非等比的情况
      info.bounds_type = OBS_BOUNDS_STRETCH;
      vec2_set(&info.bounds, canvas_size_.cx, canvas_size_.cy);
      vec2_set(&info.pos, 0.0f, 0.0f);
    }

    obs_sceneitem_set_info(item, &info);
    obs_sceneitem_crop crop = {};
    obs_sceneitem_set_crop(item, &crop);
    obs_sceneitem_defer_update_end(item);
  }
}

void FLOBSManager::SaveBasicConfigSetting() {
  config_save_safe(m_basicConfig, "tmp", nullptr);
}

void FLOBSManager::UpdateBasicConfigSetting(
    const std::vector<SettingInfo>& infoVec) {
  for (auto it : infoVec) {
    switch (it.ptype) {
      case OBS_PROPERTY_BOOL: {
        config_set_bool(m_basicConfig, it.secName.c_str(), it.proName.c_str(),
                        it.bval);
      } break;
      case OBS_PROPERTY_INT: {
        config_set_uint(m_basicConfig, it.secName.c_str(), it.proName.c_str(),
                        it.ival);
      } break;
      case OBS_PROPERTY_FLOAT: {
        config_set_double(m_basicConfig, it.secName.c_str(), it.proName.c_str(),
                          it.dval);
      } break;
      case OBS_PROPERTY_TEXT: {
        config_set_string(m_basicConfig, it.secName.c_str(), it.proName.c_str(),
                          it.tval.c_str());
      } break;
    }
  }
  SaveBasicConfigSetting();
}

void FLOBSManager::SetPreviewHWND(const HWND& _hwnd) {
  hwnd_ = _hwnd;
}

HWND FLOBSManager::GetPreviewHWND() {
  return hwnd_;
}

void FLOBSManager::SetFPSNum(uint32_t _fpsnum) {
  fpsnum_ = _fpsnum;
}

uint32_t FLOBSManager::GetFPSNum() {
  return fpsnum_;
}

void FLOBSManager::SetFPSDen(uint32_t _fpsden) {
  fpsden_ = _fpsden;
}

uint32_t FLOBSManager::GetFPSDen() {
  return fpsden_;
}

void FLOBSManager::OnPreviewChange() {
  if (g_ms_window_win) {
    g_ms_window_win->OnPreviewChange();
  }
}

void FLOBSManager::ResetOBSPreviewSize() {
  auto hwnd = GetPreviewHWND();
  if (!hwnd) {
    return;
  }

  RECT rect;
  ::GetClientRect(hwnd_, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  SetPreviewScale((float)height / base_window_size_.cy);

  ResizePreview(width, height);
}

void FLOBSManager::UpdatePreviewCanvas(int cx, int cy) {
  UpdateCanvasSize(cx, cy);
  ResetVideo();
}

void FLOBSManager::UpdateCanvasSize(int cx, int cy) {
  SettingInfo info1;
  info1.secName = "Video";
  info1.proName = "BaseCX";
  info1.ptype = OBS_PROPERTY_INT;
  info1.ival = cx;
  SettingInfo info2;
  info2.secName = "Video";
  info2.proName = "BaseCY";
  info2.ptype = OBS_PROPERTY_INT;
  info2.ival = cy;

  UpdateBasicConfigSetting({info1, info2});
  if (cx >= cy) {
    // 横变竖
    m_canvas_mode_ = kHorizontalMode;
  } else {
    // 竖变横
    m_canvas_mode_ = kVerticalMode;
  }
}

void FLOBSManager::SetFrameCallback() {
  OBSDataAutoRelease settings = obs_data_create();
  obs_data_set_int(settings, "VideoFrameCallback",
                   (long long)&OnObsOutputVideoFrameFun);
  obs_data_set_int(settings, "AudioFrameCallback",
                   (long long)&OnObsOutputAudioFrameFun);
  obs_output_update(m_liveOutput, settings);
}

void FLOBSManager::LiveOutputStart() {
  if (!m_liveOutput) {
    m_liveOutput = obs_output_create("live_output", "simple_file_output",
                                     nullptr, nullptr);

    SetFrameCallback();

    bool started = obs_output_start(m_liveOutput);

    if (!started) {
      const char* error = obs_output_get_last_error(m_liveOutput);

      LiveOutputStop();
    } else {
      FLOBSInfoReportService::GetInstance().__Init();
      FLOBSVideoDecimator::GetInstance().Reset();
    }
  }
}

void FLOBSManager::LiveOutputStop() {
  if (m_liveOutput) {
    FLOBSVideoDecimator::GetInstance().Reset();
    FLOBSInfoReportService::GetInstance().__Cleanup();
    obs_output_stop(m_liveOutput);
    obs_output_release(m_liveOutput);
    m_liveOutput = nullptr;
  }
}

void FLOBSManager::SendSceneItemData(obs_sceneitem_t* item) {
  auto [base_info, setting_info] = GetSceneItemData(item);
  SendObsInfoMessage(base_info, setting_info);
}

void FLOBSManager::SendLoadSceneItemData() {
  OBSScene scene = m_currentSceneData.scene;
  if (!scene) {
    return;
  }

  obs_scene_enum_items(
      scene,
      [](obs_scene_t* scene, obs_sceneitem_t* item, void* param) -> bool {
        FLOBSManager::GetInstance().SendSceneItemData(item);
        return true;
      },
      nullptr);
}

std::pair<OBSSourceBaseInfo, std::string> FLOBSManager::GetSceneItemData(
    obs_sceneitem_t* item) {
  auto source = obs_sceneitem_get_source(item);
  auto item_visible = obs_sceneitem_visible(item);
  auto lock_status = obs_sceneitem_locked(item);
  auto item_id = obs_sceneitem_get_id(item);
  auto source_type = obs_source_get_unversioned_id(source);
  auto source_width = obs_source_get_width(source);
  auto source_height = obs_source_get_height(source);

  auto source_name = obs_source_get_name(source);
  auto source_id = obs_source_get_id(source);

  std::string source_data_str = "";
  bool result = false;

  if (stricmp(source_type, kTextInputSourceId) == 0) {
    std::string face = "Arial";
    int flags = 0;
    std::string style = "Regular";
    int size = 256;
    OBSDataAutoRelease settings = obs_source_get_settings(source);
    if (settings) {
      OBSDataAutoRelease objdata = obs_data_get_obj(settings, "font");
      face = obs_data_get_string(objdata, "face");
      flags = obs_data_get_int(objdata, "flags");
      style = obs_data_get_string(objdata, "style");
      size = obs_data_get_int(objdata, "size");
    }

    static const char* kInputTextProName = "text";
    static const char* kInputColorProName = "color";
    static const char* kInputBkProName = "bk_color";

    ProperyInfo info1, info2, info3;
    GetInputSourcePropety(source, kInputTextProName, info1);
    GetInputSourcePropety(source, kInputColorProName, info2);
    GetInputSourcePropety(source, kInputBkProName, info3);

    flremoting::protocol::ObsTextSourceData text_source_data_msg;
    text_source_data_msg.set_source_id(source_id);
    text_source_data_msg.set_source_name(source_name);
    text_source_data_msg.set_face(face);
    text_source_data_msg.set_flags(flags);
    text_source_data_msg.set_style(style);
    text_source_data_msg.set_size(size);
    auto obs_properties_data =
        text_source_data_msg.mutable_obs_properties_data();
    SetObsPropertiesData(obs_properties_data, {info1, info2, info3});
    result = text_source_data_msg.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kVideoInputSourceId) == 0) {
    static const char* kInputProName = "local_file";
    static const char* kLoopProName = "looping";
    static const char* kIsInputProName = "is_local_file";

    ProperyInfo info1, info2, info3;
    GetInputSourcePropety(source, kInputProName, info1);
    GetInputSourcePropety(source, kLoopProName, info2);
    GetInputSourcePropety(source, kIsInputProName, info3);

    flremoting::protocol::ObsVideoSourceData video_source_data_msg;
    video_source_data_msg.set_source_id(source_id);
    video_source_data_msg.set_source_name(source_name);
    auto obs_properties_data =
        video_source_data_msg.mutable_obs_properties_data();
    SetObsPropertiesData(obs_properties_data, {info1, info2, info3});

    result = video_source_data_msg.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kImageInputSourceId) == 0) {
    static const char* kInputProName = "file";

    ProperyInfo info1;
    GetInputSourcePropety(source, kInputProName, info1);

    flremoting::protocol::ObsImageSourceData image_source_data_msg;
    image_source_data_msg.set_source_id(source_id);
    image_source_data_msg.set_source_name(source_name);
    auto obs_properties_data =
        image_source_data_msg.mutable_obs_properties_data();
    SetObsPropertiesData(obs_properties_data, {info1});
    result = image_source_data_msg.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kCameraSourceId) == 0) {
    static const char* kInputProName = "video_device_id";
    static const char* kIsBeautifyProName = "video_beautify";
    static const char* kResTypeProName = "res_type";
    static const char* kResolutionProName = "resolution";

    ProperyInfo info1, info2, info3, info4;
    GetInputSourcePropety(source, kInputProName, info1);
    GetInputSourcePropety(source, kIsBeautifyProName, info2);
    GetInputSourcePropety(source, kResTypeProName, info3);
    GetInputSourcePropety(source, kResolutionProName, info4);

    flremoting::protocol::ObsCameraSourceData camera_source_data_msg;
    camera_source_data_msg.set_source_id(source_id);
    camera_source_data_msg.set_source_name(source_name);
    auto obs_properties_data =
        camera_source_data_msg.mutable_obs_properties_data();
    SetObsPropertiesData(obs_properties_data, {info1, info2, info3, info4});

    result = camera_source_data_msg.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kMonitorCapture) == 0) {
    flremoting::protocol::ObsCaptureSourceData capture_source_data;
    capture_source_data.set_source_id(source_id);
    capture_source_data.set_source_name(source_name);
    result = capture_source_data.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kWndCapture) == 0) {
    flremoting::protocol::ObsCaptureSourceData capture_source_data;
    capture_source_data.set_source_id(source_id);
    capture_source_data.set_source_name(source_name);
    result = capture_source_data.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kGameCapture) == 0) {
    flremoting::protocol::ObsCaptureSourceData capture_source_data;
    capture_source_data.set_source_id(source_id);
    capture_source_data.set_source_name(source_name);
    result = capture_source_data.SerializeToString(&source_data_str);

  } else if (stricmp(source_type, kPhoneCastingSourceId) == 0) {
    flremoting::protocol::ObsPhoneCastingSourceData
        phonecasting_source_data_msg;
    phonecasting_source_data_msg.set_source_id(source_id);
    phonecasting_source_data_msg.set_source_name(source_name);
    result = phonecasting_source_data_msg.SerializeToString(&source_data_str);
  }

  blog(LOG_INFO,
       std::format("type: {} get item data res: {}", source_type, result)
           .c_str());

  return {{item_id, source_type, source_name, item_visible, lock_status,
           source_width, source_height},
          source_data_str};
}

obs_sceneitem_t* FLOBSManager::GetSceneItemById(int64_t _item_id) {
  struct GetSceneItemParam {
    int64_t item_id;
    obs_sceneitem_t* item;
  };

  GetSceneItemParam param;
  param.item_id = _item_id;
  param.item = nullptr;

  OBSScene scene = m_currentSceneData.scene;
  if (!scene) {
    return param.item;
  }

  obs_scene_enum_items(
      scene,
      [](obs_scene_t* scene, obs_sceneitem_t* item, void* param) -> bool {
        auto get_scene_item_param = static_cast<GetSceneItemParam*>(param);
        int64_t get_item_id = obs_sceneitem_get_id(item);
        if (get_item_id == get_scene_item_param->item_id) {
          get_scene_item_param->item = item;
          return false;
        }
        return true;
      },
      &param);

  return param.item;
}

void FLOBSManager::AddSourceData(const void* item_message) {
  const auto item_message_ptr =
      static_cast<const flremoting::protocol::ObsItemMessage*>(item_message);

  auto& source_type = item_message_ptr->source_type();

  if (source_type == kTextInputSourceId) {
    AddTextSourceData(item_message);
  } else if (source_type == kImageInputSourceId) {
    AddImageSourceData(item_message);
  } else if (source_type == kVideoInputSourceId) {
    AddVideoSourceData(item_message);
  } else if (source_type == kGameCapture || source_type == kWndCapture ||
             source_type == kMonitorCapture) {
    AddCaptureSourceData(item_message);
  } else if (source_type == kPhoneCastingSourceId) {
    AddPhoneCastingSourceData(item_message);
  } else if (source_type == kCameraSourceId) {
    AddCameraSourceData(item_message);
  }
}

void FLOBSManager::AddTextSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsTextSourceData> ptr_source_data_msg =
      std::make_unique<flremoting::protocol::ObsTextSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto& source_name = ptr_source_data_msg->source_name();
  auto& face = ptr_source_data_msg->face();
  auto flags = ptr_source_data_msg->flags();
  auto& style = ptr_source_data_msg->style();
  auto size = ptr_source_data_msg->size();

  std::string used_source_name = source_name;

  OBSSource source;
  obs_sceneitem_t* sceneitem = nullptr;
  if (item_id) {
    sceneitem =
        obs_scene_find_sceneitem_by_id(m_currentSceneData.scene, item_id);
    source = obs_sceneitem_get_source(sceneitem);
  } else {
    GetUniqueNameForSource(used_source_name);
    AddSource(source_id.c_str(), used_source_name.c_str(), item_visible,
              source);
  }

  {
    OBSDataAutoRelease settings = obs_source_get_settings(source);
    std::unique_ptr<obs_properties_t, decltype(&obs_properties_destroy)> props(
        obs_source_properties(source), obs_properties_destroy);
    OBSDataAutoRelease objdata = obs_data_get_obj(settings, "font");
    obs_data_set_string(objdata, "face", face.c_str());
    obs_data_set_int(objdata, "flags", flags);
    obs_data_set_string(objdata, "style", style.c_str());
    obs_data_set_int(objdata, "size", size);
    obs_data_set_obj(settings, "font", objdata);
    obs_property_t* property = obs_properties_get(props.get(), "font");
    obs_property_modified(property, settings);
    obs_source_update(source, settings);
  }

  std::vector<ProperyInfo> infos;
  ParseObsPropertiesData(ptr_source_data_msg->obs_properties_data(), infos);
  SetInputSourcePropety(source, infos);

  if (!item_id) {
    sceneitem = obs_scene_add(m_currentSceneData.scene, source);
    obs_sceneitem_set_visible(sceneitem, item_visible);
  }

  // save
  SaveProject();
}

void FLOBSManager::AddImageSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsImageSourceData>
      ptr_source_data_msg =
          std::make_unique<flremoting::protocol::ObsImageSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto& source_name = ptr_source_data_msg->source_name();

  OBSSource source;
  obs_sceneitem_t* scene_item = nullptr;
  if (item_id) {
    scene_item =
        obs_scene_find_sceneitem_by_id(m_currentSceneData.scene, item_id);
    source = obs_sceneitem_get_source(scene_item);
  } else {
    std::string used_source_name = source_name;
    GetUniqueNameForSource(used_source_name);

    result = AddSource(source_id.c_str(), used_source_name.c_str(),
                       item_visible, source);
    scene_item = obs_scene_add(m_currentSceneData.scene, source);
  }

  if (!result || scene_item == nullptr) {
    return;
  }

  std::vector<ProperyInfo> infos;
  ParseObsPropertiesData(ptr_source_data_msg->obs_properties_data(), infos);
  SetInputSourcePropety(source, infos);

  obs_sceneitem_set_visible(scene_item, item_visible);

  // save
  SaveProject();
}

void FLOBSManager::AddVideoSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsVideoSourceData>
      ptr_source_data_msg =
          std::make_unique<flremoting::protocol::ObsVideoSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto& source_name = ptr_source_data_msg->source_name();

  OBSSource source;
  obs_sceneitem_t* scene_item = nullptr;
  if (item_id) {
    scene_item =
        obs_scene_find_sceneitem_by_id(m_currentSceneData.scene, item_id);
    source = obs_sceneitem_get_source(scene_item);
    result = scene_item != nullptr;
  } else {
    std::string used_source_name = source_name;
    GetUniqueNameForSource(used_source_name);

    result = AddSource(source_id.c_str(), used_source_name.c_str(),
                       item_visible, source);
    // video source 默认只输出给扬声器
    obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_ONLY);

    scene_item = obs_scene_add(m_currentSceneData.scene, source);
  }

  if (!result || scene_item == nullptr) {
    return;
  }

  std::vector<ProperyInfo> infos;
  ParseObsPropertiesData(ptr_source_data_msg->obs_properties_data(), infos);
  SetInputSourcePropety(source, infos);

  obs_sceneitem_set_visible(scene_item, item_visible);

  // save
  SaveProject();
}

void FLOBSManager::AddCaptureSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsCaptureSourceData>
      ptr_source_data_msg =
          std::make_unique<flremoting::protocol::ObsCaptureSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto& source_name = ptr_source_data_msg->source_name();
  auto method = ptr_source_data_msg->method();
  auto& payload = ptr_source_data_msg->payload();
  auto priority = ptr_source_data_msg->priority();
  auto use_wildcards = ptr_source_data_msg->use_wildcards();
  auto compatibility = ptr_source_data_msg->compatibility();

  // no edit, just new
  std::string used_source_name = source_name;
  GetUniqueNameForSource(used_source_name);

  OBSSource new_source;
  result = AddSource(source_id.c_str(), used_source_name.c_str(), item_visible,
                     new_source);
  if (!result) {
    return;
  }

  OBSDataAutoRelease settings = obs_source_get_settings(new_source);

  if (source_id == kWndCapture) {
    flremoting::protocol::ObsCaptureSourceWindowInfo info;
    auto result = info.ParseFromString(ptr_source_data_msg->payload());
    if (!result) {
      return;
    }
    obs_data_set_string(settings, "window", info.window().c_str());
    obs_data_set_bool(settings, "cursor", info.cursor());
    obs_data_set_bool(settings, "client_area", info.client_area());
    obs_data_set_string(settings, "office_file", info.office_file().c_str());
  } else if (source_id == kGameCapture) {
    flremoting::protocol::ObsCaptureSourceGameInfo info;
    auto result = info.ParseFromString(ptr_source_data_msg->payload());
    if (!result) {
      return;
    }
    obs_data_set_string(settings, "capture_mode", info.capture_mode().c_str());
    obs_data_set_string(settings, "window", info.window().c_str());
    obs_data_set_bool(settings, "anti_cheat_hook", info.anti_cheat_hook());
    obs_data_set_bool(settings, "capture_overlays", info.capture_overlays());
    obs_data_set_bool(settings, "capture_cursor", info.capture_cursor());
  } else if (source_id == kMonitorCapture) {
    flremoting::protocol::ObsCaptureSourceMonitorInfo info;
    auto result = info.ParseFromString(ptr_source_data_msg->payload());
    if (!result) {
      return;
    }
    obs_data_set_int(settings, "monitor", info.monitor());
    obs_data_set_bool(settings, "use_rect", info.use_rect());
    obs_data_set_int(settings, "left", info.left());
    obs_data_set_int(settings, "right", info.right());
    obs_data_set_int(settings, "top", info.top());
    obs_data_set_int(settings, "bottom", info.bottom());
    obs_data_set_bool(settings, "capture_cursor", info.capture_cursor());
  }

  // save method
  obs_data_set_int(settings, "method", method);
  // other
  obs_data_set_int(settings, "priority", priority);
  obs_data_set_bool(settings, "use_wildcards", use_wildcards);
  obs_data_set_bool(settings, "compatibility", compatibility);

  obs_source_update(new_source, settings);

  {
    obs_sceneitem_t* sceneitem =
        obs_scene_add(m_currentSceneData.scene, new_source);
    obs_sceneitem_set_visible(sceneitem, item_visible);
  }

  // save
  SaveProject();
}

void FLOBSManager::AddPhoneCastingSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsPhoneCastingSourceData>
      ptr_source_data_msg =
          std::make_unique<flremoting::protocol::ObsPhoneCastingSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto& source_name = ptr_source_data_msg->source_name();
  auto method = ptr_source_data_msg->method();
  auto hardware_decode = ptr_source_data_msg->hardware_decode();
  auto cpu_status = ptr_source_data_msg->cpu_status();
  auto force_update = ptr_source_data_msg->force_update();
  auto audio_channel = ptr_source_data_msg->audio_channel();
  auto audio_sample_rate = ptr_source_data_msg->audio_sample_rate();
  auto casting_frame = ptr_source_data_msg->casting_frame();

  OBSSource source;
  obs_sceneitem_t* sceneitem = nullptr;
  if (item_id) {
    sceneitem =
        obs_scene_find_source(m_currentSceneData.scene, source_name.c_str());
    source = obs_sceneitem_get_source(sceneitem);
  } else {
    AddSource(source_id.c_str(), source_name.c_str(), item_visible, source);
  }

  OBSDataAutoRelease settings = obs_source_get_settings(source);

  obs_data_set_int(settings, "x_phone_casting.method", method);
  obs_data_set_bool(settings, "x_phone_casting.force_update", force_update);
  obs_data_set_bool(settings, "x_phone_casting.decode", hardware_decode);

  obs_data_set_bool(settings, "x_phone_casting.cpucap", cpu_status);
  obs_data_set_int(settings, "x_phone_casting.event_callback",
                   (long long)(&PhoneCastingEventCallback));

  if (method == static_cast<int64_t>(casting_method::android_wireless)) {
    obs_data_set_int(settings, "x_phone_casting.android_wireless.audio_channel",
                     audio_channel);
    obs_data_set_int(settings,
                     "x_phone_casting.android_wireless.audio_sample_rate",
                     audio_sample_rate);
    obs_data_set_int(settings, "x_phone_casting.casting_frame", casting_frame);
  }

  obs_source_update(source, settings);

  if (!item_id) {
    sceneitem = obs_scene_add(m_currentSceneData.scene, source);
    obs_sceneitem_set_visible(sceneitem, item_visible);
  }

  // save
  SaveProject();
}

void FLOBSManager::AddCameraSourceData(const void* _item_msg) {
  const auto ptr_obs_item_msg =
      static_cast<const flremoting::protocol::ObsItemMessage*>(_item_msg);

  std::unique_ptr<flremoting::protocol::ObsCameraSourceData>
      ptr_source_data_msg =
          std::make_unique<flremoting::protocol::ObsCameraSourceData>();

  auto item_visible = ptr_obs_item_msg->item_visible();
  auto item_id = ptr_obs_item_msg->item_id();
  auto source_width = ptr_obs_item_msg->source_width();
  auto source_height = ptr_obs_item_msg->source_height();
  auto& source_data = ptr_obs_item_msg->source_data();
  auto result = ptr_source_data_msg->ParseFromString(source_data);
  if (!result) {
    return;
  }

  auto& source_id = ptr_source_data_msg->source_id();
  auto source_name = ptr_source_data_msg->source_name();
  HWND hwnd = reinterpret_cast<HWND>(ptr_source_data_msg->source_hwnd());
  auto save_source = ptr_source_data_msg->save_source();
  auto change_id = ptr_source_data_msg->change_id();

  std::vector<ProperyInfo> infos;
  ParseObsPropertiesData(ptr_source_data_msg->obs_properties_data(), infos);

  OBSSource source;

  obs_sceneitem_t* sceneitem = nullptr;
  if (item_id) {
    sceneitem =
        obs_scene_find_source(m_currentSceneData.scene, source_name.c_str());
    source = obs_sceneitem_get_source(sceneitem);
  } else {
    GetUniqueNameForSource(source_name);
    AddSource(source_id.c_str(), source_name.c_str(), item_visible, source);
  }

  if (!item_id) {
    static const char* kInputProName = "video_device_id";
    ProperyInfo device_id_info;
    GetInputSourcePropety(source, kInputProName, device_id_info);
    if (device_id_info.vectval.size()) {
      device_id_info.ptype = OBS_PROPERTY_LIST;
      device_id_info.tval = device_id_info.vectval[0].second.c_str();
      SetInputSourcePropety(source, {device_id_info});
    }
  } else {
    if (infos.size()) {
      SetInputSourcePropety(source, infos);
    }
  }

  if (!item_id) {
    sceneitem = obs_scene_add(m_currentSceneData.scene, source);
    obs_sceneitem_set_visible(sceneitem, item_visible);
  }

  // need update resolution data
  if (change_id && item_id) {
    SendSceneItemData(sceneitem);
  }

  if (hwnd && !camera_display_) {
    camera_display_ = CreateTempDisplay(hwnd, source);
    if (camera_display_) {
      temp_hwnd_ = hwnd;
      camera_name_ = source_name;
      obs_display_set_background_color(camera_display_, 0xFF2D2D2D);
    }
  } else if (!hwnd && camera_display_) {
    TempDisplayCameraDestory(source);
  }

  OBSDataAutoRelease settings = obs_source_get_settings(source);
  obs_data_set_int(settings, "VideoBeautifyCallback",
                   (long long)&OnCameraVideoBeautifyCallback);

  if (save_source) {
    SaveProject();
  }
}

bool FLOBSManager::EditSourceItem(int64_t item_id,
                                  int action_type,
                                  const std::string& info) {
  bool result = false;
  auto item = GetSceneItemById(item_id);
  if (!item) {
    return result;
  }
  obs_source_t* source = obs_sceneitem_get_source(item);

  switch (action_type) {
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_REMOVE: {
      if (!source) {
        return result;
      }

      const char* source_name = obs_source_get_name(source);
      if (camera_name_ == source_name && camera_display_) {
        TempDisplayCameraDestory(source);
      }

      bool save_proj = false;
      RemoveSource(source, save_proj);

      if (save_proj) {
        SaveProject();
      }

      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_UP: {
      obs_sceneitem_set_order(item, OBS_ORDER_MOVE_UP);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_DOWN: {
      obs_sceneitem_set_order(item, OBS_ORDER_MOVE_DOWN);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_TOP: {
      obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_BOTTOM: {
      obs_sceneitem_set_order(item, OBS_ORDER_MOVE_BOTTOM);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_SHOW: {
      obs_sceneitem_set_visible(item, true);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_HIDE: {
      obs_sceneitem_set_visible(item, false);
      result = true;
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_RESET: {
      obs_sceneitem_defer_update_begin(item);
      obs_sceneitem_set_rot(item, 0.0f);
      obs_transform_info info;
      obs_sceneitem_get_info(item, &info);
      vec2_set(&info.scale, 1.0f, 1.0f);
      info.bounds_type = OBS_BOUNDS_NONE;
      vec2_set(&info.bounds, 0.0f, 0.0f);
      vec2_set(&info.pos, 0.0f, 0.0f);
      obs_sceneitem_set_info(item, &info);
      obs_sceneitem_crop crop = {};
      obs_sceneitem_set_crop(item, &crop);
      obs_sceneitem_defer_update_end(item);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_FULL: {
      SetSceneItemFullScreen(item, true);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_FULLNON: {
      SetSceneItemFullScreen(item, false);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_ROT: {
      SetSceneItemRot(item);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_ANTI_ROT: {
      SetSceneItemRot(item, -90.0f);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_HOR_FLIP: {
      vec2 scale;
      vec2_set(&scale, -1.0f, 1.0f);
      obs_scene_enum_items(GetCurrentScene(), MultiplySelectedItemScale,
                           &scale);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_VER_FLIP: {
      vec2 scale;
      vec2_set(&scale, 1.0f, -1.0f);
      obs_scene_enum_items(GetCurrentScene(), MultiplySelectedItemScale,
                           &scale);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_RENAME: {
      obs_source_set_name(source, info.c_str());
      SendSceneItemData(item);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_LOCK: {
      obs_sceneitem_set_locked(item, true);
      // ToDo tell ui item lock
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_UNLOCK: {
      obs_sceneitem_set_locked(item, false);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_SELECT: {
      SetAllSourceDataUnselect();
      obs_sceneitem_select(item, true);
    } break;
    case flremoting::protocol::ObsItemActionType::OBS_ITEMACTION_POSITION: {
      if (info.empty())
        break;
      auto index = std::stoi(info);
      if (index < 0)
        break;
      obs_sceneitem_set_order_position(item, index);
    } break;
    default:
      break;
  }

  return result;
}

void FLOBSManager::SetInputVolume(int value) {
  auto it = lst_iodevice_.find(AudioDeviceType::kInputDevice);
  if (it != lst_iodevice_.end()) {
    it->second->SetVolume(value);
  }
}

int FLOBSManager::GetInputVolume() {
  int volume = 0;
  auto it = lst_iodevice_.find(AudioDeviceType::kInputDevice);
  if (it != lst_iodevice_.end()) {
    volume = it->second->GetVolume();
  }
  return volume;
}

void FLOBSManager::SetOutputVolume(int value) {
  auto it = lst_iodevice_.find(AudioDeviceType::kOutputDevice);
  if (it != lst_iodevice_.end()) {
    it->second->SetVolume(value);
  }

  obs_enum_sources(
      [](void* param, obs_source_t* source) {
        auto value = *static_cast<int*>(param);
        if (obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO) {
          // here using input device
          if (stricmp(obs_source_get_id(source), kInputAudioSource) != 0) {
            obs_source_set_volume(source, static_cast<float>(value) / 100.0);
          }
        }
        return true;
      },
      &value);
}

int FLOBSManager::GetOutputVolume() {
  int volume = 0;
  auto it = lst_iodevice_.find(AudioDeviceType::kOutputDevice);
  if (it != lst_iodevice_.end()) {
    volume = it->second->GetVolume();
  }
  return volume;
}

void FLOBSManager::SendIOVolumeMessage(AudioDeviceType _type) {
  switch (_type) {
    case AudioDeviceType::kInputDevice: {
      auto value = GetInputVolume();
      auto&& msg = GenInputVolumeMessage(value);
      SendInputVolumeMessage(msg);
    } break;
    case AudioDeviceType::kOutputDevice: {
      auto value = GetOutputVolume();
      auto&& msg = GenOutputVolumeMessage(value);
      SendOutputVolumeMessage(msg);
    } break;
  }
}

std::string FLOBSManager::SetDeviceValue(const void* _msg) {
  std::string send_msg;
  if (!_msg) {
    return send_msg;
  }

  bool result = false;
  const flremoting::protocol::ObsDeviceEditMessageRequest* ptr_edit_msg =
      static_cast<const flremoting::protocol::ObsDeviceEditMessageRequest*>(
          _msg);
  AudioDeviceType send_type = AudioDeviceType::KUnknownDevice;
  switch (ptr_edit_msg->action_type()) {
    case flremoting::protocol::ObsDeviceActionType::OBS_DEVICEACTION_INPUT_ID: {
      send_type = AudioDeviceType::kInputDevice;
      auto& value = ptr_edit_msg->used_str_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetDeviceId(value);
      }
    } break;
    case flremoting::protocol::ObsDeviceActionType::
        OBS_DEVICEACTION_OUTPUT_ID: {
      send_type = AudioDeviceType::kOutputDevice;
      auto& value = ptr_edit_msg->used_str_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetDeviceId(value);
      }
    } break;
    case flremoting::protocol::ObsDeviceActionType::
        OBS_DEVICEACTION_INPUT_GAIN: {
      send_type = AudioDeviceType::kInputDevice;
      auto value = ptr_edit_msg->used_double_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetDBValue(value);
      }
    } break;
    case flremoting::protocol::ObsDeviceActionType::
        OBS_DEVICEACTION_INPUT_NOISE: {
      send_type = AudioDeviceType::kInputDevice;
      auto value = ptr_edit_msg->used_int_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetSuppressLevelValue(value);
      }
    } break;
    case flremoting::protocol::ObsDeviceActionType::
        OBS_DEVICEACTION_OUTPUT_GAIN: {
      send_type = AudioDeviceType::kOutputDevice;
      auto value = ptr_edit_msg->used_double_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetDBValue(value);
      }
    } break;
    case flremoting::protocol::ObsDeviceActionType::OBS_DEVICEACTION_INPUT_MT: {
      send_type = AudioDeviceType::kInputDevice;
      auto value = ptr_edit_msg->used_int_value();
      auto it = lst_iodevice_.find(send_type);
      if (it != lst_iodevice_.end()) {
        result = it->second->SetMonitoringType(value);
      }
    } break;
    default:
      break;
  }

  if (result) {
    SaveProject();
    GenObsDeviceMessage(send_type, false, &send_msg, true);
  }

  return send_msg;
}

void FLOBSManager::SetAllSourceDataUnselect() {
  auto unselect_fun = [](obs_scene_t* s, obs_sceneitem_t* item, void* data) {
    obs_sceneitem_select(item, false);
    return true;
  };

  obs_scene_enum_items(GetCurrentScene(), unselect_fun, nullptr);
}

int64_t FLOBSManager::GetSelectItemId() {
  auto selecte_item = GetSelectSceneItem();
  if (selecte_item) {
    return obs_sceneitem_get_id(selecte_item);
  } else {
    return -1;
  }
}

obs_sceneitem_t* FLOBSManager::GetSelectSceneItem() {
  auto select_item_fun = [](obs_scene_t*, obs_sceneitem_t* item, void* param) {
    std::vector<OBSSceneItem>* items =
        static_cast<std::vector<OBSSceneItem>*>(param);

    if (obs_sceneitem_selected(item)) {
      items->emplace_back(item);
    }
    // else if (obs_sceneitem_is_group(item)) {
    //  obs_sceneitem_group_enum_items(item, remove_items, &items);
    //}
    return true;
  };

  std::vector<OBSSceneItem> items;
  obs_scene_enum_items(GetCurrentScene(), select_item_fun, &items);
  if (items.size() != 1)
    return nullptr;

  return items[0];
}

void FLOBSManager::OnPhoneCastingDirChange(int dir) {
  auto get_phone_source_func = [](obs_scene_t*, obs_sceneitem_t* item,
                                  void* param) {
    std::vector<OBSSceneItem>* items =
        static_cast<std::vector<OBSSceneItem>*>(param);
    auto source = obs_sceneitem_get_source(item);
    if (stricmp(obs_source_get_id(source), "x_phone_casting") == 0) {
      items->push_back(item);
      return false;
    }
    return true;
  };

  std::vector<OBSSceneItem> items;
  obs_scene_enum_items(GetCurrentScene(), get_phone_source_func, &items);
  if (items.size() != 1)
    return;

  OBSSceneItem phone_item = items[0];
  obs_sceneitem_defer_update_begin(phone_item);
  obs_transform_info info;
  obs_sceneitem_get_info(phone_item, &info);
  obs_source_t* source = obs_sceneitem_get_source(phone_item);
  uint32_t base_cx = obs_source_get_width(source);
  uint32_t base_cy = obs_source_get_height(source);

  // 第一次初始化数据，竖屏左右居中，横屏上下居中
  if (!phone_source_init_ || dir == INT_MAX) {
    // 如果 dir 是变回默认图，不改变初始化状态
    if (dir == INT_MAX) {
      phone_source_init_ = false;
      dir = 0;
    } else {
      phone_source_init_ = true;
    }

    if (base_cx > 0 && base_cy > 0) {
      // 竖向
      if (dir == 0 || dir == 3)

      {
        float scale = base_window_size_.cy / (float)base_cy;
        last_scale_ = scale;
        last_phone_width_ = base_cx;
        last_phone_height_ = base_cy;
        float cx = base_cx * scale;
        float posX = (base_window_size_.cx - cx) / 2;
        // 等比的情况
        if (info.bounds_type == OBS_BOUNDS_NONE) {
          vec2_set(&info.scale, scale, scale);
        }
        // 非等比的情况
        else {
          vec2_set(&info.bounds, cx, base_window_size_.cy);
        }
        vec2_set(&info.pos, posX, 0.0f);
      }  // 横向
      else {
        float scaleX = base_window_size_.cx / (float)base_cx;
        float scaleY = base_window_size_.cy / (float)base_cy;
        float scale = std::min<float>(scaleX, scaleY);
        last_scale_ = scale;
        last_phone_width_ = base_cx;
        last_phone_height_ = base_cy;
        float posX = 0.0f, posY = 0.0f, cx = 0.0f, cy = 0.0f;
        // 需要横向居中，纵向铺满
        if (scaleX > scaleY) {
          cx = base_cx * scale;
          posX = (base_window_size_.cx - cx) / 2.0f;
          cy = base_window_size_.cy;
        } else {
          cy = base_cy * scale;
          posY = (base_window_size_.cy - cy) / 2.0f;
          cx = base_window_size_.cx;
        }
        // 等比的情况
        if (info.bounds_type == OBS_BOUNDS_NONE) {
          vec2_set(&info.scale, scale, scale);
        }  // 非等比的情况
        else {
          vec2_set(&info.bounds, cx, cy);
        }
        vec2_set(&info.pos, posX, posY);
      }
    }
  }  // 后续变化，进行横竖坐标变换
  else {
    // 从横向变成竖向
    if (dir == 0 || dir == 3) {
      float scale = base_window_size_.cy / (float)base_cy;
      float posX = info.pos.x;
      float posY = info.pos.y;
      // 等比的情况
      if (info.bounds_type == OBS_BOUNDS_NONE && last_scale_ > 0.0f) {
        float transScale = scale / last_scale_;
        last_scale_ = scale;
        // 先做坐标系的平移（移到竖向矩形的中心）
        float newPosX = posX + last_phone_width_ * info.scale.y / 2;
        float newPosY = posY + last_phone_height_ * info.scale.x / 2;
        last_phone_width_ = base_cx;
        last_phone_height_ = base_cy;
        // 坐标变换（相当于旋转90度,y变换成-x，x变换成-y）
        float newX = base_cx * info.scale.x / 2 * (-1.0f);
        float newY = base_cy * info.scale.y / 2 * (-1.0f);
        // 按比率缩放
        newX *= transScale;
        newY *= transScale;
        // 还原坐标系
        posX = newX + newPosX;
        posY = newY + newPosY;

        // posX += (BASE_WINDOW_WIDTH - transScale * base_cx) / 2;
        // posY -= (BASE_WINDOW_HEIGHT - base_cx) / 2;
        float scele = info.scale.x * transScale;
        vec2_set(&info.scale, scele, scele);
      }  // 非等比的情况
      else {
        float cx = info.bounds.x;
        float cy = info.bounds.y;
        vec2_set(&info.bounds, cx, cy);
      }
      vec2_set(&info.pos, posX, posY);
    }  // 从竖向变成横向
    else {
      float scale = std::min<float>(base_window_size_.cx / (float)base_cx,
                                    base_window_size_.cy / (float)base_cy);
      float posX = info.pos.x;
      float posY = info.pos.y;
      // 等比的情况
      if (info.bounds_type == OBS_BOUNDS_NONE) {
        float transScale = last_scale_ / scale;
        last_scale_ = scale;
        // 先做坐标系的平移（移到竖向矩形的中心）
        float newPosX = posX + last_phone_width_ * info.scale.x / 2;
        float newPosY = posY + last_phone_height_ * info.scale.y / 2;
        last_phone_width_ = base_cx;
        last_phone_height_ = base_cy;
        // 坐标变换（相当于旋转90度,y变换成-x，x变换成-y）
        float newX = base_cx * info.scale.x / 2 * (-1.0f);
        float newY = base_cy * info.scale.y / 2 * (-1.0f);
        // 按比率缩放
        newX /= transScale;
        newY /= transScale;
        // 还原坐标系
        posX = newX + newPosX;
        posY = newY + newPosY;

        // posX -= (BASE_WINDOW_WIDTH - transScale * base_cy) / 2;
        // posY += (BASE_WINDOW_HEIGHT - base_cy) / 2;
        float scele = info.scale.x / transScale;
        vec2_set(&info.scale, scele, scele);
      }
      // 非等比的情况
      else {
        float cx = info.bounds.x;
        float cy = info.bounds.y;
        vec2_set(&info.bounds, cx, cy);
      }
      vec2_set(&info.pos, posX, posY);
    }
  }

  obs_sceneitem_set_info(phone_item, &info);
  obs_sceneitem_crop crop = {};
  obs_sceneitem_set_crop(phone_item, &crop);
  obs_sceneitem_defer_update_end(phone_item);
}

void FLOBSManager::OnPhoneCastingDelete() {
  phone_source_init_ = false;
  last_scale_ = 0.0f;
  last_phone_width_ = 0;
  last_phone_height_ = 0;
}

void FLOBSManager::TempDisplayCameraDestory(OBSSource _source) {
  obs_display_remove_draw_callback(camera_display_, TempDisplayFunc, _source);
  obs_display_destroy(camera_display_);
  camera_display_ = nullptr;
  temp_hwnd_ = nullptr;
  camera_name_ = "";
}

void FLOBSManager::UpdateCurrentSourceCursorSetting(const bool _arrow) {
  auto check_fun = [](obs_scene_t* s, obs_sceneitem_t* item, void* data) {
    bool show_arrow = *(bool*)data;
    obs_source_t* source = obs_sceneitem_get_source(item);
    std::string id = obs_source_get_id(source);

    std::string cursor_name = "";
    if (id == "monitor_capture" || id == "xshm_input") {
      cursor_name = "capture_cursor";
    } else if (id == "window_capture") {
      cursor_name = "cursor";
    }

    if (cursor_name != "") {
      OBSDataAutoRelease settings = obs_source_get_settings(source);

      obs_data_set_bool(settings, cursor_name.c_str(), show_arrow);
      obs_source_update(source, settings);
    }

    return true;
  };

  obs_scene_enum_items(GetCurrentScene(), check_fun, (void*)&_arrow);

  SaveProject();
}

void FLOBSManager::UpdateInputInfo(const bool _is_activate,
                                   std::string* _output) {
  flremoting::protocol::ObsDeviceMessage device_msg;
  GenObsDeviceInfo(AudioDeviceType::kInputDevice, false, &device_msg);
  device_msg.set_device_type(
      flremoting::protocol::ObsDeviceType::OBS_DEVICE_INPUT_MIC);

  device_msg.SerializeToString(_output);
}

void FLOBSManager::UpdateOutputInfo(const bool _is_activate,
                                    std::string* _output) {
  flremoting::protocol::ObsDeviceMessage device_msg;
  GenObsDeviceInfo(AudioDeviceType::kOutputDevice, false, &device_msg);
  device_msg.set_device_type(
      flremoting::protocol::ObsDeviceType::OBS_DEVICE_OUTPUT_SPEAKER);

  device_msg.SerializeToString(_output);
}

void FLOBSManager::DropVideoFrame(bool enable, int fpsnum) {
  obs_video_info ovi;
  if (!enable || !obs_get_video_info(&ovi)) {
    FLOBSVideoDecimator::GetInstance().Reset();
    return;
  }
  auto incoming_frame_rate = ovi.fps_num;
  if (FLOBSVideoDecimator::GetInstance().ResetIfNeed(fpsnum,
                                                     incoming_frame_rate)) {
    FLOBSVideoDecimator::GetInstance().Reset();
    FLOBSVideoDecimator::GetInstance().SetIncomingFramerate(
        incoming_frame_rate);
    FLOBSVideoDecimator::GetInstance().SetTargetFramerate(fpsnum);
    FLOBSVideoDecimator::GetInstance().EnableTemporalDecimation(true);
  }
}
