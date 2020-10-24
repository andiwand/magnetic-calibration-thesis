#include <indoors/magnetics/compass.h>
#include <indoors/magnetics/filter_task.h>
#include <indoors/magnetics/hard_iron.h>
#include <indoors/magnetics/moving_average.h>
#include <indoors/magnetics/orientation_filter.h>
#include <indoors/magnetics/total_rotation.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/synchronizer.h>
#include <iostream>
#include <random>
#include <thread>

namespace indoors::magnetics {

class FilterTask::Impl {
public:
  explicit Impl(std::shared_ptr<pipeline::Platform> platform)
      : m_platform{std::move(platform)}, m_synchronizer{0.1},
        m_moving_average{0.05, 0.05}, m_madgwick{0.05, 0.01},
        m_hard_iron{std::random_device()(), 10000, 0.1},
        m_system_compass{"system compass"}, m_particle_compass{
                                                std::random_device()(), 1000,
                                                1} {
    // synchronizer
    auto accelerometer =
        m_synchronizer.create_channel(m_platform->accelerometer());
    auto gyroscope = m_synchronizer.create_channel(m_platform->gyroscope());
    auto magnetometer =
        m_synchronizer.create_channel(m_platform->magnetometer());
    auto magnetometer_uncalibrated =
        m_synchronizer.create_channel(m_platform->magnetometer_uncalibrated());
    auto magnetometer_bias =
        m_synchronizer.create_channel(m_platform->magnetometer_bias());
    auto orientation = m_synchronizer.create_channel(m_platform->orientation());

    // moving average
    auto sampled_accelerometer = m_moving_average.create_channel(accelerometer);
    auto sampled_gyroscope = m_moving_average.create_channel(gyroscope);
    m_sampled_magnetometer = m_moving_average.create_channel(magnetometer);
    m_sampled_magnetometer_uncalibrated =
        m_moving_average.create_channel(magnetometer_uncalibrated);
    m_sampled_magnetometer_bias =
        m_moving_average.create_channel(magnetometer_bias);

    // orientation
    sampled_accelerometer->plug(m_madgwick.accelerometer());
    sampled_gyroscope->plug(m_madgwick.gyroscope());

    // total rotation
    m_madgwick.orientation()->plug(m_total_rotation.orientation());

    // hard iron
    m_sampled_magnetometer_uncalibrated->plug(
        m_hard_iron.magnetometer_uncalibrated());
    m_madgwick.orientation()->plug(m_hard_iron.orientation());
    m_total_rotation.total_rotation()->plug(m_hard_iron.total_rotation());

    // system compass
    orientation->plug(m_system_compass.orientation());

    // naive compass
    m_hard_iron.magnetometer_calibrated()->plug(
        m_naive_compass.magnetometer_calibrated());
    m_madgwick.orientation()->plug(m_naive_compass.orientation());

    // particle compass
    m_hard_iron.magnetometer_calibrated()->plug(
        m_particle_compass.magnetometer_calibrated());
    m_hard_iron.var_magnetometer_calibrated()->plug(
        m_particle_compass.var_magnetometer_calibrated());
    m_madgwick.orientation()->plug(m_particle_compass.orientation());
    m_total_rotation.total_rotation()->plug(
        m_particle_compass.total_rotation());
  }

  void start() {
    m_looper = std::thread([this]() {
      while (!m_stop) {
        m_synchronizer.iterate();
        m_moving_average.iterate();
        m_madgwick.iterate();
        m_total_rotation.iterate();
        m_hard_iron.iterate();
        m_system_compass.iterate();
        m_naive_compass.iterate();
        m_particle_compass.iterate();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }

  void stop() {
    m_stop = true;

    m_looper.join();
  }

  const std::shared_ptr<pipeline::Platform> m_platform;

  pipeline::Synchronizer m_synchronizer;
  MovingAverage m_moving_average;
  MadgwickImu m_madgwick;
  TotalRotation m_total_rotation;
  HardIron m_hard_iron;
  ExtractionCompass m_system_compass;
  NaiveCompass m_naive_compass;
  ParticleCompass m_particle_compass;

  std::thread m_looper;

  std::atomic<bool> m_stop{false};

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *m_sampled_magnetometer;
  pipeline::Output<pipeline::Event<pipeline::Vector3>>
      *m_sampled_magnetometer_bias;
  pipeline::Output<pipeline::Event<pipeline::Vector3>>
      *m_sampled_magnetometer_uncalibrated;
};

FilterTask::FilterTask(std::shared_ptr<pipeline::Platform> platform)
    : StandardTask("filter"), m_impl{std::make_unique<Impl>(
                                  std::move(platform))} {}

FilterTask::~FilterTask() = default;

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
FilterTask::sampled_magnetometer() {
  return m_impl->m_sampled_magnetometer;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
FilterTask::sampled_magnetometer_bias() {
  return m_impl->m_sampled_magnetometer_bias;
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
FilterTask::sampled_magnetometer_uncalibrated() {
  return m_impl->m_sampled_magnetometer_uncalibrated;
}

pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
FilterTask::madgwick_orientation() {
  return m_impl->m_madgwick.orientation();
}

pipeline::Output<pipeline::Event<double>> *FilterTask::total_rotation() {
  return m_impl->m_total_rotation.total_rotation();
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
FilterTask::filter_hard_iron() {
  return m_impl->m_hard_iron.hard_iron();
}

pipeline::Output<pipeline::Event<pipeline::Vector3>> *
FilterTask::filter_magnetometer() {
  return m_impl->m_hard_iron.magnetometer_calibrated();
}

void FilterTask::start() { m_impl->start(); }

void FilterTask::stop() { m_impl->stop(); }

} // namespace indoors::magnetics
