/**
 * @copyright Tencent
 * @file ms_window_win.cc
 * @brief
 * @author evamwang
 * @version 1.0
 * @date 2022-04-29
 */
#include "flui/ms_window_win.h"

#include <stdio.h>
#include <windowsx.h>
#include <cmath>

#include <utility>

//
#include "flobs/flobs_manager.h"
#include "flobs/flobs_util.h"
#include "third_party/obs-studio/src/libobs/graphics/matrix4.h"

#include "flmojo/flmojo_send.h"

const wchar_t kWndClass[] = L"Channels_Live_MouseWin";
auto constexpr kSnapDistance = 10;

namespace {

MsWindowWin* g_temp_window = nullptr;

template <typename T>
T GetUserDataPtr(HWND hWnd) {
  return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

void SetUserDataPtr(HWND hWnd, void* ptr) {
  SetLastError(ERROR_SUCCESS);
  LONG_PTR result =
      ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
}

HWND CreateTempWindow(HWND _hwnd) {
  HINSTANCE hInstance = ::GetModuleHandle(nullptr);

  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = MsWindowWin::MsWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = kWndClass;
  wc.style = CS_DBLCLKS;
  RegisterClassEx(&wc);

  //   return CreateWindowEx(0, kWndClass, 0, WS_CHILD | WS_VISIBLE, 0x10, 0x40,
  //                         0x000005fc, 0x000003c2,
  //                         _hwnd, nullptr, hInstance, nullptr);
  RECT rect;
  GetClientRect(_hwnd, &rect);
  return CreateWindowEx(
      WS_EX_NOACTIVATE /*WS_EX_TOPMOST*/, kWndClass, 0,
      //      WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
      //      WS_VISIBLE,
      //        WS_CHILD | WS_VISIBLE,
      //       rect.left, rect.top, rect.right - rect.left, rect.bottom -
      //       rect.top, _hwnd, nullptr, hInstance, g_temp_window);
      WS_CHILD | WS_VISIBLE, rect.left, rect.top, rect.right - rect.left,
      rect.bottom - rect.top, _hwnd, nullptr, hInstance, g_temp_window);
}

}  // namespace

MsWindowWin::MsWindowWin(HWND _hwnd) : self_wnd_(nullptr) {
  g_temp_window = this;
  parent_hwnd_ = _hwnd;

  self_wnd_ = CreateTempWindow(_hwnd);
}

MsWindowWin::~MsWindowWin() {
  g_temp_window = nullptr;
  DestroyWindow(self_wnd_);
}

LRESULT CALLBACK MsWindowWin::MsWndProc(HWND hWnd,
                                        UINT message,
                                        WPARAM wParam,
                                        LPARAM lParam) {
  MsWindowWin* self = nullptr;
  if (message != WM_NCCREATE) {
    self = GetUserDataPtr<MsWindowWin*>(hWnd);
    if (!self) {
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }

  switch (message) {
    case WM_NCCREATE: {
      CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
      self = reinterpret_cast<MsWindowWin*>(cs->lpCreateParams);
      SetUserDataPtr(hWnd, self);
    } break;
    case WM_NCDESTROY:
      SetUserDataPtr(hWnd, nullptr);
      self->self_wnd_ = nullptr;
      self->OnDestroyed();
      break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_MOUSELEAVE:
    case WM_MOUSEWHEEL:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
      self->OnMouseEvent(message, wParam, lParam);
      break;
    case WM_SETCURSOR:
      return self->OnSetCursor(message, wParam, lParam);
      break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

void MsWindowWin::OnDestroyed() {
  PostMessage(nullptr, WM_QUIT, 0, 0);
}

void MsWindowWin::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_LBUTTONDOWN: {
      ::SetCapture(self_wnd_);

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      mLastMovePt = pt;
      mousePressEvent(pt, MouseButtonType::kLeftBtn);
    } break;
    case WM_MOUSEMOVE: {
      if (!is_mouse_track_) {
        TRACKMOUSEEVENT tme = {0};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = self_wnd_;
        if (::TrackMouseEvent(&tme)) {
          is_mouse_track_ = true;
        }
      }

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

      if (abs(pt.x - mLastMovePt.x) > 1 || abs(pt.y - mLastMovePt.y) > 1) {
        mLastMovePt = pt;
        mouseMoveEvent(pt);
      }
    } break;
    case WM_LBUTTONUP: {
      ::ReleaseCapture();

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      mouseReleaseEvent(pt);
    } break;
    case WM_RBUTTONDOWN: {
      ::SetCapture(self_wnd_);

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      mLastMovePt = pt;
      mousePressEvent(pt, MouseButtonType::kRightBtn);

      // 设置选中状态
      ProcessClick(startPos);
    } break;
    case WM_RBUTTONUP: {
      ::ReleaseCapture();

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      mouseReleaseEvent(pt);
      auto id = FLOBSManager::GetInstance().GetSelectItemId();
      if (id != -1) {
        SendShowMouseMenuMessage(id, pt.x, pt.y);
      }
    } break;
      // 想要收到 WM_MOUSELEAVE | WM_MOUSEHOVER，
      // 需要在合适的时机调用 TrackMouseEvent（如WM_MOUSEMOVE）
    case WM_MOUSELEAVE: {
      is_mouse_track_ = false;

      POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      leaveEvent(pt);
      break;
    }
    case WM_LBUTTONDBLCLK: {
      // * 双击左键，快速编辑画面源
      auto select_item = FLOBSManager::GetInstance().GetSelectSceneItem();
      if (!select_item) {
        break;
      }
      auto [base_info, data_string] =
          FLOBSManager::GetInstance().GetSceneItemData(select_item);
      SendItemEditMsg(base_info, data_string);
    } break;
    default:
      break;
  }
}

// static
HWND MsWindowWin::GetWindowHandle() {
  if (g_temp_window) {
    return g_temp_window->self_wnd_;
  }
  return nullptr;
}

//
static bool CheckItemSelected(obs_scene_t* scene,
                              obs_sceneitem_t* item,
                              void* param) {
  SceneFindData* data = reinterpret_cast<SceneFindData*>(param);
  matrix4 transform;
  vec3 transformedPos;
  vec3 pos3;

  if (!SceneItemHasVideo(item))
    return true;
  if (obs_sceneitem_is_group(item)) {
    data->group = item;
    obs_sceneitem_group_enum_items(item, CheckItemSelected, param);
    data->group = nullptr;

    if (data->item) {
      return false;
    }
  }

  vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

  obs_sceneitem_get_box_transform(item, &transform);

  if (data->group) {
    matrix4 parent_transform;
    obs_sceneitem_get_draw_transform(data->group, &parent_transform);
    matrix4_mul(&transform, &transform, &parent_transform);
  }

  matrix4_inv(&transform, &transform);
  vec3_transform(&transformedPos, &pos3, &transform);

  if (transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
      transformedPos.y >= 0.0f && transformedPos.y <= 1.0f) {
    if (obs_sceneitem_selected(item)) {
      data->item = item;
      return false;
    }
  }

  UNUSED_PARAMETER(scene);
  return true;
}

static bool FindItemAtPos(obs_scene_t* scene,
                          obs_sceneitem_t* item,
                          void* param) {
  SceneFindData* data = reinterpret_cast<SceneFindData*>(param);
  matrix4 transform;
  matrix4 invTransform;
  vec3 transformedPos;
  vec3 pos3;
  vec3 pos3_;

  if (!SceneItemHasVideo(item))
    return true;
  if (obs_sceneitem_locked(item))
    return true;

  vec3_set(&pos3, data->pos.x, data->pos.y, 0.0f);

  obs_sceneitem_get_box_transform(item, &transform);

  matrix4_inv(&invTransform, &transform);
  vec3_transform(&transformedPos, &pos3, &invTransform);
  vec3_transform(&pos3_, &transformedPos, &transform);

  if (CloseFloat(pos3.x, pos3_.x) && CloseFloat(pos3.y, pos3_.y) &&
      transformedPos.x >= 0.0f && transformedPos.x <= 1.0f &&
      transformedPos.y >= 0.0f && transformedPos.y <= 1.0f) {
    // 右键如果是选中态
    if (data->rightMenu) {
      if (obs_sceneitem_selected(item)) {
        data->item = item;
        return false;
      }
    }
    if (data->selectBelow && obs_sceneitem_selected(item)) {
      if (data->item)
        return false;
      else
        data->selectBelow = false;
    }

    data->item = item;
  }

  UNUSED_PARAMETER(scene);
  return true;
}

static bool select_one(obs_scene_t* scene, obs_sceneitem_t* item, void* param) {
  obs_sceneitem_t* selectedItem = reinterpret_cast<obs_sceneitem_t*>(param);
  if (obs_sceneitem_is_group(item))
    obs_sceneitem_group_enum_items(item, select_one, param);

  obs_sceneitem_select(item, (selectedItem == item));

  UNUSED_PARAMETER(scene);
  return true;
}

static bool move_items(obs_scene_t* scene, obs_sceneitem_t* item, void* param) {
  if (obs_sceneitem_locked(item))
    return true;

  bool selected = obs_sceneitem_selected(item);
  vec2* offset = reinterpret_cast<vec2*>(param);

  if (obs_sceneitem_is_group(item) && !selected) {
    matrix4 transform;
    vec3 new_offset;
    vec3_set(&new_offset, offset->x, offset->y, 0.0f);

    obs_sceneitem_get_draw_transform(item, &transform);
    vec4_set(&transform.t, 0.0f, 0.0f, 0.0f, 1.0f);
    matrix4_inv(&transform, &transform);
    vec3_transform(&new_offset, &new_offset, &transform);
    obs_sceneitem_group_enum_items(item, move_items, &new_offset);
  }

  if (selected) {
    vec2 pos;
    obs_sceneitem_get_pos(item, &pos);
    vec2_add(&pos, &pos, offset);
    obs_sceneitem_set_pos(item, &pos);
  }

  UNUSED_PARAMETER(scene);
  return true;
}

static vec3 GetTransformedPos(float x, float y, const matrix4& mat) {
  vec3 result;
  vec3_set(&result, x, y, 0.0f);
  vec3_transform(&result, &result, &mat);
  return result;
}

static bool FindHandleAtPos(obs_scene_t* scene,
                            obs_sceneitem_t* item,
                            void* param) {
  HandleFindData& data = *reinterpret_cast<HandleFindData*>(param);

  if (!obs_sceneitem_selected(item)) {
    if (obs_sceneitem_is_group(item)) {
      HandleFindData newData(data, item);
      obs_sceneitem_group_enum_items(item, FindHandleAtPos, &newData);
      data.item = newData.item;
      data.handle = newData.handle;
    }

    return true;
  }

  matrix4 transform;
  vec3 pos3;
  float closestHandle = data.radius;

  vec3_set(&pos3, data.pos.x, data.pos.y, 0.0f);

  obs_sceneitem_get_box_transform(item, &transform);

  auto TestHandle = [&](float x, float y, ItemHandle handle) {
    vec3 handlePos = GetTransformedPos(x, y, transform);
    vec3_transform(&handlePos, &handlePos, &data.parent_xform);

    float dist = vec3_dist(&handlePos, &pos3);
    if (dist < data.radius) {
      if (dist < closestHandle) {
        closestHandle = dist;
        data.handle = handle;
        data.item = item;
      }
    }
  };

  TestHandle(0.0f, 0.0f, ItemHandle::TopLeft);
  TestHandle(0.5f, 0.0f, ItemHandle::TopCenter);
  TestHandle(1.0f, 0.0f, ItemHandle::TopRight);
  TestHandle(0.0f, 0.5f, ItemHandle::CenterLeft);
  TestHandle(1.0f, 0.5f, ItemHandle::CenterRight);
  TestHandle(0.0f, 1.0f, ItemHandle::BottomLeft);
  TestHandle(0.5f, 1.0f, ItemHandle::BottomCenter);
  TestHandle(1.0f, 1.0f, ItemHandle::BottomRight);

  UNUSED_PARAMETER(scene);
  return true;
}

struct OffsetData {
  float clampDist;
  vec3 tl, br, offset;
};

static bool GetSourceSnapOffset(obs_scene_t* scene,
                                obs_sceneitem_t* item,
                                void* param) {
  OffsetData* data = reinterpret_cast<OffsetData*>(param);

  if (obs_sceneitem_selected(item))
    return true;

  matrix4 boxTransform;
  obs_sceneitem_get_box_transform(item, &boxTransform);

  vec3 t[4] = {GetTransformedPos(0.0f, 0.0f, boxTransform),
               GetTransformedPos(1.0f, 0.0f, boxTransform),
               GetTransformedPos(0.0f, 1.0f, boxTransform),
               GetTransformedPos(1.0f, 1.0f, boxTransform)};

  bool first = true;
  vec3 tl, br;
  vec3_zero(&tl);
  vec3_zero(&br);
  for (const vec3& v : t) {
    if (first) {
      vec3_copy(&tl, &v);
      vec3_copy(&br, &v);
      first = false;
    } else {
      vec3_min(&tl, &tl, &v);
      vec3_max(&br, &br, &v);
    }
  }

  // Snap to other source edges
#define EDGE_SNAP(l, r, x, y)                                        \
  do {                                                               \
    double dist = fabsf(l.x - data->r.x);                            \
    if (dist < data->clampDist && fabsf(data->offset.x) < EPSILON && \
        data->tl.y < br.y && data->br.y > tl.y &&                    \
        (fabsf(data->offset.x) > dist || data->offset.x < EPSILON))  \
      data->offset.x = l.x - data->r.x;                              \
  } while (false)

  EDGE_SNAP(tl, br, x, y);
  EDGE_SNAP(tl, br, y, x);
  EDGE_SNAP(br, tl, x, y);
  EDGE_SNAP(br, tl, y, x);
#undef EDGE_SNAP

  UNUSED_PARAMETER(scene);
  return true;
}

struct SelectedItemBounds {
  bool first = true;
  vec3 tl, br;
};

static bool AddItemBounds(obs_scene_t* scene,
                          obs_sceneitem_t* item,
                          void* param) {
  SelectedItemBounds* data = reinterpret_cast<SelectedItemBounds*>(param);
  vec3 t[4];

  auto add_bounds = [data, &t]() {
    for (const vec3& v : t) {
      if (data->first) {
        vec3_copy(&data->tl, &v);
        vec3_copy(&data->br, &v);
        data->first = false;
      } else {
        vec3_min(&data->tl, &data->tl, &v);
        vec3_max(&data->br, &data->br, &v);
      }
    }
  };

  if (obs_sceneitem_is_group(item)) {
    SelectedItemBounds sib;
    obs_sceneitem_group_enum_items(item, AddItemBounds, &sib);

    if (!sib.first) {
      matrix4 xform;
      obs_sceneitem_get_draw_transform(item, &xform);

      vec3_set(&t[0], sib.tl.x, sib.tl.y, 0.0f);
      vec3_set(&t[1], sib.tl.x, sib.br.y, 0.0f);
      vec3_set(&t[2], sib.br.x, sib.tl.y, 0.0f);
      vec3_set(&t[3], sib.br.x, sib.br.y, 0.0f);
      vec3_transform(&t[0], &t[0], &xform);
      vec3_transform(&t[1], &t[1], &xform);
      vec3_transform(&t[2], &t[2], &xform);
      vec3_transform(&t[3], &t[3], &xform);
      add_bounds();
    }
  }
  if (!obs_sceneitem_selected(item))
    return true;

  matrix4 boxTransform;
  obs_sceneitem_get_box_transform(item, &boxTransform);

  t[0] = GetTransformedPos(0.0f, 0.0f, boxTransform);
  t[1] = GetTransformedPos(1.0f, 0.0f, boxTransform);
  t[2] = GetTransformedPos(0.0f, 1.0f, boxTransform);
  t[3] = GetTransformedPos(1.0f, 1.0f, boxTransform);
  add_bounds();

  UNUSED_PARAMETER(scene);
  return true;
}

static inline vec2 GetOBSScreenSize() {
  obs_video_info ovi;
  vec2 size;
  vec2_zero(&size);

  if (obs_get_video_info(&ovi)) {
    size.x = static_cast<float>(ovi.base_width);
    size.y = static_cast<float>(ovi.base_height);
  }

  return size;
}
//

LRESULT MsWindowWin::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lPara) {
  if (updateCursor) {
    return TRUE;
  }
  ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
  return TRUE;
}

void MsWindowWin::ClampAspect(vec3& tl,
                              vec3& br,
                              vec2& size,
                              const vec2& baseSize) {
  float baseAspect = baseSize.x / baseSize.y;
  float aspect = size.x / size.y;
  uint32_t stretchFlags = (uint32_t)stretchHandle;

  if (stretchHandle == ItemHandle::TopLeft ||
      stretchHandle == ItemHandle::TopRight ||
      stretchHandle == ItemHandle::BottomLeft ||
      stretchHandle == ItemHandle::BottomRight) {
    if (aspect < baseAspect) {
      if ((size.y >= 0.0f && size.x >= 0.0f) ||
          (size.y <= 0.0f && size.x <= 0.0f))
        size.x = size.y * baseAspect;
      else
        size.x = size.y * baseAspect * -1.0f;
    } else {
      if ((size.y >= 0.0f && size.x >= 0.0f) ||
          (size.y <= 0.0f && size.x <= 0.0f))
        size.y = size.x / baseAspect;
      else
        size.y = size.x / baseAspect * -1.0f;
    }

  } else if (stretchHandle == ItemHandle::TopCenter ||
             stretchHandle == ItemHandle::BottomCenter) {
    if ((size.y >= 0.0f && size.x >= 0.0f) ||
        (size.y <= 0.0f && size.x <= 0.0f))
      size.x = size.y * baseAspect;
    else
      size.x = size.y * baseAspect * -1.0f;

  } else if (stretchHandle == ItemHandle::CenterLeft ||
             stretchHandle == ItemHandle::CenterRight) {
    if ((size.y >= 0.0f && size.x >= 0.0f) ||
        (size.y <= 0.0f && size.x <= 0.0f))
      size.y = size.x / baseAspect;
    else
      size.y = size.x / baseAspect * -1.0f;
  }

  size.x = std::round(size.x);
  size.y = std::round(size.y);

  if (stretchFlags & ITEM_LEFT)
    tl.x = br.x - size.x;
  else if (stretchFlags & ITEM_RIGHT)
    br.x = tl.x + size.x;

  if (stretchFlags & ITEM_TOP)
    tl.y = br.y - size.y;
  else if (stretchFlags & ITEM_BOTTOM)
    br.y = tl.y + size.y;
}

static vec2 GetItemSize(obs_sceneitem_t* item) {
  obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(item);
  vec2 size;

  if (boundsType != OBS_BOUNDS_NONE) {
    obs_sceneitem_get_bounds(item, &size);
  } else {
    obs_source_t* source = obs_sceneitem_get_source(item);
    obs_sceneitem_crop crop;
    vec2 scale;

    obs_sceneitem_get_scale(item, &scale);
    obs_sceneitem_get_crop(item, &crop);
    size.x = static_cast<float>(obs_source_get_width(source) - crop.left -
                                crop.right) *
             scale.x;
    size.y = static_cast<float>(obs_source_get_height(source) - crop.top -
                                crop.bottom) *
             scale.y;
  }

  return size;
}

OBSSceneItem MsWindowWin::GetItemAtPos(const vec2& pos,
                                       bool selectBelow,
                                       bool rightMenu) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
  if (!scene)
    return OBSSceneItem();

