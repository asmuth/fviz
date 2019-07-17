/**
 * This file is part of the "fviz" project
 *   Copyright (c) 2018 Paul Asmuth
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "layout.h"
#include "element_factory.h"
#include "graphics/layer.h"
#include "scale.h"
#include "sexpr_conv.h"
#include "sexpr_util.h"

#include <numeric>
#include <functional>

using namespace std::placeholders;

namespace fviz::elements::chart::layout {

struct PlotBorderConfig {
  Color color;
  Measure width;
};

struct PlotConfig {
  FontInfo font;
  Measure font_size;
  Color text_color;
  Color border_color;
  std::array<Measure, 4> margins;
  std::optional<Color> background;
  std::array<PlotBorderConfig, 4> borders;
  std::vector<ElementRef> body_elements;
  std::array<std::vector<ElementRef>, 4> margin_elements;
};

ReturnCode draw(
    std::shared_ptr<PlotConfig> config,
    const LayoutInfo& layout,
    Layer* layer) {
  /* convert units  */
  auto margins = config->margins;
  for (auto& m : margins) {
    convert_unit_typographic(layer->dpi, config->font_size, &m);
  }

  /* calculate the outer margin box */
  auto content_box = layout_margin_box(
      layout.content_box,
      margins[0],
      margins[1],
      margins[2],
      margins[3]);

  /* calculate the inner body box and the margin boxes */
  std::array<Measure, 4> padding;
  for (size_t i = 0; i < config->margin_elements.size(); ++i) {
    for (const auto& e : config->margin_elements[i]) {
      if (!e->size_hint) {
        continue;
      }

      double e_width = 0;
      double e_height = 0;
      if (auto rc =
            e->size_hint(
              *layer,
              content_box.w,
              content_box.h,
              &e_width,
              &e_height);
          !rc) {
        return rc;
      }

      padding[i] = from_unit(
          std::max(
              double(padding[i]),
              i % 2 ? e_width : e_height));
    }
  }

  auto body_box = layout_margin_box(
      content_box,
      padding[0],
      padding[1],
      padding[2],
      padding[3]);

  std::array<Rectangle, 4> margin_boxes = {
    Rectangle {
      body_box.x,
      content_box.y,
      body_box.w,
      padding[0],
    },
    Rectangle {
      content_box.x + content_box.w - padding[1],
      body_box.y,
      padding[1],
      body_box.h,
    },
    Rectangle {
      body_box.x,
      content_box.y + content_box.h - padding[2],
      body_box.w,
      padding[2],
    },
    Rectangle {
      content_box.x,
      body_box.y,
      padding[3],
      body_box.h,
    }
  };

  /* draw the background */
  if (config->background) {
    const auto& bg_box = body_box;
    FillStyle bg_fill;
    bg_fill.color = *config->background;

    fillRectangle(
        layer,
        Point(bg_box.x, bg_box.y),
        bg_box.w,
        bg_box.h,
        bg_fill);
  }

  /* draw the body elements  */
  for (const auto& e : config->body_elements) {
    LayoutInfo layout;
    layout.content_box = body_box;

    if (auto rc = e->draw(layout, layer); !rc) {
      return rc;
    }
  }

  /* draw the margin elements  */
  for (size_t i = 0; i < config->margin_elements.size(); ++i) {
    for (const auto& e : config->margin_elements[i]) {
      LayoutInfo layout;
      layout.content_box = margin_boxes[i];

      if (auto rc = e->draw(layout, layer); !rc) {
        return rc;
      }
    }
  }

  /* draw the top border  */
  if (config->borders[0].width > 0) {
    StrokeStyle border_style;
    border_style.line_width = config->borders[0].width;
    border_style.color = config->borders[0].color;

    strokeLine(
        layer,
        Point(content_box.x, content_box.y),
        Point(content_box.x + content_box.w, content_box.y),
        border_style);
  }

  /* draw the right border  */
  if (config->borders[1].width > 0) {
    StrokeStyle border_style;
    border_style.line_width = config->borders[1].width;
    border_style.color = config->borders[1].color;

    strokeLine(
        layer,
        Point(content_box.x + content_box.w, content_box.y),
        Point(content_box.x + content_box.w, content_box.y + content_box.h),
        border_style);
  }

  /* draw the bottom border  */
  if (config->borders[2].width > 0) {
    StrokeStyle border_style;
    border_style.line_width = config->borders[2].width;
    border_style.color = config->borders[2].color;

    strokeLine(
        layer,
        Point(content_box.x, content_box.y + content_box.h),
        Point(content_box.x + content_box.w, content_box.y + content_box.h),
        border_style);
  }

  /* draw the left border  */
  if (config->borders[3].width > 0) {
    StrokeStyle border_style;
    border_style.line_width = config->borders[3].width;
    border_style.color = config->borders[3].color;

    strokeLine(
        layer,
        Point(content_box.x, content_box.y),
        Point(content_box.x, content_box.y + content_box.h),
        border_style);
  }

  return OK;
}

