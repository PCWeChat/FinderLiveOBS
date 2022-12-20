

#ifndef FLOBS_FLOBS_UTIL_H_
#define FLOBS_FLOBS_UTIL_H_

#include <map>
#include <string>

#include "third_party/obs-studio/src/libobs/obs.h"

//
#include "third_party/obs-studio/src/libobs/graphics/matrix4.h"

void GetUniqueNameForSource(std::string& base_name);

vec3 GetItemTL(obs_sceneitem_t* item);

void SetItemTL(obs_sceneitem_t* item, const vec3& tl);

void GetItemBox(obs_sceneitem_t* item, vec3& tl, vec3& br);

void DrawSquareAtPos(float x, float y, float cw);

void DrawSquareAtPos(float x, float y);

bool CloseFloat(float a, float b, float epsilon = 0.01);

bool crop_enabled(const obs_sceneitem_crop* crop);

bool SceneItemHasVideo(obs_sceneitem_t* item);

void DrawRect(float thickness, vec2 scale);

void DrawLine(float x1,
              float y1,
              float x2,
              float y2,
              float thickness,
              vec2 scale);

void SetPreviewScale(float scale);
float GetPreviewScale();

#endif  // FLOBS_FLOBS_UTIL_H_