  SceneFindData data(pos, selectBelow, rightMenu);
  obs_scene_enum_items(scene, FindItemAtPos, &data);
  return data.item;
}

vec2 MsWindowWin::GetMouseEventPos(const POINT& event) {
  float pixelRatio = 1.0f;  // DPI_SCALE_RATIO;
  float scale = pixelRatio / GetPreviewScale();
  vec2 pos;
  vec2_set(&pos, (static_cast<float>(event.x) - 0 / pixelRatio) * scale,
           (static_cast<float>(event.y) - 0 / pixelRatio) * scale);

  return pos;
}

bool MsWindowWin::SelectedAtPos(const vec2& pos) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
  if (!scene)
    return false;

  SceneFindData data(pos, false);
  obs_scene_enum_items(scene, CheckItemSelected, &data);
  return !!data.item;
}

void MsWindowWin::mousePressEvent(const POINT& event,
                                  MouseButtonType _mouse_type) {
  // OBSBasic* main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
  float pixelRatio = 1.0f;  // DPI_SCALE_RATIO;

  float x = static_cast<float>(event.x) - 0 / pixelRatio;
  float y = static_cast<float>(event.y) - 0 / pixelRatio;

  mouse_btn_type_ = _mouse_type;
  mouseDown = _mouse_type == MouseButtonType::kLeftBtn;

  {
    std::lock_guard<std::mutex> lock(selectMutex);
    selectedItem = nullptr;
  }

  vec2_set(&startPos, x, y);
  GetStretchHandleData(startPos);

  vec2_divf(&startPos, &startPos, GetPreviewScale() / pixelRatio);
  startPos.x = std::round(startPos.x);
  startPos.y = std::round(startPos.y);

  mouseOverItems = SelectedAtPos(startPos);
  vec2_zero(&lastMoveOffset);

  mousePos = startPos;
}

