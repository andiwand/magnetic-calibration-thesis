#ifndef INDOORS_PIPELINE_NODE_H
#define INDOORS_PIPELINE_NODE_H

#include <indoors/pipeline/series_buffer.h>
#include <memory>
#include <string>
#include <vector>

namespace indoors::pipeline {

template <typename T> class Output;

class Node {
public:
  virtual ~Node() = default;

  virtual const std::string &annotation() const = 0;

  // TODO list inputs/outputs

  virtual void flush() = 0;
};

class Interface {
public:
  virtual ~Interface() = default;

  virtual const std::string &annotation() const = 0;
  virtual Node *node() const = 0;

  virtual bool open() const = 0;

  virtual double time() const = 0;

  virtual std::size_t message_count() const = 0;
  virtual std::size_t byte_count() const = 0;
};

template <typename T> class Input : virtual public Interface {
public:
  ~Input() override = default;

  virtual Output<T> *output() const = 0;

  virtual bool plugged() const = 0;
  virtual void plugged(Output<T> *output) = 0;
  virtual void unplugged() = 0;

  // TODO const
  virtual void push(T data) = 0;
  // TODO naming - blank, proceed, empty, fill, skip, forward, seek
  virtual void skip(double time) = 0;

  // TODO
  // virtual void push(const Series<T> &series) = 0;
};

// TODO rename?
template <typename T> class Output : virtual public Interface {
public:
  ~Output() override = default;

  virtual std::vector<Input<T> *> inputs() const = 0;

  virtual void plug(Input<T> *input) = 0;
  virtual void unplug(Input<T> *input) = 0;
};

class StandardNode : public Node {
public:
  explicit StandardNode(std::string annotation);

  const std::string &annotation() const override;

  void flush() override {}

private:
  const std::string m_annotation;
};

class StandardInterface : virtual public Interface {
public:
  StandardInterface(std::string annotation, Node *node);

  const std::string &annotation() const override;

  Node *node() const override;

  bool open() const override;

  double time() const override;

  std::size_t message_count() const override;

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
  StandardInput(std::string annotation, Node *node)
      : StandardInterface(std::move(annotation), node) {}

  Output<T> *output() const override { return m_output; }

  void plugged(Output<T> *output) override {
    // TODO throw if plugged?
    m_output = output;
    m_time = m_output->time();
  }

  void unplugged() override {
    // TODO throw if unplugged?
    m_output = nullptr;
  }

  bool plugged() const override { return m_output != nullptr; }

  void push(T data) override {
    if (!m_open)
      return;
    ++m_message_count;
  }

  void skip(const double time) override {
    if (!m_open)
      return;
    m_time = time;
  }

  std::size_t byte_count() const override {
    // TODO size of T might be dynamic
    return m_message_count * sizeof(T);
  }

private:
  Output<T> *m_output{nullptr};
};

template <typename T>
class StandardOutput : public StandardInterface, public Output<T> {
public:
  static_assert(EventTraits<T>::has_time, "T needs time");

  StandardOutput(std::string annotation, Node *node)
      : StandardInterface(std::move(annotation), node) {}

  std::vector<Input<T> *> inputs() const override { return m_inputs; }

  void plug(Input<T> *input) override {
    m_inputs.push_back(input);
    input->plugged(this);
  }

  void unplug(Input<T> *input) override {
    auto it = std::find(std::begin(m_inputs), std::end(m_inputs), input);
    if (it == std::end(m_inputs))
      return; // TODO throw?
    m_inputs.erase(it);
    input->unplugged();
  }

  virtual void push(T data) {
    if (!m_open)
      return;
    for (auto &&i : m_inputs) {
      i->push(data);
    }
    m_time = EventTraits<T>::time(data);
    ++m_message_count;
  }

  virtual void skip(const double time) {
    if (!m_open)
      return;
    for (auto &&i : m_inputs) {
      i->skip(time);
    }
    m_time = time;
  }

  std::size_t byte_count() const override {
    // TODO size of T might be dynamic
    return m_message_count * sizeof(T);
  }

private:
  std::vector<Input<T> *> m_inputs;
};

template <typename T> class BufferedInput : public StandardInput<T> {
public:
  BufferedInput(std::string annotation, Node *node)
      : StandardInput<T>(std::move(annotation), node) {}

  std::vector<T> swap() {
    std::vector<T> result;
    std::swap(m_buffer, result);
    return result;
  }

  void push(T data) override {
    m_buffer.push_back(data);
    StandardInput<T>::push(data);
  }

  void skip(const double time) override { StandardInput<T>::skip(time); }

private:
  std::vector<T> m_buffer;
};

template <typename T> class BufferedSeriesInput : public StandardInput<T> {
public:
  BufferedSeriesInput(std::string annotation, Node *node)
      : StandardInput<T>(std::move(annotation), node) {}
  BufferedSeriesInput(std::string annotation, Node *node,
                      const std::size_t initial_capacity)
      : StandardInput<T>(std::move(annotation), node), m_buffer{
                                                           initial_capacity} {}

  SeriesBuffer<T> &buffer() { return m_buffer; }
  const SeriesBuffer<T> &buffer() const { return m_buffer; }

  std::vector<T> swap() { return m_buffer.swap(); }

  void push(T data) override {
    m_buffer.push(data);
    StandardInput<T>::push(data);
    StandardInterface::m_time = m_buffer.time_end();
  }

  void skip(const double time) override {
    m_buffer.skip(time);
    StandardInput<T>::skip(time);
    StandardInterface::m_time = m_buffer.time_end();
  }

private:
  SeriesBuffer<T> m_buffer;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_NODE_H
