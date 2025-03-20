#pragma once

#include <atomic>

struct Object {
  virtual ~Object() = default;
};

struct SampleStreamable : public virtual Object {
  virtual void process(float *buffer, int length) = 0;
  virtual const char *processorName() = 0;
  virtual void setComplete(bool state) = 0;
  virtual bool getComplete() const = 0;
};