void MsWindowWin::ProcessClick(const vec2& pos) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
  OBSSceneItem item =
      GetItemAtPos(pos, true, mouse_btn_type_ == MouseButtonType::kRightBtn);
  if (mouse_btn_type_ == MouseButtonType::kRightBtn &&
      obs_sceneitem_selected(item)) {
    return;
  }

  obs_scene_enum_items(scene, select_one, static_cast<obs_sceneitem_t*>(item));
}

vec3 MsWindowWin::CalculateStretchPos(const vec3& tl, const vec3& br) {
  uint32_t alignment = obs_sceneitem_get_alignment(stretchItem);
  vec3 pos;

  vec3_zero(&pos);

  if (alignment & OBS_ALIGN_LEFT)
    pos.x = tl.x;
  else if (alignment & OBS_ALIGN_RIGHT)
    pos.x = br.x;
  else
    pos.x = (br.x - tl.x) * 0.5f + tl.x;

  if (alignment & OBS_ALIGN_TOP)
    pos.y = tl.y;
  else if (alignment & OBS_ALIGN_BOTTOM)
    pos.y = br.y;
  else
    pos.y = (br.y - tl.y) * 0.5f + tl.y;

  return pos;
}

vec3 MsWindowWin::GetSnapOffset(const vec3& tl, const vec3& br) {
  vec2 screenSize = GetOBSScreenSize();
  vec3 clampOffset;

  vec3_zero(&clampOffset);

  const bool snap = true;

  // 边缘吸附
  const bool screenSnap = true;
  // 中心吸附
  const bool centerSnap = false;

  const float clampDist = kSnapDistance / GetPreviewScale();
  const float centerX = br.x - (br.x - tl.x) / 2.0f;
  const float centerY = br.y - (br.y - tl.y) / 2.0f;

  // Left screen edge.
  if (screenSnap && fabsf(tl.x) < clampDist)
    clampOffset.x = -tl.x;
  // Right screen edge.
  if (screenSnap && fabsf(clampOffset.x) < EPSILON &&
      fabsf(screenSize.x - br.x) < clampDist)
    clampOffset.x = screenSize.x - br.x;
  // Horizontal center.
  if (centerSnap && fabsf(screenSize.x - (br.x - tl.x)) > clampDist &&
      fabsf(screenSize.x / 2.0f - centerX) < clampDist)
    clampOffset.x = screenSize.x / 2.0f - centerX;

  // Top screen edge.
  if (screenSnap && fabsf(tl.y) < clampDist)
    clampOffset.y = -tl.y;
  // Bottom screen edge.
  if (screenSnap && fabsf(clampOffset.y) < EPSILON &&
      fabsf(screenSize.y - br.y) < clampDist)
    clampOffset.y = screenSize.y - br.y;
  // Vertical center.
  if (centerSnap && fabsf(screenSize.y - (br.y - tl.y)) > clampDist &&
      fabsf(screenSize.y / 2.0f - centerY) < clampDist)
    clampOffset.y = screenSize.y / 2.0f - centerY;

  return clampOffset;
}

