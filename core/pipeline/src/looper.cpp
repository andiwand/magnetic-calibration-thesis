#include <indoors/pipeline/looper.h>

namespace indoors::pipeline {

Looper::Looper(std::shared_ptr<Loopable> loopable)
    : m_loopable{std::move(loopable)} {}

Looper::Looper(std::shared_ptr<Loopable> loopable,
               const std::uint32_t interval_ms)
    : m_loopable{std::move(loopable)}, m_interval{interval_ms} {}

Looper::~Looper() {
  m_done = true;
  m_thread.join();
}

void Looper::run() {
  while (!m_done) {
    m_loopable->iterate(m_iteration++);
    std::this_thread::sleep_for(m_interval);
  }
}

} // namespace indoors::pipeline
