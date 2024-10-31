#include <string>
#include <strstream>
#include <vector>
#include <variant>
#include <stack>

class CodeBlock
{
  std::vector<std::variant<std::string, CodeBlock>> lines;

public:
  CodeBlock() = default;

  CodeBlock &operator<<(const std::string &line)
  {
    lines.push_back(line);
    return *this;
  }

  CodeBlock &operator<<(const CodeBlock &block)
  {
    lines.push_back(block);
    return *this;
  }

  std::string str() const
  {
    struct CodeBlockProgress
    {
      std::vector<std::variant<std::string, CodeBlock>>::const_iterator it;
      std::vector<std::variant<std::string, CodeBlock>>::const_iterator end;
    };
    std::stack<CodeBlockProgress> blockStack;
    blockStack.emplace(lines.cbegin(), lines.cend());
    std::ostrstream out;
    do
    {
      auto &block = blockStack.top();
      blockStack.pop();
      for (; block.it != block.end; block.it++)
      {
        if (std::holds_alternative<std::string>(*block.it))
        {
          const auto indent = blockStack.size();
          for (size_t i = 0; i < indent; i++)
          {
            out << "  ";
          }
          out << std::get<std::string>(*block.it) << std::endl;
        }
        else
        {
          blockStack.emplace(block.it + 1, block.end);
          blockStack.emplace(std::get<CodeBlock>(*block.it).lines.cbegin(), std::get<CodeBlock>(*block.it).lines.cend());
          break;
        }
      }

    } while (!blockStack.empty());
  }
};