#include <indoors/pipeline/synchronizer.h>

namespace indoors::pipeline {

Synchronizer::Synchronizer(const double max_slave_delay)
    : Synchronizer("synchronizer", max_slave_delay) {}

Synchronizer::Synchronizer(std::string annotation, const double max_slave_delay)
    : StandardNode(std::move(annotation)), m_max_slave_delay{max_slave_delay} {}

void Synchronizer::iterate() {
  std::lock_guard<std::mutex> lk(m_mutex);

  if (m_channels.empty())
    return;

  double min_time = m_channels[0]->time();
  double max_time = m_channels[0]->time();
  for (auto &&channel : m_channels) {
    if (channel->time() < min_time)
      min_time = channel->time();
    if (channel->time() > max_time)
      max_time = channel->time();
  }

  const double flush_time = std::max(min_time, max_time - m_max_slave_delay);
  if (flush_time <= m_time)
    return;

  for (auto &&channel : m_channels) {
    channel->flush_until(flush_time);
  }

  m_time = flush_time;
}

} // namespace indoors::pipeline
