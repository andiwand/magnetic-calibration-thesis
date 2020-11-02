#include <indoors/magnetics/filter_task.h>
#include <indoors/magnetics/live_demo_task.h>
#include <indoors/magnetics/web_server.h>
#include <indoors/pipeline/http.h>
#include <indoors/pipeline/protocol.h>
#include <iostream>
#include <thread>

namespace indoors::magnetics {

class LiveDemoTask::Impl {
public:
  explicit Impl(std::shared_ptr<pipeline::Platform> platform)
      : m_filter_task{platform}, m_web_socket{m_encoder.output()}, m_ioc{1},
        m_web_server{m_ioc, "0.0.0.0", 8000} {
    platform->start()->plug(m_encoder.start());
    platform->stop()->plug(m_encoder.stop());

    m_encoder.create_input(m_filter_task.sampled_magnetometer_uncalibrated());
    m_encoder.create_input("system magnetometer",
                           m_filter_task.sampled_magnetometer());
    m_encoder.create_input("system hard iron",
                           m_filter_task.sampled_magnetometer_bias());
    m_encoder.create_input("madgwick orientation",
                           m_filter_task.madgwick_orientation());
    m_encoder.create_input("total rotation", m_filter_task.total_rotation());
    m_encoder.create_input("particle hard iron",
                           m_filter_task.filter_hard_iron());
    m_encoder.create_input("particle magnetometer",
                           m_filter_task.filter_magnetometer());
    // m_encoder.create_input("system compass", m_system_compass.heading());
    // m_encoder.create_input("naive compass", m_naive_compass.heading());
    // m_encoder.create_input("particle compass", m_particle_compass.heading());

    m_web_server.set_web_socket_handler(&m_web_socket);
  }

  void start() {
    m_filter_task.start();

    m_io_runner = std::thread([this]() { m_ioc.run(); });
  }

  void stop() {
    m_filter_task.stop();

    m_ioc.stop();

    m_io_runner.join();
  }

private:
  FilterTask m_filter_task;

  pipeline::ProtocolEncoder m_encoder;
  pipeline::WebSocket m_web_socket;

  boost::asio::io_context m_ioc;
  WebServer m_web_server;

  std::thread m_io_runner;
};

LiveDemoTask::LiveDemoTask(std::shared_ptr<pipeline::Platform> platform)
    : StandardTask("live demo"), m_impl{std::make_unique<Impl>(
                                     std::move(platform))} {}

LiveDemoTask::~LiveDemoTask() = default;

void LiveDemoTask::start() { m_impl->start(); }

void LiveDemoTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
