#include <MadgwickAHRS/MadgwickAHRS.h>
#include <indoors/magnetics/orientation_filter.h>

namespace indoors::magnetics {

MadgwickImu::MadgwickImu()
    : pipeline::StandardNode("madgwick"), m_accelerometer{"accelerometer",
                                                          this},
      m_gyroscope{"gyroscope", this}, m_orientation{"orientation", this} {}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
MadgwickImu::accelerometer() {
  return &m_accelerometer;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *MadgwickImu::gyroscope() {
  return &m_gyroscope;
}

pipeline::Output<pipeline::Event<pipeline::Vector4>> *
MadgwickImu::orientation() {
  return &m_orientation;
}

void MadgwickImu::iterate() {
  if (!m_initialized) {
    // TODO
    m_initialized = true;
    return;
  }

  auto &&acc = m_accelerometer.buffer().vector();
  auto &&gyr = m_gyroscope.buffer().vector();

  assert(acc.size() == gyr.size());

  q0 = (float)m_last_orientation.x;
  q1 = (float)m_last_orientation.y;
  q2 = (float)m_last_orientation.z;
  q3 = (float)m_last_orientation.w;

  for (std::size_t i = 0; i < acc.size(); ++i) {
    MadgwickAHRSupdateIMU((float)gyr[i].data.x, (float)gyr[i].data.y,
                          (float)gyr[i].data.z, (float)acc[i].data.x,
                          (float)acc[i].data.y, (float)acc[i].data.z);
    m_last_orientation.x = q0;
    m_last_orientation.y = q1;
    m_last_orientation.z = q2;
    m_last_orientation.w = q3;

    m_orientation.push({acc[i].time, m_last_orientation});
  }

  m_accelerometer.buffer().clear();
  m_gyroscope.buffer().clear();
}

} // namespace indoors::magnetics
