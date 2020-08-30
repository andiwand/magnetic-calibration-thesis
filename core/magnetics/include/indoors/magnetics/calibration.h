#ifndef INDOORS_MAGNETICS_CALIBRATION_H
#define INDOORS_MAGNETICS_CALIBRATION_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class Calibration final : public pipeline::StandardNode,
                          public pipeline::Loopable {
public:
  Calibration(std::uint_fast32_t seed, std::size_t population, float delta_time);
  ~Calibration() override;

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *magnetometer();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *orientation_output();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *hard_iron();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *external();
  pipeline::Output<pipeline::Event<pipeline::Vector2>> *north_confidence();

  void iterate() override;

private:
  class Impl;

  bool m_initialized{false};
  std::unique_ptr<Impl> m_impl;
  int m_iteration{0};

  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>> m_magnetometer;
  pipeline::BufferedInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::StandardOutput<pipeline::Event<pipeline::Quaternion>> m_orientation_output;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_hard_iron;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_external;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector2>> m_north_confidence;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_CALIBRATION_H
