#include <indoors/pipeline/file.h>
#include <indoors/pipeline/protocol.h>
#include <indoors/magnetics/magnetics_filter.h>
#include <indoors/magnetics/live_demo_task.h>
#include <iostream>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

namespace {
class PlatformDecoder final : public StandardNode {
public:
  PlatformDecoder() : StandardNode("platform player"), m_platform{std::make_shared<ComposedPlatform>("platform player")} {
    m_platform->m_start = m_decoder.start();
    m_platform->m_stop = m_decoder.stop();
    m_platform->m_tick = m_decoder.create_output<Event<Void>>("tick");
    m_platform->m_clock = m_decoder.create_output<Event<Clock>>("clock");
    m_platform->m_accelerometer = m_decoder.create_output<Event<Vector3>>("accelerometer");
    m_platform->m_gyroscope = m_decoder.create_output<Event<Vector3>>("gyroscope");
    m_platform->m_magnetometer = m_decoder.create_output<Event<Vector3>>("magnetometer");
    m_platform->m_magnetometer_uncalibrated = m_decoder.create_output<Event<Vector3>>("magnetometer_uncalibrated");
    m_platform->m_magnetometer_bias = m_decoder.create_output<Event<Vector3>>("magnetometer_bias");
    m_platform->m_orientation = m_decoder.create_output<Event<Quaternion>>("orientation");
  }

  Input<protocol::Event> *input() { return m_decoder.input(); }

  std::shared_ptr<ComposedPlatform> platform() {
    return m_platform;
  }

  void flush() override {
    m_decoder.flush();
  }

private:
  ProtocolDecoder m_decoder;
  const std::shared_ptr<ComposedPlatform> m_platform;
};

class TimeWarpAdapter final : public StandardInput<protocol::Event> {
public:
  TimeWarpAdapter() : StandardInput<protocol::Event>("", nullptr), m_output{"", nullptr} {}

  StandardOutput<protocol::Event> *output() { return &m_output; }

  void push(protocol::Event data) override {
    StandardInput<protocol::Event>::push(data);

    if (!m_started) {
      if (!m_hello.has_hello()) {
        m_hello = data;
        return;
      }

      if (data.channel() == 0) {
        m_started = true;
        m_start_time = data.t();

        m_hello.set_t(0);
        m_output.push(m_hello);

        data.set_t(0);
        m_output.push(data);
      }
    } else if (data.t() >= m_start_time) {
      data.set_t(data.t() - m_start_time);
      m_output.push(data);
    }
  }

private:
  StandardOutput<protocol::Event> m_output;
  protocol::Event m_hello;
  bool m_started{false};
  double m_start_time{0};
};
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <input> <output>" << std::endl;
    return 1;
  }

  const std::string path = argv[1];

  FileReader file(path);
  PlatformDecoder decoder;
  TimeWarpAdapter time_warp_adapter;
  file.output()->plug(&time_warp_adapter);
  time_warp_adapter.output()->plug(decoder.input());

  auto platform = decoder.platform();

  /*
  MagneticsFilter::Config config;
  MagneticsFilter filter(config);
  platform->m_accelerometer->plug(filter.accelerometer());
  platform->m_gyroscope->plug(filter.gyroscope());
  platform->m_magnetometer->plug(filter.magnetometer());
  platform->m_magnetometer_uncalibrated->plug(filter.magnetometer_uncalibrated());
  platform->m_magnetometer_bias->plug(filter.magnetometer_bias());
  platform->m_orientation->plug(filter.orientation());
   */

  LiveDemoTask live_demo_task(platform);
  live_demo_task.start();

  //std::this_thread::sleep_for(std::chrono::seconds(3));

  double time = 0;
  while (!file.exhausted()) {
    time += 0.1;
    file.flush_until(time);
    decoder.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  live_demo_task.stop();
}
