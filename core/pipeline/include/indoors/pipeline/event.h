#ifndef INDOORS_PIPELINE_EVENT_H
#define INDOORS_PIPELINE_EVENT_H

#include <indoors/pipeline/protocol/event.pb.h>
#include <utility>

namespace indoors::pipeline {

template <typename T> struct Event {
  double time{0};
  T data;

  Event() = default;
  template <typename... Args>
  Event(const double time, Args... args)
      : time{time}, data{std::forward<Args>(args)...} {}
};

template <typename T> struct DelayedEvent {
  double time{0};
  double delay{0};
  T data;

  DelayedEvent() = default;
  template <typename... Args>
  explicit DelayedEvent(const double time, const double delay, Args... args)
      : time{time}, delay{delay}, data{std::forward<Args>(args)...} {}
};

template <typename T> struct EventTraits {
  static constexpr bool has_time{false};
  static constexpr bool has_delay{false};
};

template <typename T> struct EventTraits<Event<T>> {
  static constexpr bool has_time{true};
  static constexpr bool has_delay{false};

  static constexpr double time(const Event<T> &event) { return event.time; }
};

template <typename T> struct EventTraits<DelayedEvent<T>> {
  static constexpr bool has_time{true};
  static constexpr bool has_delay{true};

  static constexpr double time(const Event<T> &event) { return event.time; }
  static constexpr double delay(const Event<T> &event) { return event.delay; }
};

template <> struct EventTraits<protocol::Event> {
  static constexpr bool has_time{true};
  static constexpr bool has_delay{false};

  static constexpr double time(const protocol::Event &event) {
    return event.t();
  }
};

template <typename T> struct Series final {
  const double time_begin;
  const double time_end;
  const T *const data_begin;
  const T *const data_end;

  const T *begin() const { return data_begin; }
  const T *end() const { return data_end; }
};

struct Void {
  Void() = default;
};

struct Clock {
  double unix_utc{0};

  Clock() = default;
  Clock(const double unix_utc) : unix_utc{unix_utc} {}
};

struct Vector3 {
  double x{0};
  double y{0};
  double z{0};

  Vector3() = default;
  Vector3(const double x, const double y, const double z) : x{x}, y{y}, z{z} {}
};

struct Quaternion {
  double w{0};
  double x{0};
  double y{0};
  double z{0};

  Quaternion() = default;
  Quaternion(const double w, const double x, const double y, const double z)
      : w{w}, x{x}, y{y}, z{z} {}
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_EVENT_H
