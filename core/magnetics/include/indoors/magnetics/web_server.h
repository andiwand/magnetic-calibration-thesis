#ifndef INDOORS_MAGNETICS_WEB_SERVER_H
#define INDOORS_MAGNETICS_WEB_SERVER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdint>
#include <indoors/pipeline/http.h>
#include <optional>
#include <string>

namespace indoors::magnetics {

class WebServer final {
public:
  using io_context = boost::asio::io_context;

  WebServer(io_context &ioc, const std::string &address, std::uint16_t port);
  WebServer(io_context &ioc, const std::string &address, std::uint16_t port,
            std::string html_path);

  void set_web_socket_handler(pipeline::WebSocketHandler *);

private:
  using acceptor = boost::asio::ip::tcp::acceptor;
  using tcp_socket = boost::asio::ip::tcp::socket;
  using web_socket = boost::beast::websocket::stream<tcp_socket>;

  WebServer(io_context &ioc, const std::string &address, std::uint16_t port,
            std::optional<std::string> html_path);

  void accept();

  acceptor m_acceptor;
  const std::optional<std::string> m_html_path;
  pipeline::WebSocketHandler *m_web_socket_handler{nullptr};
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_WEB_SERVER_H
