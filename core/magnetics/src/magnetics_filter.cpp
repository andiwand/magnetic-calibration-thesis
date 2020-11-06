#include <indoors/magnetics/magnetics_filter.h>

namespace indoors::magnetics {

MagneticsFilter::MagneticsFilter(const Config &config)
    : pipeline::StandardNode("magnetics filter"),
      m_synchronizer{config.max_slave_delay},
      m_moving_average{config.delta_time, config.delta_time},
      m_madgwick{(float)config.delta_time, (float)config.madgwick_beta},
      m_hard_iron{std::random_device()(), config.hard_iron},
      m_system_compass{"system compass"},
      m_particle_compass{std::random_device()(), config.compass} {
  // synchronizer
  auto [tmp_1, accelerometer] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Vector3>>(
          "accelerometer");
  m_accelerometer = tmp_1;
  auto [tmp_2, gyroscope] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Vector3>>(
          "gyroscope");
  m_gyroscope = tmp_2;
  auto [tmp_3, magnetometer] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Vector3>>(
          "magnetometer");
  m_magnetometer = tmp_3;
  auto [tmp_4, magnetometer_uncalibrated] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Vector3>>(
          "magnetometer uncalibrated");
  m_magnetometer_uncalibrated = tmp_4;
  auto [tmp_5, magnetometer_bias] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Vector3>>(
          "magnetometer bias");
  m_magnetometer_bias = tmp_5;
  auto [tmp_6, orientation] =
      m_synchronizer.create_channel<pipeline::Event<pipeline::Quaternion>>(
          "orientation");
  m_orientation = tmp_6;

  // moving average
  auto sampled_accelerometer = m_moving_average.create_channel(accelerometer);
  auto sampled_gyroscope = m_moving_average.create_channel(gyroscope);
  m_sampled_magnetometer = m_moving_average.create_channel(magnetometer);
  m_sampled_magnetometer_uncalibrated =
      m_moving_average.create_channel(magnetometer_uncalibrated);
  m_sampled_magnetometer_bias =
      m_moving_average.create_channel(magnetometer_bias);

  // orientation
  sampled_accelerometer->plug(m_madgwick.accelerometer());
  sampled_gyroscope->plug(m_madgwick.gyroscope());

  // total rotation
  m_madgwick.orientation()->plug(m_total_rotation.orientation());

  // hard iron
  m_sampled_magnetometer_uncalibrated->plug(
      m_hard_iron.magnetometer_uncalibrated());
  m_madgwick.orientation()->plug(m_hard_iron.orientation());
  m_total_rotation.total_rotation()->plug(m_hard_iron.total_rotation());

  // system compass
  orientation->plug(m_system_compass.orientation());

  // naive compass
  m_hard_iron.magnetometer_calibrated()->plug(
      m_naive_compass.magnetometer_calibrated());
  m_madgwick.orientation()->plug(m_naive_compass.orientation());

  // particle compass
  m_hard_iron.magnetometer_calibrated()->plug(
      m_particle_compass.magnetometer_calibrated());
  m_hard_iron.var_magnetometer_calibrated()->plug(
      m_particle_compass.var_magnetometer_calibrated());
  m_madgwick.orientation()->plug(m_particle_compass.orientation());
  m_total_rotation.total_rotation()->plug(m_particle_compass.total_rotation());
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::accelerometer() {
  return m_accelerometer;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::gyroscope() {
  return m_gyroscope;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::magnetometer() {
  return m_magnetometer;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::magnetometer_uncalibrated() {
  return m_magnetometer_uncalibrated;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::magnetometer_bias() {
  return m_magnetometer_bias;
}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
MagneticsFilter::orientation() {
  return m_orientation;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::sampled_magnetometer() {
  return m_sampled_magnetometer;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::sampled_magnetometer_bias() {
  return m_sampled_magnetometer_bias;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::sampled_magnetometer_uncalibrated() {
  return m_sampled_magnetometer_uncalibrated;
}

pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
MagneticsFilter::madgwick_orientation() {
  return m_madgwick.orientation();
}

pipeline::Output<pipeline::Event<double>> *MagneticsFilter::total_rotation() {
  return m_total_rotation.total_rotation();
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::filter_hard_iron() {
  return m_hard_iron.hard_iron();
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MagneticsFilter::filter_magnetometer() {
  return m_hard_iron.magnetometer_calibrated();
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> * MagneticsFilter::filter_var() {
  return m_hard_iron.var_magnetometer_calibrated();
}

void MagneticsFilter::flush() {
  m_synchronizer.flush();
  m_moving_average.flush();
  m_madgwick.flush();
  m_total_rotation.flush();
  m_hard_iron.flush();

  /*
  static auto begin = std::chrono::steady_clock::now();
  static double time_sum_hard_iron = 0;
  static auto last_print = begin;

  const auto begin_hi = std::chrono::steady_clock::now();
  m_hard_iron.flush();
  const auto end_hi = std::chrono::steady_clock::now();
  const double time_hi = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hi - begin_hi).count() * 1e-9;
  time_sum_hard_iron += time_hi;

  if (std::chrono::duration_cast<std::chrono::seconds>(end_hi - last_print).count() > 1) {
    const double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hi - begin).count() * 1e-9;
    std::cout << "cpu usage " << time_sum_hard_iron / time << std::endl;
    last_print = end_hi;
  }
   */

  m_system_compass.flush();
  m_naive_compass.flush();
  m_particle_compass.flush();
}

} // namespace indoors::magnetics
