#ifndef INDOORS_PIPELINE_DATA_FRAME_H
#define INDOORS_PIPELINE_DATA_FRAME_H

#include <vector>

namespace indoors::pipeline {

template <typename T>
class DataFrame {
public:
  virtual ~DataFrame() = default;

  virtual std::size_t size() const = 0;

  // TODO iterator, begin, end

  virtual const T &operator [](std::size_t index) const = 0;

  virtual const T &get(std::size_t index) const = 0;
  virtual void set(std::size_t index, const T &data) = 0;

  // TODO slice member from T
};

}

#endif // INDOORS_PIPELINE_DATA_FRAME_H
