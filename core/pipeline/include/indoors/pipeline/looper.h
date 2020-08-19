#ifndef INDOORS_PIPELINE_LOOPER_H
#define INDOORS_PIPELINE_LOOPER_H

#include <atomic>
#include <indoors/pipeline/definitions.h>
#include <thread>

namespace indoors::pipeline {

class Looper final {
public:
  explicit Looper(std::shared_ptr<Loopable> loopable);
  Looper(std::shared_ptr<Loopable> loopable, std::uint32_t interval_ms);
  ~Looper();

private:
  const std::shared_ptr<Loopable> m_loopable;
  const std::chrono::duration<std::uint32_t, std::milli> m_interval{100};
  std::thread m_thread{&Looper::run, this};
  std::uint32_t m_iteration{0};
  std::atomic<bool> m_done{false};

  void run();
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_LOOPER_H