void MsWindowWin::SnapItemMovement(vec2& offset) {
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();

  SelectedItemBounds data;
  obs_scene_enum_items(scene, AddItemBounds, &data);

  data.tl.x += offset.x;
  data.tl.y += offset.y;
  data.br.x += offset.x;
  data.br.y += offset.y;

  vec3 snapOffset = GetSnapOffset(data.tl, data.br);

  const bool snap = true;
  const bool sourcesSnap = false;  // 画面源互相吸附
  if (snap == false)
    return;
  if (sourcesSnap == false) {
    offset.x += snapOffset.x;
    offset.y += snapOffset.y;
    return;
  }

  const float clampDist = kSnapDistance / GetPreviewScale();

  OffsetData offsetData;
  offsetData.clampDist = clampDist;
  offsetData.tl = data.tl;
  offsetData.br = data.br;
  vec3_copy(&offsetData.offset, &snapOffset);

  obs_scene_enum_items(scene, GetSourceSnapOffset, &offsetData);

  if (fabsf(offsetData.offset.x) > EPSILON ||
      fabsf(offsetData.offset.y) > EPSILON) {
    offset.x += offsetData.offset.x;
    offset.y += offsetData.offset.y;
  } else {
    offset.x += snapOffset.x;
    offset.y += snapOffset.y;
  }
}

