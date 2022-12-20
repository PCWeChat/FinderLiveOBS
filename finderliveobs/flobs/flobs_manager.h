

#ifndef FLOBS_FLOBS_MANAGER_H_
#define FLOBS_FLOBS_MANAGER_H_

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "flobs/flobs_quicktransition.h"
#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"
#include "third_party/obs-studio/src/libobs/util/util.hpp"

#include "flobs/flobs_info.h"
#include "flobs/flobs_iodevice.h"

#include "flobs/flobs_ui_callback.h"

template <typename OBSRef>
struct SignalContainer {
  OBSRef ref;
  std::vector<std::shared_ptr<OBSSignal>> handlers;
  void Clear() {
    ref = nullptr;
    handlers.clear();
  }
};

struct SceneData {
  SceneData() {}

  ~SceneData() { Clear(); }

  void Clear() {
    scene = nullptr;
    sceneItem.clear();
    sceneContainer.Clear();
  }
  OBSScene scene = nullptr;
  std::vector<OBSSceneItem> sceneItem;
  SignalContainer<OBSScene> sceneContainer;
};

struct OBSOptions {
  std::vector<IOBS*> interfaces;

  void Clear() { interfaces.clear(); }
};

class FLOBSManager {
 public:
  static FLOBSManager& GetInstance() {
    static FLOBSManager instance;
    return instance;
  }

  FLOBSManager();

  FLOBSManager(const FLOBSManager&) = delete;
  FLOBSManager& operator=(const FLOBSManager&) = delete;

  ~FLOBSManager();

  bool Start();
  bool Shutdown();

  void InitObsEnv(const std::string& base_config_path, HWND hwnd);

  bool OBSInit();
  bool OBSUninit();

  int GetNowPeakVolume();
  void SetNowPeakVolume(int peak);
  bool CreatePreviewDisplay();

  OBSScene GetCurrentScene();
  ConfigFile& GetGlobalConfig();
  ConfigFile& GetBasicConfig();
  SettingInfo GetBasicConfigSetting(SettingInfo getInfo);

  void removeDisplayCallBack(IOBS* cb);

  bool AddSource(const char* id,
                 const char* source_name,
                 const bool visible,
                 OBSSource& new_source,
                 obs_data_t* settings = nullptr,
                 obs_data_t* hotkey = nullptr);

  OBSDisplay CreateDisplay(HWND hwnd);

  // 创建临时显示Display
  obs_display_t* CreateTempDisplay(HWND hwnd, OBSSource source);

  void RemoveSource(obs_source_t* source, bool& success);

  void SetOptions(const OBSOptions& options);

  bool GetBasicConfigVideoCanvas(CanvasInfo& canvasInfo);

  void ResizePreview(uint32_t cx, uint32_t cy);

  gs_vertbuffer_t* GetGxBox();

  void SetSceneItemRot(OBSSceneItem item,
                       float rotCW = 90.0f);  // 将视频源顺时针旋转N度

  void SetSceneItemFullScreen(OBSSceneItem item,
                              bool isProportional);  // 设置当前源等比适应全屏

  void SetPreviewHWND(const HWND& _hwnd);
  HWND GetPreviewHWND();

  void SetFPSNum(uint32_t _fpsnum);
  uint32_t GetFPSNum();

  void SetFPSDen(uint32_t _fpsden);
  uint32_t GetFPSDen();

  void SetVideoFPS();

  void OnPreviewChange();
  void ResetOBSPreviewSize();
  void UpdatePreviewCanvas(int cx, int cy);

  void LiveOutputStart();
  void LiveOutputStop();

  // add
  void AddSourceData(const void* item_message);

  // edit
  bool EditSourceItem(int64_t item_id,
                      int action_type,
                      const std::string& info);

  // volume
  void SetInputVolume(int value);
  int GetInputVolume();

  void SetOutputVolume(int value);
  int GetOutputVolume();

  void SendIOVolumeMessage(AudioDeviceType _type);

  // io gain/noise
  std::string SetDeviceValue(const void* _msg);

