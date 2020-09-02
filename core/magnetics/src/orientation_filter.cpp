#include <cmath>
#include <indoors/magnetics/orientation_filter.h>

namespace indoors::magnetics {

namespace {
void MadgwickAHRSupdateIMU(float dt, float beta, float gx, float gy, float gz,
                           float ax, float ay, float az, float &q0, float &q1,
                           float &q2, float &q3);
}

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

pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
MadgwickImu::orientation() {
  return &m_orientation;
}

void MadgwickImu::iterate() {
  auto &&acc = m_accelerometer.buffer().vector();
  auto &&gyr = m_gyroscope.buffer().vector();

  assert(acc.size() == gyr.size());

  for (std::size_t i = 0; i < acc.size(); ++i) {
    if (std::isnan(gyr[i].data.x) || std::isnan(gyr[i].data.y) ||
        std::isnan(gyr[i].data.z) || std::isnan(acc[i].data.x) ||
        std::isnan(acc[i].data.y) || std::isnan(acc[i].data.z)) {
      continue;
    }

    if (!m_initialized) {
      // TODO
      m_initialized = true;
    }

    MadgwickAHRSupdateIMU(0.05, 0.1, (float)gyr[i].data.x, (float)gyr[i].data.y,
                          (float)gyr[i].data.z, (float)acc[i].data.x,
                          (float)acc[i].data.y, (float)acc[i].data.z, m_q0,
                          m_q1, m_q2, m_q3);

    m_orientation.push({acc[i].time, m_q0, m_q1, m_q2, m_q3});
  }

  m_accelerometer.buffer().clear();
  m_gyroscope.buffer().clear();
}

namespace {
void MadgwickAHRSupdateIMU(float dt, float beta, float gx, float gy, float gz,
                           float ax, float ay, float az, float &q0, float &q1,
                           float &q2, float &q3) {
  float recipNorm;
  float s0, s1, s2, s3;
  float qDot1, qDot2, qDot3, qDot4;
  float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1, q2q2,
      q3q3;

  // Rate of change of quaternion from gyroscope
  qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
  qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
  qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
  qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

  // Compute feedback only if accelerometer measurement valid (avoids NaN in
  // accelerometer normalisation)
  if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

    // Normalise accelerometer measurement
    recipNorm = std::pow(ax * ax + ay * ay + az * az, -0.5);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0 = 2.0f * q0;
    _2q1 = 2.0f * q1;
    _2q2 = 2.0f * q2;
    _2q3 = 2.0f * q3;
    _4q0 = 4.0f * q0;
    _4q1 = 4.0f * q1;
    _4q2 = 4.0f * q2;
    _8q1 = 8.0f * q1;
    _8q2 = 8.0f * q2;
    q0q0 = q0 * q0;
    q1q1 = q1 * q1;
    q2q2 = q2 * q2;
    q3q3 = q3 * q3;

    // Gradient decent algorithm corrective step
    s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
    s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 +
         _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
    s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 +
         _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
    s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
    recipNorm = std::pow(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3,
                         -0.5); // normalise step magnitude
    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    // Apply feedback step
    qDot1 -= beta * s0;
    qDot2 -= beta * s1;
    qDot3 -= beta * s2;
    qDot4 -= beta * s3;
  }

  // Integrate rate of change of quaternion to yield quaternion
  q0 += qDot1 * dt;
  q1 += qDot2 * dt;
  q2 += qDot3 * dt;
  q3 += qDot4 * dt;

  // Normalise quaternion
  recipNorm = std::pow(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3, -0.5);
  q0 *= recipNorm;
  q1 *= recipNorm;
  q2 *= recipNorm;
  q3 *= recipNorm;
}
} // namespace

} // namespace indoors::magnetics
