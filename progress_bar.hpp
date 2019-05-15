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

using namespace std;

class ProgressBar {
  public:
    ProgressBar(string prompt_message, int graduation):message(prompt_message), graduation(graduation) {} ;
    void progress_increment(int progress = 1);
    void progress_end();
  private:
    string message;
    int graduation;
    int progress = 0;
    int length = 50;
};

#endif /* progress_bar_hpp */