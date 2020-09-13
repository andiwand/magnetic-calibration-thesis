#include <indoors/magnetics/task.h>
#include <indoors/pipeline/platform.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;
using namespace nlohmann;

int main(int argc, char** argv) {
  const std::string path = "/home/andreas/Downloads/200618_LGE_Nexus5X_01.json";

  json sensor_data;
  {
    std::ifstream input(path);
    input >> sensor_data;
  }

  auto replay_platform = std::make_shared<StandardPlatform>("tu replayed");
  auto default_task = Tasks::create_default(replay_platform);
  default_task->start();

  const double interval = 0.1;
  double last_time = 0;

  bool first = true;
  double start_time = 0;
  for (auto &&entry : sensor_data) {
    if (entry.empty()) continue;

    double time = entry["header"]["time_andro"];
    time -= start_time;
    if (first) {
      time = 0;
      start_time = entry["header"]["time_andro"];
      first = false;
    }

    if (entry["header"]["sensor"] == "acc") {
      replay_platform->m_accelerometer.push({time, entry["data"][0], entry["data"][1], entry["data"][2]});
    } else if (entry["header"]["sensor"] == "gyr") {
      replay_platform->m_gyroscope.push({time, entry["data"][0], entry["data"][1], entry["data"][2]});
    } else if (entry["header"]["sensor"] == "mag") {
      replay_platform->m_magnetometer.push({time, entry["data"][0], entry["data"][1], entry["data"][2]});
    } else if (entry["header"]["sensor"] == "mag_raw") {
      replay_platform->m_magnetometer_uncalibrated.push({time, entry["data"][0], entry["data"][1], entry["data"][2]});
    }

    if (time - last_time > interval) {
      std::this_thread::sleep_for(std::chrono::milliseconds(long(interval*1e3)));
      last_time = time;
    }
  }

  default_task->stop();
}
