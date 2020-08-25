#include <indoors/pipeline/protocol.h>

#include <utility>

namespace indoors::pipeline {

protocol::Void ProtocolEncoder::convert(const Void &) { return {}; }

protocol::Clock ProtocolEncoder::convert(const Clock &from) {
  protocol::Clock result;
  result.set_unix_utc(from.unix_utc);
  return result;
}

protocol::Vector3 ProtocolEncoder::convert(const Vector3 &from) {
  protocol::Vector3 result;
  result.set_x(from.x);
  result.set_y(from.y);
  result.set_z(from.z);
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Void> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_void_()->CopyFrom(convert(event.data));
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Clock> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_clock()->CopyFrom(convert(event.data));
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Vector3> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_vector3()->CopyFrom(convert(event.data));
  return result;
}

ProtocolEncoder::ProtocolEncoder() : ProtocolEncoder("encoder") {}

ProtocolEncoder::ProtocolEncoder(std::string annotation)
    : StandardNode(std::move(annotation)), m_output{"encoder output", this} {}

ProtocolEncoder::~ProtocolEncoder() {
  push(bye());
}

Output<protocol::Event> *ProtocolEncoder::output() { return &m_output; }

ProtocolEncoder::EncoderInputBase::EncoderInputBase(std::uint32_t channel_id)
    : m_channel_id{channel_id} {}

protocol::ChannelHello ProtocolEncoder::EncoderInputBase::hello() noexcept {
  protocol::ChannelHello hello;
  hello.set_channel(m_channel_id);
  hello.set_annotation(annotation());
  hello.mutable_eventexample()->CopyFrom(example());
  return hello;
}

void ProtocolEncoder::EncoderInputBase::push(protocol::Event event) noexcept {
  event.set_channel(m_channel_id);
  reinterpret_cast<ProtocolEncoder *>(node())->push(std::move(event));
}

ProtocolEncoder::EncoderOutput::EncoderOutput(std::string annotation,
                                              Node *node) noexcept
    : StandardOutput<protocol::Event>(std::move(annotation), node) {}

void ProtocolEncoder::EncoderOutput::plug(Input<protocol::Event> *input) noexcept {
  StandardOutput<protocol::Event>::plug(input);
  auto hello = reinterpret_cast<ProtocolEncoder *>(node())->hello();
  input->push(hello);
}

void ProtocolEncoder::EncoderOutput::unplug(Input<protocol::Event> *input) noexcept {
  StandardOutput<protocol::Event>::unplug(input);
  auto bye = reinterpret_cast<ProtocolEncoder *>(node())->bye();
  input->push(bye);
}

void ProtocolEncoder::EncoderOutput::push(protocol::Event event) noexcept {
  m_time = event.t();
}

protocol::Event ProtocolEncoder::hello() noexcept {
  protocol::Event event;
  event.set_t(0);
  event.mutable_hello()->set_from(annotation());
  for (auto &&channel : m_inputs) {
    event.mutable_hello()->mutable_channels()->Add()->CopyFrom(channel->hello());
  }
  return event;
}

protocol::Event ProtocolEncoder::bye() noexcept {
  protocol::Event event;
  event.set_t(m_output.time());
  event.mutable_bye();
  return event;
}

void ProtocolEncoder::push(protocol::Event event) noexcept { m_output.push(event); }

Void ProtocolDecoder::convert(const protocol::Void &) { return {}; }

Clock ProtocolDecoder::convert(const protocol::Clock &from) {
  return {from.unix_utc()};
}

Vector3 ProtocolDecoder::convert(const protocol::Vector3 &from) {
  return {from.x(), from.y(), from.z()};
}

void ProtocolDecoder::convert(const protocol::Event &from, Event<Void> &to) {
  if (!from.has_void_())
    throw; // TODO
  to.time = from.t();
}

void ProtocolDecoder::convert(const protocol::Event &from, Event<Clock> &to) {
  if (!from.has_clock())
    throw; // TODO
  to.time = from.t();
  to.data = convert(from.clock());
}

void ProtocolDecoder::convert(const protocol::Event &from, Event<Vector3> &to) {
  if (!from.has_vector3())
    throw; // TODO
  to.time = from.t();
  to.data = convert(from.vector3());
}

ProtocolDecoder::ProtocolDecoder(std::string annotation)
    : StandardNode(std::move(annotation)) {}

} // namespace indoors::pipeline
