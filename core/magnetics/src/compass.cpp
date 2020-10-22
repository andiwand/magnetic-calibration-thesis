#include <Eigen/Dense>
#include <indoors/magnetics/compass.h>
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

float quaternion_to_compass(const Eigen::Quaternionf &q) noexcept {
  const auto phone_north = q.inverse() * Eigen::Vector3f(0, 1, 0);
  const auto yaw = M_PI_2 - std::atan2(phone_north.y(), phone_north.x());
  const auto compass = clip_2pi(yaw);
  return compass;
}

pipeline::Heading
magnetic_field_to_compass(const Eigen::Vector3f &magnetic_field,
                          const float var_min, const float var_scale_xy,
                          const float var_scale_z) noexcept {
  const Eigen::Vector3f earth{0, 20, -44}; // middle europe

  pipeline::Heading result;

  const Eigen::Vector2f north{magnetic_field.x(), magnetic_field.y()};
  if (north.norm() > 1e-2) {
    const auto yaw = M_PI_2 - std::atan2(north.y(), north.x());
    result.north = clip_2pi(yaw);
  } else {
    result.north = 0;
  }

  result.var_north =
      var_min + var_scale_xy * (north - earth.head<2>()).squaredNorm() +
      var_scale_z * std::pow(magnetic_field.z() - earth.z(), 2.0f);
  return result;
}
} // namespace

ExtractionCompass::ExtractionCompass(std::string annotation)
    : pipeline::StandardNode(std::move(annotation)),
      m_orientation{"orientation", this}, m_heading{"heading", this} {}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
ExtractionCompass::orientation() {
  return &m_orientation;
}

pipeline::Output<pipeline::Event<pipeline::Heading>> *
ExtractionCompass::heading() {
  return &m_heading;
}

void ExtractionCompass::iterate() {
  auto &&ori = m_orientation.buffer().vector();

  for (auto &&i : ori) {
    const Eigen::Quaternionf orientation{i.data.w, i.data.x, i.data.y,
                                         i.data.z};
    const auto compass = quaternion_to_compass(orientation);
    m_heading.push({i.time, compass, 0.0});
  }

  m_orientation.buffer().clear();
}

NaiveCompass::NaiveCompass()
    : pipeline::StandardNode("naive compass"),
      m_magnetometer_calibrated{"magnetometer calibrated", this},
      m_orientation{"orientation", this}, m_heading{"heading", this} {}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
NaiveCompass::magnetometer_calibrated() {
  return &m_magnetometer_calibrated;
}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
NaiveCompass::orientation() {
  return &m_orientation;
}

pipeline::Output<pipeline::Event<pipeline::Heading>> *NaiveCompass::heading() {
  return &m_heading;
}

void NaiveCompass::iterate() {
  auto &&mag = m_magnetometer_calibrated.buffer().vector();
  auto &&ori = m_orientation.buffer().vector();

  // TODO doesnt work because of nan in magnetic field
  // assert(mag.size() == ori.size());
  if (mag.size() != ori.size()) {
    m_magnetometer_calibrated.buffer().clear();
    m_orientation.buffer().clear();
    return;
  }

  for (std::size_t i = 0; i < mag.size(); ++i) {
    const Eigen::Quaternionf orientation{ori[i].data.w, ori[i].data.x,
                                         ori[i].data.y, ori[i].data.z};
    const Eigen::Vector3f magnetic_field =
        orientation *
        Eigen::Vector3f(mag[i].data.x, mag[i].data.y, mag[i].data.z);
    const auto heading =
        magnetic_field_to_compass(magnetic_field, 0.005f, 0.0005f, 0.0005f);
    const auto total_orientation =
        Eigen::AngleAxisf(heading.north, Eigen::Vector3f::UnitZ()) *
        orientation;
    const auto compass = quaternion_to_compass(total_orientation);

    m_heading.push({mag[i].time, compass, heading.var_north});
  }

  m_magnetometer_calibrated.buffer().clear();
  m_orientation.buffer().clear();
}

class ParticleCompass::Impl final {
public:
  struct Particle final {
    float north{0};
    Eigen::Vector3f external = Eigen::Vector3f::Zero();
    float log_likelihood{0};
    float weight{0};
  };

  struct Estimate final {
    float north{0};
    float var_north{0};
    float confidence{0};
    Eigen::Vector2f north_confidence = Eigen::Vector2f::Zero();
    Eigen::Vector3f external = Eigen::Vector3f::Zero();
  };

  Impl(const std::uint_fast32_t seed, const std::size_t population)
      : m_population{population}, m_random{seed},
        m_particles{new Particle[population]}, m_weight_wheel{
                                                   new float[population]} {}

