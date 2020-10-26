#ifndef INDOORS_PIPELINE_WORKER_H
#define INDOORS_PIPELINE_WORKER_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>
#include <string>
#include <vector>

namespace indoors::pipeline {

class Synchronizer final : public StandardNode, public Loopable {
public:
  explicit Synchronizer(double max_slave_delay);
  Synchronizer(std::string annotation, double max_slave_delay);

  template <typename T>
  std::pair<Input<T> *, Output<T> *> create_channel(std::string annotation) {
    // TODO remove new
    auto input = new ChannelImpl<T>(std::move(annotation), this);
    m_channels.push_back(std::unique_ptr<Channel>(input));
    return {input, &input->m_output};
  }

  template <typename T> Output<T> *create_channel(Output<T> *output) {
    auto channel = create_channel<T>(output->annotation());
    output->plug(channel.first);
    return channel.second;
  }

  void iterate() override;

private:
  const double m_max_slave_delay{0};
  double m_time{0};

  class Channel : public Flushable, public virtual Interface {
  public:
    ~Channel() override = default;
  };

  template <typename T>
  class ChannelImpl : public Channel, public BufferedSeriesInput<T> {
  public:
    ChannelImpl(std::string annotation, Node *node)
        : BufferedSeriesInput<T>(annotation, node), m_output{annotation, node} {
    }

    void push(T data) override {
      std::lock_guard<std::mutex> lk(
          reinterpret_cast<Synchronizer *>(node())->m_mutex);
      BufferedSeriesInput<T>::push(data);
    }

    void skip(const double time) override {
      std::lock_guard<std::mutex> lk(
          reinterpret_cast<Synchronizer *>(node())->m_mutex);
      BufferedSeriesInput<T>::skip(time);
    }

    void flush_until(const double time) override {
      BufferedSeriesInput<T>::skip(time);
      auto series = BufferedSeriesInput<T>::buffer().pop_until(time);
      for (auto &&event : series) {
        m_output.push(event);
      }
      BufferedSeriesInput<T>::buffer().rotate();

      if (m_output.time() < time) {
        m_output.skip(time);
      }
    }

    StandardOutput<T> m_output;
  };

  std::vector<std::unique_ptr<Channel>> m_channels;
  std::mutex m_mutex;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_WORKER_H
