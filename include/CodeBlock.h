#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#ifndef JSOG_DEBUG
#define JSOG_DEBUG 0
#endif

/// @brief A class that helps with code generation by providing a way to
/// group code line by line with indent increases and decreases recorded.
class CodeBlock {
  /// @brief An empty increase indent type
  struct _Inc_Ty {};
  /// @brief An empty decrease indent type
  struct _Dec_ty {};
  /// @brief An empty discard newline type
  struct _Discard_Ty {};

  /// @brief The lines of code
  std::vector<std::variant<std::string, _Inc_Ty, _Dec_ty, _Discard_Ty>> lines;

public:
  /// @brief Publically accessible increase indent type
  static constexpr _Inc_Ty inc = _Inc_Ty();
  /// @brief Publically accessible decrease indent type
  static constexpr _Dec_ty dec = _Dec_ty();
  /// @brief Discard the next newline
  static constexpr _Discard_Ty dis = _Discard_Ty();

  /// @brief Indent to use for each level of indentation
  std::string indent;

  CodeBlock(std::string default_indent = "  ") : indent(default_indent) {};

  CodeBlock& operator<<(const std::string& line) {
    lines.push_back(line);
    return *this;
  }

  CodeBlock& operator<<(const CodeBlock& block) {
    lines.insert(lines.end(), block.lines.begin(), block.lines.end());
    return *this;
  }

  CodeBlock& operator<<(_Inc_Ty) {
    lines.emplace_back(inc);
    return *this;
  }

  CodeBlock& operator<<(_Dec_ty) {
    lines.emplace_back(dec);
    return *this;
  }

  CodeBlock& operator<<(_Discard_Ty) {
    lines.emplace_back(dis);
    return *this;
  }

  /// @brief Returns a string representation of the code block, respecting
  /// the indentation levels
  std::string str() const {
    std::ostringstream out;
    size_t indentLevel = 0;
    bool skipNewline = false;
    bool skipIndent = false;
    for (const auto& line : lines) {
      if (std::holds_alternative<std::string>(line)) {
        if (!skipIndent) {
          for (size_t i = 0; i < indentLevel; i++) {
            out << indent;
          }
        }
        skipIndent = false;
        out << std::get<std::string>(line);
        if (!skipNewline) {
          out << std::endl;
        } else {
          skipNewline = false;
          skipIndent = true;
        }
      } else if (std::holds_alternative<_Inc_Ty>(line)) {
        indentLevel++;
      } else if (std::holds_alternative<_Dec_ty>(line)) {
        indentLevel--;
      } else if (std::holds_alternative<_Discard_Ty>(line)) {
        skipNewline = true;
      } else {
        throw std::runtime_error("Unknown type in lines");
      }
    }
    return out.str();
  }
};

/// @brief A class that helps with code generation by providing a way to
/// group code line by line with indent increases and decreases recorded.
///
/// Indentation is automatically increased and decreased when the object
/// is created and destroyed.
struct Indent {
  CodeBlock& block;
  Indent(CodeBlock& block) : block(block) { block << CodeBlock::inc; }
  ~Indent() { block << CodeBlock::dec; }
};

#if JSOG_DEBUG
static std::string centerPadString(const std::string& s, size_t width) {
  if (s.size() >= width) {
    return s;
  }
  size_t leftPad = (width - s.size()) / 2;
  size_t rightPad = width - s.size() - leftPad;
  return std::string(leftPad, ' ') + s + std::string(rightPad, ' ');
}
#define BLOCK                                                                  \
  {                                                                            \
    block << CodeBlock::dis                                                    \
          << centerPadString(                                                  \
                 std::format("/*{}:{}*/", __FILE_NAME__, __LINE__), 40);       \
  }                                                                            \
  block
#else
#define BLOCK block
#endif
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#endif // CODEBLOCK_H