#ifndef EXPORTCONTROLLER_H
#define EXPORTCONTROLLER_H

#include <QIODevice>
#include <QString>

#include "../Model/recordexporttemplatemap.h"
#include "../../Fields//Controller/fielddefinitionscontroller.h"
#include "../../Records/Controller/recordscontroller.h"
#include "../../Types/Controller/typescontroller.h"

namespace Tome
{
    class ExportController
    {
        public:
            ExportController(const FieldDefinitionsController& fieldDefinitionsController, const RecordsController& recordsController, const TypesController& typesController);

            const RecordExportTemplate getRecordExportTemplate(const QString& name) const;
            const RecordExportTemplateMap& getRecordExportTemplates() const;

            void exportRecords(const RecordExportTemplate& exportTemplate, const QString& filePath);
            void exportRecords(const RecordExportTemplate& exportTemplate, QIODevice& device);
            void setRecordExportTemplates(RecordExportTemplateMap& model);

        private:
            RecordExportTemplateMap* model;

            static const QString PlaceholderComponents;
            static const QString PlaceholderComponentName;
            static const QString PlaceholderFieldId;
            static const QString PlaceholderFieldType;
            static const QString PlaceholderFieldValue;
            static const QString PlaceholderListItem;
            static const QString PlaceholderRecordFields;
            static const QString PlaceholderRecordId;
            static const QString PlaceholderRecordParentId;
            static const QString PlaceholderRecords;

            const FieldDefinitionsController& fieldDefinitionsController;
            const RecordsController& recordsController;
            const TypesController& typesController;
    };
}

#endif // EXPORTCONTROLLER_H
