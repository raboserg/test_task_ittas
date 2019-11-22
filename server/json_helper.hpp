#ifndef RESPONSE_CREATOR_HPP
#define RESPONSE_CREATOR_HPP

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

namespace pt = boost::property_tree;
using namespace std;

class json_helper {
public:
  json_helper(const string &file) : file_(file) {}

  bool close_service(const char *request_) {
    const pt::ptree pt_ = create_request_ptree(request_);
    return (pt_.get<string>(OPER) == CLOSE) ? true : false;
  }

  string get_response(const char *request_) {
    string response;
    const pt::ptree pt_ = create_request_ptree(request_);
    const string operation = pt_.get<string>(OPER);
    if (operation == GET_ALL)
      response = get_all_items();
    if (operation == GET_NAME) {
      const string name = pt_.get<string>(NAME);
      response = find_item(name);
      cout << "find " << name << " from " << get_data_file_name() << endl;
    }
    return response;
  }

  string get_all_items() {
    const pt::ptree root = create_root();
    pt::ptree target_tree;
    if (root.empty()) {
      target_tree.put(STATUS, ERROR);
      return get_json_string(target_tree);
    }
    target_tree.put(STATUS, OK);
    target_tree.put_child(pt::ptree::path_type(DATA), root);
    return get_json_string(target_tree);
  }

  string find_item(const string &name) {
    const pt::ptree root = create_root();
    pt::ptree target_tree;
    if (root.empty()) {
      target_tree.put(STATUS, ERROR);
      return get_json_string(target_tree);
    }
    try {
      pt::ptree array_child;
      for (auto &node : boost::make_iterator_range(root.equal_range(""))) {
        if (name == node.second.get<string>(NAME)) {
          pt::ptree array_element;
          array_element.add(NAME, node.second.get<string>(NAME));
          array_element.add(AGE, node.second.get<string>(AGE));
          array_element.add(PHONE, node.second.get<string>(PHONE));
          array_child.push_back(std::make_pair("", array_element));
          break;
        }
      }
      target_tree.put(STATUS, OK);
      target_tree.put_child(pt::ptree::path_type(DATA), array_child);
    } catch (const boost::exception &ex) {
      target_tree.put(STATUS, ERROR);
    }
    return get_json_string(target_tree);
  }

  string get_data_file_name() { return file_; }

private:
  string get_json_string(const pt::ptree &target_tree) {
    ostringstream oss;
    boost::property_tree::write_json(oss, target_tree, false);
    return oss.str();
  }

  pt::ptree create_request_ptree(const char *request_) {
    string response;
    stringstream ss;
    ss << request_;
    pt::ptree pt_;
    boost::property_tree::read_json(ss, pt_);
    return pt_;
  }

  pt::ptree create_root() {
    pt::ptree root_;
    if (file_.empty())
      throw invalid_argument("Invalid filename.");
    pt::read_json(file_, root_);
    return root_;
  }

  const string file_;
  const string OK = "ok";
  const string AGE = "age";
  const string DATA = "data";
  const string NAME = "name";
  const string OPER = "oper";
  const string CLOSE = "close";
  const string PHONE = "phone";
  const string ERROR = "error";
  const string STATUS = "status";
  const string GET_ALL = "get-all";
  const string GET_NAME = "get-name";
};

#endif // RESPONSE_CREATOR_HPP
