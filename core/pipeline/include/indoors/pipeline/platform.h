#ifndef INDOORS_PIPELINE_PLATFORM_H
#define INDOORS_PIPELINE_PLATFORM_H

#include <indoors/pipeline/event.h>
#include <indoors/pipeline/node.h>

namespace indoors::pipeline {

class Platform : public StandardNode {
public:
  explicit Platform(std::string annotation);

  virtual Output<Event<Clock>> *clock() = 0;

  virtual Output<Event<Vector3>> *accelerometer() = 0;
  virtual Output<Event<Vector3>> *gyroscope() = 0;
  virtual Output<Event<Vector3>> *magnetometer() = 0;

  virtual Output<Event<Vector3>> *magnetometer_uncalibrated() = 0;
};

class StandardPlatform : public Platform,
                         public std::enable_shared_from_this<StandardPlatform> {
public:
  explicit StandardPlatform(std::string annotation);

  Output<Event<Clock>> *clock() override;

  Output<Event<Vector3>> *accelerometer() override;
  Output<Event<Vector3>> *gyroscope() override;
  Output<Event<Vector3>> *magnetometer() override;

  Output<Event<Vector3>> *magnetometer_uncalibrated() override;

  StandardOutput<Event<Clock>> m_clock;
  StandardOutput<Event<Vector3>> m_accelerometer;
  StandardOutput<Event<Vector3>> m_gyroscope;
  StandardOutput<Event<Vector3>> m_magnetometer;
  StandardOutput<Event<Vector3>> m_magnetometer_uncalibrated;
};

// TODO AndroidPlatform for platform specific channels

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PLATFORM_H
