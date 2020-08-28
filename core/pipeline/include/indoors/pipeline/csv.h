#ifndef INDOORS_PIPELINE_CSV_H
#define INDOORS_PIPELINE_CSV_H

#include <fstream>
#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/event.h>
#include <indoors/pipeline/node.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace indoors::pipeline {

class CsvWriter final : public Node,
                        public std::enable_shared_from_this<CsvWriter> {
public:
  static std::string header(Event<Void>);

  static std::string to_csv(const Event<Void> &event);

  CsvWriter();
  CsvWriter(std::string annotation, std::string root);

  const std::string &annotation() const override { return m_annotation; }

  template <typename T>
  std::shared_ptr<Input<T>> create_input(std::string annotation) {
    // TODO use c++ paths?
    std::string path = annotation + ".csv";
    if (!m_root.empty())
      path = m_root + "/" + path;
    return create_input<T>(std::move(annotation), path);
  }

  template <typename T>
  std::shared_ptr<Input<T>> create_input(std::string annotation,
                                         const std::string &path) {
    return std::make_shared<CsvInput<T>>(this, std::move(annotation),
                                         std::make_unique<std::ofstream>(path));
  }

private:
  const std::string m_annotation;
  const std::string m_root;

  class CsvInputBase {
  public:
    explicit CsvInputBase(std::unique_ptr<std::ostream> output);
    void write_line(const std::string &line);

    const std::unique_ptr<std::ostream> m_output;
  };

  template <typename T>
  class CsvInput : public StandardInput<T>, public CsvInputBase {
  public:
    CsvInput(CsvWriter *csv_writer, std::string annotation,
             std::unique_ptr<std::ostream> output)
        : StandardInput<T>(std::move(annotation), csv_writer),
          CsvInputBase(std::move(output)) {
      CsvInputBase::write_line(header(T()));
    }

    void push(T data) override {
      CsvInputBase::write_line(to_csv(data));
      StandardInput<T>::push(data);
    }
  };
};

class CsvReader final : public Node,
                        public Flushable,
                        public std::enable_shared_from_this<CsvReader> {
public:
  static void from_csv(const std::string &line, Event<Void> &event);

  CsvReader();
  CsvReader(std::string annotation, std::string root);

  const std::string &annotation() const override { return m_annotation; }

  template <typename T> Output<T> *create_output(std::string annotation) {
    // TODO use c++ paths?
    std::string path = annotation + ".csv";
    if (!m_root.empty())
      path = m_root + "/" + path;
    return create_output<T>(std::move(annotation), path);
  }

  template <typename T>
  Output<T> *create_output(std::string annotation, const std::string &path) {
    // TODO lifetime
    return new CsvOutput<T>(this, std::move(annotation),
                            std::make_unique<std::ifstream>(path));
  }

  void flush_until(double time) override;

private:
  class CsvOutputBase;

  const std::string m_annotation;
  const std::string m_root;
  const std::vector<CsvOutputBase *> m_outputs;

  class CsvOutputBase : public Flushable {
  public:
    explicit CsvOutputBase(std::unique_ptr<std::istream> input);
    void flush_until(double time) override = 0;
    std::optional<std::string> read_line();

    const std::unique_ptr<std::istream> m_input;
  };

  template <typename T>
  class CsvOutput : public StandardOutput<T>, public CsvOutputBase {
  public:
    CsvOutput(CsvReader *csv_reader, std::string annotation,
              std::unique_ptr<std::istream> input)
        : StandardOutput<T>(std::move(annotation), csv_reader),
          CsvOutputBase(std::move(input)) {
      const auto line = read_line();
      if (!line)
        throw; //
      if (*line != CsvWriter::header(T()))
        throw; // TODO
      m_next = next();
    }

    void flush_until(const double time) override {
      while (m_next && m_next->time < time) {
        StandardOutput<T>::push(*m_next);
        m_next = next();
      }

      StandardOutput<T>::skip(time);
    }

  private:
    std::optional<T> m_next;

    std::optional<T> next() {
      const auto line = read_line();
      if (!line)
        return {};
      T result;
      from_csv(*line, result);
      return result;
    }
  };
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_CSV_H
