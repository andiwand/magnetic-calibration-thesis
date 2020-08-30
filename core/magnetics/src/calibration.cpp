#include <Eigen/Dense>
#include <indoors/magnetics/calibration.h>
#include <random>

namespace indoors::magnetics {

namespace {
template<typename Scalar>
Scalar log_normal_pdf(const Scalar x, const Scalar m, const Scalar s) noexcept {
  static const Scalar log_inv_sqrt_2pi = -0.9189385332046726;
  const Scalar a = (x - m) / s;
  return log_inv_sqrt_2pi - std::log(s) - 0.5 * a * a;
}
}

class Calibration::Impl final {
public:
  struct Particle final {
    float north{0};
    Eigen::Vector3f hard_iron;
    Eigen::Vector3f external;
    float log_likelihood{0};
    float weight{0};
  };

  struct Estimate final {
    float north{0};
    float confidence{0};
    Eigen::Vector2f north_confidence;
    Eigen::Vector3f hard_iron;
    Eigen::Vector3f external;
  };

  Impl(const std::uint_fast32_t seed, const std::size_t population,
       const float delta_time)
      : m_population{population}, m_delta_time{delta_time}, m_random{seed},
        m_particles{new Particle[population]}, m_weight_wheel{new float[population]} {}

  ~Impl() { delete[] m_particles; }

  Particle *particles() const { return m_particles; }

  void init(const Eigen::Quaternionf &orientation,
            const Eigen::Vector3f &magnetic_field) {
    std::uniform_real_distribution<float> north_dist(0, 2 * M_PI);
    std::normal_distribution<float> hard_iron_dist(0, 30);
    std::normal_distribution<float> noise_dist(0, 2);

    for (std::size_t i = 0; i < m_population; ++i) {
      m_particles[i].north = north_dist(m_random);
      m_particles[i].hard_iron = {hard_iron_dist(m_random),
                                  hard_iron_dist(m_random),
                                  hard_iron_dist(m_random)};

      const Eigen::Vector3f earth{0, 20, -44}; // middle europe
      const Eigen::Vector3f noise{noise_dist(m_random), noise_dist(m_random),
                                  noise_dist(m_random)};
      const auto full_orientation =
          Eigen::AngleAxisf(m_particles[i].north, Eigen::Vector3f::UnitZ()) *
          orientation;
      m_particles[i].external =
          full_orientation * (magnetic_field - m_particles[i].hard_iron) -
          earth + noise;

      m_particles[i].log_likelihood = -std::log(m_population);
      m_particles[i].weight = 1.0f / m_population;
    }
  }

  void update(const Eigen::Quaternionf &orientation,
              const Eigen::Vector3f &magnetic_field) {
    std::normal_distribution<float> north_drift_dist(0, 0.02);
    std::normal_distribution<float> hard_iron_drift_dist(0, 0.1);
    std::normal_distribution<float> noise_dist(0, 2);

    for (std::size_t i = 0; i < m_population; ++i) {
      m_particles[i].north += north_drift_dist(m_random);
      m_particles[i].hard_iron += Eigen::Vector3f(hard_iron_drift_dist(m_random), hard_iron_drift_dist(m_random), hard_iron_drift_dist(m_random));

      const Eigen::Vector3f earth{0, 20, -44}; // middle europe
      const auto full_orientation =
          Eigen::AngleAxisf(m_particles[i].north, Eigen::Vector3f::UnitZ()) *
          orientation;
      const Eigen::Vector3f prediction =
          m_particles[i].hard_iron + full_orientation.conjugate() * (m_particles[i].external + earth);

      m_particles[i].log_likelihood += log_normal_pdf(prediction.x(), magnetic_field.x(), 10.0f) +
          log_normal_pdf(prediction.y(), magnetic_field.y(), 10.0f) +
          log_normal_pdf(prediction.z(), magnetic_field.z(), 10.0f);

      const Eigen::Vector3f noise{noise_dist(m_random), noise_dist(m_random),
                                  noise_dist(m_random)};
      m_particles[i].external =
          full_orientation * (magnetic_field - m_particles[i].hard_iron) -
          earth + noise;
    }
  }

  void weight() {
    float max_log_likelihood = std::log(0);
    for (std::size_t i = 0; i < m_population; ++i) {
      if (m_particles[i].log_likelihood > max_log_likelihood) {
        max_log_likelihood = m_particles[i].log_likelihood;
      }
    }

    float weight_sum = 0;
    for (std::size_t i = 0; i < m_population; ++i) {
      m_particles[i].weight =
          std::exp(m_particles[i].log_likelihood - max_log_likelihood);
      weight_sum += m_particles[i].weight;
      m_weight_wheel[i] = weight_sum;
    }
  }

