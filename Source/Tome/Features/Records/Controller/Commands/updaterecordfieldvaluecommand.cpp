#include "updaterecordfieldvaluecommand.h"

#include "../recordscontroller.h"

using namespace Tome;


UpdateRecordFieldValueCommand::UpdateRecordFieldValueCommand(RecordsController& recordsController,
                                                             const QString& recordId,
                                                             const QString& fieldId,
                                                             const QVariant& newFieldValue)
    : recordsController(recordsController),
      recordId(recordId),
      fieldId(fieldId),
      newFieldValue(newFieldValue)
{
    this->setText("Set Field Value - " + fieldId);
}

void UpdateRecordFieldValueCommand::undo()
{
    // Restore field value.
    this->recordsController.updateRecordFieldValue(this->recordId, this->fieldId, this->oldFieldValue);
}

void UpdateRecordFieldValueCommand::redo()
{
    // Store current data.
    const RecordFieldValueMap fieldValues = this->recordsController.getRecordFieldValues(this->recordId);
    this->oldFieldValue = fieldValues[fieldId];

    // Update field value.
    this->recordsController.updateRecordFieldValue(this->recordId, this->fieldId, this->newFieldValue);
}