  void GenObsDeviceMessage(AudioDeviceType _type,
                           bool _is_update,
                           std::string* _out_msg,
                           bool _is_activate);

  void SetAllSourceDataUnselect();

  int64_t GetSelectItemId();
  obs_sceneitem_t* GetSelectSceneItem();

  std::pair<OBSSourceBaseInfo, std::string> GetSceneItemData(
      obs_sceneitem_t* item);
  obs_sceneitem_t* GetSceneItemById(int64_t _item_id);

  void OnPhoneCastingDirChange(int dir);

  void UpdateCurrentSourceCursorSetting(const bool _arrow);

  void UpdateInputInfo(const bool _is_activate, std::string* _output);
  void UpdateOutputInfo(const bool _is_activate, std::string* _output);

  void DropVideoFrame(bool enable, int fpsnum);

  bool SaveProject();

 private:
  void InitOBSCallbacks();
  void InitPrimitives();
  /* OBS Callbacks */
  //   static void SceneReordered(void* data, calldata_t* params);
  //   static void SceneRefreshed(void* data, calldata_t* params);
  //   static void SceneItemAdded(void* data, calldata_t* params);
  static void SourceCreated(void* data, calldata_t* params);
  static void SourceRemoved(void* data, calldata_t* params);
  static void SourceActivated(void* data, calldata_t* params);
  static void SourceDeactivated(void* data, calldata_t* params);
  static void SourceAudioActivated(void* data, calldata_t* params);
  static void SourceAudioDeactivated(void* data, calldata_t* params);
  static void SourceRenamed(void* data, calldata_t* params);
  static void RenderMain(void* data, uint32_t cx, uint32_t cy);

  void ActivateAudioSource(OBSSource _source);
  void DeactivateAudioSource(OBSSource _source);

  void AddScene(OBSSource source);

  void AddSceneItem(OBSSceneItem item);
  void RemoveSceneItem(OBSSceneItem item);
  void ReoderSceneItem(OBSScene scene);

  void RenameSources(OBSSource source, const char* name, const char* prevName);

  static void SceneItemAddCallback(void* data, calldata_t* params);
  static void SceneItemRemovedCallback(void* data, calldata_t* params);
  static void SceneItemReorderCallback(void* data, calldata_t* params);

  bool Load();
  bool CreateDefaultScene(bool firstStart);
  void ClearSceneData();
  void ClearQuickTransitions();
  void InitDefaultTransitions();
  void CreateDefaultQuickTransitions();
  void LoadQuickTransitions(obs_data_array_t* array);
  void RefreshQuickTransitions();

  void InitTransition(obs_source_t* transition);
  void LoadTransitions(obs_data_array_t* transitions);
  void SetTransition(OBSSource transition);
  void TransitionToScene(OBSSource scene,
                         bool force = false,
                         bool quickTransition = false,
                         int quickDuration = 0,
                         bool black = false,
                         bool manual = false);

  void LoadSceneListOrder(obs_data_array_t* array);

  obs_source_t* FindTransition(const char* name);
  void AddQuickTransitionHotkey(QuickTransition* qt);
  void RemoveQuickTransitionHotkey(QuickTransition* qt);

  void CreateFirstRunSources();

  void UpdateSceneSelection(OBSSource source);

  void SetCurrentScene(OBSSource scene, bool force);

  obs_data_array_t* SaveSceneListOrder();

  obs_data_t* GenerateSaveData(obs_data_array_t* sceneOrder,
                               obs_data_array_t* quickTransitionData,
                               int transitionDuration,
                               obs_data_array_t* transitions,
                               OBSScene& scene,
                               OBSSource& curProgramScene);

  obs_data_array_t* SaveTransitions();

  obs_data_array_t* SaveQuickTransitions();

  //
  int ResetVideo();
  bool ResetAudio();
  void ResetVolume();

  //
  void GenDeviceInfo(AudioDeviceType _type, OBSSource _source, bool _is_update);
  void GenObsDeviceInfo(AudioDeviceType _type,
                        bool _is_update,
                        void* _ptr_device_infos);

