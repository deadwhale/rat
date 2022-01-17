#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

namespace vvh{

    class menu{

        public:

            menu();
            menu(std::string _title, std::string _description);
            menu(std::string _title, std::string _description, std::vector<std::string> _items);
            menu(const menu &_menu);
            ~menu();

            void setTitle(std::string _title);
            void setDescription(std::string _description);
            void add(std::string _title);

            int length() const;
            std::string getTitle() const;
            std::string getDescription() const;
            std::string remove(int index);
            std::string& operator[](int index);

            void print();
            void run(int *_choice);

            void clear();

        private:

            std::string title;
            std::string description;
            std::vector<std::string> items;

    };
}