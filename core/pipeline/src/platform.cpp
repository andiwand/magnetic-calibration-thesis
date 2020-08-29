#include <indoors/pipeline/platform.h>

namespace indoors::pipeline {

Platform::Platform(std::string annotation)
    : StandardNode(std::move(annotation)) {}

StandardPlatform::StandardPlatform(std::string annotation)
    : Platform(std::move(annotation)),
      std::enable_shared_from_this<StandardPlatform>(), m_clock{"clock", this},
      m_accelerometer{"accelerometer", this}, m_gyroscope{"gyroscope", this},
      m_magnetometer{"magnetometer", this}, m_magnetometer_uncalibrated{
                                                "magnetometer uncalibrated",
                                                this}, m_magnetometer_bias{"magnetometer bias", this} {}

Output<Event<Clock>> *StandardPlatform::clock() { return &m_clock; }

Output<Event<Vector3>> *StandardPlatform::accelerometer() {
  return &m_accelerometer;
}

Output<Event<Vector3>> *StandardPlatform::gyroscope() { return &m_gyroscope; }

Output<Event<Vector3>> *StandardPlatform::magnetometer() {
  return &m_magnetometer;
}

Output<Event<Vector3>> *StandardPlatform::magnetometer_uncalibrated() {
  return &m_magnetometer_uncalibrated;
}

Output<Event<Vector3>> *StandardPlatform::magnetometer_bias() {
  return &m_magnetometer_bias;
}

} // namespace indoors::pipeline
