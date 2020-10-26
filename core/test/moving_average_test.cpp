#include <gtest/gtest.h>
#include <indoors/magnetics/moving_average.h>

using namespace indoors::pipeline;
using namespace indoors::magnetics;

// TODO check nan

TEST(moving_average, hello) {
  MovingAverage moving_average(2, 1);
  auto channel = moving_average.create_channel("a");

  BufferedSeriesInput<Event<Vector3>> buffer("", nullptr);
  channel.second->plug(&buffer);

  channel.first->push({0.0, 1.0, 0.0, 1.0});
  channel.first->push({1.0, 2.0, 0.0, 3.0});
  channel.first->push({2.0, 3.0, 0.0, 5.0});
  channel.first->skip(3.1);

  moving_average.iterate();

  auto data = buffer.swap();
  ASSERT_EQ(2, data.size());
}
