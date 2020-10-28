#include <gtest/gtest.h>
#include <indoors/magnetics/recorder_task.h>
#include <indoors/magnetics/player_task.h>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

TEST(recorder, hello) {
  auto platform = std::make_shared<ForwardPlatform>("test");

  RecorderTask recorder(platform, "test.rec");

  platform->m_tick.push({0});
  platform->m_tick.push({1});

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST(player, hello) {
  BufferedInput<Event<Void>> ticks("tick", nullptr);

  {
    PlayerTask player("test.rec");
    player.platform()->tick()->plug(&ticks);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  ASSERT_EQ(2, ticks.swap().size());
}
