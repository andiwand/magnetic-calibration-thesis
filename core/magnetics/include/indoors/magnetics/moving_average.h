#ifndef INDOORS_MAGNETICS_MOVING_AVERAGE_H
#define INDOORS_MAGNETICS_MOVING_AVERAGE_H

#include <indoors/pipeline/node.h>
#include <vector>

namespace indoors::magnetics {

// TODO times output?
class MovingAverage final : public pipeline::StandardNode {
public:
  MovingAverage(double window, double interval);
  MovingAverage(std::string annotation, double window, double interval);

  std::pair<pipeline::Input<pipeline::Event<pipeline::Vector3>> *,
            pipeline::Output<pipeline::Event<pipeline::Vector3>> *>
  create_channel(std::string annotation);
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  create_channel(pipeline::Output<pipeline::Event<pipeline::Vector3>> *output);

  void flush() override;

private:
  const double m_window;
  const double m_interval;

  class Channel
      : public pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Vector3>> {
  public:
    Channel(std::string annotation, Node *node);

    void process() noexcept;

    pipeline::StandardOutput<pipeline::Event<pipeline::Vector3>> m_output;
  };

  std::vector<std::unique_ptr<Channel>> m_channels;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_MOVING_AVERAGE_H
