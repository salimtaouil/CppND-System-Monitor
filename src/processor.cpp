#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
    float nonidle = LinuxParser::Jiffies()  + LinuxParser::ActiveJiffies();
    float idle = LinuxParser::IdleJiffies();

    float total = idle + nonidle;
    float totald = total - prevtotal_;
    float idled = idle - previdle_;

    previdle_ = idle;
    prevnonidle_ = nonidle;
    prevtotal_ = total;

    return (totald - idled) / totald;
}