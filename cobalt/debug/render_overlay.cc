/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/debug/render_overlay.h"

#include "cobalt/math/matrix3_f.h"
#include "cobalt/render_tree/composition_node.h"

namespace cobalt {
namespace debug {

RenderOverlay::RenderOverlay(
    const OnRenderTreeProducedCallback& render_tree_produced_callback)
    : render_tree_produced_callback_(render_tree_produced_callback),
      input_layout_(NULL, NULL, base::TimeDelta()) {}

void RenderOverlay::OnRenderTreeProduced(const LayoutResults& layout_results) {
  input_layout_ = layout_results;
  input_receipt_time_ = base::TimeTicks::HighResNow();
  Process();
}

void RenderOverlay::SetOverlay(
    const scoped_refptr<render_tree::Node>& overlay) {
  overlay_ = overlay;
  Process();
}

void RenderOverlay::Process() {
  // Calculate a modified layout time accounting for the offset between now and
  // the time the input layout was received.
  base::TimeDelta layout_time =
      input_layout_.layout_time +
      (base::TimeTicks::HighResNow() - *input_receipt_time_);

  if (overlay_) {
    render_tree::CompositionNode::Builder builder;
    builder.AddChild(input_layout_.render_tree, math::Matrix3F::Identity());
    builder.AddChild(overlay_, math::Matrix3F::Identity());
    scoped_refptr<render_tree::Node> combined_tree =
        new render_tree::CompositionNode(builder);

    render_tree_produced_callback_.Run(
        LayoutResults(combined_tree, input_layout_.animations, layout_time));
  } else {
    render_tree_produced_callback_.Run(LayoutResults(
        input_layout_.render_tree, input_layout_.animations, layout_time));
  }
}

}  // namespace debug
}  // namespace cobalt
