#ifndef INDOORS_PIPELINE_NODE_H
#define INDOORS_PIPELINE_NODE_H

#include <indoors/pipeline/buffer.h>
#include <memory>
#include <string>
#include <vector>

namespace indoors::pipeline {

template <typename T> class Output;

class Node {
public:
  virtual ~Node() = default;

  virtual const std::string &annotation() const noexcept = 0;
};

class Interface {
public:
  virtual ~Interface() = default;

  virtual const std::string &annotation() const noexcept = 0;
  virtual Node *node() const noexcept = 0;

  virtual bool open() const noexcept = 0;

  virtual double time() const noexcept = 0;

  virtual std::size_t message_count() const noexcept = 0;
  virtual std::size_t byte_count() const noexcept = 0;
};

template <typename T> class Input : virtual public Interface {
public:
  ~Input() override = default;

  virtual Output<T> *output() const noexcept = 0;

  virtual bool plugged() const noexcept = 0;
  virtual void plugged(Output<T> *output) noexcept = 0;
  virtual void unplugged() noexcept = 0;

  // TODO const
  virtual void push(T data) noexcept = 0;
  // TODO naming - blank, proceed, empty, fill, skip, forward, seek
  virtual void skip(double time) noexcept = 0;

  // TODO
  // virtual void push(const Series<T> &series) noexcept = 0;
};

// TODO rename?
template <typename T> class Output : virtual public Interface {
public:
  ~Output() override = default;

  virtual std::vector<Input<T> *> inputs() const noexcept = 0;

  virtual void plug(Input<T> *input) noexcept = 0;
  virtual void unplug(Input<T> *input) noexcept = 0;
};

class StandardNode : public Node {
public:
  explicit StandardNode(std::string annotation) noexcept;

  const std::string &annotation() const noexcept override;

private:
  const std::string m_annotation;
};

class StandardInterface : virtual public Interface {
public:
  StandardInterface(std::string annotation, Node *node) noexcept;

  const std::string &annotation() const noexcept override;

  Node *node() const noexcept override;

  bool open() const noexcept override;

  double time() const noexcept override;

  std::size_t message_count() const noexcept override;

protected:
  bool m_open{true};
  double m_time{0};
  std::size_t m_message_count{0};

private:
  const std::string m_annotation;
  Node *const m_node{nullptr};
};

template <typename T>
class StandardInput : public StandardInterface, public Input<T> {
public:
  StandardInput(std::string annotation, Node *node) noexcept
      : StandardInterface(std::move(annotation), node) {}

  Output<T> *output() const noexcept override { return m_output; }

  void plugged(Output<T> *output) noexcept override {
    // TODO throw if plugged?
    m_output = output;
    m_time = m_output->time();
  }

  void unplugged() noexcept override {
    // TODO throw if unplugged?
    m_output = nullptr;
  }

  bool plugged() const noexcept override { return m_output != nullptr; }

  void push(T data) noexcept override {
    if (!m_open)
      return;
    ++m_message_count;
  }

  void skip(const double time) noexcept override {
    if (!m_open)
      return;
    m_time = time;
  }

  std::size_t byte_count() const noexcept override {
    // TODO size of T might be dynamic
    return m_message_count * sizeof(T);
  }

private:
  Output<T> *m_output{nullptr};
};

template <typename T>
class StandardOutput : public StandardInterface, public Output<T> {
public:
  StandardOutput(std::string annotation, Node *node) noexcept
      : StandardInterface(std::move(annotation), node) {}

  std::vector<Input<T> *> inputs() const noexcept override { return m_inputs; }

  void plug(Input<T> *input) noexcept override {
    m_inputs.push_back(input);
    input->plugged(this);
  }

  void unplug(Input<T> *input) noexcept override {
    auto it = std::find(std::begin(m_inputs), std::end(m_inputs), input);
    if (it == std::end(m_inputs)) return; // TODO throw?
    m_inputs.erase(it);
    input->unplugged();
  }

  virtual void push(T data) noexcept {
    if (!m_open)
      return;
    for (auto &&i : m_inputs) {
      i->push(data);
    }
    ++m_message_count;
  }

  virtual void skip(const double time) noexcept {
    if (!m_open)
      return;
    for (auto &&i : m_inputs) {
      i->skip(time);
    }
    m_time = time;
  }

  std::size_t byte_count() const noexcept override {
    // TODO size of T might be dynamic
    return m_message_count * sizeof(T);
  }

private:
  std::vector<Input<T> *> m_inputs;
};

template <typename T> class BufferedInput : public StandardInput<T> {
public:
  BufferedInput(std::string annotation, Node *node) noexcept
      : StandardInput<T>(std::move(annotation), node) {}
  BufferedInput(std::string annotation, Node *node,
                const std::size_t initial_capacity) noexcept
      : StandardInput<T>(std::move(annotation), node), m_buffer{
                                                           initial_capacity} {}

  Buffer<T> buffer() noexcept { return m_buffer; }
  const Buffer<T> &buffer() const noexcept { return m_buffer; }

  std::vector<T> swap() noexcept { return m_buffer.swap(); }

  void push(T data) noexcept override {
    m_buffer.push(data);
    StandardInput<T>::push(data);
  }

  void skip(const double time) noexcept override {
    m_buffer.skip(time);
    StandardInput<T>::skip(time);
  }

private:
  Buffer<T> m_buffer;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_NODE_H
