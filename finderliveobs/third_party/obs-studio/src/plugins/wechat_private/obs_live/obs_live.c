#include <obs-module.h>
#include <util/platform.h>
#include <util/threading.h>
#include <windows.h>
#include "obs.h"
#include "obs_live.h"

#define do_log(level, format, ...) \
  blog(level, "[flv output: '%s'] " format, obs_output_get_name(stream->output), ##__VA_ARGS__)

#define warn(format, ...) do_log(LOG_WARNING, format, ##__VA_ARGS__)

#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("live-outputs", "en-US")
MODULE_EXPORT const char *obs_module_description(void) { return "OBS core Live outputs"; }

static const char *live_output_getname(void *unused) {
  UNUSED_PARAMETER(unused);
  return obs_module_text("FLVOutput");
}

static bool live_output_start(void *data) {
  struct live_output *stream = data;
  obs_data_t *settings;
  const char *path;

  if (!obs_output_can_begin_data_capture(stream->output, 0)) return false;

  stream->got_first_video = false;
  stream->sent_headers = false;
  os_atomic_set_bool(&stream->stopping, false);

  /* write headers and start capture */
  os_atomic_set_bool(&stream->active, true);
  obs_output_begin_data_capture(stream->output, 0);

  settings = obs_output_get_settings(stream->output);
  stream->videoFrameFun =
      (OnObsOutputVideoFrameFun)obs_data_get_int(settings, "VideoFrameCallback");
  stream->audioFrameFun =
      (OnObsOutputAudioFrameFun)obs_data_get_int(settings, "AudioFrameCallback");
  obs_data_release(settings);

  return true;
}

static void live_output_actual_stop(struct live_output *stream, int code) {
  os_atomic_set_bool(&stream->active, false);

  if (code) {
    obs_output_signal_stop(stream->output, code);
  } else {
    obs_output_end_data_capture(stream->output);
  }

  info("FLV file output complete");
}

static void live_output_stop(void *data, uint64_t ts) {
  struct live_output *stream = data;
  stream->stop_ts = ts / 1000;
  os_atomic_set_bool(&stream->stopping, true);
  live_output_actual_stop(stream, 0);
}

static void live_output_destroy(void *data) {
  struct live_output *stream = data;

  pthread_mutex_destroy(&stream->mutex);
  if (stream->video_buff) bfree(stream->video_buff);
  if (stream->audio_buff) bfree(stream->audio_buff);
  stream->video_buff = NULL;
  stream->video_buff_len = 0;
  stream->audio_buff = NULL;
  stream->audio_buff_len = 0;
  bfree(stream);
}

static void *live_output_create(obs_data_t *settings, obs_output_t *output) {
  struct live_output *stream = bzalloc(sizeof(struct live_output));
  stream->output = output;
  stream->video_buff = NULL;
  stream->video_buff_len = 0;
  stream->audio_buff = NULL;
  stream->audio_buff_len = 0;
  pthread_mutex_init(&stream->mutex, NULL);
  UNUSED_PARAMETER(settings);
  return stream;
}

static void live_output_raw_video(void* data, struct video_data* frame) {
  struct live_output* stream = data;
  int width = (int)obs_output_get_width(stream->output);
  int height = (int)obs_output_get_height(stream->output);

  struct obs_video_info oai;
  if (obs_get_video_info(&oai)) {
    if (oai.output_format == VIDEO_FORMAT_I420 && stream->videoFrameFun) {
      if (stream->videoFrameFun) {
        stream->videoFrameFun(frame->data[0], width * height * 3 / 2, width,
                              height, frame->timestamp, 0);
      }
    }
  }
}

static void live_output_raw_audio(void *data, struct audio_data *frame) {
  struct live_output *stream = data;
  int audio_planes = 0;
  struct obs_audio_info oai;
  if (obs_get_audio_info(&oai)) {
    audio_planes = oai.speakers;
  } else {
    audio_planes = frame->data[1] ? SPEAKERS_STEREO : SPEAKERS_MONO;
  }

  int dstLen = frame->frames * sizeof(short) * audio_planes;
  if (dstLen != stream->audio_buff_len && stream->audio_buff) {
    bfree(stream->audio_buff);
    stream->audio_buff = NULL;
  }
  if (stream->audio_buff == NULL) {
    stream->audio_buff_len = dstLen;
    stream->audio_buff = (uint8_t *)bzalloc(stream->audio_buff_len);
  }
  uint16_t *pDst = stream->audio_buff;
  if (pDst == NULL) {
    warn("av_malloc err dstLen=%d", dstLen);
    return;
  }

  if (stream->audioFrameFun) {
    float *inputChannel0 = (float *)frame->data[0];
    if (audio_planes == SPEAKERS_MONO)  // mono
    {
      for (int i = 0; i < frame->frames; i++) {
        float sample = *inputChannel0++;
        if (sample < -1.0f)
          sample = -1.0f;
        else if (sample > 1.0f)
          sample = 1.0f;
        pDst[i] = (short)(sample * 32767.0f);
      }
    } else if (audio_planes == SPEAKERS_STEREO) {
      float *inputChannel1 = (float *)frame->data[1];
      for (int i = 0; i < frame->frames; i++) {
        pDst[i * 2] = (short)((*inputChannel0++) * 32767.0f);
        pDst[i * 2 + 1] = (short)((*inputChannel1++) * 32767.0f);
      }
    }
    stream->audioFrameFun(pDst, dstLen, 48000, audio_planes, frame->timestamp);
  }
}

static obs_properties_t *live_output_properties(void *unused) {
  UNUSED_PARAMETER(unused);

  obs_properties_t *props = obs_properties_create();

  obs_properties_add_text(props, "path", obs_module_text("FLVOutput.FilePath"), OBS_TEXT_DEFAULT);
  return props;
}

struct obs_output_info live_output_info = {
    .id = "live_output",
    .flags = OBS_OUTPUT_AV,
    .get_name = live_output_getname,
    .create = live_output_create,
    .destroy = live_output_destroy,
    .start = live_output_start,
    .stop = live_output_stop,
    //.encoded_packet = live_output_data,
    .get_properties = live_output_properties,
    .raw_video = live_output_raw_video,
    .raw_audio = live_output_raw_audio,
};

bool obs_module_load(void) {
  obs_register_output(&live_output_info);
  return true;
}

void obs_module_unload(void) {}
