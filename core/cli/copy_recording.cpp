#include <indoors/magnetics/player_task.h>
#include <indoors/magnetics/recorder_task.h>
#include <iostream>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <input> <output>" << std::endl;
    return 1;
  }

  const std::string input = argv[1];
  const std::string output = argv[2];
  PlayerTask player_task(input);
  RecorderTask recorder_task(player_task.platform(), output);

  recorder_task.start();
  player_task.start();

  player_task.stop();
  recorder_task.stop();
}
