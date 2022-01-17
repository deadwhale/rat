#include <2ch.hpp>
#include <utils.hpp>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <random>

using namespace std;

string run_sample() {
    char buffer[128];
    string result = "";
    FILE* pipe = popen("./sample.py", "r");
    if (!pipe) {
      return "popen failed!";
   }
   while (!feof(pipe)) {
      if (fgets(buffer, 128, pipe) != NULL)
         result += buffer;
   }
   pclose(pipe);
   return result;
}

void posting(vvh::dvach *ch2) {
    string avatar = ch2->config["avatar"];

    mt19937 rng(random_device{}());
    uniform_int_distribution<> rand_min(
        atoi(ch2->config["mmin"].c_str()), 
        atoi(ch2->config["mmax"].c_str())
    );

    string comment;
    while (true) {
        comment = run_sample();
        cout << comment << endl;
        ch2->post(comment,
                 {{avatar, avatar}});
        auto wait = rand_min(rng);
        cout << "wait : " << wait << endl << string(100, '=') << endl;
        this_thread::sleep_for(chrono::minutes(wait));
    }
}

int main(int argc, char** argv) {
    vvh::dvach ch2;

    if (argc == 1) 
        return 0;
    string thread(argv[1]);

    ch2.config["thread"] = thread;
    
    cout << ch2.config["board"] << endl <<
            ch2.config["thread"] << endl <<
            ch2.config["name"] << endl <<
            avatar << endl << string(100, '=') << endl;

    
}
