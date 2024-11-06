#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

class CodeBlock {
  struct _Inc_Ty {};
  struct _Dec_ty {};
  std::vector<std::variant<std::string, _Inc_Ty, _Dec_ty>> lines;

public:
  static constexpr _Inc_Ty inc = _Inc_Ty();
  static constexpr _Dec_ty dec = _Dec_ty();

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

  CodeBlock& operator<<(const _Inc_Ty&) {
    lines.push_back(_Inc_Ty());
    return *this;
  }

  CodeBlock& operator<<(const _Dec_ty&) {
    lines.push_back(_Dec_ty());
    return *this;
  }

  std::string str() const {
    std::ostringstream out;
    size_t indent_level = 0;
    for (const auto& line : lines) {
      if (std::holds_alternative<std::string>(line)) {
        for (size_t i = 0; i < indent_level; i++) {
          out << indent;
        }
        out << std::get<std::string>(line) << std::endl;
      } else if (std::holds_alternative<_Inc_Ty>(line)) {
        indent_level++;
      } else if (std::holds_alternative<_Dec_ty>(line)) {
        indent_level--;
      } else {
        throw std::runtime_error("Unknown type in lines");
      }
    }
    return out.str();
  }

  // std::string str() const
  // {
  //   struct CodeBlockProgress
  //   {
  //     std::vector<std::variant<std::string, CodeBlock>>::const_iterator it;
  //     std::vector<std::variant<std::string, CodeBlock>>::const_iterator end;
  //   };
  //   std::stack<CodeBlockProgress> blockStack;
  //   blockStack.emplace(lines.cbegin(), lines.cend());
  //   std::ostringstream out;
  //   do
  //   {
  //     auto &block = blockStack.top();
  //     blockStack.pop();
  //     for (; block.it != block.end; block.it++)
  //     {
  //       if (std::holds_alternative<std::string>(*block.it))
  //       {
  //         const auto indent = blockStack.size();
  //         for (size_t i = 0; i < indent; i++)
  //         {
  //           out << "  ";
  //         }
  //         out << std::get<std::string>(*block.it) << std::endl;
  //       }
  //       else if (std::holds_alternative<CodeBlock>(*block.it))
  //       {
  //         blockStack.emplace(block.it + 1, block.end);
  //         blockStack.emplace(std::get<CodeBlock>(*block.it).lines.cbegin(),
  //         std::get<CodeBlock>(*block.it).lines.cend()); break;
  //       }
  //       else
  //       {
  //         throw std::runtime_error("Unknown type in lines");
  //       }
  //     }

  //   } while (!blockStack.empty());
  //   return out.str();
  // }
};