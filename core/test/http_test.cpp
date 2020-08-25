#include <chrono>
#include <gtest/gtest.h>
#include <indoors/pipeline/http.h>
#include <thread>

using namespace indoors::pipeline;

namespace {
float randf() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}
} // namespace

TEST(http, hello) {
  StandardOutput<protocol::Event> output("", nullptr);

  boost::asio::io_context ioc{1};
  WebSocket webSocket(ioc, "0.0.0.0", 8000, &output);

  std::thread([&]() {
    while (true) {
      webSocket.iterate();
      protocol::Event event;
      event.set_t(0);
      event.set_channel(0);
      event.mutable_vector3()->set_x(randf() * 10);
      event.mutable_vector3()->set_y(randf() * 10);
      event.mutable_vector3()->set_z(randf() * 10);
      output.push(event);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }).detach();

  ioc.run();
}