ReturnCode build(
    const Environment& env,
    const Expr* expr,
    ElementRef* elem) {
  auto config = std::make_shared<PlotConfig>();
  config->font = env.font;
  config->font_size = env.font_size;
  config->text_color = env.text_color;
  config->border_color = env.border_color;
  config->margins = {from_em(1), from_em(1), from_em(1), from_em(1)};

  auto config_rc = expr_walk_map(expr_next(expr), {
    {
      "margin",
      expr_calln_fn({
        bind(&expr_to_measure, _1, &config->margins[0]),
        bind(&expr_to_measure, _1, &config->margins[1]),
        bind(&expr_to_measure, _1, &config->margins[2]),
        bind(&expr_to_measure, _1, &config->margins[3]),
      })
    },
    {"margin-top", bind(&expr_to_measure, _1, &config->margins[0])},
    {"margin-right", bind(&expr_to_measure, _1, &config->margins[1])},
    {"margin-bottom", bind(&expr_to_measure, _1, &config->margins[2])},
    {"margin-left", bind(&expr_to_measure, _1, &config->margins[3])},
    {"border-top-color", bind(&expr_to_color, _1, &config->borders[0].color)},
    {"border-right-color", bind(&expr_to_color, _1, &config->borders[1].color)},
    {"border-bottom-color", bind(&expr_to_color, _1, &config->borders[2].color)},
    {"border-left-color", bind(&expr_to_color, _1, &config->borders[3].color)},
    {"border-top-width", bind(&expr_to_measure, _1, &config->borders[0].width)},
    {"border-right-width", bind(&expr_to_measure, _1, &config->borders[1].width)},
    {"border-bottom-width", bind(&expr_to_measure, _1, &config->borders[2].width)},
    {"border-left-width", bind(&expr_to_measure, _1, &config->borders[3].width)},
    {"background-color", bind(&expr_to_color_opt, _1, &config->background)},
    {
      "foreground-color",
      expr_calln_fn({
        bind(&expr_to_color, _1, &config->text_color),
        bind(&expr_to_color, _1, &config->border_color),
      })
    },
    {"text-color", bind(&expr_to_color, _1, &config->text_color)},
    {"border-color", bind(&expr_to_color, _1, &config->border_color)},
    {"body", bind(&element_build_list, env, _1, &config->body_elements)},
    {"top", bind(&element_build_list, env, _1, &config->margin_elements[0])},
    {"right", bind(&element_build_list, env, _1, &config->margin_elements[1])},
    {"bottom", bind(&element_build_list, env, _1, &config->margin_elements[2])},
    {"left", bind(&element_build_list, env, _1, &config->margin_elements[3])},
  });

  if (!config_rc) {
    return config_rc;
  }

  *elem = std::make_shared<Element>();
  (*elem)->draw = bind(&draw, config, _1, _2);
  return OK;
}

} // namespace fviz::elements::chart::layout

