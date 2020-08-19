#include <indoors/pipeline/platform.h>

namespace indoors::pipeline {

Platform::Platform(std::string annotation)
    : StandardNode(std::move(annotation)) {}

StandardPlatform::StandardPlatform(std::string annotation)
    : Platform(std::move(annotation)),
      std::enable_shared_from_this<StandardPlatform>(),
      m_clock{std::make_shared<StandardOutput<Event<Clock>>>("clock", nullptr)},
      m_accelerometer{std::make_shared<StandardOutput<Event<Vector3>>>(
          "accelerometer", nullptr)},
      m_gyroscope{std::make_shared<StandardOutput<Event<Vector3>>>("gyroscope",
                                                                   nullptr)},
      m_magnetometer{std::make_shared<StandardOutput<Event<Vector3>>>(
          "magnetometer", nullptr)},
      m_magnetometer_uncalibrated{
          std::make_shared<StandardOutput<Event<Vector3>>>(
              "magnetometer uncalibrated", nullptr)} {}

std::shared_ptr<Output<Event<Clock>>> StandardPlatform::clock() const {
  return m_clock;
}

std::shared_ptr<Output<Event<Vector3>>>
StandardPlatform::accelerometer() const {
  return m_accelerometer;
}

std::shared_ptr<Output<Event<Vector3>>> StandardPlatform::gyroscope() const {
  return m_gyroscope;
}

std::shared_ptr<Output<Event<Vector3>>> StandardPlatform::magnetometer() const {
  return m_magnetometer;
}

std::shared_ptr<Output<Event<Vector3>>>
StandardPlatform::magnetometer_uncalibrated() const {
  return m_magnetometer_uncalibrated;
}

} // namespace indoors::pipeline
