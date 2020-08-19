#ifndef INDOORS_PIPELINE_WORKER_H
#define INDOORS_PIPELINE_WORKER_H

#include <indoors/pipeline/definitions.h>
#include <thread>

namespace indoors::pipeline {

class Worker final {
public:
  explicit Worker(std::shared_ptr<Runable> runable);
  ~Worker();

private:
  const std::shared_ptr<Runable> m_runable;
  std::thread m_thread;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_WORKER_H
