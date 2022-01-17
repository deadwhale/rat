#include <2ch.hpp>

vvh::dvach::dvach() {
    Json::CharReaderBuilder rbuilder;
    this->reader = rbuilder.newCharReader();
    load_cookies();
    load_config();
    this->domain = this->config["domain"];
}

void vvh::dvach::default_config(std::string file) {
    this->config = {
        {"domain", "https://2ch.hk"},
        {"board", ""},
        {"thread", ""},
        {"name", ""}
    };
    save_config();
}

void vvh::dvach::load_config(std::string file) {
    std::ifstream config_file(file, std::ios::in);

    if (!config_file.is_open()) {
        std::cout << "no config, creating default" << std::endl;
        this->default_config(file);
        std::cin.get();
        return;
    }

    std::string line, key, value;
    std::vector<std::string> splitted;
    while (getline(config_file, line)) {
        splitted = vvh::split(line, "=", 1);
        if (splitted.size() == 2) {
            key = trim(splitted[0]);
            value = trim(splitted[1]);
            this->config[key] = value;
        }
    }
}

void vvh::dvach::save_config(std::string file) {
    std::ofstream config_file(file, std::ios::out);
    for (auto pair : this->config)
        config_file << pair.first << "=" << pair.second << std::endl;
}

cpr::Response vvh::dvach::auth_passlogin(std::string passcode) {
    this->session.SetUrl(cpr::Url{this->domain + "/user/passlogin"});
    this->session.SetMultipart({
        {"passcode", passcode},
        // {"json", 1},
    });
    cpr::Response r = this->session.Post();
    std::cout << r.status_code << " : " << r.text << std::endl;
    return r;
}

cpr::Response vvh::dvach::auth_makaba(std::string passcode) {
    this->session.SetUrl(cpr::Url{this->domain + "/makaba/makaba.fcgi"});
    this->session.SetMultipart({
        {"task", "auth"}, 
        {"usercode", passcode},
        // {"json", 1},
    });
    return this->session.Get();
}

void vvh::dvach::auth(std::string passcode, std::string type, 
                                            std::string file) {
    cpr::Response r;
    if (type == "makaba")
        r = this->auth_makaba(passcode);
    else if (type == "passlogin")
        r = this->auth_passlogin(passcode);
    else 
        r = this->auth_makaba(passcode);
    
    for (auto cook : r.cookies)
        std::cout << cook.first << " : " << cook.second << std::endl;
    this->merge_cookies(r.cookies);
    this->save_cookies(file);
    this->session.SetMultipart({});
}

void vvh::dvach::load_cookies(std::string file) {
    std::ifstream cookie_file(file, std::ios::in);
    if (!cookie_file.is_open()) {
        std::cout << "no cookies" << std::endl;
        return;
    }
    std::string line;
    std::vector<std::string> sline;
    std::map<std::string, std::string> cookie_map;
    for (auto cook : this->cookies)
        cookie_map.insert( cook );
    while (getline(cookie_file, line)) {
        sline = split(line, " : ");
        cookie_map.insert( {sline[0], sline[1]} );
    }
    this->cookies = cpr::Cookies{cookie_map};
    this->session.SetCookies(this->cookies);
}

void vvh::dvach::save_cookies(std::string file) {
    std::ofstream cookie_file(file, std::ios::out);
    for (auto cook : this->cookies)
        cookie_file << cook.first << " : " << cook.second << std::endl;
}

void vvh::dvach::merge_cookies(cpr::Cookies new_cookies) {
    std::map<std::string, std::string> cookie_map;
    for (auto cook : new_cookies)
        cookie_map.insert( cook );
    for (auto cook : this->cookies)
        cookie_map.insert( cook );
    this->cookies = cpr::Cookies{cookie_map};
}

Json::Value vvh::dvach::get_threads(std::string board) {
    if (board == "") {
        std::cout << "wrong board" << std::endl;
        return Json::Value();
    }
    cpr::Url url(this->domain + "/" + board + "/catalog.json");
    this->session.SetUrl(url);
    cpr::Response r = this->session.Get();
    Json::Value json;
    parse_json(&json, r.text);
    return json;
}

Json::Value vvh::dvach::get_posts(std::string board, std::string thread) {
    if (board == "" || thread == "") {
        std::cout << "wrong board or thread" << std::endl;
        return Json::Value();
    }
    cpr::Url url(this->domain + "/" + board + "/res/" + thread + ".json");
    this->session.SetUrl(url);
    cpr::Response r = this->session.Get();
    // std::cout << r.status_code << std::endl;
    Json::Value json;
    parse_json(&json, r.text);
    json["posts"] = json["threads"][0]["posts"];
    json["threads"] = "";
    return json;
}

