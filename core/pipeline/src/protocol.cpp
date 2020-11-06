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
    : StandardNode(std::move(annotation)), m_start{"encoder start", this},
      m_stop{"encoder stop", this}, m_output{"encoder output", this} {}

Input<Event<Void>> * ProtocolEncoder::start() { return &m_start; }

Input<Event<Void>> * ProtocolEncoder::stop() { return &m_stop; }

Output<protocol::Event> *ProtocolEncoder::output() { return &m_output; }

void ProtocolEncoder::hello(const double time) {
  assert(!m_opened);

  m_output.push(hello_(time));

  m_opened = true;
  m_closed = false;
}

void ProtocolEncoder::bye(const double time) {
  assert(!m_closed);

  m_output.push(bye_(time));

  m_opened = false;
  m_closed = true;
}

ProtocolEncoder::StartInput::StartInput(std::string annotation,
                                        ProtocolEncoder *encoder)
    : StandardInput<Event<Void>>(std::move(annotation), encoder) {}

void ProtocolEncoder::StartInput::push(Event<Void> event) {
  auto encoder = reinterpret_cast<ProtocolEncoder *>(node());
  encoder->hello(event.time);
}

ProtocolEncoder::StopInput::StopInput(std::string annotation,
                                      ProtocolEncoder *encoder)
    : StandardInput<Event<Void>>(std::move(annotation), encoder) {}

void ProtocolEncoder::StopInput::push(Event<Void> event) {
  auto encoder = reinterpret_cast<ProtocolEncoder *>(node());
  encoder->bye(event.time);
}

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
  auto encoder = reinterpret_cast<ProtocolEncoder *>(node());
  //assert(encoder->m_opened);
  if (!encoder->m_opened) return;

  event.set_channel(m_channel_id);
  encoder->m_output.push(event);
}

ProtocolEncoder::IntermediateOutput::IntermediateOutput(
    std::string annotation, ProtocolEncoder *encoder)
    : StandardOutput<protocol::Event>(std::move(annotation), encoder) {}

void ProtocolEncoder::IntermediateOutput::plug(Input<protocol::Event> *input) {
  StandardOutput<protocol::Event>::plug(input);

  auto encoder = reinterpret_cast<ProtocolEncoder *>(node());
  if (encoder->m_opened) {
    auto event = encoder->hello_(encoder->m_output.time());
    input->push(event);
  }
}

void ProtocolEncoder::IntermediateOutput::unplug(
    Input<protocol::Event> *input) {
  StandardOutput<protocol::Event>::unplug(input);

  auto encoder = reinterpret_cast<ProtocolEncoder *>(node());
  if (!encoder->m_closed) {
    auto event = bye_(encoder->m_output.time());
    input->push(event);
  }
}

protocol::Event ProtocolEncoder::hello_(const double time) {
  protocol::Event event;
  event.set_t(time);
  event.mutable_hello()->set_from(annotation());
  for (auto &&channel : m_inputs) {
    event.mutable_hello()->mutable_channels()->Add()->CopyFrom(
        channel->hello());
  }
  return event;
}

protocol::Event ProtocolEncoder::bye_(const double time) {
  protocol::Event event;
  event.set_t(time);
  event.mutable_bye();
  return event;
}

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
    : StandardNode(std::move(annotation)), m_input{"decoder input", this},
      m_start{"decoder start", this}, m_stop{"decoder stop", this} {}

Input<protocol::Event> *ProtocolDecoder::input() { return &m_input; }

Output<Event<Void>> *ProtocolDecoder::start() { return &m_start; }

Output<Event<Void>> *ProtocolDecoder::stop() { return &m_stop; }

protocol::Event ProtocolDecoder::hello() { return m_hello; }

protocol::Event ProtocolDecoder::bye() { return m_bye; }

ProtocolDecoder::DecoderInput::DecoderInput(std::string annotation,
                                            ProtocolDecoder *decoder)
    : StandardInput<protocol::Event>(std::move(annotation), decoder) {}

void ProtocolDecoder::DecoderInput::push(protocol::Event event) {
  auto decoder = reinterpret_cast<ProtocolDecoder *>(node());

  if (!decoder->m_opened) {
    decoder->hello_(std::move(event));
    return;
  }

  assert(!event.has_hello());

  if (event.has_bye()) {
    decoder->bye_(std::move(event));
    return;
  }

  auto &&mapping = decoder->m_mapping.find(event.channel());
  if (mapping != decoder->m_mapping.end())
    mapping->second->push(event);
}

void ProtocolDecoder::hello_(protocol::Event &&event) {
  assert(event.has_hello());

  m_start.push({event.t()});
  m_hello = event;

  for (auto &&output : m_outputs) {
    auto example = output->example();

    auto &&channels = *event.mutable_hello()->mutable_channels();
    for (auto channel_it = channels.begin(); channel_it != channels.end();
         ++channel_it) {
      if (channel_it->eventexample().__case() == example.__case()) {
        m_mapping[channel_it->channel()] = output.get();
        channels.erase(channel_it);
        break;
      }
    }
  }

  m_opened = true;
  m_closed = false;
}

void ProtocolDecoder::bye_(protocol::Event &&event) {
  assert(event.has_bye());

  m_stop.push({event.t()});
  m_bye = event;

  for (auto &&output : m_outputs) {
    output->skip(event.t());
  }

  m_opened = false;
  m_closed = true;
}

} // namespace indoors::pipeline
