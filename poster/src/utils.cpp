#include <utils.hpp>

vvh::param::param(std::string key, std::string value): key(key), value(value) {
    this->is_regex = (this->value.find("r:") == 0);
    if (this->is_regex) {
        this->value = this->value.substr(2, this->value.length()-2);
        this->regex = std::regex(this->value, std::regex::ECMAScript);
    }
}

bool vvh::param::operator< (const param& param) const {
    return this->key < param.key;
}

vvh::param_set::param_set (std::string param_string, std::string delimiter) {
    this->delimiter = delimiter;
    std::vector<std::string> str_params, spl_param;
    str_params = vvh::split(param_string, delimiter);
    for (auto param : str_params) {
        spl_param = split(param, "==");
        if (spl_param.size()==1)
            this->insert(vvh::param("comment", spl_param[0]));
        else
            this->insert(vvh::param(spl_param[0], spl_param[1]));
    }
}

std::string vvh::param_set::to_string() {
    std::stringstream ss;
    vvh::param_set::iterator it = this->begin();
    ss << *(it++);
    for ( ; it != this->end(); it++)
        ss << this->delimiter << *it;
    return ss.str();
}

std::ostream& vvh::operator<<(std::ostream &out, param sp) {
    std::string r_pref = sp.is_regex ? "r:" : "";
    out << sp.key << "==" << r_pref << sp.value;
    return out;

}
std::ostream& vvh::operator<<(std::ostream &out, vvh::param_set ps) {
    out << ps.to_string();
    return out;
}

cpr::Buffer vvh::img2buffer(std::string path, std::string filename) {
    std::vector<char> buf;
    std::ifstream file(path, std::ios::in);
    char c;
    while (file.get(c)) buf.push_back(c);
    u_char a[buf.size()];
    for (int i=0; i<buf.size(); i++) a[i] = (u_char)buf[i]; 
    return cpr::Buffer{a, a+buf.size(), filename.c_str()};
}

std::string vvh::make_link(std::string domain, std::string board, 
                      std::string thread, std::string post) {
    std::stringstream ss;
    ss << domain << "/" << board << "/res/" << thread << ".html#" << post;
    return ss.str(); 
}

std::vector<std::string> vvh::split(std::string s, std::string delimiter, int count) {
    std::vector<std::string> data;
    int prev = 0, cur = s.find(delimiter);
    int k = 0;
    while((cur != std::string::npos) && (k++ != count)){
        data.push_back(s.substr(prev, cur-prev));
        prev = cur + delimiter.length();
        cur = s.find(delimiter, prev);
    }
    data.push_back(s.substr(prev, cur-s.length()));
    return data;
}

std::string vvh::trim(std::string str) {
    const char* spaces = " \t\n\r\f\v";
    str.erase(str.find_last_not_of(spaces) + 1);
    str.erase(0, str.find_first_not_of(spaces));
    return str;
}

vvh::parameters vvh::load_search_params(std::string filename, 
                                        std::string delimiter) {
    std::ifstream file(filename, std::ios::in);
    std::string line;
    vvh::parameters result;
    while (getline(file, line)) {
        line = vvh::trim(line);
        if (line == "" || line.find("#") == 0)
            continue;
        result.push_back(vvh::param_set(line, delimiter));
    }
    return result;
}

bool vvh::match(Json::Value post, param_set param_set) {
    bool result = true;
    std::string temp;
    for (auto param : param_set) {
        if (!result) break;
        temp = post[param.key].asString();
        if (param.is_regex)
            result &= std::regex_search(temp, param.regex);
        else
            result &= (temp.find(param.value) != std::string::npos);
    }
    return result;
}

std::string vvh::replace (std::string string, 
                          replace_pairs pairs) {
    for (auto pair : pairs)
        string = std::regex_replace(string, std::regex(pair.first), pair.second);
    return string;
}

std::string vvh::post2str (Json::Value post) {
    std::stringstream ss;
    std::string link = vvh::make_link(
        post["domain"].asString(),
        post["board"].asString(),
        post["thread"].asString(),
        post["num"].asString()
    );
    ss << link << "\t>>" << post["num"].asString() << std::endl <<
    post["thread_name"].asString() << std::endl <<
    post["date"].asString() << "\t" << 
    post["name"].asString() + post["trip"].asString() << std::endl <<
    post["matches"].asString() << std::endl << std::endl;
    ss << "[" << vvh::json_arr2str(post["files"], "fullname") << "]" << std::endl <<
    post["comment"].asString() << std::endl << 
    std::string(100, '=') << std::endl;
    return ss.str();
}

std::string vvh::vec2str (std::vector<std::string> vec) {
    std::stringstream ss;
    std::vector<std::string>::iterator it = vec.begin();
    ss << "[" << *(it++);
    for (; it != vec.end(); it++)
        ss << ",\n" << *it;
    ss << "]";
    return ss.str();
}

std::string vvh::json_arr2str (Json::Value array, std::string key) {
    if (array.empty()) return "";
    std::stringstream ss;
    auto to_str = [](Json::Value value, std::string key) {
        if (key=="") 
            return value.asString();
        return value[key].asString();
    };
    Json::Value::iterator it = array.begin();
    ss << to_str(*it++, key);
    for (; it != array.end(); it++)
        ss << std::endl << to_str(*it, key);
    return ss.str();
}

std::string vvh::json_arr2str (Json::Value array, std::string (*func)(Json::Value)) {
    if (array.empty()) return "";
    std::stringstream ss;
    Json::Value::iterator it = array.begin();
    ss << func(*it++);
    for (; it != array.end(); it++)
        ss << std::endl << func(*it);
    return ss.str();
}

bool vvh::compare_posts (Json::Value post1, Json::Value post2) {
    return (post1["thread_name"].asString() < post2["thread_name"].asString()) ||
			((post1["thread_name"] == post2["thread_name"]) && (
                post1["timestamp"].asInt() < post2["timestamp"].asInt()
            ));
}

void vvh::progress(int i, int max, int len) {
    int p = round(i/double(max) * len);
    std::cout << '[' << std::string(p, '#') << std::string(len-p, ' ') << ']' <<
            " [" << i << "/" << max << "]\r" << std::flush;
    if (i == max) 
        std::cout << std::endl;
}

std::string vvh::input(std::string string) {
    std::cout << string;
    std::string line;
    std::getline(std::cin, line);
    return line;
}
