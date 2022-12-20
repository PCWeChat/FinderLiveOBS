// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: flmsgtype.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_flmsgtype_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_flmsgtype_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3020000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3020000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_util.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_flmsgtype_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_flmsgtype_2eproto {
  static const uint32_t offsets[];
};
PROTOBUF_NAMESPACE_OPEN
PROTOBUF_NAMESPACE_CLOSE
namespace flremoting {
namespace protocol {

enum MsgType : int {
  STARTREMOTEINFOREQ = 10001,
  STARTREMOTEINFORESP = 10002,
  LIVEOUTPUT_START = 10003,
  LIVEOUTPUT_STOP = 10004,
  LIVEOUTPUT_FRAME_VIDEO = 10005,
  LIVEOUTPUT_FRAME_AUDIO = 10006,
  OBS_SCENEITEM_INFO = 10007,
  PREVIEWWINDOWSZINFO = 10008,
  PREVIEWCANVASMODECHANGE = 10009,
  OBSITEMINFO = 10010,
  OBSTEXTSOURCE = 10011,
  OBSIMAGESOURCE = 10012,
  OBSVIDEOSOURCE = 10013,
  SETINPUTVOLUME = 10014,
  GETINPUTVOLUMEREQ = 10015,
  GETINPUTVOLUMERESP = 10016,
  SETOUTPUTVOLUME = 10017,
  GETOUTPUTVOLUMEREQ = 10018,
  GETOUTPUTVOLUMERESP = 10019,
  OBSITEMEDITINFOREQ = 10020,
  OBSITEMEDITINFORESP = 10021,
  OBSDEVICEINFO = 10022,
  SETDEVICEINFOREQ = 10023,
  SETDEVICEINFORESP = 10024,
  SETALLITEMUNSELECT = 10025,
  MOUSEMENUEVENT = 10026,
  PHONECASTINGINFO = 10027,
  PREVIEWSZCHANGE = 10028,
  OBSCAMERAVIDEOBEAUTIFYREQ = 10029,
  OBSCAMERAVIDEOBEAUTIFYRESP = 10030,
  OBSCURSORINFO = 10031,
  OBSFPSINFO = 10032,
  INPUTVOLUMEINFO = 10033,
  OUTPUTVOLUMEINFO = 10034,
  UPDATEINPUTINFOREQ = 10035,
  UPDATEINPUTINFORESP = 10036,
  UPDATEOUTPUTINFOREQ = 10037,
  UPDATEOUTPUTINFORESP = 10038,
  MEANRATEINFO = 10039,
  OBSITEMEDIT = 10040,
  REMOTEESTABLISH = 10041
};
bool MsgType_IsValid(int value);
constexpr MsgType MsgType_MIN = STARTREMOTEINFOREQ;
constexpr MsgType MsgType_MAX = REMOTEESTABLISH;
constexpr int MsgType_ARRAYSIZE = MsgType_MAX + 1;

const std::string& MsgType_Name(MsgType value);
template<typename T>
inline const std::string& MsgType_Name(T enum_t_value) {
  static_assert(::std::is_same<T, MsgType>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function MsgType_Name.");
  return MsgType_Name(static_cast<MsgType>(enum_t_value));
}
bool MsgType_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, MsgType* value);
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol
}  // namespace flremoting

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::flremoting::protocol::MsgType> : ::std::true_type {};

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_flmsgtype_2eproto
