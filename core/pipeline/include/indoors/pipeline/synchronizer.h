#ifndef INDOORS_PIPELINE_WORKER_H
#define INDOORS_PIPELINE_WORKER_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>
#include <string>
#include <vector>

namespace indoors::pipeline {

class Synchronizer final : public Node,
                           public Loopable,
                           public std::enable_shared_from_this<Synchronizer> {
public:
  explicit Synchronizer(double max_slave_delay);
  Synchronizer(std::string annotation, double max_slave_delay);

  const std::string &annotation() const noexcept override {
    return m_annotation;
  }

  template <typename T>
  std::shared_ptr<Output<T>>
  create_master_channel(std::shared_ptr<Output<T>> output) {
    auto channel = create_channel(output->annotation());
    output->plug(channel.first);
    return channel.second;
  }

  template <typename T>
  std::pair<std::shared_ptr<Input<T>>, std::shared_ptr<Output<T>>>
  create_master_channel(std::string annotation) {}

  template <typename T>
  std::shared_ptr<Output<T>>
  create_slave_channel(std::shared_ptr<Output<T>> output) {
    auto channel = create_channel(output->annotation());
    output->plug(channel.first);
    return channel.second;
  }

  template <typename T>
  std::pair<std::shared_ptr<Input<T>>, std::shared_ptr<Output<T>>>
  create_slave_channel(std::string annotation) {}

  void iterate(std::uint32_t index) override;

private:
  const std::string m_annotation;
  const double m_max_slave_delay{0};
  std::vector<std::shared_ptr<int>> m_masters;
  std::vector<std::shared_ptr<int>> m_slaves;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_WORKER_H
