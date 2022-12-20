// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: flpreview.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_flpreview_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_flpreview_2eproto

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
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_flpreview_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_flpreview_2eproto {
  static const uint32_t offsets[];
};
namespace flremoting {
namespace protocol {
class PreviewCanvasMessage;
struct PreviewCanvasMessageDefaultTypeInternal;
extern PreviewCanvasMessageDefaultTypeInternal _PreviewCanvasMessage_default_instance_;
class PreviewMouseMenuMessage;
struct PreviewMouseMenuMessageDefaultTypeInternal;
extern PreviewMouseMenuMessageDefaultTypeInternal _PreviewMouseMenuMessage_default_instance_;
class PreviewSizeMessage;
struct PreviewSizeMessageDefaultTypeInternal;
extern PreviewSizeMessageDefaultTypeInternal _PreviewSizeMessage_default_instance_;
}  // namespace protocol
}  // namespace flremoting
PROTOBUF_NAMESPACE_OPEN
template<> ::flremoting::protocol::PreviewCanvasMessage* Arena::CreateMaybeMessage<::flremoting::protocol::PreviewCanvasMessage>(Arena*);
template<> ::flremoting::protocol::PreviewMouseMenuMessage* Arena::CreateMaybeMessage<::flremoting::protocol::PreviewMouseMenuMessage>(Arena*);
template<> ::flremoting::protocol::PreviewSizeMessage* Arena::CreateMaybeMessage<::flremoting::protocol::PreviewSizeMessage>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace flremoting {
namespace protocol {

// ===================================================================

class PreviewSizeMessage final :
    public ::PROTOBUF_NAMESPACE_ID::MessageLite /* @@protoc_insertion_point(class_definition:flremoting.protocol.PreviewSizeMessage) */ {
 public:
  inline PreviewSizeMessage() : PreviewSizeMessage(nullptr) {}
  ~PreviewSizeMessage() override;
  explicit PROTOBUF_CONSTEXPR PreviewSizeMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  PreviewSizeMessage(const PreviewSizeMessage& from);
  PreviewSizeMessage(PreviewSizeMessage&& from) noexcept
    : PreviewSizeMessage() {
    *this = ::std::move(from);
  }

  inline PreviewSizeMessage& operator=(const PreviewSizeMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline PreviewSizeMessage& operator=(PreviewSizeMessage&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const PreviewSizeMessage& default_instance() {
    return *internal_default_instance();
  }
  static inline const PreviewSizeMessage* internal_default_instance() {
    return reinterpret_cast<const PreviewSizeMessage*>(
               &_PreviewSizeMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(PreviewSizeMessage& a, PreviewSizeMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(PreviewSizeMessage* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(PreviewSizeMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  PreviewSizeMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<PreviewSizeMessage>(arena);
  }
  void CheckTypeAndMergeFrom(const ::PROTOBUF_NAMESPACE_ID::MessageLite& from)  final;
  void CopyFrom(const PreviewSizeMessage& from);
  void MergeFrom(const PreviewSizeMessage& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PreviewSizeMessage* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "flremoting.protocol.PreviewSizeMessage";
  }
  protected:
  explicit PreviewSizeMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kCxFieldNumber = 1,
    kCyFieldNumber = 2,
  };
  // optional int32 cx = 1;
  bool has_cx() const;
  private:
  bool _internal_has_cx() const;
  public:
  void clear_cx();
  int32_t cx() const;
  void set_cx(int32_t value);
  private:
  int32_t _internal_cx() const;
  void _internal_set_cx(int32_t value);
  public:

  // optional int32 cy = 2;
  bool has_cy() const;
  private:
  bool _internal_has_cy() const;
  public:
  void clear_cy();
  int32_t cy() const;
  void set_cy(int32_t value);
  private:
  int32_t _internal_cy() const;
  void _internal_set_cy(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:flremoting.protocol.PreviewSizeMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  int32_t cx_;
  int32_t cy_;
  friend struct ::TableStruct_flpreview_2eproto;
};
// -------------------------------------------------------------------

class PreviewCanvasMessage final :
    public ::PROTOBUF_NAMESPACE_ID::MessageLite /* @@protoc_insertion_point(class_definition:flremoting.protocol.PreviewCanvasMessage) */ {
 public:
  inline PreviewCanvasMessage() : PreviewCanvasMessage(nullptr) {}
  ~PreviewCanvasMessage() override;
  explicit PROTOBUF_CONSTEXPR PreviewCanvasMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  PreviewCanvasMessage(const PreviewCanvasMessage& from);
  PreviewCanvasMessage(PreviewCanvasMessage&& from) noexcept
    : PreviewCanvasMessage() {
    *this = ::std::move(from);
  }

  inline PreviewCanvasMessage& operator=(const PreviewCanvasMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline PreviewCanvasMessage& operator=(PreviewCanvasMessage&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const PreviewCanvasMessage& default_instance() {
    return *internal_default_instance();
  }
  static inline const PreviewCanvasMessage* internal_default_instance() {
    return reinterpret_cast<const PreviewCanvasMessage*>(
               &_PreviewCanvasMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(PreviewCanvasMessage& a, PreviewCanvasMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(PreviewCanvasMessage* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(PreviewCanvasMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  PreviewCanvasMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<PreviewCanvasMessage>(arena);
  }
  void CheckTypeAndMergeFrom(const ::PROTOBUF_NAMESPACE_ID::MessageLite& from)  final;
  void CopyFrom(const PreviewCanvasMessage& from);
  void MergeFrom(const PreviewCanvasMessage& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PreviewCanvasMessage* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "flremoting.protocol.PreviewCanvasMessage";
  }
  protected:
  explicit PreviewCanvasMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kBaseFieldNumber = 1,
    kResFieldNumber = 3,
    kResTypeFieldNumber = 2,
    kIsPortraitFieldNumber = 4,
  };
  // optional .flremoting.protocol.PreviewSizeMessage base = 1;
  bool has_base() const;
  private:
  bool _internal_has_base() const;
  public:
  void clear_base();
  const ::flremoting::protocol::PreviewSizeMessage& base() const;
  PROTOBUF_NODISCARD ::flremoting::protocol::PreviewSizeMessage* release_base();
  ::flremoting::protocol::PreviewSizeMessage* mutable_base();
  void set_allocated_base(::flremoting::protocol::PreviewSizeMessage* base);
  private:
  const ::flremoting::protocol::PreviewSizeMessage& _internal_base() const;
  ::flremoting::protocol::PreviewSizeMessage* _internal_mutable_base();
  public:
  void unsafe_arena_set_allocated_base(
      ::flremoting::protocol::PreviewSizeMessage* base);
  ::flremoting::protocol::PreviewSizeMessage* unsafe_arena_release_base();

  // optional .flremoting.protocol.PreviewSizeMessage res = 3;
  bool has_res() const;
  private:
  bool _internal_has_res() const;
  public:
  void clear_res();
  const ::flremoting::protocol::PreviewSizeMessage& res() const;
  PROTOBUF_NODISCARD ::flremoting::protocol::PreviewSizeMessage* release_res();
  ::flremoting::protocol::PreviewSizeMessage* mutable_res();
  void set_allocated_res(::flremoting::protocol::PreviewSizeMessage* res);
  private:
  const ::flremoting::protocol::PreviewSizeMessage& _internal_res() const;
  ::flremoting::protocol::PreviewSizeMessage* _internal_mutable_res();
  public:
  void unsafe_arena_set_allocated_res(
      ::flremoting::protocol::PreviewSizeMessage* res);
  ::flremoting::protocol::PreviewSizeMessage* unsafe_arena_release_res();

  // optional int32 res_type = 2;
  bool has_res_type() const;
  private:
  bool _internal_has_res_type() const;
  public:
  void clear_res_type();
  int32_t res_type() const;
  void set_res_type(int32_t value);
  private:
  int32_t _internal_res_type() const;
  void _internal_set_res_type(int32_t value);
  public:

  // optional bool is_portrait = 4;
  bool has_is_portrait() const;
  private:
  bool _internal_has_is_portrait() const;
  public:
  void clear_is_portrait();
  bool is_portrait() const;
  void set_is_portrait(bool value);
  private:
  bool _internal_is_portrait() const;
  void _internal_set_is_portrait(bool value);
  public:

  // @@protoc_insertion_point(class_scope:flremoting.protocol.PreviewCanvasMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::flremoting::protocol::PreviewSizeMessage* base_;
  ::flremoting::protocol::PreviewSizeMessage* res_;
  int32_t res_type_;
  bool is_portrait_;
  friend struct ::TableStruct_flpreview_2eproto;
};
// -------------------------------------------------------------------

class PreviewMouseMenuMessage final :
    public ::PROTOBUF_NAMESPACE_ID::MessageLite /* @@protoc_insertion_point(class_definition:flremoting.protocol.PreviewMouseMenuMessage) */ {
 public:
  inline PreviewMouseMenuMessage() : PreviewMouseMenuMessage(nullptr) {}
  ~PreviewMouseMenuMessage() override;
  explicit PROTOBUF_CONSTEXPR PreviewMouseMenuMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  PreviewMouseMenuMessage(const PreviewMouseMenuMessage& from);
  PreviewMouseMenuMessage(PreviewMouseMenuMessage&& from) noexcept
    : PreviewMouseMenuMessage() {
    *this = ::std::move(from);
  }

  inline PreviewMouseMenuMessage& operator=(const PreviewMouseMenuMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline PreviewMouseMenuMessage& operator=(PreviewMouseMenuMessage&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const PreviewMouseMenuMessage& default_instance() {
    return *internal_default_instance();
  }
  static inline const PreviewMouseMenuMessage* internal_default_instance() {
    return reinterpret_cast<const PreviewMouseMenuMessage*>(
               &_PreviewMouseMenuMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  friend void swap(PreviewMouseMenuMessage& a, PreviewMouseMenuMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(PreviewMouseMenuMessage* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(PreviewMouseMenuMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  PreviewMouseMenuMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<PreviewMouseMenuMessage>(arena);
  }
  void CheckTypeAndMergeFrom(const ::PROTOBUF_NAMESPACE_ID::MessageLite& from)  final;
  void CopyFrom(const PreviewMouseMenuMessage& from);
  void MergeFrom(const PreviewMouseMenuMessage& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(PreviewMouseMenuMessage* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "flremoting.protocol.PreviewMouseMenuMessage";
  }
  protected:
  explicit PreviewMouseMenuMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kIdFieldNumber = 1,
    kXFieldNumber = 2,
    kYFieldNumber = 3,
  };
  // optional int64 id = 1;
  bool has_id() const;
  private:
  bool _internal_has_id() const;
  public:
  void clear_id();
  int64_t id() const;
  void set_id(int64_t value);
  private:
  int64_t _internal_id() const;
  void _internal_set_id(int64_t value);
  public:

  // optional int32 x = 2;
  bool has_x() const;
  private:
  bool _internal_has_x() const;
  public:
  void clear_x();
  int32_t x() const;
  void set_x(int32_t value);
  private:
  int32_t _internal_x() const;
  void _internal_set_x(int32_t value);
  public:

  // optional int32 y = 3;
  bool has_y() const;
  private:
  bool _internal_has_y() const;
  public:
  void clear_y();
  int32_t y() const;
  void set_y(int32_t value);
  private:
  int32_t _internal_y() const;
  void _internal_set_y(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:flremoting.protocol.PreviewMouseMenuMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  int64_t id_;
  int32_t x_;
  int32_t y_;
  friend struct ::TableStruct_flpreview_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PreviewSizeMessage

// optional int32 cx = 1;
inline bool PreviewSizeMessage::_internal_has_cx() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool PreviewSizeMessage::has_cx() const {
  return _internal_has_cx();
}
inline void PreviewSizeMessage::clear_cx() {
  cx_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline int32_t PreviewSizeMessage::_internal_cx() const {
  return cx_;
}
inline int32_t PreviewSizeMessage::cx() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewSizeMessage.cx)
  return _internal_cx();
}
inline void PreviewSizeMessage::_internal_set_cx(int32_t value) {
  _has_bits_[0] |= 0x00000001u;
  cx_ = value;
}
inline void PreviewSizeMessage::set_cx(int32_t value) {
  _internal_set_cx(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewSizeMessage.cx)
}

// optional int32 cy = 2;
inline bool PreviewSizeMessage::_internal_has_cy() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool PreviewSizeMessage::has_cy() const {
  return _internal_has_cy();
}
inline void PreviewSizeMessage::clear_cy() {
  cy_ = 0;
  _has_bits_[0] &= ~0x00000002u;
}
inline int32_t PreviewSizeMessage::_internal_cy() const {
  return cy_;
}
inline int32_t PreviewSizeMessage::cy() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewSizeMessage.cy)
  return _internal_cy();
}
inline void PreviewSizeMessage::_internal_set_cy(int32_t value) {
  _has_bits_[0] |= 0x00000002u;
  cy_ = value;
}
inline void PreviewSizeMessage::set_cy(int32_t value) {
  _internal_set_cy(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewSizeMessage.cy)
}

// -------------------------------------------------------------------

// PreviewCanvasMessage

// optional .flremoting.protocol.PreviewSizeMessage base = 1;
inline bool PreviewCanvasMessage::_internal_has_base() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  PROTOBUF_ASSUME(!value || base_ != nullptr);
  return value;
}
inline bool PreviewCanvasMessage::has_base() const {
  return _internal_has_base();
}
inline void PreviewCanvasMessage::clear_base() {
  if (base_ != nullptr) base_->Clear();
  _has_bits_[0] &= ~0x00000001u;
}
inline const ::flremoting::protocol::PreviewSizeMessage& PreviewCanvasMessage::_internal_base() const {
  const ::flremoting::protocol::PreviewSizeMessage* p = base_;
  return p != nullptr ? *p : reinterpret_cast<const ::flremoting::protocol::PreviewSizeMessage&>(
      ::flremoting::protocol::_PreviewSizeMessage_default_instance_);
}
inline const ::flremoting::protocol::PreviewSizeMessage& PreviewCanvasMessage::base() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewCanvasMessage.base)
  return _internal_base();
}
inline void PreviewCanvasMessage::unsafe_arena_set_allocated_base(
    ::flremoting::protocol::PreviewSizeMessage* base) {
  if (GetArenaForAllocation() == nullptr) {
    delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(base_);
  }
  base_ = base;
  if (base) {
    _has_bits_[0] |= 0x00000001u;
  } else {
    _has_bits_[0] &= ~0x00000001u;
  }
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:flremoting.protocol.PreviewCanvasMessage.base)
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::release_base() {
  _has_bits_[0] &= ~0x00000001u;
  ::flremoting::protocol::PreviewSizeMessage* temp = base_;
  base_ = nullptr;
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  auto* old =  reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(temp);
  temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  if (GetArenaForAllocation() == nullptr) { delete old; }
#else  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (GetArenaForAllocation() != nullptr) {
    temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  }
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
  return temp;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::unsafe_arena_release_base() {
  // @@protoc_insertion_point(field_release:flremoting.protocol.PreviewCanvasMessage.base)
  _has_bits_[0] &= ~0x00000001u;
  ::flremoting::protocol::PreviewSizeMessage* temp = base_;
  base_ = nullptr;
  return temp;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::_internal_mutable_base() {
  _has_bits_[0] |= 0x00000001u;
  if (base_ == nullptr) {
    auto* p = CreateMaybeMessage<::flremoting::protocol::PreviewSizeMessage>(GetArenaForAllocation());
    base_ = p;
  }
  return base_;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::mutable_base() {
  ::flremoting::protocol::PreviewSizeMessage* _msg = _internal_mutable_base();
  // @@protoc_insertion_point(field_mutable:flremoting.protocol.PreviewCanvasMessage.base)
  return _msg;
}
inline void PreviewCanvasMessage::set_allocated_base(::flremoting::protocol::PreviewSizeMessage* base) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  if (message_arena == nullptr) {
    delete base_;
  }
  if (base) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
        ::PROTOBUF_NAMESPACE_ID::Arena::InternalGetOwningArena(base);
    if (message_arena != submessage_arena) {
      base = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, base, submessage_arena);
    }
    _has_bits_[0] |= 0x00000001u;
  } else {
    _has_bits_[0] &= ~0x00000001u;
  }
  base_ = base;
  // @@protoc_insertion_point(field_set_allocated:flremoting.protocol.PreviewCanvasMessage.base)
}

// optional int32 res_type = 2;
inline bool PreviewCanvasMessage::_internal_has_res_type() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool PreviewCanvasMessage::has_res_type() const {
  return _internal_has_res_type();
}
inline void PreviewCanvasMessage::clear_res_type() {
  res_type_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline int32_t PreviewCanvasMessage::_internal_res_type() const {
  return res_type_;
}
inline int32_t PreviewCanvasMessage::res_type() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewCanvasMessage.res_type)
  return _internal_res_type();
}
inline void PreviewCanvasMessage::_internal_set_res_type(int32_t value) {
  _has_bits_[0] |= 0x00000004u;
  res_type_ = value;
}
inline void PreviewCanvasMessage::set_res_type(int32_t value) {
  _internal_set_res_type(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewCanvasMessage.res_type)
}

// optional .flremoting.protocol.PreviewSizeMessage res = 3;
inline bool PreviewCanvasMessage::_internal_has_res() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  PROTOBUF_ASSUME(!value || res_ != nullptr);
  return value;
}
inline bool PreviewCanvasMessage::has_res() const {
  return _internal_has_res();
}
inline void PreviewCanvasMessage::clear_res() {
  if (res_ != nullptr) res_->Clear();
  _has_bits_[0] &= ~0x00000002u;
}
inline const ::flremoting::protocol::PreviewSizeMessage& PreviewCanvasMessage::_internal_res() const {
  const ::flremoting::protocol::PreviewSizeMessage* p = res_;
  return p != nullptr ? *p : reinterpret_cast<const ::flremoting::protocol::PreviewSizeMessage&>(
      ::flremoting::protocol::_PreviewSizeMessage_default_instance_);
}
inline const ::flremoting::protocol::PreviewSizeMessage& PreviewCanvasMessage::res() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewCanvasMessage.res)
  return _internal_res();
}
inline void PreviewCanvasMessage::unsafe_arena_set_allocated_res(
    ::flremoting::protocol::PreviewSizeMessage* res) {
  if (GetArenaForAllocation() == nullptr) {
    delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(res_);
  }
  res_ = res;
  if (res) {
    _has_bits_[0] |= 0x00000002u;
  } else {
    _has_bits_[0] &= ~0x00000002u;
  }
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:flremoting.protocol.PreviewCanvasMessage.res)
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::release_res() {
  _has_bits_[0] &= ~0x00000002u;
  ::flremoting::protocol::PreviewSizeMessage* temp = res_;
  res_ = nullptr;
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  auto* old =  reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(temp);
  temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  if (GetArenaForAllocation() == nullptr) { delete old; }
#else  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (GetArenaForAllocation() != nullptr) {
    temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  }
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
  return temp;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::unsafe_arena_release_res() {
  // @@protoc_insertion_point(field_release:flremoting.protocol.PreviewCanvasMessage.res)
  _has_bits_[0] &= ~0x00000002u;
  ::flremoting::protocol::PreviewSizeMessage* temp = res_;
  res_ = nullptr;
  return temp;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::_internal_mutable_res() {
  _has_bits_[0] |= 0x00000002u;
  if (res_ == nullptr) {
    auto* p = CreateMaybeMessage<::flremoting::protocol::PreviewSizeMessage>(GetArenaForAllocation());
    res_ = p;
  }
  return res_;
}
inline ::flremoting::protocol::PreviewSizeMessage* PreviewCanvasMessage::mutable_res() {
  ::flremoting::protocol::PreviewSizeMessage* _msg = _internal_mutable_res();
  // @@protoc_insertion_point(field_mutable:flremoting.protocol.PreviewCanvasMessage.res)
  return _msg;
}
inline void PreviewCanvasMessage::set_allocated_res(::flremoting::protocol::PreviewSizeMessage* res) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  if (message_arena == nullptr) {
    delete res_;
  }
  if (res) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
        ::PROTOBUF_NAMESPACE_ID::Arena::InternalGetOwningArena(res);
    if (message_arena != submessage_arena) {
      res = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, res, submessage_arena);
    }
    _has_bits_[0] |= 0x00000002u;
  } else {
    _has_bits_[0] &= ~0x00000002u;
  }
  res_ = res;
  // @@protoc_insertion_point(field_set_allocated:flremoting.protocol.PreviewCanvasMessage.res)
}

// optional bool is_portrait = 4;
inline bool PreviewCanvasMessage::_internal_has_is_portrait() const {
  bool value = (_has_bits_[0] & 0x00000008u) != 0;
  return value;
}
inline bool PreviewCanvasMessage::has_is_portrait() const {
  return _internal_has_is_portrait();
}
inline void PreviewCanvasMessage::clear_is_portrait() {
  is_portrait_ = false;
  _has_bits_[0] &= ~0x00000008u;
}
inline bool PreviewCanvasMessage::_internal_is_portrait() const {
  return is_portrait_;
}
inline bool PreviewCanvasMessage::is_portrait() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewCanvasMessage.is_portrait)
  return _internal_is_portrait();
}
inline void PreviewCanvasMessage::_internal_set_is_portrait(bool value) {
  _has_bits_[0] |= 0x00000008u;
  is_portrait_ = value;
}
inline void PreviewCanvasMessage::set_is_portrait(bool value) {
  _internal_set_is_portrait(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewCanvasMessage.is_portrait)
}

// -------------------------------------------------------------------

// PreviewMouseMenuMessage

// optional int64 id = 1;
inline bool PreviewMouseMenuMessage::_internal_has_id() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool PreviewMouseMenuMessage::has_id() const {
  return _internal_has_id();
}
inline void PreviewMouseMenuMessage::clear_id() {
  id_ = int64_t{0};
  _has_bits_[0] &= ~0x00000001u;
}
inline int64_t PreviewMouseMenuMessage::_internal_id() const {
  return id_;
}
inline int64_t PreviewMouseMenuMessage::id() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewMouseMenuMessage.id)
  return _internal_id();
}
inline void PreviewMouseMenuMessage::_internal_set_id(int64_t value) {
  _has_bits_[0] |= 0x00000001u;
  id_ = value;
}
inline void PreviewMouseMenuMessage::set_id(int64_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewMouseMenuMessage.id)
}

// optional int32 x = 2;
inline bool PreviewMouseMenuMessage::_internal_has_x() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool PreviewMouseMenuMessage::has_x() const {
  return _internal_has_x();
}
inline void PreviewMouseMenuMessage::clear_x() {
  x_ = 0;
  _has_bits_[0] &= ~0x00000002u;
}
inline int32_t PreviewMouseMenuMessage::_internal_x() const {
  return x_;
}
inline int32_t PreviewMouseMenuMessage::x() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewMouseMenuMessage.x)
  return _internal_x();
}
inline void PreviewMouseMenuMessage::_internal_set_x(int32_t value) {
  _has_bits_[0] |= 0x00000002u;
  x_ = value;
}
inline void PreviewMouseMenuMessage::set_x(int32_t value) {
  _internal_set_x(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewMouseMenuMessage.x)
}

// optional int32 y = 3;
inline bool PreviewMouseMenuMessage::_internal_has_y() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool PreviewMouseMenuMessage::has_y() const {
  return _internal_has_y();
}
inline void PreviewMouseMenuMessage::clear_y() {
  y_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline int32_t PreviewMouseMenuMessage::_internal_y() const {
  return y_;
}
inline int32_t PreviewMouseMenuMessage::y() const {
  // @@protoc_insertion_point(field_get:flremoting.protocol.PreviewMouseMenuMessage.y)
  return _internal_y();
}
inline void PreviewMouseMenuMessage::_internal_set_y(int32_t value) {
  _has_bits_[0] |= 0x00000004u;
  y_ = value;
}
inline void PreviewMouseMenuMessage::set_y(int32_t value) {
  _internal_set_y(value);
  // @@protoc_insertion_point(field_set:flremoting.protocol.PreviewMouseMenuMessage.y)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol
}  // namespace flremoting

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_flpreview_2eproto
