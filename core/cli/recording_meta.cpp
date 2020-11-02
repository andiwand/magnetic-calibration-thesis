#include <indoors/magnetics/player_task.h>
#include <iostream>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

namespace {
template<typename T>
StandardInput<T> couter(Output<T> *output) {
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
  auto tick = couter(platform->tick());
  auto clock = couter(platform->clock());
  auto accelerometer = couter(platform->accelerometer());
  auto gyroscope = couter(platform->gyroscope());
  auto magnetometer = couter(platform->magnetometer());
  auto magnetometer_uncalibrated = couter(platform->magnetometer_uncalibrated());
  auto magnetometer_bias = couter(platform->magnetometer_bias());
  auto orientation = couter(platform->orientation());

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
