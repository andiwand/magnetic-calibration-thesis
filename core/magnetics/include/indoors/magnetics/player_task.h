#ifndef MAGNETICS_PLAYER_TASK_H
#define MAGNETICS_PLAYER_TASK_H

#include <indoors/magnetics/task.h>

namespace indoors::magnetics {

class PlayerTask final : public StandardTask {
public:
  explicit PlayerTask(std::string path);
  ~PlayerTask() override;

  void start() override;
  void stop() override;

private:
  class Impl;

  const std::unique_ptr<Impl> m_impl;
};

} // namespace indoors::magnetics

#endif // MAGNETICS_PLAYER_TASK_H