  //
  void UpdateCanvasSize(int cx, int cy);

  //
  static void OBSVolumeLevel(void* data,
                             const float magnitude[MAX_AUDIO_CHANNELS],
                             const float peak[MAX_AUDIO_CHANNELS],
                             const float inputPeak[MAX_AUDIO_CHANNELS]);

  //
  bool AddAllModulePath();

  //
  bool InitializeBasicConfig();
  bool InitializeBasicConfigDefaults();
  bool InitializeGlobalConfig();
  bool InitializeGlobalConfigDefaults();

  // add
  void AddTextSourceData(const void* _item_msg);
  void AddImageSourceData(const void* _item_msg);
  void AddVideoSourceData(const void* _item_msg);
  void AddCaptureSourceData(const void* _item_msg);
  void AddPhoneCastingSourceData(const void* _item_msg);
  void AddCameraSourceData(const void* _item_msg);

  //  bool ResetOutput();

  void SetFrameCallback();

  void SaveBasicConfigSetting();
  void UpdateBasicConfigSetting(const std::vector<SettingInfo>& infoVec);

  void SendSceneItemData(obs_sceneitem_t* item);
  void SendLoadSceneItemData();

  void OnPhoneCastingDelete();

  void TempDisplayCameraDestory(OBSSource _source);

  std::atomic<bool> is_initialize_ = false;

  ConfigFile m_basicConfig;
  ConfigFile m_globalConfig;

  std::string defaultConfigFilePath_;
  std::string defaultGlobalConfigPath_;

  std::vector<OBSSignal> signalHandlers;

  int quickTransitionIdCounter = 1;

  obs_source_t* fadeTransition;

  std::vector<QuickTransition> quickTransitions;

  std::vector<std::pair<std::string, OBSSource>> m_transitions;

  std::map<AudioDeviceType, std::unique_ptr<FLOBSIODevice>> lst_iodevice_;

  gs_vertbuffer_t* box = nullptr;
  gs_vertbuffer_t* boxLeft = nullptr;
  gs_vertbuffer_t* boxTop = nullptr;
  gs_vertbuffer_t* boxRight = nullptr;
  gs_vertbuffer_t* boxBottom = nullptr;
  gs_vertbuffer_t* circle = nullptr;

  gs_vertbuffer_t* actionSafeMargin = nullptr;
  gs_vertbuffer_t* graphicsSafeMargin = nullptr;
  gs_vertbuffer_t* fourByThreeSafeMargin = nullptr;
  gs_vertbuffer_t* leftLine = nullptr;
  gs_vertbuffer_t* topLine = nullptr;
  gs_vertbuffer_t* rightLine = nullptr;

  SceneData m_currentSceneData;

  obs_volmeter_t* obs_volmeter_ = nullptr;

  int now_peak_volume_ = INT_MIN;

  obs_output_t* m_liveOutput = nullptr;

  std::string m_currentDir;
  std::string m_canvas_mode_;

  SIZE canvas_size_ = {0, 0};

  OBSDisplay m_display;
  OBSOptions m_OBSOptions;
  int m_previewX = 0;
  int m_previewY = 0;
  int m_previewCX = 0;
  int m_previewCY = 0;

  //
  HWND hwnd_ = nullptr;
  SIZE base_window_size_ = {0, 0};
  uint32_t fpsnum_ = 0;
  uint32_t fpsden_ = 0;

  // phonecasting using
  bool phone_source_init_ = false;
  float last_scale_ = 0.0f;
  int last_phone_width_ = 0;
  int last_phone_height_ = 0;

  // camera
  std::string camera_name_;
  HWND temp_hwnd_ = nullptr;
  obs_display_t* camera_display_ = nullptr;

  // audio volume
  // * 因为我们音量是全局值，所有 input、output audio source 都是被两个值控制的
  int global_input_volume_ = 0;
  int global_output_volume_ = 0;
};

#endif  // FLOBS_FLOBS_MANAGER_H_
