#include "menu.hpp"

vvh::menu::menu(){
    title = "";
    description = "";
}

vvh::menu::menu(std::string _title, std::string _description){
    title = _title;
    description = _description;
}

vvh::menu::menu(std::string _title, std::string _description, std::vector<std::string> _items): 
    menu(_title, _description){
    items = _items;
}

vvh::menu::menu(const menu &_menu): 
    menu(_menu.title, _menu.description, _menu.items) {}

vvh::menu::~menu(){
}

void vvh::menu::add(std::string _text){
    items.push_back(_text);
}

void vvh::menu::print(){
    system(CLEAR);
    std::cout << title << std::endl;
    std::cout << description << std::endl;
    std::cout << std::endl;
    for(int i=0; i<items.size(); i++){
        std::cout << i+1 << ")" << items[i] << std::endl;
    }
    std::cout << std::endl
         << "Type 'q' to exit" << std::endl
         << "---------------------------------------------------" << std::endl;
}

void vvh::menu::run(int *_choice){
    int choice=0;
    std::stringstream ss;
    std::string s;
    while(choice<=0 || choice>items.size()){
        print();
        ss.clear();
        std::cin >> s;
        std::cin.ignore();
        if(s=="q"){
            choice=0;
            break;
        }
        ss.str("");
        ss.str(s);
        ss >> choice;
    }
    system(CLEAR);
    if(!choice){
        std::cout << "bye" << std::endl;
    }
    *_choice = choice-1;
}

std::string vvh::menu::getTitle() const{
    return title;
}
