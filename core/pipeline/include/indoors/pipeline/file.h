#ifndef INDOORS_PIPELINE_FILE_H
#define INDOORS_PIPELINE_FILE_H

#include <fstream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/event.pb.h>

namespace indoors::pipeline {

class FileWriter final : public StandardNode, public Loopable {
public:
  explicit FileWriter(const std::string &path);

  Input<protocol::Event> *input();

  void iterate() override;

private:
  std::ofstream m_file;
  BufferedInput<protocol::Event> m_input;
};

class FileReader final : public StandardNode, public Flushable {
public:
  explicit FileReader(const std::string &path);

  Output<protocol::Event> *output();

  void flush_until(double time) override;

private:
  std::ifstream m_file;
  std::unique_ptr<google::protobuf::io::ZeroCopyInputStream> m_zero_copy;
  google::protobuf::io::CodedInputStream m_coded;
  StandardOutput<protocol::Event> m_output;
  protocol::Event m_last_event;
  bool m_buffered{false};
  bool m_exhausted{false};
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_FILE_H
