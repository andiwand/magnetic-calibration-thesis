#include <indoors/magnetics/moving_average.h>
#include <indoors/magnetics/orientation_filter.h>
#include <indoors/magnetics/task.h>
#include <indoors/pipeline/synchronizer.h>
#include <indoors/pipeline/http.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol.h>
#include <iostream>
#include <thread>

namespace indoors::magnetics {

namespace {
class StandardTask : public Task {
public:
  StandardTask(std::string annotation,
               std::shared_ptr<pipeline::Platform> platform);

  const std::string &annotation() override;

  void start() override;

  void restart() override {}

  void resume() override {}

  void pause() override {}

  void stop() override;

private:
  const std::string m_annotation;
  const std::shared_ptr<pipeline::Platform> m_platform;

  pipeline::Synchronizer m_synchronizer;
  MovingAverage m_moving_average;
  MadgwickImu m_madgwick;
  pipeline::ProtocolEncoder m_encoder;
  boost::asio::io_context m_ioc;
  pipeline::WebSocket m_websocket;

  std::thread m_io_runner;
  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

StandardTask::StandardTask(std::string annotation,
                           std::shared_ptr<pipeline::Platform> platform)
    : m_annotation{std::move(annotation)}, m_platform{std::move(platform)},
      m_synchronizer{0.1},
      m_moving_average{0.05, 0.05}, m_ioc{1},
      m_websocket(m_ioc, "0.0.0.0", 8080, m_encoder.output()) {
  auto accelerometer = m_synchronizer.create_channel(m_platform->accelerometer());
  auto gyroscope = m_synchronizer.create_channel(m_platform->gyroscope());
  auto magnetometer = m_synchronizer.create_channel(m_platform->magnetometer());
  auto magnetometer_uncalibrated = m_synchronizer.create_channel(m_platform->magnetometer_uncalibrated());

  m_moving_average.create_channel(accelerometer)->plug(m_madgwick.accelerometer());
  m_moving_average.create_channel(gyroscope)->plug(m_madgwick.gyroscope());

  m_encoder.create_input(m_moving_average.create_channel(magnetometer));
  m_encoder.create_input(m_moving_average.create_channel(magnetometer_uncalibrated));
  m_encoder.create_input(m_madgwick.orientation());
}

const std::string &StandardTask::annotation() { return m_annotation; }
} // namespace

void StandardTask::start() {
  m_io_runner = std::thread([this]() { m_ioc.run(); });
  m_looper = std::thread([this]() {
    while (!m_stop) {
      m_synchronizer.iterate();
      m_moving_average.iterate();
      m_madgwick.iterate();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });
}

void StandardTask::stop() {
  m_ioc.stop();
  m_stop = true;

  m_io_runner.join();
  m_looper.join();
}

std::shared_ptr<Task>
Task::create_default(std::shared_ptr<pipeline::Platform> platform) {
  return std::make_shared<StandardTask>("standard", platform);
}

} // namespace indoors::magnetics