void MsWindowWin::StretchItem(const vec2& pos) {
  obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(stretchItem);
  uint32_t stretchFlags = (uint32_t)stretchHandle;
  vec3 tl, br, pos3;

  vec3_zero(&tl);
  vec3_set(&br, stretchItemSize.x, stretchItemSize.y, 0.0f);

  vec3_set(&pos3, pos.x, pos.y, 0.0f);
  vec3_transform(&pos3, &pos3, &screenToItem);

  if (stretchFlags & ITEM_LEFT)
    tl.x = pos3.x;
  else if (stretchFlags & ITEM_RIGHT)
    br.x = pos3.x;

  if (stretchFlags & ITEM_TOP)
    tl.y = pos3.y;
  else if (stretchFlags & ITEM_BOTTOM)
    br.y = pos3.y;

  SnapStretchingToScreen(tl, br);

  obs_source_t* source = obs_sceneitem_get_source(stretchItem);

  vec2 baseSize;
  vec2_set(&baseSize, static_cast<float>(obs_source_get_width(source)),
           static_cast<float>(obs_source_get_height(source)));

  vec2 size;
  vec2_set(&size, br.x - tl.x, br.y - tl.y);

  if (boundsType != OBS_BOUNDS_NONE) {
    if (tl.x > br.x)
      std::swap(tl.x, br.x);
    if (tl.y > br.y)
      std::swap(tl.y, br.y);

    vec2_abs(&size, &size);

    obs_sceneitem_set_bounds(stretchItem, &size);
  } else {
    obs_sceneitem_crop crop;
    obs_sceneitem_get_crop(stretchItem, &crop);

    baseSize.x -= static_cast<float>(crop.left + crop.right);
    baseSize.y -= static_cast<float>(crop.top + crop.bottom);

    ClampAspect(tl, br, size, baseSize);
    vec2_div(&size, &size, &baseSize);
    obs_sceneitem_set_scale(stretchItem, &size);
  }

  pos3 = CalculateStretchPos(tl, br);
  vec3_transform(&pos3, &pos3, &itemToScreen);

  vec2 newPos;
  vec2_set(&newPos, std::round(pos3.x), std::round(pos3.y));
  obs_sceneitem_set_pos(stretchItem, &newPos);
}

