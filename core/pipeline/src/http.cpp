#include <indoors/pipeline/http.h>

namespace indoors::pipeline {

using error_code = boost::system::error_code;

WebSocketInput::WebSocketInput(std::string annotation, Node *node,
                               web_socket &&websocket)
    : StandardInput<protocol::Event>(std::move(annotation), node), m_websocket{std::move(
                                                            websocket)} {
  m_websocket.binary(true);
}

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

WebSocket::WebSocket(Output<protocol::Event> *output)
    : WebSocket("websocket", output) {}

WebSocket::WebSocket(std::string annotation, Output<protocol::Event> *output)
    : StandardNode(std::move(annotation)), m_output{output} {}

void WebSocket::handle(web_socket &&socket) {
  // TODO remove new
  auto input = new WebSocketInput("websocket input", this, std::move(socket));
  m_inputs.push_back(std::unique_ptr<WebSocketInput>(input));
  m_output->plug(input);
}

} // namespace indoors::pipeline
