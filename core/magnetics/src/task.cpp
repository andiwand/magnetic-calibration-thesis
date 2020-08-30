#include <indoors/magnetics/calibration.h>
#include <indoors/magnetics/moving_average.h>
#include <indoors/magnetics/orientation_filter.h>
#include <indoors/magnetics/task.h>
#include <indoors/pipeline/http.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol.h>
#include <indoors/pipeline/synchronizer.h>
#include <iostream>
#include <thread>

namespace indoors::magnetics {

namespace {
class DefaultTask : public StandardTask {
public:
  DefaultTask(std::shared_ptr<pipeline::Platform> platform,
              std::optional<std::string> html);

  void start() override;
  void stop() override;

private:
  const std::shared_ptr<pipeline::Platform> m_platform;

  boost::asio::io_context m_ioc;

  pipeline::Synchronizer m_synchronizer;
  MovingAverage m_moving_average;
  MadgwickImu m_madgwick;
  Calibration m_calibration;
  pipeline::ProtocolEncoder m_encoder;
  pipeline::WebSocket m_websocket;

  std::thread m_http_runner;
  std::thread m_io_runner;
  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

DefaultTask::DefaultTask(std::shared_ptr<pipeline::Platform> platform,
                         std::optional<std::string> html)
    : StandardTask("default"), m_platform{std::move(platform)}, m_ioc{1},
      m_synchronizer{0.1}, m_moving_average{0.05, 0.05},
      m_calibration{0, 2000, 0.05},
      m_websocket(m_ioc, "0.0.0.0", 8080, m_encoder.output()) {
  // synchronizer
  auto accelerometer =
      m_synchronizer.create_channel(m_platform->accelerometer());
  auto gyroscope = m_synchronizer.create_channel(m_platform->gyroscope());
  auto magnetometer = m_synchronizer.create_channel(m_platform->magnetometer());
  auto magnetometer_uncalibrated =
      m_synchronizer.create_channel(m_platform->magnetometer_uncalibrated());
  auto magnetometer_bias =
      m_synchronizer.create_channel(m_platform->magnetometer_bias());

  // moving average
  auto sampled_accelerometer = m_moving_average.create_channel(accelerometer);
  auto sampled_gyroscope = m_moving_average.create_channel(gyroscope);
  auto sampled_magnetometer = m_moving_average.create_channel(magnetometer);
  auto sampled_magnetometer_uncalibrated = m_moving_average.create_channel(magnetometer_uncalibrated);
  auto sampled_magnetometer_bias = m_moving_average.create_channel(magnetometer_bias);

  // orientation
  sampled_accelerometer->plug(m_madgwick.accelerometer());
  sampled_gyroscope->plug(m_madgwick.gyroscope());

  // calibration
  sampled_magnetometer_uncalibrated->plug(m_calibration.magnetometer());
  m_madgwick.orientation()->plug(m_calibration.orientation());

  // encoder and websocket
  //m_encoder.create_input(sampled_magnetometer);
  //m_encoder.create_input(sampled_magnetometer_uncalibrated);
  m_encoder.create_input(m_madgwick.orientation());

  // TODO remove
  //m_encoder.create_input(m_calibration.orientation_output());
  m_encoder.create_input(sampled_magnetometer_bias);
  m_encoder.create_input(m_calibration.hard_iron());
  m_encoder.create_input(m_calibration.external());
  m_encoder.create_input(m_calibration.north_confidence());

  // TODO html
}

void DefaultTask::start() {
  m_io_runner = std::thread([this]() { m_ioc.run(); });
  m_looper = std::thread([this]() {
    while (!m_stop) {
      m_synchronizer.iterate();
      m_moving_average.iterate();
      m_madgwick.iterate();
      m_calibration.iterate();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });
}

void DefaultTask::stop() {
  m_ioc.stop();
  m_stop = true;

  m_io_runner.join();
  m_looper.join();
}
} // namespace

std::shared_ptr<Task>
Tasks::create_default(std::shared_ptr<pipeline::Platform> platform) {
  return std::make_shared<DefaultTask>(platform, std::optional<std::string>());
}

std::shared_ptr<Task>
Tasks::create_default(std::shared_ptr<pipeline::Platform> platform,
                      std::string html) {
  return std::make_shared<DefaultTask>(platform, std::move(html));
}

StandardTask::StandardTask(std::string annotation)
    : m_annotation{std::move(annotation)} {}

const std::string &StandardTask::annotation() { return m_annotation; }

void StandardTask::start() {}

void StandardTask::restart() {}

void StandardTask::resume() {}

void StandardTask::pause() {}

void StandardTask::stop() {}

} // namespace indoors::magnetics
