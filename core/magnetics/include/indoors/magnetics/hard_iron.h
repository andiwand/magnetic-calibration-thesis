#ifndef INDOORS_MAGNETICS_HARD_IRON_H
#define INDOORS_MAGNETICS_HARD_IRON_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class HardIron final : public pipeline::StandardNode,
                       public pipeline::Loopable {
public:
  HardIron(std::uint_fast32_t seed, std::size_t population, float delta_time);
  ~HardIron() override;

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_uncalibrated();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *system_calibration();

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  magnetometer_calibrated();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *hard_iron();

  void iterate() override;

private:
  class Impl;

  bool m_initialized{false};
  std::unique_ptr<Impl> m_impl;
  int m_iteration{0};

  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_uncalibrated;
  pipeline::BufferedInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>
      m_system_calibration;

  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_hard_iron;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_HARD_IRON_H