void MsWindowWin::MoveItems(const vec2& pos) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();

  vec2 offset, moveOffset;
  vec2_sub(&offset, &pos, &startPos);
  vec2_sub(&moveOffset, &offset, &lastMoveOffset);

  SnapItemMovement(moveOffset);

  vec2_add(&lastMoveOffset, &lastMoveOffset, &moveOffset);

  obs_scene_enum_items(scene, move_items, &moveOffset);
}

void MsWindowWin::SnapStretchingToScreen(vec3& tl, vec3& br) {
  uint32_t stretchFlags = (uint32_t)stretchHandle;
  vec3 newTL = GetTransformedPos(tl.x, tl.y, itemToScreen);
  vec3 newTR = GetTransformedPos(br.x, tl.y, itemToScreen);
  vec3 newBL = GetTransformedPos(tl.x, br.y, itemToScreen);
  vec3 newBR = GetTransformedPos(br.x, br.y, itemToScreen);
  vec3 boundingTL;
  vec3 boundingBR;

  vec3_copy(&boundingTL, &newTL);
  vec3_min(&boundingTL, &boundingTL, &newTR);
  vec3_min(&boundingTL, &boundingTL, &newBL);
  vec3_min(&boundingTL, &boundingTL, &newBR);

  vec3_copy(&boundingBR, &newTL);
  vec3_max(&boundingBR, &boundingBR, &newTR);
  vec3_max(&boundingBR, &boundingBR, &newBL);
  vec3_max(&boundingBR, &boundingBR, &newBR);

  vec3 offset = GetSnapOffset(boundingTL, boundingBR);
  vec3_add(&offset, &offset, &newTL);
  vec3_transform(&offset, &offset, &screenToItem);
  vec3_sub(&offset, &offset, &tl);

  if (stretchFlags & ITEM_LEFT)
    tl.x += offset.x;
  else if (stretchFlags & ITEM_RIGHT)
    br.x += offset.x;

  if (stretchFlags & ITEM_TOP)
    tl.y += offset.y;
  else if (stretchFlags & ITEM_BOTTOM)
    br.y += offset.y;
}

