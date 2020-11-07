#include "text_input_event.hpp"

TextInputEvent::TextInputEvent(const eastl::wstring &text):
  text_{text}
{}

eastl::wstring TextInputEvent::text() const
{
  return text_;
}
