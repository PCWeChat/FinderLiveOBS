/**
 * @copyright Tencent
 * @file ms_window_win.h
 * @brief
 * @author evamwang
 * @version 1.0
 * @date 2022-04-29
 */
#ifndef FLUI_MS_WINDOW_WIN_H_
#define FLUI_MS_WINDOW_WIN_H_

#include <windows.h>
#include <mutex>

//
#define ITEM_LEFT (1 << 0)
#define ITEM_RIGHT (1 << 1)
#define ITEM_TOP (1 << 2)
#define ITEM_BOTTOM (1 << 3)
#include "flobs/flobs_ui_def.h"
#include "third_party/obs-studio/src/libobs/graphics/matrix4.h"
#include "third_party/obs-studio/src/libobs/obs.h"
#include "third_party/obs-studio/src/libobs/obs.hpp"
#include "third_party/obs-studio/src/libobs/util/util.hpp"

#include "flui/dpi_win.h"

enum class ItemHandle : uint32_t {
  None = 0,
  TopLeft = ITEM_TOP | ITEM_LEFT,
  TopCenter = ITEM_TOP,
  TopRight = ITEM_TOP | ITEM_RIGHT,
  CenterLeft = ITEM_LEFT,
  CenterRight = ITEM_RIGHT,
  BottomLeft = ITEM_BOTTOM | ITEM_LEFT,
  BottomCenter = ITEM_BOTTOM,
  BottomRight = ITEM_BOTTOM | ITEM_RIGHT,
};

struct SceneFindData {
  const vec2& pos;
  OBSSceneItem item;
  bool selectBelow;
  bool rightMenu = false;

  obs_sceneitem_t* group = nullptr;

  SceneFindData(const SceneFindData&) = delete;
  SceneFindData(SceneFindData&&) = delete;
  SceneFindData& operator=(const SceneFindData&) = delete;
  SceneFindData& operator=(SceneFindData&&) = delete;

  inline SceneFindData(const vec2& pos_, bool selectBelow_)
      : pos(pos_), selectBelow(selectBelow_) {}

  inline SceneFindData(const vec2& pos_, bool selectBelow_, bool rightMenu_)
      : pos(pos_), selectBelow(selectBelow_), rightMenu(rightMenu_) {}
};

struct HandleFindData {
  const vec2& pos;
  const float radius;
  matrix4 parent_xform;

  OBSSceneItem item;
  ItemHandle handle = ItemHandle::None;

  HandleFindData(const HandleFindData&) = delete;
  HandleFindData(HandleFindData&&) = delete;
  HandleFindData& operator=(const HandleFindData&) = delete;
  HandleFindData& operator=(HandleFindData&&) = delete;

  inline HandleFindData(const vec2& pos_, float scale)
      : pos(pos_),
        radius(HANDLE_SEL_RADIUS * display::win::GetDPIScale() / scale) {
    matrix4_identity(&parent_xform);
  }

  inline HandleFindData(const HandleFindData& hfd, obs_sceneitem_t* parent)
      : pos(hfd.pos), radius(hfd.radius), item(hfd.item), handle(hfd.handle) {
    obs_sceneitem_get_draw_transform(parent, &parent_xform);
  }
};

class MsWindowWin {
 public:
  enum class MouseButtonType { kUnknownBtn, kLeftBtn, kScrollBtn, kRightBtn };

  static HWND GetWindowHandle();

  explicit MsWindowWin(HWND _hwnd);

  MsWindowWin(const MsWindowWin&) = delete;
  MsWindowWin& operator=(const MsWindowWin&) = delete;

  ~MsWindowWin();

  static LRESULT CALLBACK MsWndProc(HWND hWnd,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam);

  //
  bool HasHoverItem(obs_sceneitem_t* item);
  void OnPreviewChange();

 private:
  void OnDestroyed();

  void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam);

  //
  LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lPara);

  // 相对窗口的鼠标位置
  void mousePressEvent(const POINT& event, MouseButtonType _mouse_type);
  void mouseReleaseEvent(const POINT& event);
  void mouseMoveEvent(const POINT& event);
  void leaveEvent(const POINT& event);
  void GetStretchHandleData(const vec2& pos);
  bool SelectedAtPos(const vec2& pos);
  vec2 GetMouseEventPos(const POINT& event);
  void ProcessClick(const vec2& pos);
  void StretchItem(const vec2& pos);
  void MoveItems(const vec2& pos);
  void SnapStretchingToScreen(vec3& tl, vec3& br);
  static vec3 GetSnapOffset(const vec3& tl, const vec3& br);
  static void SnapItemMovement(vec2& offset);
  void ClampAspect(vec3& tl, vec3& br, vec2& size, const vec2& baseSize);
  vec3 CalculateStretchPos(const vec3& tl, const vec3& br);
  void UpdateCursor(uint32_t& flags);
  void DoSelect(const vec2& pos);
  // 根据当前位置拿到对用的item
  OBSSceneItem GetItemAtPos(const vec2& pos,
                            bool selectBelow,
                            bool rightMenu = false);

  HWND parent_hwnd_ = nullptr;
  HWND self_wnd_ = nullptr;

  //
  mutable std::mutex selectMutex;
  vec2 lastMoveOffset;
  vec2 startPos;
  vec2 mousePos;
  POINT mLastMovePt;
  MouseButtonType mouse_btn_type_ = MouseButtonType::kUnknownBtn;
  bool mouseDown = false;
  bool mouseMoved = false;
  bool is_mouse_track_ = false;
  bool mouseOverItems = false;
  bool selectionBox = false;
  // 如果要支持多选，改为std::vector<obs_sceneitem_t> selectedItems;
  obs_sceneitem_t* selectedItem = nullptr;
  // 如果要支持多选，改为std::vector<obs_sceneitem_t*> hoveredPreviewItem;
  obs_sceneitem_t* hoveredPreviewItem = nullptr;
  bool updateCursor = false;
  ItemHandle stretchHandle = ItemHandle::None;
  float stretchHandle_rotate_ = 0.0f;
  vec2 stretchItemSize;
  OBSSceneItem stretchItem;
  matrix4 screenToItem;
  matrix4 itemToScreen;
  obs_sceneitem_crop startCrop;
  vec2 startItemPos;
  vec2 cropSize;
};

#endif  // FLUI_MS_WINDOW_WIN_H_
