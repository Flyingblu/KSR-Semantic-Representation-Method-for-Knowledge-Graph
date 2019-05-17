//
//  progress_bar.hpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#ifndef progress_bar_hpp
#define progress_bar_hpp

#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>

using namespace std;

class ProgressBar {
  public:
    ProgressBar(string prompt_message, long long graduation = -1):
      message(prompt_message), 
      graduation(graduation), 
      start_time(chrono::system_clock::now()) {
        cout << endl;
      } ;
      ~ProgressBar() {
        delete this->prog_bar_td;
      }
    void progress_increment(long long progress = 1);
    void progress_begin();
    void progress_end();
    long long progress = 0;
    
  private:
    string message;
    long long graduation;
    bool ended = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    thread* prog_bar_td;
    void detect_progress();
};

#endif /* progress_bar_hpp */