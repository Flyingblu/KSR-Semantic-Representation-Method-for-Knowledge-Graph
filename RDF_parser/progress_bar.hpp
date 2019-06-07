//
//  progress_bar.hpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#ifndef progress_bar_hpp
#define progress_bar_hpp

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>

using namespace std;

class ProgressBar {
  public:
    ProgressBar(string prompt_message, long long graduation = -1, string save_path = ""):
      message(prompt_message), 
      graduation(graduation), 
      save_path(save_path), 
      start_time(chrono::system_clock::now()) {} ;
      ~ProgressBar() {
        delete this->prog_bar_td;
      }
    void progress_begin();
    void progress_end();
    long long progress = 0;
    
  private:
    string message;
    string save_path;
    long long graduation;
    bool ended = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;
    thread* prog_bar_td;
    void detect_progress();
};

#endif /* progress_bar_hpp */