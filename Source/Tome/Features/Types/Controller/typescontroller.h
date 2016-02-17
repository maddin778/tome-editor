#ifndef TYPESCONTROLLER_H
#define TYPESCONTROLLER_H

#include "../Model/customtypelist.h"


namespace Tome
{
    class TypesController
    {
        public:
            TypesController();

            const CustomType addEnumeration(const QString& name, const QStringList& enumeration);
            const CustomType addList(const QString& name, const QString& itemType);

            /**
             * @brief getCustomType Returns the custom type with the specified name.
             * @param name Name of the type to get.
             * @return Custom type with the specified name.
             */
            const CustomType& getCustomType(const QString& name) const;

            const CustomTypeList& getCustomTypes() const;

            /**
             * @brief getCustomTypeNames Returns a list of all type names of this project, including built-in types.
             * @return List containing the names of all types avaialable in this project.
             */
            const QStringList getTypeNames() const;

            int indexOf(const CustomType& customType) const;
            bool isCustomType(const QString& name) const;
            void removeCustomTypeAt(const int index);
            void setCustomTypes(CustomTypeList& model);
            void updateEnumeration(const QString& oldName, const QString& newName, const QStringList& enumeration);
            void updateList(const QString& oldName, const QString& newName, const QString& itemType);

        private:
            CustomTypeList* model;

            CustomType* getCustomTypeByName(const QString& name) const;
    };
}

#endif // TYPESCONTROLLER_H