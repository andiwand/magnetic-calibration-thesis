#include <gtest/gtest.h>
#include <indoors/pipeline/protocol.h>

using namespace indoors::pipeline;

TEST(protocol, hello) {
  ProtocolEncoder encoder;
  auto channel_a = encoder.create_input<Event<Void>>("a");
  auto channel_b = encoder.create_input<Event<Void>>("b");

  ProtocolDecoder decoder;
  BufferedInput<Event<Void>> buffer_a("a", nullptr);
  BufferedInput<Event<Void>> buffer_b("b", nullptr);
  decoder.create_output(&buffer_a);
  decoder.create_output(&buffer_b);

  encoder.output()->plug(decoder.input());

  channel_a->push({0});
  channel_a->push({1});
  decoder.flush();
  ASSERT_EQ(2, buffer_a.swap().size());
  ASSERT_EQ(0, buffer_b.swap().size());

  channel_b->push({1});
  channel_a->push({2});
  decoder.flush();
  ASSERT_EQ(1, buffer_a.swap().size());
  ASSERT_EQ(1, buffer_b.swap().size());
}
