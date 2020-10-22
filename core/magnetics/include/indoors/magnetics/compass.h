#ifndef INDOORS_MAGNETICS_COMPASS_H
#define INDOORS_MAGNETICS_COMPASS_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class ExtractionCompass final : public pipeline::StandardNode,
                                public pipeline::Loopable {
public:
  explicit ExtractionCompass(std::string annotation);

  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Heading>> *heading();

  void iterate() override;

private:
  pipeline::BufferedInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
};

class NaiveCompass final : public pipeline::StandardNode,
                           public pipeline::Loopable {
public:
  NaiveCompass();

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_calibrated();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Heading>> *heading();

  void iterate() override;

private:
  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::BufferedInput<pipeline::Event<pipeline::Quaternion>> m_orientation;

  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
};

class ParticleCompass final : public pipeline::StandardNode,
                              public pipeline::Loopable {
public:
  ParticleCompass(std::uint_fast32_t seed, std::size_t population,
                  float min_rotation);
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

  void iterate() override;

private:
  class Impl;

  const float m_min_rotation;

  bool m_initialized{false};
  double m_last_update{0};
  float m_last_total_rotation{0};
  std::unique_ptr<Impl> m_impl;

  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>
      m_var_magnetometer_calibrated;
  pipeline::BufferedInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::BufferedInput<pipeline::Event<double>> m_total_rotation;

  pipeline::StandardOutput<pipeline::Event<pipeline::Heading>> m_heading;
  pipeline::StandardOutput<pipeline::Event<pipeline::Quaternion>>
      m_total_orientation;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector2>>
      m_north_confidence;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_external;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_COMPASS_H
