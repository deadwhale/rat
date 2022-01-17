#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>
#include <regex>
#include <json/json.h>
#include <cpr/cpr.h>
#include <ctime>
#include <thread>

#include <utils.hpp>
#include <menu.hpp>

// const std::string domain = "https://2ch.hk/";

namespace vvh{
    typedef std::pair<std::string, std::string> file;
    typedef std::vector<vvh::file> files;

    class dvach {

        public:
            dvach();

            void default_config(std::string file = "2ch.conf");
            void save_config(std::string file = "2ch.conf");
            void load_config(std::string file = "2ch.conf");
            // auth variants
            cpr::Response auth_passlogin(std::string passcode);
            cpr::Response auth_makaba(std::string passcode);
            void auth(std::string passcode, std::string type = "makaba", 
                                            std::string file = "cookies_2ch");
            
            void load_cookies(std::string file = "cookies_2ch");
            void save_cookies(std::string file = "cookies_2ch");
            void merge_cookies(cpr::Cookies new_cookies);

            Json::Value get_threads(std::string board);
            Json::Value get_posts(std::string board, std::string thread);

            void post(std::string board, 
                    std::string thread, 
                    std::string name,
                    std::string comment,
                    vvh::files files = {});
            void post(std::string comment,
                    vvh::files files = {});

            std::vector<Json::Value> search_thread(std::string board, std::string thread, 
                                                vvh::parameters params);
                                                
            std::vector<Json::Value> search(std::string board, vvh::parameters params);
            std::vector<Json::Value> search_async(std::string board, vvh::parameters params);

            void menu();

            std::string domain;
            std::map<std::string, std::string> config;

            std::string get_param(std::string param);
            
        private:
            Json::CharReader* reader;
            cpr::Cookies cookies;
            cpr::Session session;
            void parse_json(Json::Value* json, std::string text);
    };


    class choicer {
        public:
            choicer(vvh::menu* _menu,
                       dvach* _ch2) { menu = _menu; ch2 = _ch2; }
            void start(bool loop = true);
        private:
            vvh::menu* menu;
            dvach* ch2;
    };

    void out_result(std::vector<Json::Value> result);

    namespace callbacks {
        void search_string(dvach* ch2);
        void search_file(dvach* ch2);
        void search_thread_string(dvach* ch2);
        void search_thread_file(dvach* ch2);
        void get_threads(dvach* ch2);
        void auth(dvach* ch2);
        void parameters(dvach* ch2);
        void domain(dvach* ch2);
        void post(dvach* ch2);
    }
}
