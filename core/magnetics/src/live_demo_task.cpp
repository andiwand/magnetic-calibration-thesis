#include <indoors/magnetics/live_demo_task.h>
#include <indoors/magnetics/live_demo_filter.h>
#include <iostream>
#include <thread>

namespace indoors::magnetics {

class LiveDemoTask::Impl {
public:
  explicit Impl(std::shared_ptr<pipeline::Platform> platform)
      : m_ioc{1}, m_live_demo_filter({}, platform, m_ioc) {}

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        m_live_demo_filter.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });

    m_io_runner = std::thread([this]() { m_ioc.run(); });
  }

  void stop() {
    m_ioc.stop();

    m_looper.join();
    m_io_runner.join();
  }

private:
  boost::asio::io_context m_ioc;

  LiveDemoFilter m_live_demo_filter;

  std::thread m_io_runner;
  std::thread m_looper;

  std::atomic<bool> m_stop{false};
};

LiveDemoTask::LiveDemoTask(std::shared_ptr<pipeline::Platform> platform)
    : StandardTask("live demo"), m_impl{std::make_unique<Impl>(
                                     std::move(platform))} {}

LiveDemoTask::~LiveDemoTask() = default;

void LiveDemoTask::start() { m_impl->start(); }

void LiveDemoTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
