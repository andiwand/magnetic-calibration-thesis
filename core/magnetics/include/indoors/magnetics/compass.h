#ifndef INDOORS_MAGNETICS_COMPASS_H
#define INDOORS_MAGNETICS_COMPASS_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class ExtractionCompass final : public pipeline::StandardNode {
public:
  explicit ExtractionCompass(std::string annotation);

  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Heading>> *heading();

  void flush() override;

private:
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Quaternion>>
      m_orientation;
  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
};

class NaiveCompass final : public pipeline::StandardNode {
public:
  NaiveCompass();

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_calibrated();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Heading>> *heading();

  void flush() override;

private:
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Quaternion>>
      m_orientation;

  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
};

class ParticleCompass final : public pipeline::StandardNode {
public:
  struct Config {
    std::size_t population{1000};
    double min_rotation{1};
    double drift_rate{0.01};
    double resampling_rate{0.05};
  };

  ParticleCompass(std::uint_fast32_t seed, const Config &config);
  ~ParticleCompass() override;

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_calibrated();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  var_magnetometer_calibrated();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();
  pipeline::Input<pipeline::Event<double>> *total_rotation();

  pipeline::Output<pipeline::Event<pipeline::Heading>> *heading();
  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *total_orientation();
  pipeline::Output<pipeline::Event<pipeline::Vector2>> *north_confidence();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *external();

  void flush() override;

private:
  class Impl;

  bool m_initialized{false};
  double m_last_update{0};
  float m_last_total_rotation{0};
  std::unique_ptr<Impl> m_impl;

  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>>
      m_var_magnetometer_calibrated;
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Quaternion>>
      m_orientation;
  pipeline::BufferedSeriesInput<pipeline::Event<double>> m_total_rotation;

  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
  pipeline::StandardOutput<pipeline::Event<pipeline::Quaternion>>
      m_total_orientation;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector2>>
      m_north_confidence;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_external;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_COMPASS_H
