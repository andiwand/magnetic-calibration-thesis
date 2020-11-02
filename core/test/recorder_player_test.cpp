#include <gtest/gtest.h>
#include <indoors/magnetics/recorder_task.h>
#include <indoors/magnetics/player_task.h>
#include <thread>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

TEST(recorder, hello) {
  auto platform = std::make_shared<ForwardPlatform>("test");

  RecorderTask recorder(platform, "test.rec");
  recorder.start();

  platform->m_start.push({0});
  platform->m_tick.push({0});
  platform->m_tick.push({1});
  platform->m_stop.push({2});

  recorder.stop();
}

TEST(player, hello) {
  BufferedInput<Event<Void>> ticks("tick", nullptr);

  {
    PlayerTask player("test.rec");
    player.start();

    player.platform()->tick()->plug(&ticks);

    player.stop();
  }

  ASSERT_EQ(2, ticks.swap().size());
}
