
#include "SyncedSchema/ArrayProperties.h"
#include "CodeBlock.h"
#include "SyncedSchema.h"
#include <format>

ArrayProperties::ArrayProperties(
    const IndexedSyncedSchema& schema,
    const std::vector<std::unique_ptr<SyncedSchema>>& syncedSchemas)
    : items_(schema.items_.has_value() ? *syncedSchemas[schema.items_.value()]
                                       : SyncedSchema::getTrueSchema()) {
  if (schema.tupleableItems_.has_value()) {
    tupleableItems_ = std::vector<std::reference_wrapper<const SyncedSchema>>();
    for (auto index : schema.tupleableItems_.value()) {
      tupleableItems_.value().push_back(*syncedSchemas[index]);
    }
  }
  if (schema.maxItems_.has_value()) {
    maxItems_ = schema.maxItems_.value();
  }
  if (schema.minItems_.has_value()) {
    minItems_ = schema.minItems_.value();
  }
  if (schema.uniqueItems_.has_value()) {
    uniqueItems_ = schema.uniqueItems_.value();
  }
  if (schema.contains_.has_value()) {
    contains_ = *syncedSchemas[schema.contains_.value()];
  }
}

CodeBlock ArrayProperties::arrayConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_array()) {{", inputJsonVariableName);

  {
    Indent _(block);
    BLOCK << "auto array = Array();";
    if (tupleableItems_.has_value()) {
      for (size_t i = 0; i < tupleableItems_->size(); i++) {
        BLOCK << std::format("if({}.size() > {}) {{", inputJsonVariableName, i)
              << CodeBlock::inc;
        if (codeProperties.minItemsMakeTupleableRequired_ &&
            minItems_.has_value() && i < minItems_.value()) {
          BLOCK << std::format("array.item{} = {}::construct({}[{}]).value();",
                               i, (*tupleableItems_)[i].get().identifier_,
                               inputJsonVariableName, i);
        } else {
          BLOCK << std::format("array.item{} = {}::construct({}[{}]);", i,
                               (*tupleableItems_)[i].get().identifier_,
                               inputJsonVariableName, i);
        }
        BLOCK << CodeBlock::dec << "}";
      }
    }
    BLOCK << std::format(
        "for (size_t i = {}; i < {}.size(); i++) {{",
        tupleableItems_
            .value_or(std::vector<std::reference_wrapper<const SyncedSchema>>())
            .size(),
        inputJsonVariableName);
    {
      Indent _(block);
      BLOCK << std::format(
          "array.items.push_back({}::construct(json[i]).value());",
          items_.get().identifier_);
    }
    BLOCK << "}";
    BLOCK << std::format("{} = std::move(array);", outSchemaVariableName);
  }
  BLOCK << "}";

  return block;
}

std::string ArrayProperties::getArrayType(std::string namespaceLocation) const {
  return std::format("{}::Array", namespaceLocation);
}

CodeBlock
ArrayProperties::arrayClassDefinition(const CodeProperties& codeProperties,
                                      std::string schemaType) const {
  CodeBlock block(codeProperties.indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " array class definition*/";
#endif
  // Array declaration
  BLOCK << "class Array {" << CodeBlock::inc;
  {
    // Declare the construct function to be a friend, so it can fill out
    // private members, should it be necessary
    BLOCK << std::format(
        "friend std::optional<{}> construct(const nlohmann::json&);",
        schemaType);
    // Tupleable item declaration
    BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
    if (tupleableItems_.has_value()) {
      for (size_t i = 0; i < tupleableItems_->size(); i++) {
        if (codeProperties.minItemsMakeTupleableRequired_ &&
            minItems_.has_value() && i < minItems_.value()) {
          BLOCK << std::format("{} item{};",
                               (*tupleableItems_)[i].get().getType(), i);
        } else {
          BLOCK << std::format("std::optional<{}> item{};",
                               (*tupleableItems_)[i].get().getType(), i);
        }
      }
    }
    BLOCK << std::format("std::vector<{}> items;", items_.get().getType());

    BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
    BLOCK << "template <size_t N>";
    BLOCK << "auto get() const {";
    BLOCK << CodeBlock::inc;
    {
      if (tupleableItems_.has_value() && tupleableItems_->size() > 0) {
        for (size_t i = 0; i < tupleableItems_->size(); i++) {
          BLOCK << std::format("if constexpr(N == {}) {{", i) << CodeBlock::inc;
          {
            if (codeProperties.minItemsMakeTupleableRequired_ &&
                minItems_.has_value() && i < minItems_.value()) {
              BLOCK << std::format("return item{};", i);
            } else {
              BLOCK << std::format("if (item{}.has_value()) {{", i)
                    << CodeBlock::inc;
            }
            BLOCK << std::format("return item{}.value();", i);
            BLOCK << CodeBlock::dec << "}";
          }
          BLOCK << CodeBlock::dec << CodeBlock::dis << "} else ";
        }
        BLOCK << "{" << CodeBlock::inc
              << std::format("if(N - {} < items.size()) {{",
                             tupleableItems_->size())
              << CodeBlock::inc;
        {
          BLOCK << std::format("return items[N - {}];",
                               tupleableItems_->size());
        }
        BLOCK << CodeBlock::dec << "}" << CodeBlock::dec << "}";
      } else {
        BLOCK << "if(N < items.size()) {" << CodeBlock::inc
              << "return items[N];" << CodeBlock::dec << "}";
      }
      BLOCK << "throw std::range_error(std::string(\"Item \") + "
               "std::to_string(N) + \" out of range\");";
    }
    BLOCK << CodeBlock::dec << "}";
    BLOCK << std::format("inline {}& get(size_t n) {{", items_.get().getType())
          << CodeBlock::inc << "if(n >= items.size()) {"
          << "throw std::range_error(\"Item \" + std::to_string(n) + \" out "
             "of range\");"
          << "}"
          << "return items[n];" << CodeBlock::dec << "}";
  }
  BLOCK << CodeBlock::dec;
  BLOCK << "};";

  return block;
}