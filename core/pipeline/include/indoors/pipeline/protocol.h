#ifndef INDOORS_PIPELINE_PROTOCOL_H
#define INDOORS_PIPELINE_PROTOCOL_H

#include <indoors/pipeline/message.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/message.pb.h>

namespace indoors::pipeline {

class ProtocolEncoder final : public StandardNode {
public:
  static protocol::Void convert(const Void &from);
  static protocol::Clock convert(const Clock &from);
  static protocol::Vector3 convert(const Vector3 &from);

  static protocol::Event convert(const Event<Void> &from);
  static protocol::Event convert(const Event<Clock> &from);
  static protocol::Event convert(const Event<Vector3> &from);

  ProtocolEncoder();
  explicit ProtocolEncoder(std::string annotation);
  ~ProtocolEncoder() override;

  template <typename T> Input<T> *create_input(std::string annotation) {
    const auto channel_id = m_next_channel_id++;
    auto input = new EncoderInput<T>(this, std::move(annotation), channel_id);
    m_inputs.push_back(std::unique_ptr<EncoderInput<T>>(input));
    return input;
  }

  Output<protocol::Event> *output();

private:
  class EncoderInputBase : virtual public Interface {
  public:
    explicit EncoderInputBase(std::uint32_t channel_id);

    virtual protocol::Event example() noexcept = 0;
    protocol::ChannelHello hello() noexcept;

    void push(protocol::Event event) noexcept;

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

    protocol::Event example() noexcept override { return convert(T()); }

    void push(T data) noexcept override {
      EncoderInputBase::push(convert(data));
      StandardInput<T>::push(data);
    }
  };

  class EncoderOutput : public StandardOutput<protocol::Event> {
  public:
    EncoderOutput(std::string annotation, Node *node) noexcept;
    void plug(Input<protocol::Event> *input) noexcept override;
    void unplug(Input<protocol::Event> *input) noexcept override;
    void push(protocol::Event data) noexcept override;
  };

  protocol::Event hello() noexcept;
  protocol::Event bye() noexcept;
  void push(protocol::Event data) noexcept;

  std::uint32_t m_next_channel_id{0};

  std::vector<std::unique_ptr<EncoderInputBase>> m_inputs;
  EncoderOutput m_output;
};

class ProtocolDecoder final : public StandardNode {
public:
  static Void convert(const protocol::Void &from);
  static Clock convert(const protocol::Clock &from);
  static Vector3 convert(const protocol::Vector3 &from);

  static void convert(const protocol::Event &from, Event<Void> &to);
  static void convert(const protocol::Event &from, Event<Clock> &to);
  static void convert(const protocol::Event &from, Event<Vector3> &to);

  explicit ProtocolDecoder(std::string annotation);

private:
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_PROTOCOL_H
