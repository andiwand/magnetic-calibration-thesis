#include <indoors/magnetics/live_demo_filter.h>
#include <indoors/magnetics/magnetics_filter.h>
#include <indoors/pipeline/file.h>
#include <indoors/pipeline/protocol.h>
#include <iostream>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

namespace {
class PlatformDecoder final : public StandardNode {
public:
  PlatformDecoder()
      : StandardNode("platform player"),
        m_platform{std::make_shared<ComposedPlatform>("platform player")} {
    m_platform->m_start = m_decoder.start();
    m_platform->m_stop = m_decoder.stop();
    m_platform->m_tick = m_decoder.create_output<Event<Void>>("tick");
    m_platform->m_clock = m_decoder.create_output<Event<Clock>>("clock");
    m_platform->m_accelerometer =
        m_decoder.create_output<Event<Vector3>>("accelerometer");
    m_platform->m_gyroscope =
        m_decoder.create_output<Event<Vector3>>("gyroscope");
    m_platform->m_magnetometer =
        m_decoder.create_output<Event<Vector3>>("magnetometer");
    m_platform->m_magnetometer_uncalibrated =
        m_decoder.create_output<Event<Vector3>>("magnetometer_uncalibrated");
    m_platform->m_magnetometer_bias =
        m_decoder.create_output<Event<Vector3>>("magnetometer_bias");
    m_platform->m_orientation =
        m_decoder.create_output<Event<Quaternion>>("orientation");
  }

  Input<protocol::Event> *input() { return m_decoder.input(); }

  std::shared_ptr<ComposedPlatform> platform() { return m_platform; }

  void flush() override { m_decoder.flush(); }

private:
  ProtocolDecoder m_decoder;
  const std::shared_ptr<ComposedPlatform> m_platform;
};

class TimeWarpAdapter final : public StandardInput<protocol::Event> {
public:
  explicit TimeWarpAdapter(const double delta_time)
      : StandardInput<protocol::Event>("", nullptr),
        m_delta_time{delta_time}, m_output{"", nullptr} {}

  StandardOutput<protocol::Event> *output() { return &m_output; }

  void push(protocol::Event data) override {
    StandardInput<protocol::Event>::push(data);
    data.set_t(data.t() - m_delta_time);
    m_output.push(data);
  }

private:
  const double m_delta_time;
  StandardOutput<protocol::Event> m_output;
};
} // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <input> <output>" << std::endl;
    return 1;
  }

  const std::string input = argv[1];
  const std::string output = argv[2];

  FileReader file(input);
  protocol::Event hello = file.read();
  protocol::Event first_tick;

  while (true) {
    first_tick = file.read();
    if (first_tick.channel() == 0)
      break;
  }

  PlatformDecoder decoder;
  TimeWarpAdapter time_warp_adapter(first_tick.t());
  file.output()->plug(&time_warp_adapter);
  time_warp_adapter.output()->plug(decoder.input());

  boost::asio::io_context ioc;

  MagneticsFilter::Config config;
  LiveDemoFilter live_demo_filter(config, decoder.platform(), ioc);

  FileWriter output_file(output);
  ProtocolEncoder encoder("evaluation");
  encoder.output()->plug(output_file.input());
  decoder.platform()->start()->plug(encoder.start());
  decoder.platform()->stop()->plug(encoder.stop());
  encoder.create_input(decoder.platform()->tick());
  encoder.create_input(live_demo_filter.magnetics_filter().total_rotation());
  encoder.create_input(live_demo_filter.magnetics_filter().sampled_magnetometer_uncalibrated());
  encoder.create_input(live_demo_filter.magnetics_filter().sampled_magnetometer_bias());
  encoder.create_input(live_demo_filter.magnetics_filter().filter_hard_iron());
  encoder.create_input(live_demo_filter.magnetics_filter().filter_var());

  std::thread ioc_runner([&]() { ioc.run(); });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  hello.set_t(0);
  decoder.input()->push(hello);
  first_tick.set_t(0);
  decoder.input()->push(first_tick);

  const double start_time = first_tick.t();
  double time = start_time;
  while (!file.exhausted()) {
    time += 0.1;

    file.flush_until(time);
    decoder.flush();
    live_demo_filter.flush();
    encoder.flush();
    output_file.flush();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  ioc.stop();
  ioc_runner.join();
}
