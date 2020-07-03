#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

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
  string os, version, kernel;
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
  float memory_util;
  string memtotal;
  string memfree;
  string memavailable;
  string line;
  string key;
  string value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memtotal = value;
        } else if (key == "MemFree:") {
          memfree = value;
        } else if (key == "MemAvailable:") {
          memavailable = value;
        }
      }
    }
    memory_util =
        (std::stof(memtotal) - std::stof(memfree)) / (std::stof(memtotal));
    return memory_util;
  }
  return 0.0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime;
  string line;
  string upTime;
  string idle;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime >> idle;
  }
  uptime = std::stol(upTime);
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long jiffies;
  vector<string> cpudata = LinuxParser::CpuUtilization();
  for (std::size_t i = 0; i < cpudata.size(); i++) {
    jiffies += stol(cpudata[i]);
  }
  return jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
// long LinuxParser::ActiveJiffies(int pid) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long activejiffies = LinuxParser::Jiffies() - LinuxParser::IdleJiffies();
  return activejiffies;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idlejiffies;
  vector<string> cpudata = LinuxParser::CpuUtilization();
  idlejiffies = stol(cpudata[3]) + stol(cpudata[4]);
  return idlejiffies;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpudata;
  string line;
  string title;
  string user;
  string nice;
  string system;
  string idle;
  string iowait;
  string irq;
  string softirq;
  string steal;
  string guest;
  string guest_nice;

  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.good()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> title >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
    if (title == "cpu") {
      cpudata.push_back(user);
      cpudata.push_back(nice);
      cpudata.push_back(system);
      cpudata.push_back(idle);
      cpudata.push_back(iowait);
      cpudata.push_back(irq);
      cpudata.push_back(softirq);
      cpudata.push_back(steal);
      cpudata.push_back(guest);
      cpudata.push_back(guest_nice);
    }
  }

  return cpudata;
}

float LinuxParser::CpuUtilization(int pid) {
  vector<float> processcpu;
  vector<string> lineinfo;
  string line;
  float utime;
  float stime;
  float cutime;
  float cstime;
  float starttime;
  float cpuUsage;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (string line; linestream >> line;) {
      lineinfo.push_back(line);
    }
    utime = std::stof(lineinfo[13]);
    stime = std::stof(lineinfo[14]);
    cutime = std::stof(lineinfo[15]);
    cstime = std::stof(lineinfo[16]);
    starttime = std::stof(lineinfo[21]);
  }
  processcpu.push_back(utime);
  processcpu.push_back(stime);
  processcpu.push_back(cutime);
  processcpu.push_back(cstime);
  processcpu.push_back(starttime);

  long uptime = LinuxParser::UpTime();
  if (processcpu.size() == 5) {
    float totaltime =
        processcpu[0] + processcpu[1] + processcpu[2] + processcpu[3];
    float seconds = uptime - (processcpu[4] / sysconf(_SC_CLK_TCK));
    cpuUsage = ((totaltime / sysconf(_SC_CLK_TCK)) / seconds);
  }
  return cpuUsage;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int totalprocesses;
  string total;
  string line;
  string title;
  string user;

  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> title >> user) {
        if (title == "processes") {
          total = user;
        }
      }
    }
    totalprocesses = std::stoi(total);
    return totalprocesses;
  }
  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int runningprocesses;
  string running;
  string line;
  string title;
  string user;

  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> title >> user) {
        if (title == "procs_running") {
          running = user;
        }
      }
    }
    runningprocesses = std::stoi(running);
    return runningprocesses;
  }
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string cmd;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, cmd);
    return cmd;
  }
  return string();
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string stringram;
  string ram;
  string line;
  string key;
  string value;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          stringram = value;
        }
      }
    }
  }
  long numberram = std::stol(stringram) / 1000;
  ram = std::to_string(numberram);
  return ram;
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;
  string key;
  string value = "";
  string other;
  // find user name for this user ID in /etc/passwd
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> value >> other >> key) {
        if (key == uid) {
          return value;
        }
      }
    }
  }
  return value;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  long uptime;
  string line;
  string UpTime;
  vector<string> result;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (string line; linestream >> line;) {
      result.push_back(line);
    }
    UpTime = result[21];
  }
  uptime = std::stol(UpTime);
  uptime = LinuxParser::UpTime() - (uptime / (sysconf(_SC_CLK_TCK)));
  return uptime;
}
