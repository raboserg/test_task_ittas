#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "json_helper.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;
using namespace boost::asio;
using namespace boost::asio::ip;

class session : public enable_shared_from_this<session> {
public:
  session(io_context &io_context, const string data_file)
      : socket_(io_context), json_helper_(data_file) {}

  ~session() { stop_service(); }

  tcp::socket &socket() { return socket_; }

  void start() {
    cout << "client connected " << request_.c_array() << endl;
    socket_.async_read_some(buffer(request_, max_length),
                            bind(&session::handle_write, this,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred));
  }

private:
  void handle_write(const boost::system::error_code &error, size_t size) {
    if (error)
      delete this;
    cout << "receive<-" << request_.c_array();
    if (!json_helper_.close_service(request_.c_array())) {
      const string response = json_helper_.get_response(request_.c_array());
      copy(response.begin(), response.end(), response_.c_array());
      boost::asio::async_write(socket_, buffer(response_, max_length),
                               boost::bind(&session::handle_read, this,
                                           boost::asio::placeholders::error));
      cout << "send->: " << response_.c_array() << endl;
      response_.assign(0);
    } else {
      delete this;
    }
  }

  void handle_read(const boost::system::error_code &error) {
    if (error)
      delete this;
    request_.assign(0);
    socket_.async_read_some(
        buffer(request_, max_length),
        boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

  void stop_service() {
    socket_.close();
    socket_.get_io_context().stop();
    cout << endl << "Close service." << endl;
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  boost::array<char, max_length> request_;
  boost::array<char, max_length> response_;
  json_helper json_helper_;
};

class server {
public:
  server(boost::asio::io_context &io_context, short port, const string data)
      : io_context_(io_context), data_file(data),
        acceptor_(io_context,
                  tcp::endpoint(tcp::v4(), static_cast<unsigned short>(port))) {
    start_accept();
  }

private:
  void start_accept() {
    session *new_session = new session(io_context_, data_file);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(session *new_session,
                     const boost::system::error_code &error) {
    if (!error) {
      new_session->start();
    } else {
      delete new_session;
    }
    start_accept();
  }

  boost::asio::io_context &io_context_;
  const string data_file;
  tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: server <port>\n";
    return 1;
  }
  try {
    boost::asio::io_context io_context;
    server srv(io_context, static_cast<short>(std::atoi(argv[1])),
               "server.json");
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
