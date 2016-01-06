/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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

#include "cobalt/layout/used_style.h"

#include <string>
#include <vector>

#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/cssom/absolute_url_value.h"
#include "cobalt/cssom/calc_value.h"
#include "cobalt/cssom/font_style_value.h"
#include "cobalt/cssom/font_weight_value.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/length_value.h"
#include "cobalt/cssom/matrix_function.h"
#include "cobalt/cssom/percentage_value.h"
#include "cobalt/cssom/rgba_color_value.h"
#include "cobalt/cssom/rotate_function.h"
#include "cobalt/cssom/scale_function.h"
#include "cobalt/cssom/string_value.h"
#include "cobalt/cssom/transform_function_visitor.h"
#include "cobalt/cssom/transform_matrix_function_value.h"
#include "cobalt/cssom/translate_function.h"
#include "cobalt/math/transform_2d.h"
#include "cobalt/render_tree/composition_node.h"
#include "cobalt/render_tree/image_node.h"
#include "cobalt/render_tree/rounded_corners.h"

namespace cobalt {
namespace layout {

namespace {

struct BackgroundImageTransformData {
  BackgroundImageTransformData(math::SizeF image_node_size,
                               math::Matrix3F image_node_transform_matrix,
                               math::Matrix3F composition_node_transform_matrix)
      : image_node_size(image_node_size),
        image_node_transform_matrix(image_node_transform_matrix),
        composition_node_transform_matrix(composition_node_transform_matrix) {}

