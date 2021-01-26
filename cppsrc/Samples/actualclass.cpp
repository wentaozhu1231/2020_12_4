#include "actualclass.h"

ActualClass::ActualClass(const double&value){
    this->value_ = value;
}

double ActualClass::get_value()const
{
  return this->value_;
}

double ActualClass::add(const double& to_add)
{
  this->value_ += to_add;
  return this->value_;
}