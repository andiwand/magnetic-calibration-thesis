#include <indoors/pipeline/csv.h>
#include <sstream>

namespace indoors::pipeline {

std::string CsvWriter::header(Event<Void>) { return "time"; }

std::string CsvWriter::to_csv(const Event<Void> &event) {
  return std::to_string(event.time);
}

CsvWriter::CsvWriter() : CsvWriter("csv_writer", "") {}

CsvWriter::CsvWriter(std::string annotation, std::string root)
    : m_annotation{std::move(annotation)}, m_root{std::move(root)} {}

CsvWriter::CsvInputBase::CsvInputBase(std::unique_ptr<std::ostream> output)
    : m_output{std::move(output)} {}

void CsvWriter::CsvInputBase::write_line(const std::string &line) {
  *m_output << line << std::endl;
}

void CsvReader::from_csv(const std::string &line, Event<Void> &event) {
  std::istringstream iss(line);
  if (!(iss >> event.time))
    throw; // TODO
}

CsvReader::CsvReader() : CsvReader("csv_reader", "") {}

CsvReader::CsvReader(std::string annotation, std::string root)
    : m_annotation{std::move(annotation)}, m_root{std::move(root)} {}

CsvReader::CsvOutputBase::CsvOutputBase(std::unique_ptr<std::istream> input)
    : m_input{std::move(input)} {}

void CsvReader::flush_until(const double time) {
  for (auto &&o : m_outputs) {
    o->flush_until(time);
  }
}

std::optional<std::string> CsvReader::CsvOutputBase::read_line() {
  std::string result;
  if (!std::getline(*m_input, result))
    return {};
  return result;
}

} // namespace indoors::pipeline
