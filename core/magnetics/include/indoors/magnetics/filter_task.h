#ifndef INDOORS_MAGNETICS_FILTER_TASK_H
#define INDOORS_MAGNETICS_FILTER_TASK_H

#include <indoors/magnetics/task.h>

namespace indoors::magnetics {

class FilterTask final : public StandardTask {
public:
  explicit FilterTask(std::shared_ptr<pipeline::Platform> platform);
  ~FilterTask() override;

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *sampled_magnetometer();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  sampled_magnetometer_bias();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  sampled_magnetometer_uncalibrated();

  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
  madgwick_orientation();
  pipeline::Output<pipeline::Event<double>> *total_rotation();

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *filter_hard_iron();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *filter_magnetometer();

  void start() override;
  void stop() override;

private:
  class Impl;

  const std::unique_ptr<Impl> m_impl;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_FILTER_TASK_H
