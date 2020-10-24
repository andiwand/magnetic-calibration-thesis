#include <indoors/magnetics/recorder_task.h>
#include <indoors/pipeline/protocol.h>
#include <thread>

namespace indoors::magnetics {

class RecorderTask::Impl {
public:
  Impl(std::shared_ptr<pipeline::Platform> platform, std::string path) {
    m_encoder.create_input(platform->tick());

    m_encoder.create_input(platform->clock());

    m_encoder.create_input(platform->accelerometer());
    m_encoder.create_input(platform->gyroscope());
    m_encoder.create_input(platform->magnetometer());

    m_encoder.create_input(platform->magnetometer_uncalibrated());
    m_encoder.create_input(platform->magnetometer_bias());

    m_encoder.create_input(platform->orientation());
  }

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }

  void stop() {
    m_stop = true;

    m_looper.join();
  }

private:
  const std::shared_ptr<pipeline::Platform> m_platform;

  pipeline::ProtocolEncoder m_encoder;

  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

RecorderTask::RecorderTask(std::shared_ptr<pipeline::Platform> platform,
                           std::string path)
    : StandardTask("recorder"), m_impl{std::make_unique<Impl>(
                                    std::move(platform), std::move(path))} {}

RecorderTask::~RecorderTask() = default;

void RecorderTask::start() { m_impl->start(); }

void RecorderTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
