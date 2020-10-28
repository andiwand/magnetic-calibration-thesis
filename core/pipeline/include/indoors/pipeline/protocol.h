#ifndef INDOORS_PIPELINE_PROTOCOL_H
#define INDOORS_PIPELINE_PROTOCOL_H

#include <indoors/pipeline/event.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/event.pb.h>

namespace indoors::pipeline {

class ProtocolEncoder final : public StandardNode {
public:
  static protocol::Void encode(const Void &from);
  static protocol::Clock encode(const Clock &from);
  static protocol::Vector2 encode(const Vector2 &from);
  static protocol::Vector3 encode(const Vector3 &from);
  static protocol::Quaternion encode(const Quaternion &from);
  static protocol::Heading encode(const Heading &from);

  static protocol::Event encode(const Event<Void> &from);
  static protocol::Event encode(const Event<Clock> &from);
  static protocol::Event encode(const Event<double> &from);
  static protocol::Event encode(const Event<Vector2> &from);
  static protocol::Event encode(const Event<Vector3> &from);
  static protocol::Event encode(const Event<Quaternion> &from);
  static protocol::Event encode(const Event<Heading> &from);

  ProtocolEncoder();
  explicit ProtocolEncoder(std::string annotation);

  template <typename T> Input<T> *create_input(std::string annotation) {
    const auto channel_id = m_next_channel_id++;
    auto input = std::make_unique<EncoderInput<T>>(this, std::move(annotation), channel_id);
    auto result = input.get();
    m_inputs.push_back(std::move(input));
    return result;
  }

  template <typename T> void create_input(Output<T> *output) {
    output->plug(create_input<T>(output->annotation()));
  }

  template <typename T>
  void create_input(std::string annotation, Output<T> *output) {
    output->plug(create_input<T>(std::move(annotation)));
  }

  Output<protocol::Event> *output();

  void hello(double time);
  void bye(double time);

private:
  class EncoderInputBase : virtual public Interface {
  public:
    explicit EncoderInputBase(std::uint32_t channel_id);

    virtual protocol::Event example() = 0;
    protocol::ChannelHello hello();

    void push(protocol::Event event);

  private:
    const std::uint32_t m_channel_id{0};
  };

  template <typename T>
  class EncoderInput final : public StandardInput<T>, public EncoderInputBase {
  public:
    EncoderInput(ProtocolEncoder *encoder, std::string annotation,
                 std::uint32_t channel_id)
        : StandardInput<T>(std::move(annotation), encoder),
          EncoderInputBase(channel_id) {}

    protocol::Event example() override { return encode(T()); }

    void push(T data) override {
      EncoderInputBase::push(encode(data));
      StandardInput<T>::push(data);
    }
  };

  class IntermediateOutput final : public StandardOutput<protocol::Event> {
  public:
    IntermediateOutput(std::string annotation, ProtocolEncoder *encoder);
    void plug(Input<protocol::Event> *input) override;
    void unplug(Input<protocol::Event> *input) override;
  };

  protocol::Event hello_(double time);
  static protocol::Event bye_(double time);

  bool m_opened{false};
  bool m_closed{false};
  std::uint32_t m_next_channel_id{0};

  std::vector<std::unique_ptr<EncoderInputBase>> m_inputs;
  IntermediateOutput m_output;
};

class ProtocolDecoder final : public StandardNode {
public:
  static Void decode(const protocol::Void &from);
  static Clock decode(const protocol::Clock &from);
  static Vector2 decode(const protocol::Vector2 &from);
  static Vector3 decode(const protocol::Vector3 &from);
  static Quaternion decode(const protocol::Quaternion &from);
  static Heading decode(const protocol::Heading &from);

  static void decode(const protocol::Event &from, Event<Void> &to);
  static void decode(const protocol::Event &from, Event<Clock> &to);
  static void decode(const protocol::Event &from, Event<double> &to);
  static void decode(const protocol::Event &from, Event<Vector2> &to);
  static void decode(const protocol::Event &from, Event<Vector3> &to);
  static void decode(const protocol::Event &from, Event<Quaternion> &to);
  static void decode(const protocol::Event &from, Event<Heading> &to);

  ProtocolDecoder();
  explicit ProtocolDecoder(std::string annotation);

  template <typename T> Output<T> *create_output(std::string annotation) {
    auto output = std::make_unique<DecoderOutput<T>>(this, std::move(annotation));
    auto result = output.get();
    m_outputs.push_back(std::move(output));
    return result;
  }

  template <typename T> void create_output(Input<T> *input) {
    create_output<T>(input->annotation())->plug(input);
  }

  template <typename T>
  void create_output(std::string annotation, Input<T> *input) {
    create_output<T>(std::move(annotation))->plug(input);
  }

  Input<protocol::Event> *input();

  protocol::Event hello();
  protocol::Event bye();

private:
  class DecoderInput final : public StandardInput<protocol::Event> {
  public:
    DecoderInput(std::string annotation, ProtocolDecoder *decoder);
    void push(protocol::Event event) override;
  };

  class DecoderOutputBase {
  public:
    virtual ~DecoderOutputBase() = default;
    virtual protocol::Event example() = 0;
    virtual void push(protocol::Event event) = 0;
    virtual void skip(double time) = 0;
  };

  template <typename T>
  class DecoderOutput final : public StandardOutput<T>, public DecoderOutputBase {
  public:
    DecoderOutput(ProtocolDecoder *decoder, std::string annotation)
        : StandardOutput<T>(std::move(annotation), decoder) {}

    protocol::Event example() override { return ProtocolEncoder::encode(T()); }

    void push(protocol::Event event) override {
      T decoded;
      decode(event, decoded);
      StandardOutput<T>::push(decoded);
    }

    void skip(const double time) override {
      StandardOutput<T>::skip(time);
    }
  };

  void hello_(protocol::Event &&event);
  void bye_(protocol::Event &&event);

  DecoderInput m_input;
  std::vector<std::unique_ptr<DecoderOutputBase>> m_outputs;
  std::unordered_map<std::uint32_t, DecoderOutputBase *> m_mapping;

  protocol::Event m_hello;
  protocol::Event m_bye;
  bool m_opened{false};
  bool m_closed{false};
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PROTOCOL_H
