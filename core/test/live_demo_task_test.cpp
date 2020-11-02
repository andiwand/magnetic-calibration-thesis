#include <gtest/gtest.h>
#include <indoors/pipeline/platform.h>
#include <indoors/magnetics/live_demo_task.h>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

TEST(live_demo_task, smoke) {
  auto platform = std::make_shared<ForwardPlatform>("test");

  LiveDemoTask task(platform);
  task.start();
  task.stop();
}
