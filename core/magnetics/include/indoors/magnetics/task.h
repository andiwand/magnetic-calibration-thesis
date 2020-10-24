#ifndef INDOORS_MAGNETICS_TASK_H
#define INDOORS_MAGNETICS_TASK_H

#include <indoors/pipeline/platform.h>

namespace indoors::magnetics {

class Task {
public:
  virtual ~Task() = default;

  virtual const std::string &annotation() = 0;

  virtual void start() = 0;
  virtual void stop() = 0;
};

class StandardTask : public Task {
public:
  explicit StandardTask(std::string annotation);

  const std::string &annotation() override;

  void start() override;
  void stop() override;

private:
  const std::string m_annotation;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_TASK_H
