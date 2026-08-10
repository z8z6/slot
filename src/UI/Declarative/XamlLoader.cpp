#include "UI/Declarative/XamlLoader.h"

#include "UI/Declarative/ControlFactory.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/Layout.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace {
using z8::ui::BaseNode;
using z8::ui::ControlFactory;
using z8::ui::XamlLoadResult;

class XmlParser {
public:
  XmlParser(std::string_view source, ControlFactory& factory)
      : Source(source), Factory(factory) {}

  XamlLoadResult Parse() {
    SkipTrivia();
    auto root = ParseElement();
    if (!Error.empty()) return {nullptr, Error, ErrorAt};
    SkipTrivia();
    if (Pos != Source.size()) Fail("Unexpected content after the root control.");
    return Error.empty() ? XamlLoadResult{std::move(root), {}, 0}
                         : XamlLoadResult{nullptr, Error, ErrorAt};
  }

private:
  std::string_view Source;
  ControlFactory& Factory;
  size_t Pos = 0;
  size_t ErrorAt = 0;
  std::string Error;
  std::unordered_set<std::string> Keys;

  void Fail(std::string message) {
    if (!Error.empty()) return;
    ErrorAt = Pos;
    Error = std::move(message);
  }

  bool Starts(std::string_view token) const {
    return Source.substr(Pos, token.size()) == token;
  }

  void SkipWhitespace() {
    while (Pos < Source.size() && std::isspace(static_cast<unsigned char>(Source[Pos]))) ++Pos;
  }

  // 跳过无效字符
  void SkipTrivia() {
    for (;;) {
      SkipWhitespace();
      if (Starts("<!--")) {
        const auto end = Source.find("-->", Pos + 4);
        if (end == std::string_view::npos) { Fail("XML comment is not closed."); return; }
        Pos = end + 3;
      } else if (Starts("<?")) {
        const auto end = Source.find("?>", Pos + 2);
        if (end == std::string_view::npos) { Fail("XML declaration is not closed."); return; }
        Pos = end + 2;
      } else return;
    }
  }

  std::string ParseName() {
    const size_t begin = Pos;
    while (Pos < Source.size()) {
      const unsigned char c = static_cast<unsigned char>(Source[Pos]);
      if (!std::isalnum(c) && c != '_' && c != '-') break;
      ++Pos;
    }
    if (begin == Pos) Fail("Expected a control or attribute name.");
    return std::string(Source.substr(begin, Pos - begin));
  }

  std::string ParseValue() {
    if (Pos >= Source.size() || (Source[Pos] != '\'' && Source[Pos] != '"')) {
      Fail("Attribute values must be quoted.");
      return {};
    }
    const char quote = Source[Pos++];
    const size_t begin = Pos;
    while (Pos < Source.size() && Source[Pos] != quote) ++Pos;
    if (Pos == Source.size()) { Fail("Attribute value is not closed."); return {}; }
    std::string value(Source.substr(begin, Pos - begin));
    ++Pos;
    // 支持声明中最常用的 XML 实体；未知实体保持原样，避免静默损坏数据。
    const std::pair<std::string_view, std::string_view> entities[] = {
      {"&quot;", "\""}, {"&apos;", "'"}, {"&lt;", "<"}, {"&gt;", ">"}, {"&amp;", "&"}
    };
    for (const auto& [from, to] : entities) {
      size_t at = 0;
      while ((at = value.find(from, at)) != std::string::npos) {
        value.replace(at, from.size(), to);
        at += to.size();
      }
    }
    return value;
  }

  std::unique_ptr<BaseNode> ParseElement() {
    SkipTrivia();
    if (Pos >= Source.size() || Source[Pos++] != '<' || Starts("/")) {
      Fail("Expected a control start tag.");
      return nullptr;
    }

    const std::string type = ParseName();
    auto node = Factory.Create(type);
    if (!node) { Fail("Unknown control type: " + type); return nullptr; }

    for (;;) {
      SkipWhitespace();
      if (Starts("/>")) { Pos += 2; return node; }
      if (Starts(">")) { ++Pos; break; }
      const std::string name = ParseName();
      if (!Error.empty()) return nullptr;
      SkipWhitespace();
      if (Pos >= Source.size() || Source[Pos++] != '=') { Fail("Attribute is missing '=': " + name); return nullptr; }
      SkipWhitespace();
      const std::string value = ParseValue();
      if (!node->SetProperty(name, value)) { Fail("Control " + type + " does not support attribute " + name); return nullptr; }
      if ((name == "Id" || name == "Key" || name == "Name") &&
          !Keys.insert(value).second) {
        Fail("Duplicate control key: " + value);
        return nullptr;
      }
    }

    for (;;) {
      SkipTrivia();
      if (Starts("</")) {
        Pos += 2;
        const std::string close = ParseName();
        SkipWhitespace();
        if (Pos >= Source.size() || Source[Pos++] != '>') { Fail("Closing tag is missing '>'."); return nullptr; }
        if (close != type) { Fail("Mismatched closing tag. Expected " + type + ", got " + close); return nullptr; }
        return node;
      }
      if (Pos >= Source.size()) { Fail("Control " + type + " is missing a closing tag."); return nullptr; }
      if (Source[Pos] != '<') { Fail("Text nodes are not supported by this XAML subset."); return nullptr; }
      auto child = ParseElement();
      if (!child) return nullptr;
      node->ContentHost()->AddChild(std::move(child));
    }
  }
};
} // namespace

using namespace z8::ui;

XamlLoader::XamlLoader(ControlFactory& factory) : Factory(&factory) {}
XamlLoader::XamlLoader() : XamlLoader(ControlFactory::Instance()) {}

XamlLoadResult XamlLoader::Load(std::string_view source) const {
  return XmlParser(source, *Factory).Parse();
}

XamlLoadResult XamlLoader::LoadInto(Layout& layout, std::string_view source) const {
  auto result = Load(source);
  if (result) layout.SetRoot(std::move(result.Root));
  return result;
}

XamlLoadResult XamlLoader::LoadFileInto(Layout& layout, const std::string& fileName) const {
  std::ifstream input(fileName, std::ios::binary);
  if (!input) return {nullptr, "Unable to open XAML file: " + fileName, 0};
  std::ostringstream content;
  content << input.rdbuf();
  return LoadInto(layout, content.str());
}
