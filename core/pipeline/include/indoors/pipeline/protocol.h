#ifndef INDOORS_PIPELINE_PROTOCOL_H
#define INDOORS_PIPELINE_PROTOCOL_H

#include <indoors/pipeline/event.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/event.pb.h>

namespace indoors::pipeline {

class ProtocolEncoder final : public StandardNode {
public:
  static protocol::Void convert(const Void &from);
  static protocol::Clock convert(const Clock &from);
  static protocol::Vector3 convert(const Vector3 &from);
  static protocol::Quaternion convert(const Quaternion &from);

  static protocol::Event convert(const Event<Void> &from);
  static protocol::Event convert(const Event<Clock> &from);
  static protocol::Event convert(const Event<Vector3> &from);
  static protocol::Event convert(const Event<Quaternion> &from);

  ProtocolEncoder();
  explicit ProtocolEncoder(std::string annotation);
  ~ProtocolEncoder() override;

  template <typename T> Input<T> *create_input(std::string annotation) {
    const auto channel_id = m_next_channel_id++;
    auto input = new EncoderInput<T>(this, std::move(annotation), channel_id);
    m_inputs.push_back(std::unique_ptr<EncoderInput<T>>(input));
    return input;
  }

  template <typename T> void create_input(Output<T> *output) {
    output->plug(create_input<T>(output->annotation()));
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

    protocol::Event example() override { return convert(T()); }

    void push(T data) override {
      EncoderInputBase::push(convert(data));
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

class ProtocolDecoder final : public StandardNode {
public:
  static Void convert(const protocol::Void &from);
  static Clock convert(const protocol::Clock &from);
  static Vector3 convert(const protocol::Vector3 &from);
  static Quaternion convert(const protocol::Quaternion &from);

  static void convert(const protocol::Event &from, Event<Void> &to);
  static void convert(const protocol::Event &from, Event<Clock> &to);
  static void convert(const protocol::Event &from, Event<Vector3> &to);
  static void convert(const protocol::Event &from, Event<Quaternion> &to);

  explicit ProtocolDecoder(std::string annotation);

private:
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PROTOCOL_H
