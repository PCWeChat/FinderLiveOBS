

#include "flobs/flobs_util.h"
#include "flobs/flobs_ui_def.h"

#include "flui/dpi_win.h"

#include <algorithm>

static double PI = 3.141592653589793;
static float previewScale = 0.0f;

void GetUniqueNameForSource(std::string& base_name) {
  int i = 0;
  obs_source_t* source = nullptr;
  std::string name = base_name;
  while (source = obs_get_source_by_name(name.c_str())) {
    obs_source_release(source);
    name = base_name + std::to_string(++i);
  }
  base_name = name;
}

void GetItemBox(obs_sceneitem_t* item, vec3& tl, vec3& br) {
  matrix4 boxTransform;
  obs_sceneitem_get_box_transform(item, &boxTransform);

  vec3_set(&tl, M_INFINITE, M_INFINITE, 0.0f);
  vec3_set(&br, -M_INFINITE, -M_INFINITE, 0.0f);

  auto GetMinPos = [&](float x, float y) {
    vec3 pos;
    vec3_set(&pos, x, y, 0.0f);
    vec3_transform(&pos, &pos, &boxTransform);
    vec3_min(&tl, &tl, &pos);
    vec3_max(&br, &br, &pos);
  };

  GetMinPos(0.0f, 0.0f);
  GetMinPos(1.0f, 0.0f);
  GetMinPos(0.0f, 1.0f);
  GetMinPos(1.0f, 1.0f);
}

vec3 GetItemTL(obs_sceneitem_t* item) {
  vec3 tl, br;
  GetItemBox(item, tl, br);
  return tl;
}

void SetItemTL(obs_sceneitem_t* item, const vec3& tl) {
  vec3 newTL;
  vec2 pos;

  obs_sceneitem_get_pos(item, &pos);
  newTL = GetItemTL(item);
  pos.x += tl.x - newTL.x;
  pos.y += tl.y - newTL.y;
  obs_sceneitem_set_pos(item, &pos);
}

bool SceneItemHasVideo(obs_sceneitem_t* item) {
  obs_source_t* source = obs_sceneitem_get_source(item);
  uint32_t flags = obs_source_get_output_flags(source);
  return (flags & OBS_SOURCE_VIDEO) != 0;
}

static void getRotatedPos(float& x, float& y, float cw) {
  // 先水平移动，改变坐标原点
  x -= 0.5f;
  y -= 0.5f;
  // 旋转一定的角度
  float temp = x;
  x = x * cos(cw * PI / 180) - y * sin(cw * PI / 180);
  y = temp * sin(cw * PI / 180) + y * cos(cw * PI / 180);
  // 还原坐标原点
  x += 0.5f;
  y += 0.5f;

  x *= -1;
  y *= -1;
}

void DrawSquareAtPos(float x, float y, float cw) {
  struct vec3 pos;
  vec3_set(&pos, x, y, 0.0f);

  struct matrix4 matrix;
  gs_matrix_get(&matrix);
  vec3_transform(&pos, &pos, &matrix);

  gs_matrix_push();
  gs_matrix_identity();
  gs_matrix_translate(&pos);

  float radius = display::win::GetDPIScale() * HANDLE_RADIUS;
  // 解决画面源旋转问题，需要转化
  float transX = x;
  float transY = y;
  getRotatedPos(transX, transY, cw);
  gs_matrix_translate3f(radius * transX * 2, radius * transY * 2, 0.0f);
  gs_matrix_scale3f(radius * 2, radius * 2, 1.0f);
  gs_draw(GS_TRISTRIP, 0, 0);
  gs_matrix_pop();
}

void DrawSquareAtPos(float x, float y) {
  struct vec3 pos;
  vec3_set(&pos, x, y, 0.0f);

  struct matrix4 matrix;
  gs_matrix_get(&matrix);
  vec3_transform(&pos, &pos, &matrix);

  gs_matrix_push();
  gs_matrix_identity();
  gs_matrix_translate(&pos);

  gs_matrix_translate3f(-HANDLE_RADIUS, -HANDLE_RADIUS, 0.0f);
  gs_matrix_scale3f(HANDLE_RADIUS * 2, HANDLE_RADIUS * 2, 1.0f);
  gs_draw(GS_TRISTRIP, 0, 0);
  gs_matrix_pop();
}

bool CloseFloat(float a, float b, float epsilon) {
  return std::abs(a - b) <= epsilon;
}

bool crop_enabled(const obs_sceneitem_crop* crop) {
  return crop->left > 0 || crop->top > 0 || crop->right > 0 || crop->bottom > 0;
}

void DrawRect(float thickness, vec2 scale) {
  gs_render_start(true);

  gs_vertex2f(0.0f, 0.0f);
  gs_vertex2f(0.0f + (thickness / scale.x), 0.0f);
  gs_vertex2f(0.0f, 1.0f);
  gs_vertex2f(0.0f + (thickness / scale.x), 1.0f);
  gs_vertex2f(0.0f, 1.0f - (thickness / scale.y));
  gs_vertex2f(1.0f, 1.0f);
  gs_vertex2f(1.0f, 1.0f - (thickness / scale.y));
  gs_vertex2f(1.0f - (thickness / scale.x), 1.0f);
  gs_vertex2f(1.0f, 0.0f);
  gs_vertex2f(1.0f - (thickness / scale.x), 0.0f);
  gs_vertex2f(1.0f, 0.0f + (thickness / scale.y));
  gs_vertex2f(0.0f, 0.0f);
  gs_vertex2f(0.0f, 0.0f + (thickness / scale.y));

  gs_vertbuffer_t* rect = gs_render_save();

  gs_load_vertexbuffer(rect);
  gs_draw(GS_TRISTRIP, 0, 0);
  gs_vertexbuffer_destroy(rect);
}

void DrawLine(float x1,
              float y1,
              float x2,
              float y2,
              float thickness,
              vec2 scale) {
  float ySide = (y1 == y2) ? (y1 < 0.5f ? 1.0f : -1.0f) : 0.0f;
  float xSide = (x1 == x2) ? (x1 < 0.5f ? 1.0f : -1.0f) : 0.0f;

  gs_render_start(true);

  gs_vertex2f(x1, y1);
  gs_vertex2f(x1 + (xSide * (thickness / scale.x)),
              y1 + (ySide * (thickness / scale.y)));
  gs_vertex2f(x2, y2);
  gs_vertex2f(x2 + (xSide * (thickness / scale.x)),
              y2 + (ySide * (thickness / scale.y)));

  gs_vertbuffer_t* line = gs_render_save();

  gs_load_vertexbuffer(line);
  gs_draw(GS_TRISTRIP, 0, 0);
  gs_vertexbuffer_destroy(line);
}

void SetPreviewScale(float scale) {
  previewScale = scale;
}

float GetPreviewScale() {
  return previewScale;
}
