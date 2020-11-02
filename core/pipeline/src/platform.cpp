#include <indoors/pipeline/platform.h>

namespace indoors::pipeline {

StandardPlatform::StandardPlatform(std::string annotation)
    : m_annotation{std::move(annotation)} {}

const std::string &StandardPlatform::annotation() const { return m_annotation; }

ForwardPlatform::ForwardPlatform(std::string annotation)
    : StandardPlatform(std::move(annotation)),
      std::enable_shared_from_this<ForwardPlatform>(),
      m_start{"start", nullptr}, m_stop{"stop", nullptr}, m_tick{"tick",
                                                                 nullptr},
      m_clock{"clock", nullptr}, m_accelerometer{"accelerometer", nullptr},
      m_gyroscope{"gyroscope", nullptr}, m_magnetometer{"magnetometer",
                                                        nullptr},
      m_magnetometer_uncalibrated{"magnetometer uncalibrated", nullptr},
      m_magnetometer_bias{"magnetometer bias", nullptr}, m_orientation{
                                                             "orientation",
                                                             nullptr} {}

Output<Event<Void>> *ForwardPlatform::start() { return &m_start; }

Output<Event<Void>> *ForwardPlatform::stop() { return &m_stop; }

Output<Event<Void>> *ForwardPlatform::tick() { return &m_tick; }

Output<Event<Clock>> *ForwardPlatform::clock() { return &m_clock; }

Output<Event<Vector3>> *ForwardPlatform::accelerometer() {
  return &m_accelerometer;
}

Output<Event<Vector3>> *ForwardPlatform::gyroscope() { return &m_gyroscope; }

Output<Event<Vector3>> *ForwardPlatform::magnetometer() {
  return &m_magnetometer;
}

Output<Event<Vector3>> *ForwardPlatform::magnetometer_uncalibrated() {
  return &m_magnetometer_uncalibrated;
}

Output<Event<Vector3>> *ForwardPlatform::magnetometer_bias() {
  return &m_magnetometer_bias;
}

Output<Event<Quaternion>> *ForwardPlatform::orientation() {
  return &m_orientation;
}

ComposedPlatform::ComposedPlatform(std::string annotation)
    : StandardPlatform(std::move(annotation)) {}

ComposedPlatform::ComposedPlatform(
    std::string annotation, Output<Event<Void>> *start,
    Output<Event<Void>> *stop, Output<Event<Void>> *tick,
    Output<Event<Clock>> *clock, Output<Event<Vector3>> *accelerometer,
    Output<Event<Vector3>> *gyroscope, Output<Event<Vector3>> *magnetometer,
    Output<Event<Vector3>> *magnetometer_uncalibrated,
    Output<Event<Vector3>> *magnetometer_bias,
    Output<Event<Quaternion>> *orientation)
    : StandardPlatform(std::move(annotation)), m_start{start}, m_stop{stop},
      m_tick{tick}, m_clock{clock}, m_accelerometer{accelerometer},
      m_gyroscope{gyroscope}, m_magnetometer{magnetometer},
      m_magnetometer_uncalibrated{magnetometer_uncalibrated},
      m_magnetometer_bias{magnetometer_bias}, m_orientation{orientation} {}

Output<Event<Void>> *ComposedPlatform::start() { return m_start; }

Output<Event<Void>> *ComposedPlatform::stop() { return m_stop; }

Output<Event<Void>> *ComposedPlatform::tick() { return m_tick; }

Output<Event<Clock>> *ComposedPlatform::clock() { return m_clock; }

Output<Event<Vector3>> *ComposedPlatform::accelerometer() {
  return m_accelerometer;
}

Output<Event<Vector3>> *ComposedPlatform::gyroscope() { return m_gyroscope; }

Output<Event<Vector3>> *ComposedPlatform::magnetometer() {
  return m_magnetometer;
}

Output<Event<Vector3>> *ComposedPlatform::magnetometer_uncalibrated() {
  return m_magnetometer_uncalibrated;
}

Output<Event<Vector3>> *ComposedPlatform::magnetometer_bias() {
  return m_magnetometer_bias;
}

Output<Event<Quaternion>> *ComposedPlatform::orientation() {
  return m_orientation;
}

} // namespace indoors::pipeline
