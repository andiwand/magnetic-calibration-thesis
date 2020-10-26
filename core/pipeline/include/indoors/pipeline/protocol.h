#ifndef INDOORS_PIPELINE_PROTOCOL_H
#define INDOORS_PIPELINE_PROTOCOL_H

#include <indoors/pipeline/definitions.h>
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
  ~ProtocolEncoder() override;

  template <typename T> Input<T> *create_input(std::string annotation) {
    const auto channel_id = m_next_channel_id++;
    // TODO new
    auto input = new EncoderInput<T>(this, std::move(annotation), channel_id);
    m_inputs.push_back(std::unique_ptr<EncoderInput<T>>(input));
    return input;
  }

  template <typename T> void create_input(Output<T> *output) {
    output->plug(create_input<T>(output->annotation()));
  }

  template <typename T>
  void create_input(std::string annotation, Output<T> *output) {
    output->plug(create_input<T>(std::move(annotation)));
  }

  Output<protocol::Event> *output();

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
  class EncoderInput : public StandardInput<T>, public EncoderInputBase {
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

  class EncoderOutput : public StandardOutput<protocol::Event> {
  public:
    EncoderOutput(std::string annotation, Node *node);
    void plug(Input<protocol::Event> *input) override;
    void unplug(Input<protocol::Event> *input) override;
  };

  protocol::Event hello();
  protocol::Event bye();
  void push(protocol::Event data);

  std::uint32_t m_next_channel_id{0};

  std::vector<std::unique_ptr<EncoderInputBase>> m_inputs;
  EncoderOutput m_output;
};

class ProtocolDecoder final : public StandardNode, public Loopable {
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
    // TODO new
    auto output = new DecoderOutput<T>(this, std::move(annotation));
    m_outputs.push_back(std::unique_ptr<DecoderOutput<T>>(output));
    return output;
  }

  template <typename T> void create_output(Input<T> *input) {
    create_output<T>(input->annotation())->plug(input);
  }

  template <typename T>
  void create_output(std::string annotation, Input<T> *input) {
    create_output<T>(std::move(annotation))->plug(input);
  }

  Input<protocol::Event> *input();

  void iterate() override;

private:
  class DecoderOutputBase;

  BufferedInput<protocol::Event> m_input;
  std::vector<std::unique_ptr<DecoderOutputBase>> m_outputs;
  std::unordered_map<std::uint32_t, DecoderOutputBase *> m_mapping;
  bool m_initialized{false};

  class DecoderOutputBase {
  public:
    virtual ~DecoderOutputBase() = default;
    virtual protocol::Event example() = 0;
    virtual void push(protocol::Event event) = 0;
  };

  template <typename T>
  class DecoderOutput : public StandardOutput<T>, public DecoderOutputBase {
  public:
    DecoderOutput(ProtocolDecoder *decoder, std::string annotation)
        : StandardOutput<T>(std::move(annotation), decoder) {}

    protocol::Event example() override { return encode(T()); }

    void push(protocol::Event event) override {
      T decoded;
      decode(event, decoded);
      StandardOutput<T>::push(decoded);
    }
  };
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PROTOCOL_H
