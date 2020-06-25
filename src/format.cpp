#include "format.h"

#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  string time = "00:00:00";
  long value = 0;
  if (seconds > 0) {
    value = seconds / 3600;
    time = TimeToString(value) + ":";
    value = (seconds / 60) % 60;
    time += TimeToString(value) + ":";
    value = seconds % 60;
    time += TimeToString(value);
  }

  return time;
}

// created a function to convert time to a string and add a 0 to the number if
// it is less than 10
std::string Format::TimeToString(long time) {
  if (time < 10) {
    return "0" + std::to_string(time);
  } else
    return std::to_string(time % 100);
}