#ifndef INDOORS_MAGNETICS_ORIENTATION_FILTER_H
#define INDOORS_MAGNETICS_ORIENTATION_FILTER_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class MadgwickImu final : public pipeline::StandardNode,
                          public pipeline::Loopable {
public:
  MadgwickImu();

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *accelerometer();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *gyroscope();

  // transforms phone coordinates into world coordinates
  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *orientation();

  void iterate() override;

private:
  bool m_initialized{false};
  pipeline::Quaternion m_last_orientation{1, 0, 0, 0};

  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>> m_accelerometer;
  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>> m_gyroscope;
  pipeline::StandardOutput<pipeline::Event<pipeline::Quaternion>> m_orientation;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_ORIENTATION_FILTER_H
