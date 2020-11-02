#include <indoors/magnetics/web_server.h>
#include <boost/beast.hpp>

namespace indoors::magnetics {

WebServer::WebServer(io_context &ioc, const std::string &address,
                     const std::uint16_t port)
    : WebServer(ioc, address, port, std::optional<std::string>()) {}

WebServer::WebServer(io_context &ioc, const std::string &address,
                     const std::uint16_t port, std::string html_path)
    : WebServer(ioc, address, port,
                std::optional<std::string>(std::move(html_path))) {}

WebServer::WebServer(io_context &ioc, const std::string &address,
                     const std::uint16_t port,
                     std::optional<std::string> html_path)
    : m_acceptor{ioc, {boost::asio::ip::make_address(address), port}},
      m_html_path{std::move(html_path)} {
  accept();
}

void WebServer::set_web_socket_handler(
    pipeline::WebSocketHandler *web_socket_handler) {
  m_web_socket_handler = web_socket_handler;
}

void WebServer::accept() {
  m_acceptor.async_accept(
      [this](const boost::system::error_code &error, tcp_socket socket) {
        if (error)
          return;
        boost::beast::tcp_stream stream(std::move(socket));

        boost::beast::flat_buffer buffer;
        boost::beast::http::request<boost::beast::http::string_body> req;
        boost::beast::http::read(stream, buffer, req);

        if (boost::beast::websocket::is_upgrade(req)) {
          boost::beast::websocket::stream<tcp_socket> ws(stream.release_socket());
          ws.accept(req);
          if (m_web_socket_handler)
            m_web_socket_handler->handle(std::move(ws));
        } else {
          // TODO handle http
        }

        accept();
      });
}

} // namespace indoors::magnetics
