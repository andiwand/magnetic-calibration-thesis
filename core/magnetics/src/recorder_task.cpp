#include <indoors/magnetics/recorder_task.h>
#include <indoors/pipeline/protocol.h>
#include <indoors/pipeline/file.h>
#include <thread>

namespace indoors::magnetics {

class RecorderTask::Impl {
public:
  Impl(std::shared_ptr<pipeline::Platform> platform, std::string path) : m_file{path} {
    m_encoder.create_input(platform->tick());

    m_encoder.create_input(platform->clock());

    m_encoder.create_input(platform->accelerometer());
    m_encoder.create_input(platform->gyroscope());
    m_encoder.create_input(platform->magnetometer());

    m_encoder.create_input(platform->magnetometer_uncalibrated());
    m_encoder.create_input(platform->magnetometer_bias());

    m_encoder.create_input(platform->orientation());

    m_encoder.output()->plug(m_file.input());
  }

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        // TODO
        m_file.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }

  void stop() {
    m_stop = true;

    m_looper.join();

    m_file.flush();
  }

private:
  pipeline::FileWriter m_file;
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