  math::SizeF image_node_size;
  math::Matrix3F image_node_transform_matrix;
  math::Matrix3F composition_node_transform_matrix;
};

render_tree::FontStyle ConvertCSSOMFontValuesToRenderTreeFontStyle(
    cssom::FontStyleValue::Value style, cssom::FontWeightValue::Value weight) {
  render_tree::FontStyle::Weight font_weight;
  switch (weight) {
    case cssom::FontWeightValue::kThinAka100:
      font_weight = render_tree::FontStyle::kThinWeight;
      break;
    case cssom::FontWeightValue::kExtraLightAka200:
      font_weight = render_tree::FontStyle::kExtraLightWeight;
      break;
    case cssom::FontWeightValue::kLightAka300:
      font_weight = render_tree::FontStyle::kLightWeight;
      break;
    case cssom::FontWeightValue::kNormalAka400:
      font_weight = render_tree::FontStyle::kNormalWeight;
      break;
    case cssom::FontWeightValue::kMediumAka500:
      font_weight = render_tree::FontStyle::kMediumWeight;
      break;
    case cssom::FontWeightValue::kSemiBoldAka600:
      font_weight = render_tree::FontStyle::kSemiBoldWeight;
      break;
    case cssom::FontWeightValue::kBoldAka700:
      font_weight = render_tree::FontStyle::kBoldWeight;
      break;
    case cssom::FontWeightValue::kExtraBoldAka800:
      font_weight = render_tree::FontStyle::kExtraBoldWeight;
      break;
    case cssom::FontWeightValue::kBlackAka900:
      font_weight = render_tree::FontStyle::kBlackWeight;
      break;
    default:
      font_weight = render_tree::FontStyle::kNormalWeight;
  }

  render_tree::FontStyle::Slant font_slant =
      style == cssom::FontStyleValue::kItalic
          ? render_tree::FontStyle::kItalicSlant
          : render_tree::FontStyle::kUprightSlant;

  return render_tree::FontStyle(font_weight, font_slant);
}

BackgroundImageTransformData GetImageTransformationData(
    UsedBackgroundSizeProvider* used_background_size_provider,
    UsedBackgroundPositionProvider* used_background_position_provider,
    UsedBackgroundRepeatProvider* used_background_repeat_provider,
    const math::SizeF& frame_size, const math::SizeF& single_image_size) {
  // The initial value of following variables are for no-repeat horizontal and
  // vertical.
  math::SizeF image_node_size = single_image_size;
  float image_node_translate_matrix_x = 0.0f;
  float image_node_translate_matrix_y = 0.0f;
  float image_node_scale_matrix_x =
      used_background_size_provider->width() / single_image_size.width();
  float image_node_scale_matrix_y =
      used_background_size_provider->height() / single_image_size.height();
  float composition_node_translate_matrix_x =
      used_background_position_provider->translate_x();
  float composition_node_translate_matrix_y =
      used_background_position_provider->translate_y();

  if (used_background_repeat_provider->repeat_x() ||
      frame_size.width() < image_node_size.width()) {
    // When the background repeat horizontally or the width of frame is smaller
    // than the width of image, image node does the transform in horizontal
    // direction.
    image_node_size.set_width(frame_size.width());
    image_node_translate_matrix_x =
        used_background_position_provider->translate_x_relative_to_frame();
    image_node_scale_matrix_x =
        used_background_size_provider->width_scale_relative_to_frame();
    composition_node_translate_matrix_x = 0.0f;
  }

  if (used_background_repeat_provider->repeat_y() ||
      frame_size.height() < image_node_size.height()) {
    // When the background repeat vertically or the width of frame is smaller
    // than the height of image, image node does the transform in vertical
    // direction.
    image_node_size.set_height(frame_size.height());
    image_node_translate_matrix_y =
        used_background_position_provider->translate_y_relative_to_frame();
    image_node_scale_matrix_y =
        used_background_size_provider->height_scale_relative_to_frame();
    composition_node_translate_matrix_y = 0.0f;
  }

  BackgroundImageTransformData background_image_transform_data(
      image_node_size, math::TranslateMatrix(image_node_translate_matrix_x,
                                             image_node_translate_matrix_y) *
                           math::ScaleMatrix(image_node_scale_matrix_x,
                                             image_node_scale_matrix_y),
      math::TranslateMatrix(composition_node_translate_matrix_x,
                            composition_node_translate_matrix_y));
  return background_image_transform_data;
}

class UsedBackgroundTranslateProvider
    : public cssom::NotReachedPropertyValueVisitor {
 public:
  UsedBackgroundTranslateProvider(float frame_length, float image_length)
      : frame_length_(frame_length), image_length_(image_length) {}

  void VisitCalc(cssom::CalcValue* calc) OVERRIDE;

  // Returns the value based on the left top.
  float translate() { return translate_; }

 private:
  const float frame_length_;
  const float image_length_;

  float translate_;

  DISALLOW_COPY_AND_ASSIGN(UsedBackgroundTranslateProvider);
};

// A percentage for the horizontal offset is relative to (width of background
// positioning area - width of background image). A percentage for the vertical
// offset is relative to (height of background positioning area - height of
// background image), where the size of the image is the size given by
// 'background-size'.
//   http://www.w3.org/TR/css3-background/#the-background-position
void UsedBackgroundTranslateProvider::VisitCalc(cssom::CalcValue* calc) {
  DCHECK_EQ(cssom::kPixelsUnit, calc->length_value()->unit());

  translate_ =
      calc->percentage_value()->value() * (frame_length_ - image_length_) +
      calc->length_value()->value();
}

//   http://www.w3.org/TR/css3-background/#the-background-size
class UsedBackgroundSizeScaleProvider
    : public cssom::NotReachedPropertyValueVisitor {
 public:
  UsedBackgroundSizeScaleProvider(float frame_length, int image_length)
      : frame_length_(frame_length),
        image_length_(image_length),
        scale_(1.0f),
        auto_keyword_(false) {
    DCHECK_GT(image_length, 0);
  }

  void VisitKeyword(cssom::KeywordValue* keyword) OVERRIDE;
  void VisitLength(cssom::LengthValue* length) OVERRIDE;
  void VisitPercentage(cssom::PercentageValue* percentage) OVERRIDE;

  float scale() const { return scale_; }
  bool auto_keyword() const { return auto_keyword_; }

 private:
  const float frame_length_;
  const int image_length_;

  float scale_;
  bool auto_keyword_;

  DISALLOW_COPY_AND_ASSIGN(UsedBackgroundSizeScaleProvider);
};

void UsedBackgroundSizeScaleProvider::VisitKeyword(
    cssom::KeywordValue* keyword) {
  switch (keyword->value()) {
    case cssom::KeywordValue::kAuto: {
      auto_keyword_ = true;
      break;
    }
    case cssom::KeywordValue::kAbsolute:
    case cssom::KeywordValue::kAlternate:
    case cssom::KeywordValue::kAlternateReverse:
    case cssom::KeywordValue::kBackwards:
    case cssom::KeywordValue::kBaseline:
    case cssom::KeywordValue::kBlock:
    case cssom::KeywordValue::kBoth:
    case cssom::KeywordValue::kBottom:
    case cssom::KeywordValue::kBreakWord:
    case cssom::KeywordValue::kCenter:
    case cssom::KeywordValue::kClip:
    case cssom::KeywordValue::kContain:
    case cssom::KeywordValue::kCover:
    case cssom::KeywordValue::kCurrentColor:
    case cssom::KeywordValue::kCursive:
    case cssom::KeywordValue::kEllipsis:
    case cssom::KeywordValue::kFantasy:
    case cssom::KeywordValue::kForwards:
    case cssom::KeywordValue::kFixed:
    case cssom::KeywordValue::kHidden:
    case cssom::KeywordValue::kInfinite:
    case cssom::KeywordValue::kInherit:
    case cssom::KeywordValue::kInitial:
    case cssom::KeywordValue::kInline:
    case cssom::KeywordValue::kInlineBlock:
    case cssom::KeywordValue::kLeft:
    case cssom::KeywordValue::kMiddle:
    case cssom::KeywordValue::kMonospace:
    case cssom::KeywordValue::kNone:
    case cssom::KeywordValue::kNoRepeat:
    case cssom::KeywordValue::kNormal:
    case cssom::KeywordValue::kNoWrap:
    case cssom::KeywordValue::kPre:
    case cssom::KeywordValue::kRelative:
    case cssom::KeywordValue::kRepeat:
    case cssom::KeywordValue::kReverse:
    case cssom::KeywordValue::kRight:
    case cssom::KeywordValue::kSansSerif:
    case cssom::KeywordValue::kSerif:
    case cssom::KeywordValue::kSolid:
    case cssom::KeywordValue::kStatic:
    case cssom::KeywordValue::kTop:
    case cssom::KeywordValue::kUppercase:
    case cssom::KeywordValue::kVisible:
    default:
      NOTREACHED();
  }
}

void UsedBackgroundSizeScaleProvider::VisitLength(cssom::LengthValue* length) {
  DCHECK_EQ(cssom::kPixelsUnit, length->unit());
  scale_ = length->value() / image_length_;
}

void UsedBackgroundSizeScaleProvider::VisitPercentage(
    cssom::PercentageValue* percentage) {
  scale_ = frame_length_ * percentage->value() / image_length_;
}

// TODO(***REMOVED***): Factor in generic families.
//   http://www.w3.org/TR/css3-fonts/#font-family-prop
class UsedFontFamilyProvider : public cssom::NotReachedPropertyValueVisitor {
 public:
  explicit UsedFontFamilyProvider(std::vector<std::string>* family_names)
      : family_names_(family_names) {}

