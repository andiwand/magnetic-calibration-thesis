#include <indoors/magnetics/player_task.h>
#include <indoors/pipeline/protocol.h>
#include <indoors/pipeline/file.h>
#include <thread>

namespace indoors::magnetics {

namespace {
class PlayerPlatform : public pipeline::ComposedPlatform {
public:
  explicit PlayerPlatform(pipeline::ProtocolDecoder *m_decoder) : pipeline::ComposedPlatform("player") {
    m_tick = m_decoder->create_output<pipeline::Event<pipeline::Void>>("tick");
    m_clock = m_decoder->create_output<pipeline::Event<pipeline::Clock>>("clock");
    m_accelerometer = m_decoder->create_output<pipeline::Event<pipeline::Vector3>>("accelerometer");
    m_gyroscope = m_decoder->create_output<pipeline::Event<pipeline::Vector3>>("gyroscope");
    m_magnetometer = m_decoder->create_output<pipeline::Event<pipeline::Vector3>>("magnetometer");
    m_magnetometer_uncalibrated = m_decoder->create_output<pipeline::Event<pipeline::Vector3>>("magnetometer_uncalibrated");
    m_magnetometer_bias = m_decoder->create_output<pipeline::Event<pipeline::Vector3>>("magnetometer_bias");
    m_orientation = m_decoder->create_output<pipeline::Event<pipeline::Quaternion>>("orientation");
  }
};
}

class PlayerTask::Impl {
public:
  explicit Impl(std::string path) : m_file{path}, m_platform{std::make_shared<PlayerPlatform>(&m_decoder)} {
    m_file.output()->plug(m_decoder.input());
  }

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        m_file.flush(); // TODO that will flush the whole file
        m_decoder.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }

  void stop() {
    m_stop = true;

    m_looper.join();
  }

  pipeline::FileReader m_file;
  pipeline::ProtocolDecoder m_decoder;
  const std::shared_ptr<PlayerPlatform> m_platform;

  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

PlayerTask::PlayerTask(std::string path)
    : StandardTask("player"), m_impl{std::make_unique<Impl>(std::move(path))} {}

PlayerTask::~PlayerTask() = default;

std::shared_ptr<pipeline::Platform> PlayerTask::platform() { return m_impl->m_platform; }

void PlayerTask::start() { m_impl->start(); }

void PlayerTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
