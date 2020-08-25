#ifndef INDOORS_PIPELINE_MESSAGE_H
#define INDOORS_PIPELINE_MESSAGE_H

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

template <typename T> struct Series final {
  const double time_begin;
  const double time_end;
  const T *const data_begin;
  const T *const data_end;
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

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_MESSAGE_H