  void VisitKeyword(cssom::KeywordValue* keyword) OVERRIDE;
  void VisitPropertyList(cssom::PropertyListValue* property_list) OVERRIDE;
  void VisitString(cssom::StringValue* percentage) OVERRIDE;

 private:
  std::vector<std::string>* family_names_;

  DISALLOW_COPY_AND_ASSIGN(UsedFontFamilyProvider);
};

void UsedFontFamilyProvider::VisitKeyword(cssom::KeywordValue* keyword) {
  switch (keyword->value()) {
    case cssom::KeywordValue::kCurrentColor:
    case cssom::KeywordValue::kCursive:
    case cssom::KeywordValue::kFantasy:
    case cssom::KeywordValue::kMonospace:
    case cssom::KeywordValue::kSansSerif:
    case cssom::KeywordValue::kSerif:
      family_names_->push_back(keyword->ToString());
      break;
    case cssom::KeywordValue::kAbsolute:
    case cssom::KeywordValue::kAlternate:
    case cssom::KeywordValue::kAlternateReverse:
    case cssom::KeywordValue::kAuto:
    case cssom::KeywordValue::kBackwards:
    case cssom::KeywordValue::kBaseline:
    case cssom::KeywordValue::kBlock:
    case cssom::KeywordValue::kBoth:
    case cssom::KeywordValue::kBottom:
    case cssom::KeywordValue::kBreakWord:
    case cssom::KeywordValue::kCenter:
    case cssom::KeywordValue::kClip:
    case cssom::KeywordValue::kContain:
    case cssom::KeywordValue::kCover:
    case cssom::KeywordValue::kEllipsis:
    case cssom::KeywordValue::kFixed:
    case cssom::KeywordValue::kForwards:
    case cssom::KeywordValue::kHidden:
    case cssom::KeywordValue::kInfinite:
    case cssom::KeywordValue::kInherit:
    case cssom::KeywordValue::kInitial:
    case cssom::KeywordValue::kInline:
    case cssom::KeywordValue::kInlineBlock:
    case cssom::KeywordValue::kLeft:
    case cssom::KeywordValue::kMiddle:
    case cssom::KeywordValue::kNone:
    case cssom::KeywordValue::kNoRepeat:
    case cssom::KeywordValue::kNormal:
    case cssom::KeywordValue::kNoWrap:
    case cssom::KeywordValue::kPre:
    case cssom::KeywordValue::kRelative:
    case cssom::KeywordValue::kRepeat:
    case cssom::KeywordValue::kReverse:
    case cssom::KeywordValue::kRight:
    case cssom::KeywordValue::kSolid:
    case cssom::KeywordValue::kStatic:
    case cssom::KeywordValue::kTop:
    case cssom::KeywordValue::kUppercase:
    case cssom::KeywordValue::kVisible:
    default:
      NOTREACHED();
  }
}

void UsedFontFamilyProvider::VisitPropertyList(
    cssom::PropertyListValue* property_list) {
  size_t size = property_list->value().size();
  family_names_->reserve(size);
  for (size_t i = 0; i < size; ++i) {
    property_list->value()[i]->Accept(this);
  }
}

void UsedFontFamilyProvider::VisitString(cssom::StringValue* string) {
  family_names_->push_back(string->value());
}

}  // namespace

UsedStyleProvider::UsedStyleProvider(loader::image::ImageCache* image_cache,
                                     dom::FontCache* font_cache)
    : image_cache_(image_cache), font_cache_(font_cache) {}

scoped_refptr<dom::FontList> UsedStyleProvider::GetUsedFontList(
    const scoped_refptr<cssom::PropertyValue>& font_family_refptr,
    const scoped_refptr<cssom::PropertyValue>& font_size_refptr,
    const scoped_refptr<cssom::PropertyValue>& font_style_refptr,
    const scoped_refptr<cssom::PropertyValue>& font_weight_refptr) {
  // Grab the font size prior to making the last font comparisons. The reason
  // that font size does not use the same property value comparisons as the
  // the rest of the properties is that the mechanism for generating a computed
  // font size results in numerous font size property values with the same
  // underlying size. Comparing the font size property pointer results in many
  // font lists with identical values incorrectly being treated as different.
  // This issue does not occur with the other font properties.
  cssom::LengthValue* font_size_length =
      base::polymorphic_downcast<cssom::LengthValue*>(font_size_refptr.get());
  DCHECK_EQ(cssom::kPixelsUnit, font_size_length->unit());
  float font_size = font_size_length->value();

  // Check if the last font list matches the current font list. If it does, then
  // it can simply be returned.
  if (last_font_list_ != NULL && last_font_list_->size() == font_size &&
      last_font_family_refptr_.get() == font_family_refptr.get() &&
      last_font_style_refptr_.get() == font_style_refptr.get() &&
      last_font_weight_refptr_.get() == font_weight_refptr.get()) {
    return last_font_list_;
  }

  // Populate the font list key
  font_list_key_.family_names.clear();
  UsedFontFamilyProvider font_family_provider(&font_list_key_.family_names);
  font_family_refptr->Accept(&font_family_provider);

  cssom::FontStyleValue* font_style =
      base::polymorphic_downcast<cssom::FontStyleValue*>(
          font_style_refptr.get());
  cssom::FontWeightValue* font_weight =
      base::polymorphic_downcast<cssom::FontWeightValue*>(
          font_weight_refptr.get());
  font_list_key_.style = ConvertCSSOMFontValuesToRenderTreeFontStyle(
      font_style->value(), font_weight->value());

  font_list_key_.size = font_size;

  // Update the last font properties and grab the new last font list from the
  // font cache. In the case where it did not previously exist, the font cache
  // will create it.
  last_font_family_refptr_ = font_family_refptr;
  last_font_style_refptr_ = font_style_refptr;
  last_font_weight_refptr_ = font_weight_refptr;
  last_font_list_ = font_cache_->GetFontList(font_list_key_);

  return last_font_list_;
}

scoped_refptr<render_tree::Image> UsedStyleProvider::ResolveURLToImage(
    const GURL& url) {
  return image_cache_->CreateCachedResource(url)->TryGetResource();
}

void UsedStyleProvider::CleanupAfterLayout() {
  // Clear out the last font properties prior to requesting that the font cache
  // remove unused font lists. The reason for this is that the font cache will
  // remove any font lists where it holds the exclusive reference, and the
  // |last_font_list_| could potentially hold a second reference, thereby
  // interfering with the removal.
  last_font_family_refptr_ = NULL;
  last_font_style_refptr_ = NULL;
  last_font_weight_refptr_ = NULL;
  last_font_list_ = NULL;

  font_cache_->RemoveUnusedFontLists();
}

UsedStyleProviderLayoutScope::UsedStyleProviderLayoutScope(
    UsedStyleProvider* used_style_provider)
    : used_style_provider_(used_style_provider) {}

UsedStyleProviderLayoutScope::~UsedStyleProviderLayoutScope() {
  used_style_provider_->CleanupAfterLayout();
}

render_tree::ColorRGBA GetUsedColor(
    const scoped_refptr<cssom::PropertyValue>& color_refptr) {
  cssom::RGBAColorValue* color =
      base::polymorphic_downcast<cssom::RGBAColorValue*>(color_refptr.get());
  return render_tree::ColorRGBA(color->value());
}

float GetUsedLength(const scoped_refptr<cssom::PropertyValue>& length_refptr) {
  cssom::LengthValue* length =
      base::polymorphic_downcast<cssom::LengthValue*>(length_refptr.get());
  DCHECK_EQ(length->unit(), cssom::kPixelsUnit);
  return length->value();
}

UsedBackgroundNodeProvider::UsedBackgroundNodeProvider(
    const math::SizeF& frame_size,
    const scoped_refptr<cssom::PropertyValue>& background_size,
    const scoped_refptr<cssom::PropertyValue>& background_position,
    const scoped_refptr<cssom::PropertyValue>& background_repeat,
    UsedStyleProvider* used_style_provider)
    : frame_size_(frame_size),
      background_size_(background_size),
      background_position_(background_position),
      background_repeat_(background_repeat),
      used_style_provider_(used_style_provider) {}

void UsedBackgroundNodeProvider::VisitAbsoluteURL(
    cssom::AbsoluteURLValue* url_value) {
  // Deal with the case that background image is an image resource as opposed to
  // "linear-gradient".
  scoped_refptr<render_tree::Image> used_background_image =
      used_style_provider_->ResolveURLToImage(url_value->value());

  if (used_background_image) {
    UsedBackgroundSizeProvider used_background_size_provider(
        frame_size_, used_background_image->GetSize());
    background_size_->Accept(&used_background_size_provider);

    math::SizeF single_image_size =
        math::SizeF(used_background_size_provider.width(),
                    used_background_size_provider.height());
    UsedBackgroundPositionProvider used_background_position_provider(
        frame_size_, single_image_size);
    background_position_->Accept(&used_background_position_provider);

    UsedBackgroundRepeatProvider used_background_repeat_provider;
    background_repeat_->Accept(&used_background_repeat_provider);

    BackgroundImageTransformData background_image_transform_data =
        GetImageTransformationData(
            &used_background_size_provider, &used_background_position_provider,
            &used_background_repeat_provider, frame_size_, single_image_size);

    scoped_refptr<render_tree::ImageNode> image_node(new render_tree::ImageNode(
        used_background_image, background_image_transform_data.image_node_size,
        background_image_transform_data.image_node_transform_matrix));

    render_tree::CompositionNode::Builder image_composition_node_builder;
    image_composition_node_builder.AddChild(
        image_node,
        background_image_transform_data.composition_node_transform_matrix);
    background_node_ =
        new render_tree::CompositionNode(image_composition_node_builder.Pass());
  }
}

//   http://www.w3.org/TR/css3-background/#the-background-position
UsedBackgroundPositionProvider::UsedBackgroundPositionProvider(
    const math::SizeF& frame_size, const math::SizeF& image_actual_size)
    : frame_size_(frame_size), image_actual_size_(image_actual_size) {}

void UsedBackgroundPositionProvider::VisitPropertyList(
    cssom::PropertyListValue* property_list_value) {
  DCHECK_EQ(property_list_value->value().size(), 2);
  UsedBackgroundTranslateProvider width_translate_provider(
      frame_size_.width(), image_actual_size_.width());
  property_list_value->value()[0]->Accept(&width_translate_provider);
  translate_x_ = width_translate_provider.translate();

  UsedBackgroundTranslateProvider height_translate_provider(
      frame_size_.height(), image_actual_size_.height());
  property_list_value->value()[1]->Accept(&height_translate_provider);
  translate_y_ = height_translate_provider.translate();
}

UsedBackgroundRepeatProvider::UsedBackgroundRepeatProvider()
    : repeat_x_(false), repeat_y_(false) {}

void UsedBackgroundRepeatProvider::VisitPropertyList(
    cssom::PropertyListValue* background_repeat_list) {
  DCHECK_EQ(background_repeat_list->value().size(), 2);

  repeat_x_ =
      background_repeat_list->value()[0] == cssom::KeywordValue::GetRepeat()
          ? true
          : false;

  repeat_y_ =
      background_repeat_list->value()[1] == cssom::KeywordValue::GetRepeat()
          ? true
          : false;
}

UsedBackgroundSizeProvider::UsedBackgroundSizeProvider(
    const math::SizeF& frame_size, const math::Size& image_size)
    : frame_size_(frame_size),
      image_size_(image_size),
      width_(1.0f),
      height_(1.0f) {}

// The first value gives the width of the corresponding image, and the second
// value gives its height.
//   http://www.w3.org/TR/css3-background/#the-background-size
void UsedBackgroundSizeProvider::VisitPropertyList(
    cssom::PropertyListValue* property_list_value) {
  DCHECK_EQ(property_list_value->value().size(), 2);

  UsedBackgroundSizeScaleProvider used_background_width_provider(
      frame_size_.width(), image_size_.width());
  property_list_value->value()[0]->Accept(&used_background_width_provider);

  UsedBackgroundSizeScaleProvider used_background_height_provider(
      frame_size_.height(), image_size_.height());
  property_list_value->value()[1]->Accept(&used_background_height_provider);

  bool background_width_auto = used_background_width_provider.auto_keyword();
  bool background_height_auto = used_background_height_provider.auto_keyword();

  float width_scale;
  float height_scale;
  if (background_width_auto && background_height_auto) {
    // If both values are 'auto' then the intrinsic width and/or height of the
    // image should be used.
    width_scale = height_scale = 1.0f;
  } else if (!background_width_auto && !background_height_auto) {
    width_scale = used_background_width_provider.scale();
    height_scale = used_background_height_provider.scale();
  } else {
    // An 'auto' value for one dimension is resolved by using the image's
    // intrinsic ratio and the size of the other dimension.
    width_scale = height_scale = background_width_auto
                                     ? used_background_height_provider.scale()
                                     : used_background_width_provider.scale();
  }

  ConvertWidthAndHeightScale(width_scale, height_scale);
}

void UsedBackgroundSizeProvider::VisitKeyword(cssom::KeywordValue* keyword) {
  switch (keyword->value()) {
    case cssom::KeywordValue::kContain: {
      // Scale the image, while preserving its intrinsic aspect ratio (if any),
      // to the largest size such that both its width and its height can
      // fit inside the background positioning area.
      float width_scale = frame_size_.width() / image_size_.width();
      float height_scale = frame_size_.height() / image_size_.height();

      float selected_scale =
          width_scale < height_scale ? width_scale : height_scale;
      ConvertWidthAndHeightScale(selected_scale, selected_scale);
      break;
    }
    case cssom::KeywordValue::kCover: {
      // Scale the image, while preserving its intrinsic aspect ratio (if any),
      // to the smallest size such that both its width and its height can
      // completely cover the background positioning area.
      float width_scale = frame_size_.width() / image_size_.width();
      float height_scale = frame_size_.height() / image_size_.height();

      float selected_scale =
          width_scale > height_scale ? width_scale : height_scale;
      ConvertWidthAndHeightScale(selected_scale, selected_scale);
      break;
    }
    case cssom::KeywordValue::kAbsolute:
    case cssom::KeywordValue::kAlternate:
    case cssom::KeywordValue::kAlternateReverse:
    case cssom::KeywordValue::kAuto:
    case cssom::KeywordValue::kBackwards:
    case cssom::KeywordValue::kBaseline:
    case cssom::KeywordValue::kBoth:
    case cssom::KeywordValue::kBottom:
    case cssom::KeywordValue::kBlock:
    case cssom::KeywordValue::kBreakWord:
    case cssom::KeywordValue::kCenter:
    case cssom::KeywordValue::kCurrentColor:
    case cssom::KeywordValue::kCursive:
    case cssom::KeywordValue::kClip:
    case cssom::KeywordValue::kEllipsis:
    case cssom::KeywordValue::kFantasy:
    case cssom::KeywordValue::kFixed:
    case cssom::KeywordValue::kForwards:
    case cssom::KeywordValue::kHidden:
    case cssom::KeywordValue::kInfinite:
    case cssom::KeywordValue::kInherit:
    case cssom::KeywordValue::kInitial:
    case cssom::KeywordValue::kInline:
    case cssom::KeywordValue::kInlineBlock:
    case cssom::KeywordValue::kLeft:
    case cssom::KeywordValue::kMiddle:
    case cssom::KeywordValue::kMonospace:
    case cssom::KeywordValue::kNone:
    case cssom::KeywordValue::kNoRepeat:
    case cssom::KeywordValue::kNormal:
    case cssom::KeywordValue::kNoWrap:
    case cssom::KeywordValue::kPre:
    case cssom::KeywordValue::kRelative:
    case cssom::KeywordValue::kRepeat:
    case cssom::KeywordValue::kReverse:
    case cssom::KeywordValue::kRight:
    case cssom::KeywordValue::kSansSerif:
    case cssom::KeywordValue::kSerif:
    case cssom::KeywordValue::kSolid:
    case cssom::KeywordValue::kStatic:
    case cssom::KeywordValue::kTop:
    case cssom::KeywordValue::kUppercase:
    case cssom::KeywordValue::kVisible:
    default:
      NOTREACHED();
  }
}

void UsedBackgroundSizeProvider::ConvertWidthAndHeightScale(
    float width_scale, float height_scale) {
  if (frame_size_.width() <= 0 || frame_size_.height() <= 0) {
    DLOG(WARNING) << "Frame size is not positive.";
    width_ = height_ = 0.0f;
    return;
  }

  width_ = width_scale * image_size_.width();
  height_ = height_scale * image_size_.height();
}

UsedBorderRadiusProvider::UsedBorderRadiusProvider(
    const math::SizeF& frame_size)
    : rounded_corners_(render_tree::RoundedCorner()), frame_size_(frame_size) {}

void UsedBorderRadiusProvider::VisitLength(cssom::LengthValue* length) {
  render_tree::RoundedCorner rounded_corner =
      render_tree::RoundedCorner(length->value(), length->value());
  rounded_corners_ = render_tree::RoundedCorners(rounded_corner);
}

void UsedBorderRadiusProvider::VisitPercentage(
    cssom::PercentageValue* percentage) {
  render_tree::RoundedCorner rounded_corner =
      render_tree::RoundedCorner(percentage->value() * frame_size_.width(),
                                 percentage->value() * frame_size_.height());
  rounded_corners_ = render_tree::RoundedCorners(rounded_corner);
}

UsedLineHeightProvider::UsedLineHeightProvider(
    const render_tree::FontMetrics& font_metrics)
    : font_metrics_(font_metrics) {}

void UsedLineHeightProvider::VisitKeyword(cssom::KeywordValue* keyword) {
  DCHECK_EQ(cssom::KeywordValue::kNormal, keyword->value());
  used_line_height_ =
      font_metrics_.ascent + font_metrics_.descent + font_metrics_.leading;
  UpdateHalfLeading();
}

void UsedLineHeightProvider::VisitLength(cssom::LengthValue* length) {
  DCHECK_EQ(cssom::kPixelsUnit, length->unit());
  used_line_height_ = length->value();
  UpdateHalfLeading();
}

void UsedLineHeightProvider::UpdateHalfLeading() {
  // Determine the leading L, where L = "line-height" - AD,
  // AD = A (ascent) + D (descent).
  //   http://www.w3.org/TR/CSS21/visudet.html#leading
  half_leading_ =
      (used_line_height_ - (font_metrics_.ascent + font_metrics_.descent)) / 2;
}

cssom::TransformMatrix GetTransformMatrix(cssom::PropertyValue* value) {
  if (value->GetTypeId() ==
      base::GetTypeId<cssom::TransformMatrixFunctionValue>()) {
    cssom::TransformMatrixFunctionValue* matrix_function =
        base::polymorphic_downcast<cssom::TransformMatrixFunctionValue*>(value);
    return matrix_function->value();
  } else {
    cssom::TransformFunctionListValue* transform_list =
        base::polymorphic_downcast<cssom::TransformFunctionListValue*>(value);
    return transform_list->ToMatrix();
  }
}

namespace {

class UsedLengthValueProvider : public cssom::NotReachedPropertyValueVisitor {
 public:
  explicit UsedLengthValueProvider(float percentage_base)
      : percentage_base_(percentage_base) {}

