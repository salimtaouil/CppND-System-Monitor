#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
 float previdle_ = 0;
 float prevnonidle_ = 0;
 float prevtotal_ = 0;
};

#endif