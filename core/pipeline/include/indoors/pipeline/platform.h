#ifndef INDOORS_PIPELINE_PLATFORM_H
#define INDOORS_PIPELINE_PLATFORM_H

#include <indoors/pipeline/message.h>
#include <indoors/pipeline/node.h>

namespace indoors::pipeline {

class Platform : public StandardNode {
public:
  explicit Platform(std::string annotation);

  virtual std::shared_ptr<Output<Event<Clock>>> clock() const = 0;

  virtual std::shared_ptr<Output<Event<Vector3>>> accelerometer() const = 0;
  virtual std::shared_ptr<Output<Event<Vector3>>> gyroscope() const = 0;
  virtual std::shared_ptr<Output<Event<Vector3>>> magnetometer() const = 0;

  virtual std::shared_ptr<Output<Event<Vector3>>>
  magnetometer_uncalibrated() const = 0;
};

class StandardPlatform : public Platform,
                         public std::enable_shared_from_this<StandardPlatform> {
public:
  explicit StandardPlatform(std::string annotation);

  std::shared_ptr<Output<Event<Clock>>> clock() const override;

  std::shared_ptr<Output<Event<Vector3>>> accelerometer() const override;
  std::shared_ptr<Output<Event<Vector3>>> gyroscope() const override;
  std::shared_ptr<Output<Event<Vector3>>> magnetometer() const override;

  std::shared_ptr<Output<Event<Vector3>>>
  magnetometer_uncalibrated() const override;

  std::shared_ptr<StandardOutput<Event<Clock>>> m_clock;
  std::shared_ptr<StandardOutput<Event<Vector3>>> m_accelerometer;
  std::shared_ptr<StandardOutput<Event<Vector3>>> m_gyroscope;
  std::shared_ptr<StandardOutput<Event<Vector3>>> m_magnetometer;
  std::shared_ptr<StandardOutput<Event<Vector3>>> m_magnetometer_uncalibrated;
};

// TODO AndroidPlatform for platform specific channels

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PLATFORM_H
