#ifndef INDOORS_MAGNETICS_HARD_IRON_H
#define INDOORS_MAGNETICS_HARD_IRON_H

#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class HardIron final : public pipeline::StandardNode {
public:
  struct Config {
    std::size_t population{10000};
    double min_rotation{0.1};
    double init_std{100};
    double drift_rate{1.0};
    double prediction_std{5};
    double resampling_rate{1e-2};
  };

  HardIron(std::uint_fast32_t seed, const Config &config);
  ~HardIron() override;

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_uncalibrated();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();
  pipeline::Input<pipeline::Event<double>> *total_rotation();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *system_calibration();

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  magnetometer_calibrated();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  var_magnetometer_calibrated();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *hard_iron();

  void flush() override;

private:
  class Impl;

  std::unique_ptr<Impl> m_impl;
  double m_last_update{0};
  float m_last_total_rotation{0};
  bool m_initialized{false};

  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_uncalibrated;
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::BufferedSeriesInput<pipeline::Event<double>> m_total_rotation;
  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>>
      m_system_calibration;

  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>>
      m_magnetometer_calibrated;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>>
      m_var_magnetometer_calibrated;
  pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_hard_iron;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_HARD_IRON_H
