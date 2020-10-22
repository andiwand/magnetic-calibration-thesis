#include <Eigen/Dense>
#include <indoors/magnetics/total_rotation.h>

namespace indoors::magnetics {

TotalRotation::TotalRotation()
    : pipeline::StandardNode("total rotation"), m_orientation{"orientation",
                                                              this},
      m_delta_rotation{"delta rotation", this}, m_total_rotation{
                                                    "total rotation", this} {}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
TotalRotation::orientation() {
  return &m_orientation;
}

pipeline::Output<pipeline::Event<double>> *TotalRotation::delta_rotation() {
  return &m_delta_rotation;
}

pipeline::Output<pipeline::Event<double>> *TotalRotation::total_rotation() {
  return &m_total_rotation;
}

void TotalRotation::iterate() {
  auto &&ori = m_orientation.buffer().vector();

  for (auto &&o : ori) {
    if (!m_initialized) {
      m_last_orientation = o.data;
      m_initialized = true;
      continue;
    }

    auto dq = Eigen::Quaternionf(o.data.w, o.data.x, o.data.y, o.data.z) *
              Eigen::Quaternionf(m_last_orientation.w, m_last_orientation.x,
                                 m_last_orientation.y, m_last_orientation.z)
                  .conjugate();
    dq.w() = std::min(1.0f, std::max(-1.0f, dq.w()));
    const auto dw = 2 * std::acos(dq.w());
    m_last_total_rotation += dw;

    m_delta_rotation.push({o.time, dw});
    m_total_rotation.push({o.time, m_last_total_rotation});

    m_last_orientation = o.data;
  }

  m_orientation.buffer().clear();
}

} // namespace indoors::magnetics
