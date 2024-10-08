#ifndef INDOORS_PIPELINE_PLATFORM_H
#define INDOORS_PIPELINE_PLATFORM_H

#include <indoors/pipeline/event.h>
#include <indoors/pipeline/node.h>

namespace indoors::pipeline {

class Platform {
public:
  virtual ~Platform() = default;

  virtual const std::string &annotation() const = 0;

  virtual Output<Event<Void>> *start() = 0;
  virtual Output<Event<Void>> *stop() = 0;

  virtual Output<Event<Void>> *tick() = 0;

  virtual Output<Event<Clock>> *clock() = 0;

  virtual Output<Event<Vector3>> *accelerometer() = 0;
  virtual Output<Event<Vector3>> *gyroscope() = 0;
  virtual Output<Event<Vector3>> *magnetometer() = 0;

  virtual Output<Event<Vector3>> *magnetometer_uncalibrated() = 0;
  virtual Output<Event<Vector3>> *magnetometer_bias() = 0;

  virtual Output<Event<Quaternion>> *orientation() = 0;
};

class StandardPlatform : public Platform {
public:
  explicit StandardPlatform(std::string annotation);

  const std::string &annotation() const override;

private:
  const std::string m_annotation;
};

class ForwardPlatform : public StandardPlatform,
                        public std::enable_shared_from_this<ForwardPlatform> {
public:
  explicit ForwardPlatform(std::string annotation);

  Output<Event<Void>> *start() override;
  Output<Event<Void>> *stop() override;

  Output<Event<Void>> *tick() override;

  Output<Event<Clock>> *clock() override;

  Output<Event<Vector3>> *accelerometer() override;
  Output<Event<Vector3>> *gyroscope() override;
  Output<Event<Vector3>> *magnetometer() override;

  Output<Event<Vector3>> *magnetometer_uncalibrated() override;
  Output<Event<Vector3>> *magnetometer_bias() override;

  Output<Event<Quaternion>> *orientation() override;

  StandardOutput<Event<Void>> m_start;
  StandardOutput<Event<Void>> m_stop;
  StandardOutput<Event<Void>> m_tick;
  StandardOutput<Event<Clock>> m_clock;
  StandardOutput<Event<Vector3>> m_accelerometer;
  StandardOutput<Event<Vector3>> m_gyroscope;
  StandardOutput<Event<Vector3>> m_magnetometer;
  StandardOutput<Event<Vector3>> m_magnetometer_uncalibrated;
  StandardOutput<Event<Vector3>> m_magnetometer_bias;
  StandardOutput<Event<Quaternion>> m_orientation;
};

class ComposedPlatform : public StandardPlatform,
                         public std::enable_shared_from_this<ComposedPlatform> {
public:
  explicit ComposedPlatform(std::string annotation);
  ComposedPlatform(std::string annotation, Output<Event<Void>> *start,
                   Output<Event<Void>> *stop, Output<Event<Void>> *tick,
                   Output<Event<Clock>> *clock,
                   Output<Event<Vector3>> *accelerometer,
                   Output<Event<Vector3>> *gyroscope,
                   Output<Event<Vector3>> *magnetometer,
                   Output<Event<Vector3>> *magnetometer_uncalibrated,
                   Output<Event<Vector3>> *magnetometer_bias,
                   Output<Event<Quaternion>> *m_orientation);

  Output<Event<Void>> *start() override;
  Output<Event<Void>> *stop() override;

  Output<Event<Void>> *tick() override;

  Output<Event<Clock>> *clock() override;

  Output<Event<Vector3>> *accelerometer() override;
  Output<Event<Vector3>> *gyroscope() override;
  Output<Event<Vector3>> *magnetometer() override;

  Output<Event<Vector3>> *magnetometer_uncalibrated() override;
  Output<Event<Vector3>> *magnetometer_bias() override;

  Output<Event<Quaternion>> *orientation() override;

  Output<Event<Void>> *m_start{nullptr};
  Output<Event<Void>> *m_stop{nullptr};
  Output<Event<Void>> *m_tick{nullptr};
  Output<Event<Clock>> *m_clock{nullptr};
  Output<Event<Vector3>> *m_accelerometer{nullptr};
  Output<Event<Vector3>> *m_gyroscope{nullptr};
  Output<Event<Vector3>> *m_magnetometer{nullptr};
  Output<Event<Vector3>> *m_magnetometer_uncalibrated{nullptr};
  Output<Event<Vector3>> *m_magnetometer_bias{nullptr};
  Output<Event<Quaternion>> *m_orientation{nullptr};
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PLATFORM_H
