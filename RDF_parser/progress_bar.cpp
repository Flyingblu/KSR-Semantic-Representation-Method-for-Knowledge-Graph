//
//  progress_bar.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "progress_bar.hpp"

void ProgressBar::progress_begin() {
  cout << endl;
  this->prog_bar_td = new thread(&ProgressBar::detect_progress, this);
}

void ProgressBar::detect_progress() {
  string output = this->message + "\t";
  int cnt = 0;
  bool save = (this->save_path != "");
  ofstream parse_speed_log(this->save_path + "parse_speed.log");
  ofstream memory_log(this->save_path + "memory_usage.log");
  ofstream read_speed_log(this->save_path + "read_speed.log");
  ofstream cpu_log(this->save_path + "cpu.log");
  cpu_log << sysconf(_SC_CLK_TCK) << endl;
  string pid = to_string(getpid());
  string proc_mem_path = "/proc/" + pid + "/statm";
  string proc_read_path = "/proc/" + pid + "/io";
  string proc_cpu_path = "/proc/" + pid + "/stat";
  string tmp;

  auto detect = [&]() -> void {
    printf("%c[2K", 27);
    cout << '\r';
    if(this->graduation != -1) {

      double percentage = double(this->progress) / double(this->graduation);
      cout << setprecision(4) << output << percentage * 100 << '%' << '\r' << flush;

    } else {

      cout << output << this->progress << '\r' << flush;
    }
  };
  
  while(!this->ended) {
    detect();
    this_thread::sleep_for(chrono::milliseconds(50));
    cnt += 1;
    if(save && cnt % 20 == 0) {
      parse_speed_log << this->progress << endl;

      ifstream proc_mem(proc_mem_path);
      getline(proc_mem, tmp);
      proc_mem.close();
      memory_log << tmp << endl;

      ifstream proc_read(proc_read_path);
      getline(proc_read, tmp);
      proc_read.close();
      read_speed_log << tmp << endl;

      ifstream proc_cpu(proc_cpu_path);
      getline(proc_cpu, tmp);
      proc_cpu.close();
      cpu_log << tmp << endl;
      ifstream uptime("/proc/uptime");
      getline(uptime, tmp);
      cpu_log << tmp << endl;

      cnt = 0;
    }
  }
  detect();
  parse_speed_log.close();
  memory_log.close();
  read_speed_log.close();
  cpu_log.close();
}

void ProgressBar::progress_end() {

  if (this->ended) {

      cerr << "ProgressBar: progress already ended! " << endl;
      return;
  }

  this->ended = true;
  this->prog_bar_td->join();
  cout << endl;

  auto end_time = std::chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end_time-this->start_time;
  time_t end_time_t = std::chrono::system_clock::to_time_t(end_time);

  cout << "Finished at: " << ctime(&end_time_t);
  cout << "Elapsed time: " << elapsed_seconds.count() << 's' << endl;
}