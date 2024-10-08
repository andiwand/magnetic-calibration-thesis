#include <Eigen/Dense>
#include <indoors/magnetics/hard_iron.h>
#include <random>

namespace indoors::magnetics {

namespace {
template <typename Scalar> Scalar clip_2pi(const Scalar x) noexcept {
  return x - 2 * M_PI * std::floor(x / (2 * M_PI));
}

template <typename Scalar>
Scalar angle_distance(const Scalar x, const Scalar y) noexcept {
  Scalar result = x - y;
  result = result + M_PI;
  result = result - std::floor(result * M_1_PI * 0.5) * M_PI * 2;
  result = result - M_PI;
  return result;
}

template <typename Scalar>
Scalar log_normal_pdf(const Scalar x, const Scalar m, const Scalar s) noexcept {
  static const Scalar log_inv_sqrt_2pi = -0.9189385332046726;
  const Scalar a = (x - m) / s;
  return log_inv_sqrt_2pi - std::log(s) - 0.5 * a * a;
}
} // namespace

class HardIron::Impl final {
public:
  struct Particle final {
    Eigen::Vector3f hard_iron;
    Eigen::Vector3f external;
    float log_likelihood{0};
    float weight{0};
  };

  struct Estimate final {
    Eigen::Vector3f hard_iron = Eigen::Vector3f::Zero();
    Eigen::Vector3f external = Eigen::Vector3f::Zero();
    Eigen::Vector3f var_hard_iron = Eigen::Vector3f::Zero();
  };

  Impl(const std::uint_fast32_t seed, const Config &config)
      : m_config{config}, m_random{seed},
        m_particles{new Particle[config.population]},
        m_weight_wheel{new float[config.population]} {}

  ~Impl() { delete[] m_particles; }

  std::size_t population() const { return m_config.population; }

  Particle *particles() const { return m_particles; }

  void init(const Eigen::Quaternionf &orientation,
            const Eigen::Vector3f &magnetic_field) {
    std::normal_distribution<float> hard_iron_dist(0, m_config.init_std);

    for (std::size_t i = 0; i < m_config.population; ++i) {
      m_particles[i].hard_iron = {hard_iron_dist(m_random),
                                  hard_iron_dist(m_random),
                                  hard_iron_dist(m_random)};

      m_particles[i].external =
          orientation * (magnetic_field - m_particles[i].hard_iron);

      m_particles[i].log_likelihood = -std::log(m_config.population);
      m_particles[i].weight = 1.0f / m_config.population;
    }
  }

  void update(const float delta_time, const Eigen::Quaternionf &orientation,
              const Eigen::Vector3f &magnetic_field) {
    const float drift_std = delta_time * m_config.drift_rate;
    std::normal_distribution<float> drift_dist(0, drift_std);

    for (std::size_t i = 0; i < m_config.population; ++i) {
      const Eigen::Vector3f drift(drift_dist(m_random), drift_dist(m_random),
                                  drift_dist(m_random));

      m_particles[i].hard_iron += drift;

      const Eigen::Vector3f prediction =
          m_particles[i].hard_iron +
          orientation.conjugate() * m_particles[i].external;

      m_particles[i].log_likelihood +=
          log_normal_pdf(prediction.x(), magnetic_field.x(), (float) m_config.prediction_std) +
          log_normal_pdf(prediction.y(), magnetic_field.y(), (float) m_config.prediction_std) +
          log_normal_pdf(prediction.z(), magnetic_field.z(), (float) m_config.prediction_std);

      m_particles[i].external =
          orientation * (magnetic_field - m_particles[i].hard_iron);
    }
  }

  void weight() {
    float max_log_likelihood = std::log(0);
    for (std::size_t i = 0; i < m_config.population; ++i) {
      if (m_particles[i].log_likelihood > max_log_likelihood) {
        max_log_likelihood = m_particles[i].log_likelihood;
      }
    }

    float weight_sum = 0;
    for (std::size_t i = 0; i < m_config.population; ++i) {
      m_particles[i].weight =
          std::exp(m_particles[i].log_likelihood - max_log_likelihood);
      weight_sum += m_particles[i].weight;
      m_weight_wheel[i] = weight_sum;
    }
  }

  float effective_particles() {
    float result = 0;
    for (std::size_t i = 0; i < m_config.population; ++i) {
      result += std::pow(
          m_particles[i].weight / m_weight_wheel[m_config.population - 1], 2);
    }
    return 1.0f / result;
  }

  void resample() {
    const float weight_sum = m_weight_wheel[m_config.population - 1];
    auto *new_particles = new Particle[m_config.population];

    std::uniform_real_distribution<float> distribution(0, weight_sum);

    for (std::size_t i = 0; i < m_config.population; ++i) {
      const auto d = distribution(m_random);
      const std::size_t j =
          std::upper_bound(m_weight_wheel, m_weight_wheel + m_config.population,
                           d) -
          m_weight_wheel;
      new_particles[i] = m_particles[j];
      new_particles[i].log_likelihood = -std::log(m_config.population);
      new_particles[i].weight = 1.0f / m_config.population;
    }

    delete[] m_particles;
    m_particles = new_particles;
  }

