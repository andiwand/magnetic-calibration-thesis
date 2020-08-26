#include <indoors/magnetics/moving_average.h>

namespace indoors::magnetics {

MovingAverage::MovingAverage(const double window, const double interval)
    : MovingAverage("moving average", window, interval) {}

MovingAverage::MovingAverage(std::string annotation, const double window,
                             const double interval)
    : pipeline::StandardNode(std::move(annotation)), m_window{window},
      m_interval{interval} {}

std::pair<pipeline::Input<pipeline::Event<pipeline::Vector3>> *,
          pipeline::Output<pipeline::Event<pipeline::Vector3>> *>
MovingAverage::create_channel(std::string annotation) {
  // TODO remove new
  auto input = new Channel(std::move(annotation), this);
  m_channels.push_back(std::unique_ptr<Channel>(input));
  return {input, &input->m_output};
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
MovingAverage::create_channel(
    pipeline::Output<pipeline::Event<pipeline::Vector3>> *output) {
  auto &&channel = create_channel(output->annotation());
  output->plug(channel.first);
  return channel.second;
}

void MovingAverage::iterate() {
  for (auto &&channel : m_channels) {
    channel->process();
  }
}

MovingAverage::Channel::Channel(std::string annotation, Node *node)
    : pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>(
          std::move(annotation), node),
      m_output{annotation, node} {}

void MovingAverage::Channel::process() noexcept {
  auto parent = reinterpret_cast<MovingAverage *>(node());
  while ((time() > parent->m_window) &&
         (time() - m_output.time() > parent->m_interval)) {
    const double time =
        std::max(parent->m_window, m_output.time() + parent->m_interval);
    const auto series = buffer().front(time);

    pipeline::Event<pipeline::Vector3> result;
    result.time = time;
    for (auto i = series.data_begin; i != series.data_end; ++i) {
      result.data.x += i->data.x;
      result.data.y += i->data.y;
      result.data.z += i->data.z;
    }
    const std::size_t size = series.data_end - series.data_begin;
    result.data.x /= size;
    result.data.y /= size;
    result.data.z /= size;
    m_output.push(result);
    m_output.skip(result.time); // TODO

    buffer().clear_until(time - parent->m_window);
  }
}

} // namespace indoors::magnetics