  void resample() {
    const float weight_sum = m_weight_wheel[m_population - 1];
    auto *new_particles = new Particle[m_population];

    std::uniform_real_distribution<float> distribution(0, weight_sum);

    for (std::size_t i = 0; i < m_population; ++i) {
      const auto d = distribution(m_random);
      const std::size_t j =
          std::upper_bound(m_weight_wheel, m_weight_wheel + m_population, d) - m_weight_wheel;
      new_particles[i] = m_particles[j];
      new_particles[i].log_likelihood = - std::log(m_population);
      new_particles[i].weight = 1.0f / m_population;
    }

    delete[] m_particles;
    m_particles = new_particles;
  }

  Estimate estimate() {
    Estimate result;
    result.north = 0;
    result.north_confidence.setZero();
    result.hard_iron.setZero();
    result.external.setZero();

    float weight_sum = 0;
    for (std::size_t i = 0; i < m_population; ++i) {
      weight_sum += m_particles[i].weight;

      result.north_confidence += Eigen::Vector2f(std::cos(m_particles[i].north), std::sin(m_particles[i].north)) * m_particles[i].weight;

      result.hard_iron += m_particles[i].hard_iron * m_particles[i].weight;
      result.external += m_particles[i].external * m_particles[i].weight;
    }

    result.north_confidence /= weight_sum;
    result.hard_iron /= weight_sum;
    result.external /= weight_sum;

    result.north = std::atan2(result.north_confidence.y(), result.north_confidence.x());
    result.confidence = result.north_confidence.norm();

    return result;
  }

private:
  const std::size_t m_population;
  const float m_delta_time;

  std::default_random_engine m_random;

  Particle *m_particles;
  float *m_weight_wheel;
};

Calibration::Calibration(const std::uint_fast32_t seed,
                         const std::size_t population, const float delta_time)
    : pipeline::StandardNode("calibration"), m_impl{std::make_unique<Impl>(
                                                 seed, population, delta_time)},
      m_magnetometer{"magnetometer", this}, m_orientation{"orientation", this},
      m_orientation_output{"orientation", this},
      m_hard_iron{"hard iron", this},
      m_external{"external", this},
      m_north_confidence{"north confidence", this} {
}

Calibration::~Calibration() = default;

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
Calibration::magnetometer() {
  return &m_magnetometer;
}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
Calibration::orientation() {
  return &m_orientation;
}

pipeline::Output<pipeline::Event<pipeline::Quaternion>> * Calibration::orientation_output() {
  return &m_orientation_output;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> * Calibration::hard_iron() {
  return &m_hard_iron;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> * Calibration::external() {
  return &m_external;
}

pipeline::Output<pipeline::Event<pipeline::Vector2>> * Calibration::north_confidence() {
  return &m_north_confidence;
}

void Calibration::iterate() {
  auto &&mag = m_magnetometer.buffer().vector();
  auto &&ori = m_orientation.buffer().vector();

  //assert(mag.size() == ori.size()); // TODO doesnt work because of nan in magnetic field
  if (mag.size() != ori.size()) {
      m_magnetometer.buffer().clear();
      m_orientation.buffer().clear();
      return;
  }

  for (std::size_t i = 0; i < mag.size(); ++i) {
    if (std::isnan(mag[i].data.x) || std::isnan(mag[i].data.y) ||
      std::isnan(mag[i].data.z)) {
      continue;
    }

    const Eigen::Vector3f magnetic_field{mag[i].data.x, mag[i].data.y, mag[i].data.z};
    const Eigen::Quaternionf orientation{ori[i].data.w, ori[i].data.x, ori[i].data.y, ori[i].data.z};

    if (!m_initialized) {
      m_impl->init(orientation, magnetic_field);
      m_initialized = true;
      continue;
    }

    // TODO performance
    if (m_iteration++ % 2 == 0) {
      m_impl->update(orientation, magnetic_field);
      m_impl->weight();
      const auto estimate = m_impl->estimate();
      m_impl->resample();

      const auto result = Eigen::AngleAxisf(estimate.north, Eigen::Vector3f::UnitZ()) * orientation;
      m_orientation_output.push({mag[i].time, result.w(), result.x(), result.y(), result.z()});

      m_hard_iron.push({mag[i].time, estimate.hard_iron.x(), estimate.hard_iron.y(), estimate.hard_iron.z()});
      m_external.push({mag[i].time, estimate.external.x(), estimate.external.y(), estimate.external.z()});

      const auto yaw = orientation.conjugate().toRotationMatrix().eulerAngles(0, 1, 2).z();
      const auto north_confidence = Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitZ()) * Eigen::Vector3f(estimate.north_confidence.x(), estimate.north_confidence.y(), 0);
      m_north_confidence.push({mag[i].time, north_confidence.x(), north_confidence.y()});
    }
  }

  m_magnetometer.buffer().clear();
  m_orientation.buffer().clear();
}

} // namespace indoors::magnetics
