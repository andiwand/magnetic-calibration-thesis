#include <indoors/magnetics/player_task.h>
#include <iostream>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

namespace {
template<typename T>
StandardInput<T> counter(Output<T> *output) {
  StandardInput<T> input("", nullptr);
  output->plug(&input);
  return input;
}

template<typename T>
BufferedInput<T> buffer(Output<T> *output) {
  BufferedInput<T> input("", nullptr);
  output->plug(&input);
  return input;
}
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <recording>" << std::endl;
    return 1;
  }

  const std::string path = argv[1];
  PlayerTask player_task(path);

  auto platform = player_task.platform();
  auto start = buffer(platform->start());
  auto stop = buffer(platform->stop());
  auto tick = counter(platform->tick());
  auto clock = counter(platform->clock());
  auto accelerometer = counter(platform->accelerometer());
  auto gyroscope = counter(platform->gyroscope());
  auto magnetometer = counter(platform->magnetometer());
  auto magnetometer_uncalibrated =
      counter(platform->magnetometer_uncalibrated());
  auto magnetometer_bias = counter(platform->magnetometer_bias());
  auto orientation = counter(platform->orientation());

  player_task.start();
  player_task.stop();

  std::cout << "duration " << stop.swap()[0].time - start.swap()[0].time << std::endl;
  std::cout << "tick count " << tick.message_count() << std::endl;
  std::cout << "clock count " << clock.message_count() << std::endl;
  std::cout << "accelerometer count " << accelerometer.message_count() << std::endl;
  std::cout << "gyroscope count " << gyroscope.message_count() << std::endl;
  std::cout << "magnetometer count " << magnetometer.message_count() << std::endl;
  std::cout << "magnetometer uncalibrated count " << magnetometer_uncalibrated.message_count() << std::endl;
  std::cout << "magnetometer bias count " << magnetometer_bias.message_count() << std::endl;
  std::cout << "orientation count " << orientation.message_count() << std::endl;
}