void vvh::dvach::post(std::string board, 
                      std::string thread, 
                      std::string name,
                      std::string comment, 
                      vvh::files files) {
    if (board == "" || thread == "") {
        std::cout << "wrong board or thread" << std::endl;
        return;
    }
    cpr::Url url{this->domain + "/makaba/posting.fcgi?json=1"};
    this->session.SetUrl(url);
    cpr::Multipart multipart{
                            // {"captcha_type", "passcode"}, 
                            {"task", "post"}, 
                            {"board", board},
                            {"thread", thread},
                            {"name", name},
                            {"comment", comment},
                        };
    for (auto file : files) {
        if (file.first != file.second)
            multipart.parts.push_back(cpr::Part{"file[]", 
                                      vvh::img2buffer(file.first, file.second)});
        else
            multipart.parts.push_back(cpr::Part{"file[]", cpr::File(file.first)});
    }
    this->session.SetMultipart(multipart);
    cpr::Response r = this->session.Post();
    std::cout << r.status_code << " : " << r.text << std::endl;
    this->merge_cookies(r.cookies);
    this->save_cookies();
    this->session.SetMultipart(cpr::Multipart{});
}

void vvh::dvach::post(std::string comment, 
                      vvh::files files) {
    this->post(this->config["board"],
               this->config["thread"],
               this->config["name"],
               comment,
               files);
}

void vvh::dvach::parse_json(Json::Value* json, std::string text) {
    this->reader->parse(text.c_str(), text.c_str() + text.size(), json, NULL);
}

std::vector<Json::Value> vvh::dvach::search_thread(std::string board, std::string thread, 
                                                   vvh::parameters params) {
    Json::Value posts = this->get_posts(board, thread);
    Json::Value post;
    std::vector<Json::Value> result;
    int posts_count = posts["posts_count"].asInt();
    for (int i=0; i<=posts_count; i++) {
        post = posts["posts"][i];
        std::vector<std::string> matches;
        bool match_flag = false;
        for (auto param_set : params){
            if (match(post, param_set)) {
                match_flag = true;
                matches.push_back(param_set.to_string());
            }
        }
        if (match_flag) {
            post["matches"] = vvh::vec2str(matches);
            post["comment"] = vvh::replace(post["comment"].asString(), 
                                               vvh::default_pairs);
            // char stime[50];
            // time_t t = post["timestamp"].asInt();
            // std::strftime(stime, 50, "%c", gmtime(&t));
            // post["timestamp_str"] = stime;
            post["thread"] = thread;
            post["thread_name"] = posts["title"].asString();
            post["board"] = board;
            post["domain"] = this->domain;
            result.push_back(post);
        }
    }
    return result;
}

std::vector<Json::Value> vvh::dvach::search(std::string board, vvh::parameters params) {
    Json::Value threads = this->get_threads(board)["threads"];
    int max = threads.size();
    vvh::progress(0, max, 50);
    std::vector<Json::Value> result;
    for (int i=0; i<max; i++) {
        std::vector<Json::Value> thread_res = this->search_thread(
            board,
            threads[i]["num"].asString(),
            params
        );
        result.insert(result.end(), thread_res.begin(), thread_res.end());
        vvh::progress(i+1, max, 50);
    }
    return result;
}


// TODO: make async methods (get_posts, etc...)
// MB: mutex? mb slow?
std::vector<Json::Value> vvh::dvach::search_async(std::string board, vvh::parameters params) {
    Json::Value threads = this->get_threads(board)["threads"];
    int cpu_count = std::thread::hardware_concurrency();
    int thread_count = threads.size();
    int n = thread_count / cpu_count;
    int r = thread_count % cpu_count;
    int num = 0, offset = 0;
    std::vector<Json::Value> result[cpu_count];
    std::thread searchers[cpu_count];
    for (int i=0; i<cpu_count; i++) {
        num = n + (i < r);
        searchers[i] = std::thread([&](){
            if (!i) vvh::progress(0, num, 50);
            for(int j=offset; j<offset+num; j++) {
                std::vector<Json::Value> thread_res = this->search_thread(
                    board,
                    threads[j]["num"].asString(),
                    params
                );
                result[i].insert(result[i].end(), thread_res.begin(), thread_res.end());
                if (!i) vvh::progress(j-offset+1, num, 50);
            }
        });
        offset += num;
    }
    for (int i=0; i<cpu_count; i++) 
        searchers[i].join();
    for (int i=1; i<cpu_count; i++) 
        result[0].insert(result[0].end(), result[i].begin(), result[i].end());
    return result[0];
}

std::string vvh::dvach::get_param(std::string param) {
    if (this->config[param] != "")
        return this->config[param];
    else
        return vvh::input(param + ": ");
}


