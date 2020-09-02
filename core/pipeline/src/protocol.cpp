#include <indoors/pipeline/protocol.h>

#include <utility>

namespace indoors::pipeline {

protocol::Void ProtocolEncoder::convert(const Void &) { return {}; }

protocol::Clock ProtocolEncoder::convert(const Clock &from) {
  protocol::Clock result;
  result.set_unix_utc(from.unix_utc);
  return result;
}

protocol::Vector2 ProtocolEncoder::convert(const Vector2 &from) {
  protocol::Vector2 result;
  result.set_x(from.x);
  result.set_y(from.y);
  return result;
}

protocol::Vector3 ProtocolEncoder::convert(const Vector3 &from) {
  protocol::Vector3 result;
  result.set_x(from.x);
  result.set_y(from.y);
  result.set_z(from.z);
  return result;
}

protocol::Quaternion ProtocolEncoder::convert(const Quaternion &from) {
  protocol::Quaternion result;
  result.set_w(from.w);
  result.set_x(from.x);
  result.set_y(from.y);
  result.set_z(from.z);
  return result;
}

protocol::Heading ProtocolEncoder::convert(const Heading &from) {
    protocol::Heading result;
    result.set_north(from.north);
    result.set_var_north(from.var_north);
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

protocol::Event ProtocolEncoder::convert(const Event<Vector2> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_vector2()->CopyFrom(convert(event.data));
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Vector3> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_vector3()->CopyFrom(convert(event.data));
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Quaternion> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_quaternion()->CopyFrom(convert(event.data));
  return result;
}

protocol::Event ProtocolEncoder::convert(const Event<Heading> &event) {
    protocol::Event result;
    result.set_t(event.time);
    result.mutable_heading()->CopyFrom(convert(event.data));
    return result;
}

ProtocolEncoder::ProtocolEncoder() : ProtocolEncoder("encoder") {}

ProtocolEncoder::ProtocolEncoder(std::string annotation)
    : StandardNode(std::move(annotation)), m_output{"encoder output", this} {}

ProtocolEncoder::~ProtocolEncoder() { push(bye()); }

Output<protocol::Event> *ProtocolEncoder::output() { return &m_output; }

ProtocolEncoder::EncoderInputBase::EncoderInputBase(std::uint32_t channel_id)
    : m_channel_id{channel_id} {}

protocol::ChannelHello ProtocolEncoder::EncoderInputBase::hello() {
  protocol::ChannelHello hello;
  hello.set_channel(m_channel_id);
  hello.set_annotation(annotation());
  hello.mutable_eventexample()->CopyFrom(example());
  return hello;
}

void ProtocolEncoder::EncoderInputBase::push(protocol::Event event) {
  event.set_channel(m_channel_id);
  reinterpret_cast<ProtocolEncoder *>(node())->push(std::move(event));
}

ProtocolEncoder::EncoderOutput::EncoderOutput(std::string annotation,
                                              Node *node)
    : StandardOutput<protocol::Event>(std::move(annotation), node) {}

void ProtocolEncoder::EncoderOutput::plug(Input<protocol::Event> *input) {
  StandardOutput<protocol::Event>::plug(input);
  auto hello = reinterpret_cast<ProtocolEncoder *>(node())->hello();
  input->push(hello);
}

void ProtocolEncoder::EncoderOutput::unplug(Input<protocol::Event> *input) {
  StandardOutput<protocol::Event>::unplug(input);
  auto bye = reinterpret_cast<ProtocolEncoder *>(node())->bye();
  input->push(bye);
}

protocol::Event ProtocolEncoder::hello() {
  protocol::Event event;
  event.set_t(0);
  event.mutable_hello()->set_from(annotation());
  for (auto &&channel : m_inputs) {
    event.mutable_hello()->mutable_channels()->Add()->CopyFrom(
        channel->hello());
  }
  return event;
}

protocol::Event ProtocolEncoder::bye() {
  protocol::Event event;
  event.set_t(m_output.time());
  event.mutable_bye();
  return event;
}

void ProtocolEncoder::push(protocol::Event event) { m_output.push(event); }

Void ProtocolDecoder::convert(const protocol::Void &) { return {}; }

Clock ProtocolDecoder::convert(const protocol::Clock &from) {
  return {from.unix_utc()};
}

Vector2 ProtocolDecoder::convert(const protocol::Vector2 &from) {
  return {from.x(), from.y()};
}

Vector3 ProtocolDecoder::convert(const protocol::Vector3 &from) {
  return {from.x(), from.y(), from.z()};
}

Quaternion ProtocolDecoder::convert(const protocol::Quaternion &from) {
  return {from.w(), from.x(), from.y(), from.z()};
}

Heading ProtocolDecoder::convert(const protocol::Heading &from) {
    return {from.north(), from.var_north()};
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

void ProtocolDecoder::convert(const protocol::Event &from, Event<Vector2> &to) {
  if (!from.has_vector2())
    throw; // TODO
  to.time = from.t();
  to.data = convert(from.vector2());
}

void ProtocolDecoder::convert(const protocol::Event &from, Event<Vector3> &to) {
  if (!from.has_vector3())
    throw; // TODO
  to.time = from.t();
  to.data = convert(from.vector3());
}

void ProtocolDecoder::convert(const protocol::Event &from,
                              Event<Quaternion> &to) {
  if (!from.has_quaternion())
    throw; // TODO
  to.time = from.t();
  to.data = convert(from.quaternion());
}

void ProtocolDecoder::convert(const protocol::Event &from,
                              Event<Heading> &to) {
    if (!from.has_heading())
        throw; // TODO
    to.time = from.t();
    to.data = convert(from.heading());
}

ProtocolDecoder::ProtocolDecoder(std::string annotation)
    : StandardNode(std::move(annotation)) {}

} // namespace indoors::pipeline
