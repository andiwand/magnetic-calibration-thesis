#ifndef INDOORS_PIPELINE_HTTP_H
#define INDOORS_PIPELINE_HTTP_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/message.pb.h>

namespace indoors::pipeline {

class HttpServerEvents : public StandardNode {
public:
private:
};

class WebSocketInput : public StandardInput<protocol::Event> {
public:
  using tcp_socket = boost::asio::ip::tcp::socket;
  using web_socket = boost::beast::websocket::stream<tcp_socket>;

  WebSocketInput(std::string annotation, Node *node, web_socket &&websocket);

  void push(protocol::Event event) noexcept override;

private:
  web_socket m_websocket;
};

class WebSocket : public StandardNode, public Loopable {
public:
  using io_context = boost::asio::io_context;

  WebSocket(io_context &ioc, const std::string &address, std::uint16_t port,
            Output<protocol::Event> *output);
  WebSocket(std::string annotation, io_context &ioc, const std::string &address,
            std::uint16_t port, Output<protocol::Event> *output);

  void iterate(std::uint32_t index) override;

private:
  using acceptor = boost::asio::ip::tcp::acceptor;
  using tcp_socket = boost::asio::ip::tcp::socket;
  using web_socket = boost::beast::websocket::stream<tcp_socket>;

  void accept();

  Output<protocol::Event> *m_output;
  io_context &m_ioc;
  acceptor m_acceptor;
  std::vector<std::unique_ptr<WebSocketInput>> m_inputs;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_HTTP_H