// callbacks for menu

void vvh::out_result(std::vector<Json::Value> result) {
    sort(result.begin(), result.end(), vvh::compare_posts);
    std::ofstream out ("result.txt", std::ios::out);
    std::stringstream sout;
    sout << "==== Total number : " << result.size() << " ====" << std::endl;
    sout << std::string(100, '=') << std::endl;
    for (auto post : result) {
        sout << vvh::post2str(post);
    }
    std::cout << sout.str() << std::flush;
    out << sout.str() << std::flush;
    std::cin.get();
}

void vvh::callbacks::search_string(dvach* ch2) {
    std::string board = ch2->get_param("board");
    if (board == "") return;
    vvh::parameters params = {vvh::param_set(vvh::input("params: "))};
    auto result = ch2->search(board, params);
    vvh::out_result(result);
}

void vvh::callbacks::search_file(dvach* ch2) {
    std::string board = ch2->get_param("board");
    if (board == "") return;
    vvh::parameters params = vvh::load_search_params();
    auto result = ch2->search(board, params);
    vvh::out_result(result);
}

void vvh::callbacks::search_thread_string(dvach* ch2) {
    std::string board = ch2->get_param("board");
    if (board == "") return;
    std::string thread = ch2->get_param("thread");
    if (thread == "") return;
    vvh::parameters params = {vvh::param_set(vvh::input("params: "))};
    auto result = ch2->search_thread(board,
                                     thread,
                                     params);
    vvh::out_result(result);
}

void vvh::callbacks::search_thread_file(dvach* ch2) {
    std::string board = ch2->get_param("board");
    if (board == "") return;
    std::string thread = ch2->get_param("thread");
    if (thread == "") return;
    vvh::parameters params = vvh::load_search_params();
    auto result = ch2->search_thread(board,
                                     thread,
                                     params);
    vvh::out_result(result);
}

void vvh::callbacks::get_threads(dvach* ch2) {
    Json::Value threads = ch2->get_threads(vvh::input("board: "));
    auto to_str = [](Json::Value value){
        return value["num"].asString() + " : " + \
               value["subject"].asString();
    };
    std::cout << vvh::json_arr2str(threads["threads"], to_str) << std::endl;
    std::cin.get();
}

void vvh::callbacks::auth(dvach* ch2) {
    ch2->auth(vvh::input("passcode: "), 
              vvh::input("type (makaba, passlogin): "));
    ch2->load_cookies();
    std::cin.get();
}

void vvh::callbacks::parameters(dvach* ch2) {
    std::string temp;
    for (std::string p : {"board", "thread", "name"}) {
        temp = vvh::input(p + ": " + ch2->config[p] + "\n");
        if (temp != "")
            ch2->config[p] = temp;
    }
    ch2->save_config();
}

void vvh::callbacks::domain(dvach* ch2) {
    std::string new_domain = vvh::input("domain (current " + ch2->domain + "): ");
    if (new_domain != "")
        ch2->domain = new_domain;
    ch2->save_config();
}

void vvh::callbacks::post(dvach* ch2) {
    std::string comment = vvh::input("comment: \n");
    vvh::files files;
    std::string fnum = vvh::input("file number: ");
    std::string fname;
    for (int i=0; i<atoi(fnum.c_str()); i++) {
        std::getline(std::cin, fname);
        files.push_back({fname, fname});
    }
    ch2->post(comment, files);
    std::cin.get();
}

void vvh::choicer::start(bool loop) {
    int ch = -1;
    do{
        menu->run(&ch);
        switch(ch){
            case 0:
                vvh::callbacks::search_string(ch2);
                break;
            case 1:
                vvh::callbacks::search_thread_string(ch2);
                break;
            case 2:
                vvh::callbacks::search_file(ch2);
                break;
            case 3:
                vvh::callbacks::search_thread_file(ch2);
                break;
            case 4:
                vvh::callbacks::get_threads(ch2);
                break;
            case 5:
                vvh::callbacks::auth(ch2);
                break;
            case 6:
                vvh::callbacks::parameters(ch2);
                break;
            case 7:
                vvh::callbacks::domain(ch2);
                break;
            case 8:
                vvh::callbacks::post(ch2);
                break;
        }
    } while (loop && ch!=-1);
}

void vvh::dvach::menu() {
    vvh::menu menu ("2ch client", "client for 2ch", {
        "search on board",
        "search in thread",
        "search on board (from file)",
        "search in thread (from file)",
        "get threads",
        "auth",
        "set parameters",
        "set domain",
        "post"
    });
    vvh::choicer ch (&menu, this);
    ch.start();
}
