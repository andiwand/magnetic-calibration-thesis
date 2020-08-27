#ifndef INDOORS_PIPELINE_BUFFER_H
#define INDOORS_PIPELINE_BUFFER_H

#include <algorithm>
#include <indoors/pipeline/event.h>
#include <vector>

namespace indoors::pipeline {

// TODO performance: store timestamps in its own vector
// TODO memory: auto rotate
template <typename T> class Buffer {
public:
  Buffer() = default;
  explicit Buffer(const std::size_t initial_capacity)
      : m_initial_capacity{initial_capacity}, m_buffer{create_buffer()} {}
  explicit Buffer(const double time)
      : m_buffer{create_buffer()}, m_time_begin{time}, m_time_end{time} {}
  Buffer(const std::size_t initial_capacity, const double time)
      : m_initial_capacity{initial_capacity}, m_buffer{create_buffer()},
        m_time_begin{time}, m_time_end{time} {}

  void reset(const double time) {
    clear();
    m_time_begin = m_time_end = time;
  }

  std::vector<T> &vector() { return m_buffer; }

  std::size_t size() const { return m_buffer.size - m_buffer_begin; }

  double time_begin() const { return m_time_begin; }

  double time_end() const { return m_time_end; }

  void push(T data) {
    m_buffer.push_back(std::move(data));
    skip(data.time);
  }

  void skip(const double time) { m_time_end = time; }

  T pop() {
    if (m_buffer.empty())
      throw; // TODO
    T result = m_buffer.front();
    ++m_buffer_begin;
    m_time_begin = result.time;
    return result;
  }

  Series<T> pop_until(const double time) const {
    auto result = front(time);
    const auto buffer_end =
        std::begin(m_buffer) + result.data_end - m_buffer.data();
    m_buffer_begin = buffer_end - std::begin(m_buffer);
    return result;
  }

  Series<T> front(const double time_end) const {
    const auto buffer_being =
        m_buffer.data() + m_buffer_begin;
    const auto buffer_end =
        m_buffer.data() + (find_next(time_end) - std::begin(m_buffer));
    return {m_time_begin, time_end, buffer_being, buffer_end};
  }

  void clear() {
    m_buffer.clear();
    m_buffer_begin = 0;
  }

  void clear_until(const double time) {
    const auto find = find_next(time);
    m_buffer.erase(std::begin(m_buffer), find);
    m_buffer_begin = 0;
    m_time_begin = time;
  }

  void clear_front(const Series<T> &series) {
    m_buffer.erase(std::begin(m_buffer),
                   (std::begin(m_buffer) + m_buffer_begin) + (series.data_end - m_buffer.data()));
    m_buffer_begin = 0;
    m_time_begin = series.time_end;
  }

  std::vector<T> swap() {
    rotate();
    std::vector<T> result = create_buffer();
    std::swap(m_buffer, result);
    m_time_begin = m_time_end;
    return std::move(result);
  }

  void rotate() {
    m_buffer.erase(std::begin(m_buffer), std::begin(m_buffer) + m_buffer_begin);
    m_buffer_begin = 0;
  }

private:
  const std::size_t m_initial_capacity{0};
  std::vector<T> m_buffer;
  std::size_t m_buffer_begin{0};
  double m_time_begin{0};
  double m_time_end{0};

  std::vector<T> create_buffer() const {
    if (m_initial_capacity > 0)
      return std::vector<T>(m_initial_capacity);
    return std::vector<T>();
  }

  auto find_next(const double time) const {
    if ((time < m_time_begin) || (time > m_time_end))
      throw; // TODO
    return std::find_if(std::begin(m_buffer) + m_buffer_begin, std::end(m_buffer),
                        [time](const auto &i) { return i.time >= time; });
  }
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_BUFFER_H
