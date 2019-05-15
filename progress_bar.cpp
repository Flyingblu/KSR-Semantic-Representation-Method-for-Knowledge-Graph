//
//  progress_bar.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "progress_bar.hpp"

void ProgressBar::progress_increment(int progress) {
  this->progress += progress;
  float percentage = float(this->progress) / float(this->graduation);
  int blocks = percentage * this->length;
  string output = this->message + "\t" + "[";
  for(int i = 0; i < this->length; ++i) {
    if(i < blocks) {
      output.push_back('=');
    } else {
      output.push_back(' ');
    }
  }
  cout << output << ']' << '\r' << flush;
}

void ProgressBar::progress_end() {
  cout << endl;
}