#ifndef INDOORS_MAGNETICS_MAGNETICS_FILTER_H
#define INDOORS_MAGNETICS_MAGNETICS_FILTER_H

#include <indoors/magnetics/compass.h>
#include <indoors/magnetics/hard_iron.h>
#include <indoors/magnetics/moving_average.h>
#include <indoors/magnetics/orientation_filter.h>
#include <indoors/magnetics/total_rotation.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/synchronizer.h>
#include <iostream>
#include <random>

namespace indoors::magnetics {

class MagneticsFilter final : public pipeline::StandardNode {
public:
  struct Config {
    double max_slave_delay{0.1};
    double delta_time{0.05};
    double madgwick_beta{0.01};
    HardIron::Config hard_iron;
    ParticleCompass::Config compass;
  };

  explicit MagneticsFilter(const Config &config);

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *accelerometer();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *gyroscope();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *magnetometer();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *
  magnetometer_uncalibrated();
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *magnetometer_bias();
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *orientation();

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *sampled_magnetometer();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  sampled_magnetometer_bias();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *
  sampled_magnetometer_uncalibrated();

  pipeline::Output<pipeline::Event<pipeline::Quaternion>> *
  madgwick_orientation();
  pipeline::Output<pipeline::Event<double>> *total_rotation();

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *filter_hard_iron();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *filter_magnetometer();
  pipeline::Output<pipeline::Event<pipeline::Vector3>> *filter_var();

  void flush() override;

private:
  pipeline::Synchronizer m_synchronizer;
  MovingAverage m_moving_average;
  MadgwickImu m_madgwick;
  TotalRotation m_total_rotation;
  HardIron m_hard_iron;
  ExtractionCompass m_system_compass;
  NaiveCompass m_naive_compass;
  ParticleCompass m_particle_compass;

  pipeline::Input<pipeline::Event<pipeline::Vector3>> *m_accelerometer;
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *m_gyroscope;
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *m_magnetometer;
  pipeline::Input<pipeline::Event<pipeline::Vector3>>
      *m_magnetometer_uncalibrated;
  pipeline::Input<pipeline::Event<pipeline::Vector3>> *m_magnetometer_bias;
  pipeline::Input<pipeline::Event<pipeline::Quaternion>> *m_orientation;

  pipeline::Output<pipeline::Event<pipeline::Vector3>> *m_sampled_magnetometer;
  pipeline::Output<pipeline::Event<pipeline::Vector3>>
      *m_sampled_magnetometer_bias;
  pipeline::Output<pipeline::Event<pipeline::Vector3>>
      *m_sampled_magnetometer_uncalibrated;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_MAGNETICS_FILTER_H