  ~Impl() { delete[] m_particles; }

  std::size_t population() const { return m_population; }

  Particle *particles() const { return m_particles; }

  void init() {
    std::uniform_real_distribution<float> north_dist(0, 2 * M_PI);

    for (std::size_t i = 0; i < m_population; ++i) {
      m_particles[i].north = north_dist(m_random);
      m_particles[i].log_likelihood = -std::log(m_population);
      m_particles[i].weight = 1.0f / m_population;
    }
  }

  void update(const float delta_time, const Eigen::Quaternionf &orientation,
              const Eigen::Vector3f &magnetic_field,
              const Eigen::Vector3f &var_magnetic_field) {
    // TODO parameter
    const float drift_std = delta_time * 0.01f;

    std::normal_distribution<float> north_drift_dist(0, drift_std);
    std::normal_distribution<float> magnetic_field_x_dist(
        magnetic_field.x(), std::sqrt(var_magnetic_field.x()));
    std::normal_distribution<float> magnetic_field_y_dist(
        magnetic_field.y(), std::sqrt(var_magnetic_field.y()));
    std::normal_distribution<float> magnetic_field_z_dist(
        magnetic_field.z(), std::sqrt(var_magnetic_field.z()));

    for (std::size_t i = 0; i < m_population; ++i) {
      const Eigen::Vector3f mag(magnetic_field_x_dist(m_random),
                                magnetic_field_y_dist(m_random),
                                magnetic_field_z_dist(m_random));

      m_particles[i].north += north_drift_dist(m_random);

      const auto total_orientation =
          Eigen::AngleAxisf(m_particles[i].north, Eigen::Vector3f::UnitZ()) *
          orientation;
      m_particles[i].external = total_orientation * mag - m_earth;

      const auto naive_heading =
          magnetic_field_to_compass(orientation * mag, 0.1f, 0.01f, 0.01f);
      m_particles[i].log_likelihood += log_normal_pdf(
          angle_distance(m_particles[i].north, (float)naive_heading.north),
          0.0f, std::sqrt((float)naive_heading.var_north));
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

  float effective_particles() {
    float result = 0;
    for (std::size_t i = 0; i < m_population; ++i) {
      result +=
          std::pow(m_particles[i].weight / m_weight_wheel[m_population - 1], 2);
    }
    return 1.0f / result;
  }

  void resample() {
    const float weight_sum = m_weight_wheel[m_population - 1];
    auto *new_particles = new Particle[m_population];

    std::uniform_real_distribution<float> distribution(0, weight_sum);

    for (std::size_t i = 0; i < m_population; ++i) {
      const auto d = distribution(m_random);
      const std::size_t j =
          std::upper_bound(m_weight_wheel, m_weight_wheel + m_population, d) -
          m_weight_wheel;
      new_particles[i] = m_particles[j];
      new_particles[i].log_likelihood = -std::log(m_population);
      new_particles[i].weight = 1.0f / m_population;
    }

    delete[] m_particles;
    m_particles = new_particles;
  }

  Estimate estimate() {
    Estimate result;
    result.north = 0;
    result.var_north = 0;

    float weight_sum = 0;
    for (std::size_t i = 0; i < m_population; ++i) {
      weight_sum += m_particles[i].weight;

      result.north_confidence +=
          Eigen::Vector2f(std::cos(M_PI_2 - m_particles[i].north),
                          std::sin(M_PI_2 - m_particles[i].north)) *
          m_particles[i].weight;

      result.external += m_particles[i].external * m_particles[i].weight;
    }

    result.north_confidence /= weight_sum;
    result.external /= weight_sum;

    result.north = M_PI_2 - std::atan2(result.north_confidence.y(),
                                       result.north_confidence.x());
    result.confidence = result.north_confidence.norm();

    for (std::size_t i = 0; i < m_population; ++i) {
      result.var_north +=
          std::pow(angle_distance(result.north, m_particles[i].north), 2.0f) *
          m_particles[i].weight;
    }

    result.var_north /= weight_sum;

    m_last_estimate = result;
    return result;
  }

  const Estimate &last_estimate() const { return m_last_estimate; }

private:
  const Eigen::Vector3f m_earth{0, 20, -44}; // middle europe

  const std::size_t m_population;

  std::default_random_engine m_random;

  Particle *m_particles;
  float *m_weight_wheel;

  Estimate m_last_estimate;
};

ParticleCompass::ParticleCompass(const std::uint_fast32_t seed,
                                 const std::size_t population,
                                 const float min_rotation)
    : pipeline::StandardNode("particle compass"), m_impl{std::make_unique<Impl>(
                                                      seed, population)},
      m_min_rotation{min_rotation},
      m_magnetometer_calibrated{"magnetometer calibrated", this},
      m_var_magnetometer_calibrated{"magnetometer calibrated variance", this},
      m_orientation{"orientation", this}, m_heading{"heading", this},
      m_total_rotation{"total rotation", this},
      m_total_orientation{"total orientation", this},
      m_north_confidence{"north confidence", this}, m_external{"external",
                                                               this} {}

ParticleCompass::~ParticleCompass() = default;

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
ParticleCompass::magnetometer_calibrated() {
  return &m_magnetometer_calibrated;
}

pipeline::Input<pipeline::Event<pipeline::Vector3>> *
ParticleCompass::var_magnetometer_calibrated() {
  return &m_var_magnetometer_calibrated;
}

pipeline::Input<pipeline::Event<pipeline::Quaternion>> *
ParticleCompass::orientation() {
  return &m_orientation;
}

pipeline::Input<pipeline::Event<double>> *ParticleCompass::total_rotation() {
  return &m_total_rotation;
}

pipeline::Output<pipeline::Event<pipeline::Heading>> *
ParticleCompass::heading() {
  return &m_heading;
}

pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
ParticleCompass::total_orientation() {
  return &m_total_orientation;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
ParticleCompass::external() {
  return &m_external;
}

pipeline::Output<pipeline::Event<pipeline::Vector2>> *
ParticleCompass::north_confidence() {
  return &m_north_confidence;
}

void ParticleCompass::iterate() {
  auto &&mag = m_magnetometer_calibrated.buffer().vector();
  auto &&var_mag = m_var_magnetometer_calibrated.buffer().vector();
  auto &&ori = m_orientation.buffer().vector();
  auto &&rot = m_total_rotation.buffer().vector();

  // TODO doesnt work because of nan in magnetic field
  // assert(mag.size() == ori.size());
  if (mag.size() != ori.size()) {
    m_magnetometer_calibrated.buffer().clear();
    m_var_magnetometer_calibrated.buffer().clear();
    m_orientation.buffer().clear();
    m_total_rotation.buffer().clear();
    return;
  }

  for (std::size_t i = 0; i < mag.size(); ++i) {
    if (std::isnan(mag[i].data.x) || std::isnan(mag[i].data.y) ||
        std::isnan(mag[i].data.z)) {
      continue;
    }

    const Eigen::Vector3f magnetic_field{mag[i].data.x, mag[i].data.y,
                                         mag[i].data.z};
    const Eigen::Vector3f var_magnetic_field{
            var_mag[i].data.x, var_mag[i].data.y, var_mag[i].data.z};
    const Eigen::Quaternionf orientation{ori[i].data.w, ori[i].data.x,
                                         ori[i].data.y, ori[i].data.z};

    // TODO we should only update when we moved
    if (!m_initialized || rot[i].data - m_last_total_rotation >= m_min_rotation) {
      if (!m_initialized) {
        m_impl->init();
        m_impl->estimate();
        m_initialized = true;
      } else {
        const double delta_time = ori.data()->time - m_last_update;
        m_impl->update((float)delta_time, orientation, magnetic_field,
                       var_magnetic_field);
        m_impl->weight();
        m_impl->estimate();

        // TODO parameter
        if (m_impl->effective_particles() <= m_impl->population() * 0.05) {
          m_impl->resample();
        }
      }

      m_last_total_rotation = rot[i].data;
    }

    const auto estimate = m_impl->last_estimate();

    const auto total_orientation =
        Eigen::AngleAxisf(estimate.north, Eigen::Vector3f::UnitZ()) *
        orientation;
    const auto compass = quaternion_to_compass(total_orientation);

    m_heading.push({mag[i].time, compass, estimate.var_north});

    m_total_orientation.push({mag[i].time, total_orientation.w(),
                              total_orientation.x(), total_orientation.y(),
                              total_orientation.z()});

    m_external.push({mag[i].time, estimate.external.x(), estimate.external.y(),
                     estimate.external.z()});

    const auto north_confidence =
        Eigen::AngleAxisf(-compass, Eigen::Vector3f::UnitZ()) *
        Eigen::Vector3f(estimate.north_confidence.x(),
                        estimate.north_confidence.y(), 0);
    m_north_confidence.push(
        {mag[i].time, north_confidence.x(), north_confidence.y()});
  }

  m_magnetometer_calibrated.buffer().clear();
  m_var_magnetometer_calibrated.buffer().clear();
  m_orientation.buffer().clear();
  m_total_rotation.buffer().clear();
}

} // namespace indoors::magnetics