void MsWindowWin::UpdateCursor(uint32_t& flags) {
  // if (!flags && cursor().shape() != Qt::OpenHandCursor)
  //  unsetCursor();
  // if (cursor().shape() != Qt::ArrowCursor)
  //  return;

  if ((flags & ITEM_LEFT && flags & ITEM_TOP) ||
      (flags & ITEM_RIGHT && flags & ITEM_BOTTOM)) {
    if (CloseFloat(stretchHandle_rotate_, 90.0f) ||
        CloseFloat(stretchHandle_rotate_, 270.0f)) {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
    } else {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
    }
  } else if ((flags & ITEM_LEFT && flags & ITEM_BOTTOM) ||
             (flags & ITEM_RIGHT && flags & ITEM_TOP)) {
    if (CloseFloat(stretchHandle_rotate_, 90.0f) ||
        CloseFloat(stretchHandle_rotate_, 270.0f)) {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
    } else {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
    }
  } else if (flags & ITEM_LEFT || flags & ITEM_RIGHT) {
    if (CloseFloat(stretchHandle_rotate_, 90.0f) ||
        CloseFloat(stretchHandle_rotate_, 270.0f)) {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
    } else {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
    }
  } else if (flags & ITEM_TOP || flags & ITEM_BOTTOM) {
    if (CloseFloat(stretchHandle_rotate_, 90.0f) ||
        CloseFloat(stretchHandle_rotate_, 270.0f)) {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
    } else {
      ::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
    }
  } else {
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
  }
}

void MsWindowWin::mouseMoveEvent(const POINT& event) {
  updateCursor = false;
  bool cursorSizeAll = false;
  if (mouseDown) {
    vec2 pos = GetMouseEventPos(event);

    if (!mouseMoved && !mouseOverItems && stretchHandle == ItemHandle::None) {
      ProcessClick(startPos);
      mouseOverItems = SelectedAtPos(startPos);
    }

    pos.x = std::round(pos.x);
    pos.y = std::round(pos.y);

    if (stretchHandle != ItemHandle::None) {
      selectionBox = false;

      // 获取当前的scene
      // Qt::KeyboardModifiers modifiers =
      // QGuiApplication::keyboardModifiers(); OBSBasic* main =
      // reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
      // obs_sceneitem_t* group =
      //    obs_sceneitem_get_group(scene, stretchItem);
      // if (group) {
      //    vec3 group_pos;
      //    vec3_set(&group_pos, pos.x, pos.y, 0.0f);
      //    vec3_transform(&group_pos, &group_pos,
      //        &invGroupTransform);
      //    pos.x = group_pos.x;
      //    pos.y = group_pos.y;
      // }

      // if (cropping)
      //    CropItem(pos);
      // else
      StretchItem(pos);

    } else if (mouseOverItems) {
      selectionBox = false;
      ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
      MoveItems(pos);
    } else {
      selectionBox = true;
      if (!mouseMoved)
        DoSelect(startPos);
    }

    mouseMoved = true;
    mousePos = pos;
  } else {
    vec2 pos = GetMouseEventPos(event);
    OBSSceneItem item = GetItemAtPos(pos, true);

    std::lock_guard<std::mutex> lock(selectMutex);
    hoveredPreviewItem = item;

    if (!mouseMoved && hoveredPreviewItem) {
      mousePos = pos;
      float scale = 1.0f;  // DPI_SCALE_RATIO;
      float x = static_cast<float>(event.x) - 0 / scale;
      float y = static_cast<float>(event.y) - 0 / scale;
      vec2_set(&startPos, x, y);
      updateCursor = true;
    }
  }
  if (updateCursor) {
    GetStretchHandleData(startPos);
    uint32_t stretchFlags = static_cast<uint32_t>(stretchHandle);
    UpdateCursor(stretchFlags);
  }
}

