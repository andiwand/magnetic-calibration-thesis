#include <gtest/gtest.h>
#include <indoors/pipeline/synchronizer.h>

using namespace indoors::pipeline;

TEST(synchronizer, hello) {
  Synchronizer synchronizer(1);
  auto channel_a = synchronizer.create_channel<Event<Void>>("a");
  auto channel_b = synchronizer.create_channel<Event<Void>>("b");

  BufferedSeriesInput<Event<Void>> buffer("", nullptr);
  channel_a.second->plug(&buffer);

  channel_a.first->push({0.0});
  channel_a.first->push({1.0});

  synchronizer.flush();
  auto data = buffer.swap();
  ASSERT_EQ(0, data.size());
  ASSERT_EQ(channel_a.second->time(), channel_b.second->time());

  channel_a.first->push({2.0});

  synchronizer.flush();
  data = buffer.swap();
  ASSERT_EQ(1, data.size());
  ASSERT_EQ(channel_a.second->time(), channel_b.second->time());

  channel_a.first->skip(3.1);

  synchronizer.flush();
  data = buffer.swap();
  ASSERT_EQ(2, data.size());
  ASSERT_EQ(channel_a.second->time(), channel_b.second->time());
}
