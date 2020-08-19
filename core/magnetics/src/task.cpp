#include <indoors/magnetics/task.h>
#include <thread>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/http.h>
#include <indoors/pipeline/protocol.h>
#include <iostream>

namespace indoors::magnetics {

namespace {
class StandardTask : public Task {
public:
  StandardTask(std::string annotation, std::shared_ptr<pipeline::Platform> platform);

  const std::string & annotation() override;

  void start() override;

  void restart() override {}

  void resume() override {}

  void pause() override {}

  void stop() override;

private:
  const std::string m_annotation;
  const std::shared_ptr<pipeline::Platform> m_platform;

  pipeline::ProtocolEncoder m_encoder;
  boost::asio::io_context m_ioc;
  pipeline::WebSocket m_websocket;

  std::thread m_io_runner;
};

StandardTask::StandardTask(std::string annotation, std::shared_ptr<pipeline::Platform> platform) :
  m_annotation{std::move(annotation)}, m_platform{std::move(platform)},
  m_ioc{1}, m_websocket(m_ioc, "0.0.0.0", 8080, m_encoder.output()) {
  m_platform->magnetometer()->plug(m_encoder.create_input<pipeline::Event<pipeline::Vector3>>("magnetometer"));
  m_platform->magnetometer_uncalibrated()->plug(m_encoder.create_input<pipeline::Event<pipeline::Vector3>>("magnetometer uncalibrated"));
}

const std::string & StandardTask::annotation() { return m_annotation; }
}

void StandardTask::start() {
  m_io_runner = std::thread([this]() {
      m_ioc.run();
    });
}

void StandardTask::stop() {
  m_ioc.stop();
  m_io_runner.join();
}

std::shared_ptr<Task> Task::create_default(std::shared_ptr<pipeline::Platform> platform) {
  return std::make_shared<StandardTask>("standard", platform);
}

} // namespace indoors::magnetics
