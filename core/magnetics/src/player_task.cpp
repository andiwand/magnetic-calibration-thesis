#include <indoors/magnetics/player_task.h>
#include <indoors/pipeline/protocol.h>
#include <indoors/pipeline/file.h>
#include <thread>

namespace indoors::magnetics {

class PlayerTask::Impl {
public:
  explicit Impl(std::string path) : m_file{path} {}

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        // TODO
        // m_file.flush_until()
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

  pipeline::FileReader m_file;
  pipeline::ProtocolEncoder m_encoder;

  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

PlayerTask::PlayerTask(std::string path)
    : StandardTask("player"), m_impl{std::make_unique<Impl>(std::move(path))} {}

PlayerTask::~PlayerTask() = default;

void PlayerTask::start() { m_impl->start(); }

void PlayerTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
