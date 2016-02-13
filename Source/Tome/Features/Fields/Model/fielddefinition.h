#ifndef FIELDDEFINITION_H
#define FIELDDEFINITION_H

#include <QString>

#include "../../Components/Model/component.h"


namespace Tome
{
    class FieldDefinition
    {
        public:
            FieldDefinition();

            Component component;
            QString defaultValue;
            QString description;
            QString displayName;
            QString fieldType;
            QString id;
    };

    inline bool operator==(const FieldDefinition& lhs, const FieldDefinition& rhs){ return lhs.id == rhs.id; }
    inline bool operator!=(const FieldDefinition& lhs, const FieldDefinition& rhs){ return !(lhs == rhs); }

    inline bool fieldDefinitionLessThanDisplayName(const FieldDefinition& e1, const FieldDefinition& e2)
    {
        return e1.displayName.toLower() < e2.displayName.toLower();
    }
}

#endif // FIELDDEFINITION_H
