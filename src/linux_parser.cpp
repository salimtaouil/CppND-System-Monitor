#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string key, value;
  string memfree, memtotal;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if(key == "MemTotal:") memtotal = value;
    if(key == "MemFree:") memfree = value;
    }
  }
  return (stof(memtotal) - stof(memfree)) / stof(memtotal); 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime_idl, uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> uptime_idl;
  } 
  return std::stol(uptime); 
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  string cpu, system, user, nice;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system;
      if (cpu == "cpu") return std::stol(system);
    }
  }
  return std::stol(system); 
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  string cpu, system, user, nice, idle, iowait, irq, softirq, steal, guest, guest_nice;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
      if (cpu == "cpu") return std::stol(user) + std::stol(nice) + std::stol(irq) + std::stol(softirq) + std::stol(steal);
    }
  }
  return std::stol(user) + std::stol(nice) + std::stol(irq) + std::stol(softirq) + std::stol(steal); 
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string cpu, system, user, nice, idle, iowait, irq, softirq, steal, guest, guest_nice;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
      if (cpu == "cpu") return std::stol(idle) + std::stol(iowait);
    }
  }
  return std::stol(idle) + std::stol(iowait); 
}

// TODO: Read and return CPU utilization
float LinuxParser::CpuUtilization(int pid) {
  vector<int> pids = LinuxParser::Pids();
  string value;
  string line;
  long utime, stime, cutime, cstime, starttime;
  long total_time;
  float seconds, cpu_usage;
  for(auto id : pids){
    if(id == pid){
      std::ifstream stream(kProcDirectory + to_string(id) + "/" + kStatFilename);
      if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        for (auto i = 1; i <= 22; i++){
          linestream >> value;
          if (i == 14) utime = std::stol(value);
          if (i == 15) stime = std::stol(value);
          if (i == 16) cutime = std::stol(value);
          if (i == 17) cstime = std::stol(value);
          if (i == 22) starttime = std::stol(value);
        }
        total_time = utime + stime + cutime + cstime;
        seconds = LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
        cpu_usage = 100 * ((total_time / sysconf(_SC_CLK_TCK))/seconds);
        return cpu_usage/100;
      }
    }
  }
  return cpu_usage;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {  
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") return std::stoi(value);
    } 
  }
  return std::stoi(value);
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") return std::stoi(value);
    }
  }
  return std::stoi(value);
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  vector<int> pids = LinuxParser::Pids();
  string line;
  for(auto id : pids){
    if (id == pid){
      std::ifstream stream(kProcDirectory + to_string(id) + "/" + kCmdlineFilename);
      if (stream.is_open()) {
        std::getline(stream, line);
        return line;
      }
    }
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  vector<int> pids = LinuxParser::Pids();
  string key, value;
  string line;
  long to_mb = 0;
  for(auto id : pids){
    if (id == pid){
      std::ifstream stream(kProcDirectory + to_string(id) + "/" + kStatusFilename);
      if (stream.is_open()) {
        while (std::getline(stream, line)){
        std::istringstream linestream(line);
        linestream >> key >> value; 
        if (key == "VmSize:") {
          to_mb = std::stol(value)/1000;
          return to_string(to_mb);
          }
        }
      }    
    }
  }
  return to_string(to_mb);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  vector<int> pids = LinuxParser::Pids();
  string key, value;
  string line;
  for(auto id : pids){
    if (id == pid){
      std::ifstream stream(kProcDirectory + to_string(id) + "/" + kStatusFilename);
      if (stream.is_open()) {
        while(std::getline(stream, line)){
        std::istringstream linestream(line);
        linestream >> key >> value;
        if (key == "Uid:") return value;
        }
      }    
    }
  }
  return value;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string user, x, value;
  string line;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while(std::getline(stream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> x >> value;
      if (value == uid) return user;
    }
  }    
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  vector<int> pids = LinuxParser::Pids();
  string line;
  string value;
  for(auto id : pids){
    if (id == pid){
      std::ifstream stream(kProcDirectory + to_string(id) + "/" + kStatFilename);
      if (stream.is_open()) {
        while(std::getline(stream, line)){
          std::istringstream linestream(line);
          for (auto i = 1; i <= 22; i++){
            linestream >> value;
            if (i == 22) return std::stol(value)/sysconf(_SC_CLK_TCK);
          }
        }
      }    
    }
  }
  return std::stol(value)/sysconf(_SC_CLK_TCK);
}
