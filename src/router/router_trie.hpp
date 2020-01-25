#ifndef GODA_ROUTER_TRIE_H
#define GODA_ROUTER_TRIE_H
#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace Goda {

    class Node {

    public:
        Node(std::string part, bool isWild):m_part(part), m_isWild(isWild) {}
        Node(){}

        ~Node();

        void clean(std::vector<Node *> childern);

        Node *matchChild(std::string part);

        std::vector<Node *> matchChildren(std::string part);

        void insert(std::string pattern, std::vector<std::string> parts, size_t height);

        Node *search(std::vector<std::string> parts, size_t height);

        std::map<std::string, std::string> getParams(std::string path);

    public:
        std::string m_pattern;
        std::string m_part;
        std::vector<Node *> m_children;
        bool m_isWild;
    };

    typedef std::tuple<Node *, std::map<std::string, std::string>> tuple_ret;
    typedef std::map<std::string, std::string> map_handle;

    class Router {


    public:
        Router(){}

        ~Router();

        std::vector<std::string> parsePattern(std::string pattern, std::string separator = "/");

        void addRoute(std::string method, std::string patter, std::string handler);

        std::string vcStringJoin(std::vector<std::string> vstr, size_t start_pos);

        tuple_ret getRoute(std::string method, std::string path);

        std::string getHandler(std::string method, std::string pattern) {
        HANDLER:
            std::string key = method + "-" + pattern;
            auto pos = m_handlers.find(key);
            if (pos != m_handlers.end()) {
                return m_handlers[key];
            } else {
                if (method != "ANY") {
                    method = "ANY";
                    goto HANDLER;
                }
            }
             
            return "";
        }

    private:
        std::map<std::string, Node *> m_roots;
        map_handle m_handlers;
    };

}

#endif