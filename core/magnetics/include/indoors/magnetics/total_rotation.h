#ifndef INDOORS_MAGNETICS_TOTAL_ROTATION_H
#define INDOORS_MAGNETICS_TOTAL_ROTATION_H

#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>

namespace indoors::magnetics {

class TotalRotation final : public pipeline::StandardNode,
                            public pipeline::Loopable {
public:
  TotalRotation();

  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<double>> *delta_rotation();
  pipeline::Output<pipeline::Event<double>> *total_rotation();

  void iterate() override;

private:
  pipeline::Quaternion m_last_orientation;
  double m_last_total_rotation{0};
  bool m_initialized{false};

  pipeline::BufferedSeriesInput<pipeline::Event<pipeline::Quaternion>> m_orientation;
  pipeline::StandardOutput<pipeline::Event<double>> m_delta_rotation;
  pipeline::StandardOutput<pipeline::Event<double>> m_total_rotation;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_TOTAL_ROTATION_H
