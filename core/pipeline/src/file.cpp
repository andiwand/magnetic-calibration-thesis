#include <google/protobuf/util/delimited_message_util.h>
#include <indoors/pipeline/file.h>

namespace indoors::pipeline {

FileWriter::FileWriter(const std::string &path)
    : StandardNode("file writer"), m_file{path}, m_input{"file writer input",
                                                         this} {}

Input<protocol::Event> *FileWriter::input() { return &m_input; }

void FileWriter::flush() {
  auto &&events = m_input.swap();

  for (auto &&event : events) {
    google::protobuf::util::SerializeDelimitedToOstream(event, &m_file);
  }
}

FileReader::FileReader(const std::string &path)
    : StandardNode("file reader"), m_file{path},
      m_zero_copy{
          std::make_unique<google::protobuf::io::IstreamInputStream>(&m_file)},
      m_coded{m_zero_copy.get()}, m_output{"file reader output", this} {}

Output<protocol::Event> *FileReader::output() { return &m_output; }

bool FileReader::exhausted() const { return m_exhausted; }

void FileReader::flush() {
  flush_until(std::numeric_limits<double>::max());
}

void FileReader::flush_until(const double time) {
  while (!m_exhausted) {
    if (!m_buffered) {
      m_last_event.Clear();
      google::protobuf::util::ParseDelimitedFromCodedStream(
          &m_last_event, &m_coded, &m_exhausted);
      if (m_exhausted) return;
      m_buffered = true;
    }

    if (m_last_event.t() >= time) break;

    m_output.push(m_last_event);
    m_buffered = false;
  }
}

} // namespace indoors::pipeline
