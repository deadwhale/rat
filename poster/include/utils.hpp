#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <set>
#include <map>
#include <vector>
#include <regex>
#include <cpr/cpr.h>
#include <json/json.h>
#include <cmath>

namespace vvh {
    
    struct param {
        param(std::string key, std::string value);
        bool operator< (const param& param) const;
        std::string key;
        std::string value;
        std::regex regex;
        bool is_regex;
    };
    struct param_set: std::set<param> {
        std::string delimiter;
        param_set(std::string param_string, std::string delimiter = "; ");
        void from_string(std::string param_string);
        std::string to_string();
    };
    typedef std::vector<param_set> parameters;
    
    std::ostream& operator<<(std::ostream &out, param sp);
    std::ostream& operator<<(std::ostream &out, param_set ps);

    cpr::Buffer img2buffer(std::string path, std::string filename);
    std::string make_link(std::string domain, std::string board, 
                          std::string thread, std::string post);
    std::vector<std::string> split(std::string s, std::string  delimiter = " ", int count = -1);
    std::string trim(std::string str);
    parameters load_search_params(std::string filename = "search_params.txt", 
                                  std::string delimiter = "; ");
    
    bool match(Json::Value post, param_set param_set);

    typedef std::vector<std::pair<std::string, std::string>> replace_pairs;
    const replace_pairs default_pairs {
        {R"(<br>)",     "\n"},
        {R"(&gt;)",     ">"},
        {R"(&lt;)",     "<"},
        {R"(&#47;)",    "/"},
        {R"(&quot;)",   "\""},
        {R"(&#39;)",    "'"},
        {R"(<[^>]*>)",  ""}
    };

    std::string replace (std::string string, 
                         replace_pairs pairs);

    std::string post2str (Json::Value post);
    std::string vec2str (std::vector<std::string> vec);
    std::string json_arr2str (Json::Value array, std::string key = "");
    std::string json_arr2str (Json::Value array, std::string (*func)(Json::Value));

    bool compare_posts (Json::Value post1, Json::Value post2);

    void progress(int i, int max, int len=50);

    std::string input(std::string string = "");

}
