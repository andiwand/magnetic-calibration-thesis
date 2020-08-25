#ifndef INDOORS_PIPELINE_DEFINITIONS_H
#define INDOORS_PIPELINE_DEFINITIONS_H

#include <cstdint>

namespace indoors::pipeline {

class Flushable {
public:
  virtual ~Flushable() = default;

  virtual void flush_until(double time) = 0;
};

class Loopable {
public:
  virtual ~Loopable() = default;

  virtual void iterate() = 0;

  // TODO parallel?

  // TODO signal iteration with conditional variable
};

class Runable {
public:
  virtual ~Runable() = default;

  virtual void run() = 0;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_DEFINITIONS_H
