#ifndef INDOORS_MAGNETICS_LIVE_DEMO_FILTER_H
#define INDOORS_MAGNETICS_LIVE_DEMO_FILTER_H

#include <indoors/magnetics/magnetics_filter.h>
#include <indoors/magnetics/web_server.h>
#include <indoors/pipeline/http.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/platform.h>
#include <indoors/pipeline/protocol.h>

namespace indoors::magnetics {

class LiveDemoFilter final : public pipeline::StandardNode {
public:
  LiveDemoFilter(const MagneticsFilter::Config &config,
                 std::shared_ptr<pipeline::Platform> platform,
                 boost::asio::io_context &ioc);

  MagneticsFilter &magnetics_filter();

  void flush() override;

private:
  MagneticsFilter m_magnetics_filter;
  pipeline::ProtocolEncoder m_encoder;
  pipeline::WebSocket m_web_socket;
  WebServer m_web_server;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_LIVE_DEMO_FILTER_H
