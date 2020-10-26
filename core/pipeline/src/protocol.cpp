#include <indoors/pipeline/protocol.h>

#include <utility>

namespace indoors::pipeline {

protocol::Void ProtocolEncoder::encode(const Void &) { return {}; }

protocol::Clock ProtocolEncoder::encode(const Clock &from) {
  protocol::Clock result;
  result.set_unix_utc(from.unix_utc);
  return result;
}

protocol::Vector2 ProtocolEncoder::encode(const Vector2 &from) {
  protocol::Vector2 result;
  result.set_x(from.x);
  result.set_y(from.y);
  return result;
}

protocol::Vector3 ProtocolEncoder::encode(const Vector3 &from) {
  protocol::Vector3 result;
  result.set_x(from.x);
  result.set_y(from.y);
  result.set_z(from.z);
  return result;
}

protocol::Quaternion ProtocolEncoder::encode(const Quaternion &from) {
  protocol::Quaternion result;
  result.set_w(from.w);
  result.set_x(from.x);
  result.set_y(from.y);
  result.set_z(from.z);
  return result;
}

protocol::Heading ProtocolEncoder::encode(const Heading &from) {
  protocol::Heading result;
  result.set_north(from.north);
  result.set_var_north(from.var_north);
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Void> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_void_()->CopyFrom(encode(event.data));
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Clock> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_clock()->CopyFrom(encode(event.data));
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<double> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.set_scalar(event.data);
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Vector2> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_vector2()->CopyFrom(encode(event.data));
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Vector3> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_vector3()->CopyFrom(encode(event.data));
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Quaternion> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_quaternion()->CopyFrom(encode(event.data));
  return result;
}

protocol::Event ProtocolEncoder::encode(const Event<Heading> &event) {
  protocol::Event result;
  result.set_t(event.time);
  result.mutable_heading()->CopyFrom(encode(event.data));
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

Void ProtocolDecoder::decode(const protocol::Void &) { return {}; }

Clock ProtocolDecoder::decode(const protocol::Clock &from) {
  return {from.unix_utc()};
}

Vector2 ProtocolDecoder::decode(const protocol::Vector2 &from) {
  return {from.x(), from.y()};
}

Vector3 ProtocolDecoder::decode(const protocol::Vector3 &from) {
  return {from.x(), from.y(), from.z()};
}

Quaternion ProtocolDecoder::decode(const protocol::Quaternion &from) {
  return {from.w(), from.x(), from.y(), from.z()};
}

Heading ProtocolDecoder::decode(const protocol::Heading &from) {
  return {from.north(), from.var_north()};
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<Void> &to) {
  assert(from.has_void_());
  to.time = from.t();
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<Clock> &to) {
  assert(from.has_clock());
  to.time = from.t();
  to.data = decode(from.clock());
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<double> &to) {
  assert(from.__case() == protocol::Event::Case::kScalar);
  to.time = from.t();
  to.data = to.data;
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<Vector2> &to) {
  assert(from.has_vector2());
  to.time = from.t();
  to.data = decode(from.vector2());
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<Vector3> &to) {
  assert(from.has_vector3());
  to.time = from.t();
  to.data = decode(from.vector3());
}

void ProtocolDecoder::decode(const protocol::Event &from,
                             Event<Quaternion> &to) {
  assert(from.has_quaternion());
  to.time = from.t();
  to.data = decode(from.quaternion());
}

void ProtocolDecoder::decode(const protocol::Event &from, Event<Heading> &to) {
  assert(from.has_heading());
  to.time = from.t();
  to.data = decode(from.heading());
}

ProtocolDecoder::ProtocolDecoder() : ProtocolDecoder("decoder") {}

ProtocolDecoder::ProtocolDecoder(std::string annotation)
    : StandardNode(std::move(annotation)), m_input{"decoder input", this} {}

Input<protocol::Event> *ProtocolDecoder::input() { return &m_input; }

void ProtocolDecoder::iterate() {
  auto &&events = m_input.swap();

  for (auto &&event : events) {
    if (!m_initialized) {
      assert(event.has_hello());
      auto &&hello = *event.mutable_hello();
      for (auto &&output : m_outputs) {
        auto example = output->example();

        auto &&channels = *hello.mutable_channels();
        for (auto channel_it = channels.begin(); channel_it != channels.end();
             ++channel_it) {
          if (channel_it->eventexample().__case() == example.__case()) {
            m_mapping[channel_it->channel()] = output.get();
            channels.erase(channel_it);
          }
        }
      }
      m_initialized = true;
      continue;
    }
    assert(!event.has_hello());

    if (event.has_bye()) {
      // TODO
      return;
    }

    auto &&mapping = m_mapping.find(event.channel());
    if (mapping != m_mapping.end())
      mapping->second->push(event);
  }
}

} // namespace indoors::pipeline
