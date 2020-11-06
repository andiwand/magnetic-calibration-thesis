#include <indoors/magnetics/live_demo_filter.h>

namespace indoors::magnetics {

LiveDemoFilter::LiveDemoFilter(const MagneticsFilter::Config &config,
                               std::shared_ptr<pipeline::Platform> platform,
                               boost::asio::io_context &ioc)
    : pipeline::StandardNode("live demo filter"), m_magnetics_filter(config),
      m_web_socket{m_encoder.output()}, m_web_server{ioc, "0.0.0.0", 8000} {
  platform->start()->plug(m_encoder.start());
  platform->stop()->plug(m_encoder.stop());
  platform->accelerometer()->plug(m_magnetics_filter.accelerometer());
  platform->gyroscope()->plug(m_magnetics_filter.gyroscope());
  platform->magnetometer()->plug(m_magnetics_filter.magnetometer());
  platform->magnetometer_uncalibrated()->plug(m_magnetics_filter.magnetometer_uncalibrated());
  platform->magnetometer_bias()->plug(m_magnetics_filter.magnetometer_bias());
  platform->orientation()->plug(m_magnetics_filter.orientation());

  m_encoder.create_input(
      m_magnetics_filter.sampled_magnetometer_uncalibrated());
  m_encoder.create_input("system magnetometer",
                         m_magnetics_filter.sampled_magnetometer());
  m_encoder.create_input("system hard iron",
                         m_magnetics_filter.sampled_magnetometer_bias());
  m_encoder.create_input("madgwick orientation",
                         m_magnetics_filter.madgwick_orientation());
  m_encoder.create_input("total rotation", m_magnetics_filter.total_rotation());
  m_encoder.create_input("particle hard iron",
                         m_magnetics_filter.filter_hard_iron());
  m_encoder.create_input("particle magnetometer",
                         m_magnetics_filter.filter_magnetometer());
  // m_encoder.create_input("system compass", m_system_compass.heading());
  // m_encoder.create_input("naive compass", m_naive_compass.heading());
  // m_encoder.create_input("particle compass", m_particle_compass.heading());

  m_web_server.set_web_socket_handler(&m_web_socket);
}

MagneticsFilter &LiveDemoFilter::magnetics_filter() {
  return m_magnetics_filter;
}

void LiveDemoFilter::flush() { m_magnetics_filter.flush(); }

} // namespace indoors::magnetics
