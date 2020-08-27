#include <indoors/pipeline/http.h>

namespace indoors::pipeline {

using error_code = boost::system::error_code;

WebSocketInput::WebSocketInput(std::string annotation, Node *node,
                               web_socket &&websocket)
    : StandardInput<protocol::Event>(annotation, node), m_websocket{std::move(
                                                              websocket)} {}

void WebSocketInput::push(protocol::Event event) {
  if (!m_open)
    return;
  StandardInput<protocol::Event>::push(event);
  int size = event.ByteSize();
  // TODO keep buffer alive
  char *array = new char[size];
  // TODO when client disconnects this will throw
  event.SerializeToArray(array, size);
  boost::beast::error_code ec;
    m_websocket.write(boost::asio::buffer(array, size), ec);
  if (ec)
    m_open = false;
  delete[] array;
}

WebSocket::WebSocket(io_context &ioc, const std::string &address,
                     std::uint16_t port, Output<protocol::Event> *output)
    : WebSocket("websocket", ioc, address, port, output) {}

WebSocket::WebSocket(std::string annotation, io_context &ioc,
                     const std::string &address, std::uint16_t port,
                     Output<protocol::Event> *output)
    : StandardNode(std::move(annotation)), m_ioc{ioc},
      m_acceptor{ioc, {boost::asio::ip::make_address(address), port}},
      m_output{output} {
  accept();
}

void WebSocket::iterate() {
}

void WebSocket::accept() {
  m_acceptor.async_accept([this](const error_code &error, tcp_socket peer) {
    if (error)
      return;

    // TODO serialization will happen multiple times
    web_socket web_peer(std::move(peer));
    web_peer.accept();
    web_peer.binary(true);

    // TODO remove new
    auto input = new WebSocketInput("websocket input", this, std::move(web_peer));
    m_inputs.push_back(std::unique_ptr<WebSocketInput>(input));
    m_output->plug(input);

    accept();
  });
}

} // namespace indoors::pipeline
