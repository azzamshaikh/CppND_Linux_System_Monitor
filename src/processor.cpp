#include "processor.h"

#include <unistd.h>

#include "linux_parser.h"
// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  std::vector<std::string> cpudata = LinuxParser::CpuUtilization();
  float user = stof(cpudata[0]);
  float nice = stof(cpudata[1]);
  float system = stof(cpudata[2]);
  float idle = stof(cpudata[3]);
  float iowait = stof(cpudata[4]);
  float irq = stof(cpudata[5]);
  float softirq = stof(cpudata[6]);
  float steal = stof(cpudata[7]);
  // guest and guest nice are already accounted for in user according to the
  // stackoverflow post

  float PrevIdle = previdle - previowait;
  float Idle = idle + iowait;

  float PrevNonIdle =
      prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal;
  float NonIdle = user + nice + system + irq + softirq + steal;

  float PrevTotal = PrevIdle + PrevNonIdle;
  float Total = Idle + NonIdle;

  float totald = Total - PrevTotal;
  float idled = Idle - PrevIdle;

  float output = (totald - idled) / totald;

  prevuser = user;
  prevnice = nice;
  prevsystem = system;
  previdle = idle;
  previowait = iowait;
  previrq = irq;
  prevsoftirq = softirq;
  prevsteal = steal;

  return output;
}