  Estimate estimate() {
    Estimate result;

    float weight_sum = 0;
    for (std::size_t i = 0; i < m_config.population; ++i) {
      weight_sum += m_particles[i].weight;

      result.hard_iron += m_particles[i].hard_iron * m_particles[i].weight;
      result.external += m_particles[i].external * m_particles[i].weight;
    }

    result.hard_iron /= weight_sum;
    result.external /= weight_sum;

    for (std::size_t i = 0; i < m_config.population; ++i) {
      result.var_hard_iron +=
          (result.hard_iron - m_particles[i].hard_iron).cwiseAbs2() *
          m_particles[i].weight;
    }

    result.var_hard_iron /= weight_sum;

    m_last_estimate = result;
    return result;
  }

  const Estimate &last_estimate() const { return m_last_estimate; }

  const Config m_config;

  std::default_random_engine m_random;

  Particle *m_particles;
  float *m_weight_wheel;

  Estimate m_last_estimate;
};

HardIron::HardIron(const std::uint_fast32_t seed, const Config &config)
    : pipeline::StandardNode("HardIron"), m_impl{std::make_unique<Impl>(
                                              seed, config)},
      m_magnetometer_uncalibrated{"magnetometer uncalibrated", this},
      m_orientation{"orientation", this}, m_total_rotation{"total rotation",
                                                           this},
      m_system_calibration{"system calibration", this},
      m_magnetometer_calibrated{"magnetometer calibrated", this},
      m_var_magnetometer_calibrated{"magnetometer calibrated variance", this},
      m_hard_iron{"hard iron", this} {}

HardIron::~HardIron() = default;

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
HardIron::magnetometer_uncalibrated() {
  return &m_magnetometer_uncalibrated;
}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
HardIron::orientation() {
  return &m_orientation;
}

pipeline::Input<pipeline::Event<double>> *HardIron::total_rotation() {
  return &m_total_rotation;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
HardIron::system_calibration() {
  return &m_system_calibration;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
HardIron::magnetometer_calibrated() {
  return &m_magnetometer_calibrated;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
HardIron::var_magnetometer_calibrated() {
  return &m_var_magnetometer_calibrated;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *HardIron::hard_iron() {
  return &m_hard_iron;
}

void HardIron::flush() {
  auto &&mag = m_magnetometer_uncalibrated.buffer().vector();
  auto &&ori = m_orientation.buffer().vector();
  auto &&rot = m_total_rotation.buffer().vector();
  auto &&sys = m_system_calibration.buffer().vector();
  // TODO deduplicate system calibration
  // TODO use system calibration

  // TODO assert doesnt work because of nan in magnetic field
  // assert(mag.size() == ori.size());
  if ((mag.size() != ori.size()) || (mag.size() != rot.size())) {
    m_magnetometer_uncalibrated.buffer().clear();
    m_orientation.buffer().clear();
    m_total_rotation.buffer().clear();
    m_system_calibration.buffer().clear();
    return;
  }

  for (std::size_t i = 0; i < mag.size(); ++i) {
    if (std::isnan(mag[i].data.x) || std::isnan(mag[i].data.y) ||
        std::isnan(mag[i].data.z)) {
      continue;
    }

    const Eigen::Vector3f magnetic_field{mag[i].data.x, mag[i].data.y,
                                         mag[i].data.z};
    const Eigen::Quaternionf orientation{ori[i].data.w, ori[i].data.x,
                                         ori[i].data.y, ori[i].data.z};

    if (!m_initialized ||
        rot[i].data - m_last_total_rotation >= m_impl->m_config.min_rotation) {
      if (!m_initialized) {
        m_impl->init(orientation, magnetic_field);
        m_impl->estimate();
        m_initialized = true;
      } else {
        const double delta_time = ori[i].time - m_last_update;
        m_impl->update((float)delta_time, orientation, magnetic_field);
        m_impl->weight();
        m_impl->estimate();

        if (m_impl->effective_particles() <= m_impl->population() * m_impl->m_config.resampling_rate) {
          m_impl->resample();
        }
      }

      m_last_total_rotation = rot[i].data;
      m_last_update = ori[i].time;
    }

    const auto estimate = m_impl->last_estimate();

    const Eigen::Vector3f magnetometer_calibrated =
        magnetic_field - estimate.hard_iron;
    const Eigen::Vector3f var_magnetometer_calibrated = estimate.var_hard_iron;

    m_magnetometer_calibrated.push({mag[i].time, magnetometer_calibrated.x(),
                                    magnetometer_calibrated.y(),
                                    magnetometer_calibrated.z()});

    m_var_magnetometer_calibrated.push(
        {mag[i].time, var_magnetometer_calibrated.x(),
         var_magnetometer_calibrated.y(), var_magnetometer_calibrated.z()});

    m_hard_iron.push({mag[i].time, estimate.hard_iron.x(),
                      estimate.hard_iron.y(), estimate.hard_iron.z()});
  }

  m_magnetometer_uncalibrated.buffer().clear();
  m_orientation.buffer().clear();
  m_total_rotation.buffer().clear();
  m_system_calibration.buffer().clear();
}

} // namespace indoors::magnetics
