/*
 * Copyright 2015 Google Inc. All Rights Reserved.
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

#ifndef DOM_XML_DOCUMENT_H_
#define DOM_XML_DOCUMENT_H_

#include "cobalt/dom/document.h"

namespace cobalt {
namespace dom {

class XMLDocument : public Document {
 public:
  XMLDocument() : Document(NULL, Document::Options()) {}
  explicit XMLDocument(const Options& options) : Document(NULL, options) {}

  // Custom, not in any spec: Node.
  scoped_refptr<Node> Duplicate() const OVERRIDE {
    return new XMLDocument(Document::Options(url_));
  }

  // Custom, not in any spec: Document.
  bool IsXMLDocument() const OVERRIDE { return true; }

  DEFINE_WRAPPABLE_TYPE(XMLDocument);

 private:
  ~XMLDocument() {}

  DISALLOW_COPY_AND_ASSIGN(XMLDocument);
};

}  // namespace dom
}  // namespace cobalt

#endif  // DOM_XML_DOCUMENT_H_