  void VisitLength(cssom::LengthValue* length) OVERRIDE {
    depends_on_containing_block_ = false;

    DCHECK_EQ(cssom::kPixelsUnit, length->unit());
    used_length_ = length->value();
  }

  void VisitPercentage(cssom::PercentageValue* percentage) OVERRIDE {
    depends_on_containing_block_ = true;
    used_length_ = percentage->value() * percentage_base_;
  }

  bool depends_on_containing_block() const {
    return depends_on_containing_block_;
  }
  const base::optional<float>& used_length() const { return used_length_; }

 protected:
  bool depends_on_containing_block_;

 private:
  const float percentage_base_;

  base::optional<float> used_length_;

  DISALLOW_COPY_AND_ASSIGN(UsedLengthValueProvider);
};

class UsedLengthProvider : public UsedLengthValueProvider {
 public:
  explicit UsedLengthProvider(float percentage_base)
      : UsedLengthValueProvider(percentage_base) {}

  void VisitKeyword(cssom::KeywordValue* keyword) OVERRIDE {
    switch (keyword->value()) {
      case cssom::KeywordValue::kAuto:
        depends_on_containing_block_ = true;

        // Leave |used_length_| in disengaged state to indicate that "auto"
        // was the value.
        break;

      case cssom::KeywordValue::kAbsolute:
      case cssom::KeywordValue::kAlternate:
      case cssom::KeywordValue::kAlternateReverse:
      case cssom::KeywordValue::kBackwards:
      case cssom::KeywordValue::kBaseline:
      case cssom::KeywordValue::kBlock:
      case cssom::KeywordValue::kBoth:
      case cssom::KeywordValue::kBottom:
      case cssom::KeywordValue::kBreakWord:
      case cssom::KeywordValue::kCenter:
      case cssom::KeywordValue::kClip:
      case cssom::KeywordValue::kContain:
      case cssom::KeywordValue::kCover:
      case cssom::KeywordValue::kCurrentColor:
      case cssom::KeywordValue::kCursive:
      case cssom::KeywordValue::kEllipsis:
      case cssom::KeywordValue::kFantasy:
      case cssom::KeywordValue::kForwards:
      case cssom::KeywordValue::kFixed:
      case cssom::KeywordValue::kHidden:
      case cssom::KeywordValue::kInfinite:
      case cssom::KeywordValue::kInherit:
      case cssom::KeywordValue::kInitial:
      case cssom::KeywordValue::kInline:
      case cssom::KeywordValue::kInlineBlock:
      case cssom::KeywordValue::kLeft:
      case cssom::KeywordValue::kMiddle:
      case cssom::KeywordValue::kMonospace:
      case cssom::KeywordValue::kNone:
      case cssom::KeywordValue::kNoRepeat:
      case cssom::KeywordValue::kNormal:
      case cssom::KeywordValue::kNoWrap:
      case cssom::KeywordValue::kPre:
      case cssom::KeywordValue::kRelative:
      case cssom::KeywordValue::kRepeat:
      case cssom::KeywordValue::kReverse:
      case cssom::KeywordValue::kRight:
      case cssom::KeywordValue::kSansSerif:
      case cssom::KeywordValue::kSerif:
      case cssom::KeywordValue::kSolid:
      case cssom::KeywordValue::kStatic:
      case cssom::KeywordValue::kTop:
      case cssom::KeywordValue::kUppercase:
      case cssom::KeywordValue::kVisible:
      default:
        NOTREACHED();
    }
  }
};

class UsedMaxLengthProvider : public UsedLengthValueProvider {
 public:
  explicit UsedMaxLengthProvider(float percentage_base)
      : UsedLengthValueProvider(percentage_base) {}

