#include <iostream>
#include "router_trie.hpp"

using namespace Goda;

Node *Node::matchChild(std::string part) {
    for (auto c : m_children) {
        if (c->m_part == part || c->m_isWild) {
            return c;
        }
    }
    return nullptr;
}

std::vector<Node *> Node::matchChildren(std::string part) {
    std::vector<Node *> nodes;

    Node *node = matchChild(part);

    if (node) {
        nodes.push_back(node);
    }

    return nodes;
}

void Node::insert(std::string pattern, std::vector<std::string> parts, int height) {
    if (parts.size() == height) {
        m_pattern = pattern;
        return;
    }

    std::string part = parts[height];
    Node *child = matchChild(part);
    if (child == nullptr) {
        child = new Node(part, part[0] == ':' || part[0] == '*');
        m_children.push_back(child);
    }

    child->insert(pattern, parts, height + 1);
}

Node *Node::search(std::vector<std::string> parts, int height) {
    if (parts.size() == height || m_part[0] == '*') {
        if (m_pattern.empty()) {
            return nullptr;
        }
        return this;
    }

    std::string part = parts[height];
    std::vector<Node *> children = matchChildren(part);

    for (auto child : children) {
        Node *result = child->search(parts, height + 1);
        if (result != nullptr) {
            return result;
        }
    }

    return nullptr;
}

Node::~Node() {
    clean(m_children);
}

void Node::clean(std::vector<Node *> childern) {
    for (auto node : childern) {
        std::cout << "我在删除：" + node->m_part + " pattern :" + node->m_pattern << std::endl;
        delete node;
    }
}
        
Router::~Router(){
    for (auto r : m_roots) {
        Node *node = r.second;
        delete node;
    }
}

void Router::addRoute(std::string method, std::string pattern, std::string handler) {
    auto parts = parsePattern(pattern);
    
    std::string key = method + '-' + pattern;
    auto iter = m_roots.find(method);
    if (iter == m_roots.end()) {
        m_roots[method] = new Node();
    }

    m_roots[method]->insert(pattern, parts, 0);
    m_handlers[key] = handler;
}

tuple_ret Router::getRoute(std::string method, std::string path) {
    auto searchParts = parsePattern(path);

    Node *root = nullptr;

    tuple_ret ret;

    auto iter = m_roots.find(method);
    if (iter != m_roots.end()) {
        root = m_roots[method];
    }

    auto n = root->search(searchParts, 0);
    std::map<std::string, std::string> params;

    if (n != nullptr) {
        auto parts = parsePattern(n->m_pattern);
        for (size_t i = 0; i < parts.size(); i++) {
            std::string part = parts[i];
            if (part[0] == ':') {
                params[part.substr(1, part.length()-1)] = searchParts[i];
            }
            if (part[0] == '*' && part.length() > 1) {
                params[part.substr(1, part.length()-1)] = vcStringJoin(searchParts, i);
            }
        }

        ret = std::make_tuple(n, params);
        return ret;
    }
    return ret;
}

std::string Router::vcStringJoin(std::vector<std::string> vstr, int start_pos) {
    std::string new_str;
    size_t len = vstr.size();
    for (size_t i = 0; i < len; i++) {
        if (i >= start_pos) {
            new_str += vstr[i] + "/";
        }
    }

    return new_str.substr(0, new_str.length()-1);
}


std::vector<std::string> Router::parsePattern(std::string pattern, std::string separator) {
    std::vector<std::string> vs;
    if (pattern == "") {
            return vs;
        }

        std::string str = pattern + separator;

        size_t size = str.size();
        size_t pos = str.find(separator);

        while (pos != str.npos) {
            std::string temp = str.substr(0, pos);
            if (temp != "") {
                vs.push_back(temp);
            }
            
            str = str.substr(pos + 1, size);
            pos = str.find(separator);
        }
        return vs;
}