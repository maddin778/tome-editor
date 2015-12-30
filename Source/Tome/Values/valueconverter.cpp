#include "valueconverter.h"

using namespace Tome;


ValueConverter::ValueConverter()
{
}

QString ValueConverter::BoolToString(const bool b) const
{
    return b ? "True" : "False";
}

QString ValueConverter::FieldTypeToString(const FieldType::FieldType& fieldType) const
{
    switch (fieldType)
    {
        case FieldType::None:
            return "None";

        case FieldType::Boolean:
            return "Boolean";

        case FieldType::Color:
            return "Color";

        case FieldType::Integer:
            return "Integer";

        case FieldType::Real:
            return "Real";

        case FieldType::Reference:
            return "Reference";

        case FieldType::String:
            return "String";
    }

    return "Invalid";
}

bool ValueConverter::StringToBool(const QString& s) const
{
    return s == "True";
}

FieldType::FieldType ValueConverter::StringToFieldType(const QString& s) const
{
    if (s == "Boolean")
    {
        return FieldType::Boolean;
    }
    if (s == "Color")
    {
        return FieldType::Color;
    }
    if (s == "Integer")
    {
        return FieldType::Integer;
    }
    if (s == "Real")
    {
        return FieldType::Real;
    }
    if (s == "Reference")
    {
        return FieldType::Reference;
    }
    if (s == "String")
    {
        return FieldType::String;
    }

    return FieldType::None;
}
