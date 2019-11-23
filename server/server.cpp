#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "json_helper.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
namespace pt = boost::property_tree;

class session : public enable_shared_from_this<session> {
public:
  typedef shared_ptr<session> pointer;

  static pointer create(io_service &ios, const string data_file) {
    return pointer(new session(ios, data_file));
  }

  session(io_service &ios, const string data_file)
      : socket_(ios), json_helper_(data_file) {}

  ~session() {}

  tcp::socket &socket() { return socket_; }

  void start() {
    cout << "client connected " << request_.c_array() << endl;
    socket_.async_read_some(buffer(request_, max_length),
                            bind(&session::handle_write, shared_from_this(),
                                 boost::asio::placeholders::error));
  }

private:
  void handle_write(const error_code &error) {
    if (error) {
      cerr << BOOST_CURRENT_FUNCTION << error << endl;
      delete this;
    }
    cout << "receive<-" << request_.c_array();
    if (!json_helper_.close_service(request_.c_array())) {
      response_.assign(0);
      const string response = json_helper_.get_response(request_.c_array());
      copy(response.begin(), response.end(), response_.c_array());
      async_write(socket_, buffer(response_, response_.size()),
                  boost::bind(&session::handle_read, shared_from_this(),
                              boost::asio::placeholders::error));
      cout << "send->: " << response_.c_array() << endl;
    } else {
      stop_service();
    }
  }

  void handle_read(const error_code &error) {
    if (error) {
      cerr << BOOST_CURRENT_FUNCTION << error << endl;
      delete this;
    }
    request_.assign(0);
    socket_.async_read_some(buffer(request_, max_length),
                            boost::bind(&session::handle_write,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
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
  server(io_service &ios, tcp::endpoint endpoint, const string data)
      : acceptor_(ios, endpoint), data_file_(data) {
    start_accept();
  }

private:
  void start_accept() {
    session::pointer new_session =
        session::create(acceptor_.get_io_service(), data_file_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(session::pointer new_session, const error_code &error) {
    if (!error) {
      new_session->start();
      start_accept();
    }
  }

  tcp::acceptor acceptor_;
  const string data_file_;
};

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: server <port>\n";
    return 1;
  }
  try {
    io_service ios;
    auto port = static_cast<unsigned short>(atoi(argv[1]));
    tcp::endpoint endpoint = tcp::endpoint(tcp::v4(), port);
    server srv(ios, endpoint, "server.json");
    ios.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
