// This file was GENERATED by command:
//     pump.py mozjs_callback_function.h.pump
// DO NOT EDIT BY HAND!!!

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

// clang-format off

#ifndef COBALT_SCRIPT_MOZJS_MOZJS_CALLBACK_FUNCTION_H_
#define COBALT_SCRIPT_MOZJS_MOZJS_CALLBACK_FUNCTION_H_

#include "base/logging.h"
#include "cobalt/script/callback_function.h"
#include "cobalt/script/mozjs/conversion_helpers.h"
#include "cobalt/script/mozjs/convert_callback_return_value.h"
#include "third_party/mozjs/js/src/jsapi.h"
#include "third_party/mozjs/js/src/jscntxt.h"

namespace cobalt {
namespace script {
namespace mozjs {

// First, we forward declare the Callback class template. This informs the
// compiler that the template only has 1 type parameter which is the base
// CallbackFunction template class with parameters.
//
// See base/callback.h.pump for further discussion on this pattern.
template <typename Sig>
class MozjsCallbackFunction;

template <typename R>
class MozjsCallbackFunction<R(void)>
    : public CallbackFunction<R(void)> {
 public:
  typedef CallbackFunction<R()> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run()
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 0;

    JSBool call_result = JS::Call(context_, this_value, function_, 0, NULL,
        return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1>
class MozjsCallbackFunction<R(A1)>
    : public CallbackFunction<R(A1)> {
 public:
  typedef CallbackFunction<R(A1)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 1;

    JS::Value args[1];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2>
class MozjsCallbackFunction<R(A1, A2)>
    : public CallbackFunction<R(A1, A2)> {
 public:
  typedef CallbackFunction<R(A1, A2)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 2;

    JS::Value args[2];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2, typename A3>
class MozjsCallbackFunction<R(A1, A2, A3)>
    : public CallbackFunction<R(A1, A2, A3)> {
 public:
  typedef CallbackFunction<R(A1, A2, A3)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2,
      typename base::internal::CallbackParamTraits<A3>::ForwardType a3)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 3;

    JS::Value args[3];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));
    ToJSValue(context_, a3, auto_array_rooter.handleAt(2));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class MozjsCallbackFunction<R(A1, A2, A3, A4)>
    : public CallbackFunction<R(A1, A2, A3, A4)> {
 public:
  typedef CallbackFunction<R(A1, A2, A3, A4)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2,
      typename base::internal::CallbackParamTraits<A3>::ForwardType a3,
      typename base::internal::CallbackParamTraits<A4>::ForwardType a4)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 4;

    JS::Value args[4];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));
    ToJSValue(context_, a3, auto_array_rooter.handleAt(2));
    ToJSValue(context_, a4, auto_array_rooter.handleAt(3));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class MozjsCallbackFunction<R(A1, A2, A3, A4, A5)>
    : public CallbackFunction<R(A1, A2, A3, A4, A5)> {
 public:
  typedef CallbackFunction<R(A1, A2, A3, A4, A5)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2,
      typename base::internal::CallbackParamTraits<A3>::ForwardType a3,
      typename base::internal::CallbackParamTraits<A4>::ForwardType a4,
      typename base::internal::CallbackParamTraits<A5>::ForwardType a5)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 5;

    JS::Value args[5];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));
    ToJSValue(context_, a3, auto_array_rooter.handleAt(2));
    ToJSValue(context_, a4, auto_array_rooter.handleAt(3));
    ToJSValue(context_, a5, auto_array_rooter.handleAt(4));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class MozjsCallbackFunction<R(A1, A2, A3, A4, A5, A6)>
    : public CallbackFunction<R(A1, A2, A3, A4, A5, A6)> {
 public:
  typedef CallbackFunction<R(A1, A2, A3, A4, A5, A6)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2,
      typename base::internal::CallbackParamTraits<A3>::ForwardType a3,
      typename base::internal::CallbackParamTraits<A4>::ForwardType a4,
      typename base::internal::CallbackParamTraits<A5>::ForwardType a5,
      typename base::internal::CallbackParamTraits<A6>::ForwardType a6)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 6;

    JS::Value args[6];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));
    ToJSValue(context_, a3, auto_array_rooter.handleAt(2));
    ToJSValue(context_, a4, auto_array_rooter.handleAt(3));
    ToJSValue(context_, a5, auto_array_rooter.handleAt(4));
    ToJSValue(context_, a6, auto_array_rooter.handleAt(5));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
class MozjsCallbackFunction<R(A1, A2, A3, A4, A5, A6, A7)>
    : public CallbackFunction<R(A1, A2, A3, A4, A5, A6, A7)> {
 public:
  typedef CallbackFunction<R(A1, A2, A3, A4, A5, A6, A7)> BaseType;

  explicit MozjsCallbackFunction(JSContext* context, JS::HandleObject function)
      : context_(context), function_(function) {
    DCHECK(context_);
    DCHECK(JS_ObjectIsFunction(context_, function_));
  }

  CallbackResult<R> Run(
      typename base::internal::CallbackParamTraits<A1>::ForwardType a1,
      typename base::internal::CallbackParamTraits<A2>::ForwardType a2,
      typename base::internal::CallbackParamTraits<A3>::ForwardType a3,
      typename base::internal::CallbackParamTraits<A4>::ForwardType a4,
      typename base::internal::CallbackParamTraits<A5>::ForwardType a5,
      typename base::internal::CallbackParamTraits<A6>::ForwardType a6,
      typename base::internal::CallbackParamTraits<A7>::ForwardType a7)
      const OVERRIDE {
    JSAutoRequest auto_request(context_);
    JSAutoCompartment auto_compartment(context_, function_);

    // https://www.w3.org/TR/WebIDL/#es-invoking-callback-functions
    // Callback 'this' is set to null, unless overridden by other specifications
    JS::Value this_value(JS::NullValue());
    JS::RootedValue return_value(context_);
    const int kNumArguments = 7;

    JS::Value args[7];
    js::SetValueRangeToNull(args, kNumArguments);
    js::AutoValueArray auto_array_rooter(context_, args, kNumArguments);
    ToJSValue(context_, a1, auto_array_rooter.handleAt(0));
    ToJSValue(context_, a2, auto_array_rooter.handleAt(1));
    ToJSValue(context_, a3, auto_array_rooter.handleAt(2));
    ToJSValue(context_, a4, auto_array_rooter.handleAt(3));
    ToJSValue(context_, a5, auto_array_rooter.handleAt(4));
    ToJSValue(context_, a6, auto_array_rooter.handleAt(5));
    ToJSValue(context_, a7, auto_array_rooter.handleAt(6));

    JSBool call_result = JS::Call(context_, this_value, function_,
        kNumArguments, args, return_value.address());

    CallbackResult<R> callback_result;
    if (!call_result) {
      DLOG(WARNING) << "Exception in callback.";
      callback_result.exception = true;
    } else {
      callback_result = ConvertCallbackReturnValue<R>(context_, return_value);
    }
    return callback_result;
  }

  JSObject* handle() const { return function_; }

 private:
  JSContext* context_;
  mutable JS::Heap<JSObject*> function_;
};

template <typename Signature>
struct TypeTraits<CallbackFunction<Signature> > {
  typedef MozjsUserObjectHolder<MozjsCallbackFunction<Signature> >
      ConversionType;
  typedef const ScriptObject<CallbackFunction<Signature> >* ReturnType;
};

}  // namespace mozjs
}  // namespace script
}  // namespace cobalt

#endif  // COBALT_SCRIPT_MOZJS_MOZJS_CALLBACK_FUNCTION_H_
