#ifndef INDOORS_MAGNETICS_ORIENTATION_FILTER_H
#define INDOORS_MAGNETICS_ORIENTATION_FILTER_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class MadgwickImu final : public pipeline::StandardNode,
                          public pipeline::Loopable {
public:
  MadgwickImu(float delta_time, float beta);

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *accelerometer();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *gyroscope();

  // transforms phone coordinates into world coordinates
  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *orientation();

  void iterate() override;

private:
  const float m_delta_time;
  const float m_beta;
  bool m_initialized{false};
  float m_q0{1};
  float m_q1{0};
  float m_q2{0};
  float m_q3{0};

  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>> m_accelerometer;
  pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>> m_gyroscope;
  pipeline::StandardOutput<pipeline::Event<pipeline::Quaternion>> m_orientation;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_ORIENTATION_FILTER_H
