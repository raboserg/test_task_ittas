#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
namespace pt = boost::property_tree;

enum { MAX_LENGTH = 1024 };

string get_json_string(pt::ptree &target_tree) {
  ostringstream oss;
  boost::property_tree::write_json(oss, target_tree, false);
  return oss.str();
}

string create_request(const string &cmd) {
  string request;
  pt::ptree pt;
  if (cmd == "close") {
    pt.put("oper", "close");
    request = get_json_string(pt);
  } else {
    const string token = cmd.substr(cmd.find_first_of(" ") + 1);
    if (token == "--all") {
      pt.put("oper", "get-all");
      request = get_json_string(pt);
    } else if (token.substr(0, token.find(" ")) == "--get-name") {
      string name = token.substr(token.find_first_of(" ") + 1);
      if (!name.empty() && name != " " && name != "--get-name") {
        for (const auto c : {'"', ' '})
          name.erase(std::remove(name.begin(), name.end(), c), name.end());
        pt.put("oper", "get-name");
        pt.put("name", name);
        request = get_json_string(pt);
      } else
        throw invalid_argument("Invalid command: input name.");
    } else
      throw invalid_argument(
          "Invalid command: get --all; get --get-name \"Xxxx\"; close.");
  }
  return request;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    cerr << "Usage: client <host> <port>\n";
    return 1;
  }
  //  io_context io_context;
  //  tcp::resolver resolver(io_context);
  //  tcp::resolver::results_type endpoints =
  //      resolver.resolve(tcp::v4(), argv[1], argv[2]);

  //  tcp::socket client_socket(io_context);
  //  connect(client_socket, endpoints);
  io_service ios;
  tcp::socket client_socket(ios);
  tcp::resolver resolver(ios);
  connect(client_socket, resolver.resolve({argv[1], argv[2]}));

  bool session = true;
  while (session) {
    cout << "cmd > ";
    string cmd;
    getline(std::cin, cmd);
    if (cmd == "close")
      session = false;
    try {
      const string request = create_request(cmd);
      write(client_socket, buffer(request, request.length()));
      cout << "send ->  " << request;
      char response[MAX_LENGTH];
      size_t reply_length =
          client_socket.read_some(buffer(response, MAX_LENGTH));
      if (reply_length > 0) {
        cout << "receive <- " << response;
      }
    } catch (exception &ex) {
      cerr << ex.what() << "\n";
    }
  }
  return 0;
}
