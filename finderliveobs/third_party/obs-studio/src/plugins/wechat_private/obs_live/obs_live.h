
EXPORT void obs_register_output_s(const struct obs_output_info *info, size_t size);

#define obs_register_output(info) obs_register_output_s(info, sizeof(struct obs_output_info))

typedef void (*OnObsOutputVideoFrameFun)(char *data, uint32_t lenght, uint32_t width,
                                         uint32_t height, uint64_t timestamp, int rotation);
typedef void (*OnObsOutputAudioFrameFun)(char *data, uint32_t lenght, uint32_t sampleRate,
                                         uint32_t channel, uint64_t timestamp);
struct live_output {
  obs_output_t *output;
  FILE *file;
  volatile bool active;
  volatile bool stopping;
  uint64_t stop_ts;
  bool sent_headers;
  int64_t last_packet_ts;

  OnObsOutputVideoFrameFun videoFrameFun;
  OnObsOutputAudioFrameFun audioFrameFun;

  pthread_mutex_t mutex;

  bool got_first_video;
  int32_t start_dts_offset;
  uint8_t *video_buff;
  int video_buff_len;
  uint8_t *audio_buff;
  int audio_buff_len;
};
