#ifndef INDOORS_MAGNETICS_TASK_H
#define INDOORS_MAGNETICS_TASK_H

#include <cstdint>
#include <indoors/pipeline/platform.h>

namespace indoors::magnetics {

class Task {
public:
  static std::shared_ptr<Task>
  create_default(std::shared_ptr<pipeline::Platform> platform);

  virtual ~Task() = default;

  virtual const std::string &annotation() = 0;

  virtual void start() = 0;
  virtual void restart() = 0;
  virtual void resume() = 0;
  virtual void pause() = 0;
  virtual void stop() = 0;
};

class StandardTask : public Task {
public:
  explicit StandardTask(std::string annotation);

  const std::string &annotation() override;

  void start() override;
  void restart() override;
  void resume() override;
  void pause() override;
  void stop() override;

private:
  const std::string m_annotation;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_TASK_H
