#ifndef MAGNETICS_LIVE_DEMO_TASK_H
#define MAGNETICS_LIVE_DEMO_TASK_H

#include <indoors/magnetics/task.h>

namespace indoors::magnetics {

class LiveDemoTask final : public StandardTask {
public:
  explicit LiveDemoTask(std::shared_ptr<pipeline::Platform> platform);
  ~LiveDemoTask() override;

  void start() override;
  void stop() override;

private:
  class Impl;

  const std::unique_ptr<Impl> m_impl;
};

} // namespace indoors::magnetics

#endif // MAGNETICS_LIVE_DEMO_TASK_H