  void VisitKeyword(cssom::KeywordValue* keyword) OVERRIDE {
    switch (keyword->value()) {
      case cssom::KeywordValue::kNone:
        depends_on_containing_block_ = true;

        // Leave |used_length_| in disengaged state to indicate that "none"
        // was the value.
        break;

      case cssom::KeywordValue::kAbsolute:
      case cssom::KeywordValue::kAlternate:
      case cssom::KeywordValue::kAlternateReverse:
      case cssom::KeywordValue::kAuto:
      case cssom::KeywordValue::kBackwards:
      case cssom::KeywordValue::kBaseline:
      case cssom::KeywordValue::kBlock:
      case cssom::KeywordValue::kBoth:
      case cssom::KeywordValue::kBottom:
      case cssom::KeywordValue::kBreakWord:
      case cssom::KeywordValue::kCenter:
      case cssom::KeywordValue::kClip:
      case cssom::KeywordValue::kContain:
      case cssom::KeywordValue::kCover:
      case cssom::KeywordValue::kCurrentColor:
      case cssom::KeywordValue::kCursive:
      case cssom::KeywordValue::kEllipsis:
      case cssom::KeywordValue::kFantasy:
      case cssom::KeywordValue::kForwards:
      case cssom::KeywordValue::kFixed:
      case cssom::KeywordValue::kHidden:
      case cssom::KeywordValue::kInfinite:
      case cssom::KeywordValue::kInherit:
      case cssom::KeywordValue::kInitial:
      case cssom::KeywordValue::kInline:
      case cssom::KeywordValue::kInlineBlock:
      case cssom::KeywordValue::kLeft:
      case cssom::KeywordValue::kMiddle:
      case cssom::KeywordValue::kMonospace:
      case cssom::KeywordValue::kNoRepeat:
      case cssom::KeywordValue::kNormal:
      case cssom::KeywordValue::kNoWrap:
      case cssom::KeywordValue::kPre:
      case cssom::KeywordValue::kRelative:
      case cssom::KeywordValue::kRepeat:
      case cssom::KeywordValue::kReverse:
      case cssom::KeywordValue::kRight:
      case cssom::KeywordValue::kSansSerif:
      case cssom::KeywordValue::kSerif:
      case cssom::KeywordValue::kSolid:
      case cssom::KeywordValue::kStatic:
      case cssom::KeywordValue::kTop:
      case cssom::KeywordValue::kUppercase:
      case cssom::KeywordValue::kVisible:
      default:
        NOTREACHED();
    }
  }
};

}  // namespace

base::optional<float> GetUsedLeftIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/visuren.html#position-props
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->left()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedTopIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to height of containing block.
  //   http://www.w3.org/TR/CSS21/visuren.html#position-props
  UsedLengthProvider used_length_provider(containing_block_size.height());
  computed_style->top()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedRightIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/visuren.html#position-props
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->right()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedBottomIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to height of containing block.
  //   http://www.w3.org/TR/CSS21/visuren.html#position-props
  UsedLengthProvider used_length_provider(containing_block_size.height());
  computed_style->bottom()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedWidthIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size,
    bool* width_depends_on_containing_block) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#the-width-property
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->width()->Accept(&used_length_provider);
  if (width_depends_on_containing_block != NULL) {
    *width_depends_on_containing_block =
        used_length_provider.depends_on_containing_block();
  }
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMaxHeightIfNotNone(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size,
    bool* height_depends_on_containing_block) {
  // Percentages: refer to height of containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#propdef-max-height
  UsedMaxLengthProvider used_length_provider(containing_block_size.height());
  computed_style->max_height()->Accept(&used_length_provider);
  if (height_depends_on_containing_block != NULL) {
    *height_depends_on_containing_block =
        used_length_provider.depends_on_containing_block();
  }
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMaxWidthIfNotNone(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size,
    bool* width_depends_on_containing_block) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#propdef-max-width
  UsedMaxLengthProvider used_length_provider(containing_block_size.width());
  computed_style->max_width()->Accept(&used_length_provider);
  if (width_depends_on_containing_block != NULL) {
    *width_depends_on_containing_block =
        used_length_provider.depends_on_containing_block();
  }
  return used_length_provider.used_length();
}

float GetUsedMinHeight(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size,
    bool* height_depends_on_containing_block) {
  // Percentages: refer to height of containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#propdef-max-height
  UsedLengthValueProvider used_length_provider(containing_block_size.height());
  computed_style->min_height()->Accept(&used_length_provider);
  if (height_depends_on_containing_block != NULL) {
    *height_depends_on_containing_block =
        used_length_provider.depends_on_containing_block();
  }
  return *used_length_provider.used_length();
}

float GetUsedMinWidth(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size,
    bool* width_depends_on_containing_block) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#propdef-min-width
  UsedLengthValueProvider used_length_provider(containing_block_size.width());
  computed_style->min_width()->Accept(&used_length_provider);
  if (width_depends_on_containing_block != NULL) {
    *width_depends_on_containing_block =
        used_length_provider.depends_on_containing_block();
  }
  return *used_length_provider.used_length();
}

