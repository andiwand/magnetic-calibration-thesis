#ifndef INDOORS_PIPELINE_HTTP_H
#define INDOORS_PIPELINE_HTTP_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <indoors/pipeline/definitions.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/protocol/event.pb.h>

namespace indoors::pipeline {

class WebSocketHandler {
public:
  using tcp_socket = boost::asio::ip::tcp::socket;
  using web_socket = boost::beast::websocket::stream<tcp_socket>;

  virtual ~WebSocketHandler() = default;

  virtual void handle(web_socket &&socket) = 0;
};

class WebSocketInput final : public StandardInput<protocol::Event> {
public:
  using tcp_socket = boost::asio::ip::tcp::socket;
  using web_socket = boost::beast::websocket::stream<tcp_socket>;

  WebSocketInput(std::string annotation, Node *node, web_socket &&websocket);

  void push(protocol::Event event) override;

private:
  web_socket m_websocket;
};

class WebSocket final : public StandardNode, public WebSocketHandler {
public:
  explicit WebSocket(Output<protocol::Event> *output);
  WebSocket(std::string annotation, Output<protocol::Event> *output);

  void handle(web_socket &&socket) override;

private:
  Output<protocol::Event> *m_output;
  std::vector<std::unique_ptr<WebSocketInput>> m_inputs;
};

} // namespace indoors::pipeline

#endif // INDOORS_PIPELINE_HTTP_H
