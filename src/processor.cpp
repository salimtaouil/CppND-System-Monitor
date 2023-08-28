#include "processor.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
    float totald = LinuxParser::Jiffies()  + LinuxParser::ActiveJiffies();
    float idled = LinuxParser::IdleJiffies();
    return (totald - idled) / totald;
}