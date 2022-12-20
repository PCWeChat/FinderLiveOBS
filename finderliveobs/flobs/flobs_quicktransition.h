

#ifndef FLOBS_FLOBS_QUICKTRANSITION_H_
#define FLOBS_FLOBS_QUICKTRANSITION_H_

#include "third_party/obs-studio/src/libobs/obs.hpp"
#include "third_party/obs-studio/src/libobs/obs-hotkey.h"

struct QuickTransition {
  // QPushButton *button = nullptr;
  OBSSource source;
  obs_hotkey_id hotkey = OBS_INVALID_HOTKEY_ID;
  std::int64_t duration = 0;
  std::int64_t id = 0;
  bool fadeToBlack = false;

  inline QuickTransition() {}
  inline QuickTransition(OBSSource source_,
                         std::int64_t duration_,
                         std::int64_t id_,
                         bool fadeToBlack_ = false)
      : source(source_),
        duration(duration_),
        id(id_),
        fadeToBlack(fadeToBlack_),
        renamedSignal(
            std::make_shared<OBSSignal>(obs_source_get_signal_handler(source),
                                        "rename",
                                        SourceRenamed,
                                        this)) {}

 private:
  static void SourceRenamed(void* param, calldata_t* data) {}
  std::shared_ptr<OBSSignal> renamedSignal;
};

#endif  // FLOBS_FLOBS_QUICKTRANSITION_H_