void MsWindowWin::leaveEvent(const POINT& event) {
  std::lock_guard<std::mutex> lock(selectMutex);
  if (!selectionBox) {
    hoveredPreviewItem = nullptr;
  }
}

void MsWindowWin::mouseReleaseEvent(const POINT& event) {
  if (mouseDown) {
    vec2 pos = GetMouseEventPos(event);

    if (!mouseMoved) {
      ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
      ProcessClick(pos);
    }
    if (selectionBox) {
      std::lock_guard<std::mutex> lock(selectMutex);
      bool select = true;
      obs_sceneitem_select(hoveredPreviewItem, select);
    }

    stretchItem = nullptr;
    mouseDown = false;
    mouseMoved = false;
    selectionBox = false;

    OBSSceneItem item = GetItemAtPos(pos, true);
    std::lock_guard<std::mutex> lock(selectMutex);
    hoveredPreviewItem = item;
    selectedItem = nullptr;
  }
}

void MsWindowWin::GetStretchHandleData(const vec2& pos) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
  if (!scene)
    return;

  float scale = GetPreviewScale() / 1.0f;  // DPI_SCALE_RATIO;
  vec2 scaled_pos = pos;
  vec2_divf(&scaled_pos, &scaled_pos, scale);
  HandleFindData data(scaled_pos, scale);
  obs_scene_enum_items(scene, FindHandleAtPos, &data);

  stretchItem = std::move(data.item);
  stretchHandle = data.handle;

  if (stretchHandle != ItemHandle::None) {
    matrix4 boxTransform;
    vec3 itemUL;
    float itemRot;

    stretchItemSize = GetItemSize(stretchItem);

    obs_sceneitem_get_box_transform(stretchItem, &boxTransform);
    itemRot = obs_sceneitem_get_rot(stretchItem);
    stretchHandle_rotate_ = itemRot;
    vec3_from_vec4(&itemUL, &boxTransform.t);

    /* build the item space conversion matrices */
    matrix4_identity(&itemToScreen);
    matrix4_rotate_aa4f(&itemToScreen, &itemToScreen, 0.0f, 0.0f, 1.0f,
                        RAD(itemRot));
    matrix4_translate3f(&itemToScreen, &itemToScreen, itemUL.x, itemUL.y, 0.0f);

    matrix4_identity(&screenToItem);
    matrix4_translate3f(&screenToItem, &screenToItem, -itemUL.x, -itemUL.y,
                        0.0f);
    matrix4_rotate_aa4f(&screenToItem, &screenToItem, 0.0f, 0.0f, 1.0f,
                        RAD(-itemRot));

    obs_sceneitem_get_crop(stretchItem, &startCrop);
    obs_sceneitem_get_pos(stretchItem, &startItemPos);

    obs_source_t* source = obs_sceneitem_get_source(stretchItem);
    cropSize.x = static_cast<float>(obs_source_get_width(source) -
                                    startCrop.left - startCrop.right);
    cropSize.y = static_cast<float>(obs_source_get_height(source) -
                                    startCrop.top - startCrop.bottom);

    // stretchGroup = obs_sceneitem_get_group(scene, stretchItem);
    // if (stretchGroup) {
    //  obs_sceneitem_get_draw_transform(stretchGroup,
    //      &invGroupTransform);
    //  matrix4_inv(&invGroupTransform, &invGroupTransform);
    //  obs_sceneitem_defer_group_resize_begin(stretchGroup);
    // }
  }
}

void MsWindowWin::DoSelect(const vec2& pos) {
  // 获取当前的scene
  OBSScene scene = FLOBSManager::GetInstance().GetCurrentScene();
  OBSSceneItem item = GetItemAtPos(pos, true);

  obs_scene_enum_items(scene, select_one, static_cast<obs_sceneitem_t*>(item));
}

bool MsWindowWin::HasHoverItem(obs_sceneitem_t* item) {
  std::lock_guard<std::mutex> lock(selectMutex);
  return hoveredPreviewItem == item;
}

void MsWindowWin::OnPreviewChange() {
  RECT rect;
  GetClientRect(parent_hwnd_, &rect);
  SetWindowPos(self_wnd_, nullptr, rect.left, rect.top, rect.right - rect.left,
               rect.bottom - rect.top, SWP_NOACTIVATE);
  FLOBSManager::GetInstance().ResetOBSPreviewSize();
}