base::optional<float> GetUsedHeightIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // The percentage is calculated with respect to the height of the generated
  // box's containing block.
  //   http://www.w3.org/TR/CSS21/visudet.html#the-height-property
  UsedLengthProvider used_length_provider(containing_block_size.height());
  computed_style->height()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMarginLeftIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#margin-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->margin_left()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMarginTopIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#margin-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->margin_top()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMarginRightIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#margin-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->margin_right()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

base::optional<float> GetUsedMarginBottomIfNotAuto(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#margin-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->margin_bottom()->Accept(&used_length_provider);
  return used_length_provider.used_length();
}

float GetUsedBorderLeft(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style) {
  return base::polymorphic_downcast<const cssom::LengthValue*>(
             computed_style->border_left_width().get())
      ->value();
}

float GetUsedBorderTop(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style) {
  return base::polymorphic_downcast<const cssom::LengthValue*>(
             computed_style->border_top_width().get())
      ->value();
}

float GetUsedBorderRight(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style) {
  return base::polymorphic_downcast<const cssom::LengthValue*>(
             computed_style->border_right_width().get())
      ->value();
}

float GetUsedBorderBottom(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style) {
  return base::polymorphic_downcast<const cssom::LengthValue*>(
             computed_style->border_bottom_width().get())
      ->value();
}

float GetUsedPaddingLeft(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#padding-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->padding_left()->Accept(&used_length_provider);
  return *used_length_provider.used_length();
}

float GetUsedPaddingTop(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#padding-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->padding_top()->Accept(&used_length_provider);
  return *used_length_provider.used_length();
}

float GetUsedPaddingRight(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#padding-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->padding_right()->Accept(&used_length_provider);
  return *used_length_provider.used_length();
}

float GetUsedPaddingBottom(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& computed_style,
    const math::SizeF& containing_block_size) {
  // Percentages: refer to width of containing block.
  //   http://www.w3.org/TR/CSS21/box.html#padding-properties
  UsedLengthProvider used_length_provider(containing_block_size.width());
  computed_style->padding_bottom()->Accept(&used_length_provider);
  return *used_length_provider.used_length();
}

}  // namespace layout
}  // namespace cobalt
