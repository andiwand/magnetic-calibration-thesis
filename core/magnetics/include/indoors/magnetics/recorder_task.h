#ifndef MAGNETICS_RECORDER_TASK_H
#define MAGNETICS_RECORDER_TASK_H

#include <indoors/magnetics/task.h>

namespace indoors::magnetics {

class RecorderTask final : public StandardTask {
public:
  RecorderTask(std::shared_ptr<pipeline::Platform> platform, std::string path);
  ~RecorderTask() override;

  void start() override;
  void stop() override;

private:
  class Impl;

  const std::unique_ptr<Impl> m_impl;
};

} // namespace indoors::magnetics

#endif // MAGNETICS_RECORDER_TASK_H
