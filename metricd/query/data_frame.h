/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2016 Paul Asmuth, FnordCorp B.V. <paul@asmuth.com>
 *   Copyright (c) 2016 Laura Schlimmer, FnordCorp B.V. <laura@eventql.io>
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <string>
#include <list>
#include <vector>
#include <utility>
#include "metricd/util/return_code.h"
#include "metricd/types.h"

namespace fnordmetric {


/**
 * A named vector of numeric values and an optional corresponding list of
 * timestamps
 */
class DataFrame {
public:

  DataFrame(tval_type type);
  DataFrame(DataFrame&& other);
  DataFrame(const DataFrame& other) = delete;
  ~DataFrame();

  DataFrame& operator=(DataFrame&& other);
  DataFrame& operator=(const DataFrame& other) = delete;

  const std::string& getID() const;
  void setID(const std::string& id);

  const void* getData() const;
  void* getData();

  const uint64_t* getTime() const;
  uint64_t* getTime();

  size_t getSize() const;
  size_t getEntrySize() const;

  void resize(size_t len);

  void addValue(uint64_t time, const void* data, size_t data_len);

  void debugPrint() const;

protected:
  tval_type type_;
  std::string id_;
  void* data_;
  size_t size_;
  size_t capacity_;
};

class DataFrameBundle {
public:

  DataFrameBundle();
  DataFrameBundle(const DataFrameBundle& other) = delete;
  DataFrameBundle(DataFrameBundle&& other);

  DataFrameBundle& operator=(DataFrameBundle&& other);
  DataFrameBundle& operator=(const DataFrameBundle& other) = delete;

  size_t getFrameCount() const;
  const DataFrame* getFrame(size_t idx) const;
  DataFrame* addFrame(tval_type type);

  void debugPrint() const;

protected:
  std::vector<DataFrame> frames_;
};

} // namespace fnordmetric

