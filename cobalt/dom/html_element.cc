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

#include "cobalt/dom/html_element.h"

#include "cobalt/dom/document.h"
#include "cobalt/dom/html_body_element.h"
#include "cobalt/dom/html_div_element.h"
#include "cobalt/dom/html_element_factory.h"
#include "cobalt/dom/html_head_element.h"
#include "cobalt/dom/html_html_element.h"
#include "cobalt/dom/html_link_element.h"
#include "cobalt/dom/html_media_element.h"
#include "cobalt/dom/html_script_element.h"
#include "cobalt/dom/html_span_element.h"
#include "cobalt/dom/html_style_element.h"
#include "cobalt/dom/html_unknown_element.h"

namespace cobalt {
namespace dom {

scoped_refptr<Node> HTMLElement::Duplicate() const {
  HTMLElement* new_html_element =
      html_element_factory_->CreateHTMLElement(tag_name());
  new_html_element->attribute_map_ = attribute_map_;
  return new_html_element;
}

scoped_refptr<HTMLBodyElement> HTMLElement::AsHTMLBodyElement() { return NULL; }

scoped_refptr<HTMLDivElement> HTMLElement::AsHTMLDivElement() { return NULL; }

scoped_refptr<HTMLHeadElement> HTMLElement::AsHTMLHeadElement() { return NULL; }

scoped_refptr<HTMLHtmlElement> HTMLElement::AsHTMLHtmlElement() { return NULL; }

scoped_refptr<HTMLLinkElement> HTMLElement::AsHTMLLinkElement() { return NULL; }

scoped_refptr<HTMLMediaElement> HTMLElement::AsHTMLMediaElement() {
  return NULL;
}

scoped_refptr<HTMLScriptElement> HTMLElement::AsHTMLScriptElement() {
  return NULL;
}

scoped_refptr<HTMLSpanElement> HTMLElement::AsHTMLSpanElement() { return NULL; }

scoped_refptr<HTMLStyleElement> HTMLElement::AsHTMLStyleElement() {
  return NULL;
}

HTMLElement::HTMLElement(HTMLElementFactory* html_element_factory,
                         cssom::CSSParser* css_parser)
    : Element(html_element_factory),
      style_(new cssom::CSSStyleDeclaration(css_parser)) {}

void HTMLElement::SetOpeningTagLocation(
    const base::SourceLocation& /*opening_tag_location*/) {}

HTMLElement::~HTMLElement() {}

void HTMLElement::AttachToDocument(Document* document) {
  Node::AttachToDocument(document);
  style_->set_mutation_observer(document);
}

}  // namespace dom
}  // namespace cobalt
