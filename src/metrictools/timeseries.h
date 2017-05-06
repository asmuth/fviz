/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *   Copyright (c) 2016 Paul Asmuth, FnordCorp B.V.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <metrictools/util/return_code.h>

namespace fnordmetric {

template <typename T>
struct Timeseries {

  /**
   * A list of timestamps, formatted as the number of microseconds since epoch
   */
  std::vector<uint64_t> timestamps;
  
  /**
   * A list of values, one for each element in the timestamps vector
   */
  std::vector<T> values;

  /**
   * Return the number of elements
   */
  size_t size() const;

};

/**
 * Convert a timeseries from one value type to another
 */
template <typename T1, typename T2>
ReturnCode convertTimeseries(
    const Timeseries<T1>& in,
    Timeseries<T2>* out);

/**
 * Print a debug dump of the timeseries
 */
template <typename T>
std::ostream& operator<<(std::ostream& out, const Timeseries<T>& op);

} // namespace fnordmetric

#include "timeseries_impl.h"

