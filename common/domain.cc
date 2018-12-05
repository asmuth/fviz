/**
 * This file is part of the "plotfx" project
 *   Copyright (c) 2011-2014 Paul Asmuth, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "domain.h"

namespace plotfx {

DomainConfig::DomainConfig() :
    kind(DomainKind::LINEAR),
    inverted(false),
    padding(0.0f) {}

void domain_fit_linear(const Series& data_raw, DomainConfig* domain, bool snap_zero) {
  auto data = series_to_float(data_raw);
  bool fit_min = !domain->min;
  bool fit_max = !domain->max;

  for (const auto& d : data) {
    if (fit_min && (!domain->min || *domain->min > d)) {
      domain->min = std::optional<double>(d);
    }
    if (fit_max && (!domain->max || *domain->max < d)) {
      domain->max = std::optional<double>(d);
    }
  }

  auto range = domain->max.value_or(0) - domain->min.value_or(0);
  if (fit_max) {
    domain->max = std::optional<double>(
        domain->max.value_or(0) + range * domain->padding);
  }

  if (fit_min) {
    if (snap_zero && domain->min.value_or(0) > 0) {
      domain->min = std::optional<double>(0);
    } else {
      domain->min = std::optional<double>(
          domain->min.value_or(0) - range * domain->padding);
    }
  }
}

void domain_fit_categorical(const Series& data, DomainConfig* domain) {
  std::set<std::string> cache;
  for (const auto& d : domain->categories) {
    cache.insert(d);
  }

  for (const auto& d : data) {
    if (cache.count(d) > 0) {
      continue;
    }

    domain->categories.emplace_back(d);
    cache.insert(d);
  }
}

void domain_fit(const Series& data, DomainConfig* domain, bool snap_zero) {
  switch (domain->kind) {
    case DomainKind::LINEAR:
      return domain_fit_linear(data, domain, snap_zero);
    case DomainKind::CATEGORICAL:
      return domain_fit_categorical(data, domain);
  }
}

std::vector<double> domain_translate_linear(
    const DomainConfig& domain,
    const Series& series) {
  auto min = domain.min.value_or(0.0f);
  auto max = domain.max.value_or(0.0f);

  std::vector<double> mapped;
  for (const auto& v : series) {
    auto vf = value_to_float(v);
    auto vt = (vf - min) / (max - min);

    if (domain.inverted) {
      vt = 1.0 - vt;
    }

    mapped.push_back(vt);
  }

  return mapped;
}

std::vector<double> domain_translate_categorical(
    const DomainConfig& domain,
    const Series& series) {
  std::unordered_map<std::string, double> cache;
  for (size_t i = 0; i < domain.categories.size(); ++i) {
    cache.emplace(domain.categories[i], double(i));
  }

  double category_count = domain.categories.size();

  std::vector<double> mapped;
  for (const auto& v : series) {
    auto vt = (cache[v] / category_count) + (0.5 / category_count);

    if (domain.inverted) {
      vt = 1.0 - vt;
    }

    mapped.push_back(vt);
  }

  return mapped;
}

std::vector<double> domain_translate(
    const DomainConfig& domain,
    const Series& series) {
  switch (domain.kind) {
    case DomainKind::LINEAR:
      return domain_translate_linear(domain, series);
    case DomainKind::CATEGORICAL:
      return domain_translate_categorical(domain, series);
  }
}

double domain_untranslate(const DomainConfig& domain, double vt) {
  auto min = domain.min.value_or(0.0f);
  auto max = domain.max.value_or(0.0f);

  if (domain.inverted) {
    vt = 1.0 - vt;
  }

  auto v = 0.0;
  switch (domain.kind) {
    case DomainKind::LINEAR:
      v = min + (max - min) * vt;
      break;
  }

  return v;
}

ReturnCode confgure_domain_kind(
    const plist::Property& prop,
    DomainKind* kind) {
  if (plist::is_value(prop, "linear")) {
    *kind = DomainKind::LINEAR;
    return OK;
  }

  if (plist::is_value(prop, "categorical")) {
    *kind = DomainKind::CATEGORICAL;
    return OK;
  }

  return ERROR_INVALID_ARGUMENT;
}

namespace chart {

/*
const char AnyDomain::kDimensionLetters[] = "xyz";
const int AnyDomain::kDefaultNumTicks = 8;
const double AnyDomain::kDefaultDomainPadding = 0.1;

template <> Domain<int64_t>*
    Domain<int64_t>::mkDomain() {
  return new ContinuousDomain<int64_t>();
}

template <> Domain<double>*
    Domain<double>::mkDomain() {
  return new ContinuousDomain<double>();
}

template <> Domain<UnixTime>*
    Domain<UnixTime>::mkDomain() {
  return new TimeDomain();
}

template <> Domain<std::string>* Domain<std::string>::mkDomain() {
  return new DiscreteDomain<std::string>();
}
*/

}
}

