#include "fielddefinitionstablemodel.h"

#include <QColor>

#include "../Values/valueconverter.h"

using namespace Tome;


bool lessThanFieldDefinitions(const QSharedPointer<FieldDefinition> e1, const QSharedPointer<FieldDefinition> e2)
{
    return e1->displayName.toLower() < e2->displayName.toLower();
}


FieldDefinitionsTableModel::FieldDefinitionsTableModel(QObject *parent, QSharedPointer<Tome::Project> project)
    : QAbstractTableModel(parent),
      project(project)
{
}

int FieldDefinitionsTableModel::rowCount(const QModelIndex& parent) const
{
   Q_UNUSED(parent);
   return this->project->fieldDefinitionSets[0]->fieldDefinitions.size();
}

int FieldDefinitionsTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant FieldDefinitionsTableModel::data(const QModelIndex& index, int role) const
{
    // Validate index.
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= this->rowCount())
    {
        return QVariant();
    }

    // Select field definition.
    QSharedPointer<FieldDefinitionSet> fieldDefinitionSet = this->project->fieldDefinitionSets[0];
    QSharedPointer<FieldDefinition> fieldDefinition = fieldDefinitionSet->fieldDefinitions[index.row()];

    if (role == Qt::DisplayRole)
    {
        // Show data.
        QSharedPointer<ValueConverter> valueConverter = QSharedPointer<ValueConverter>::create();

        switch (index.column())
        {
            case 0:
                return fieldDefinition->id;
            case 1:
                return fieldDefinition->displayName;
            case 2:
                return valueConverter->FieldTypeToString(fieldDefinition->fieldType);
            case 3:
                return fieldDefinition->defaultValue;
            case 4:
                return fieldDefinition->description;
        }

        return QVariant();
    }
    else if (role == Qt::DecorationRole && index.column() == 3 && fieldDefinition->fieldType == FieldType::Color)
    {
        // Show color preview.
        QColor color;
        color.setNamedColor(fieldDefinition->defaultValue);
        return color;
    }

    return QVariant();
}

QVariant FieldDefinitionsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Id");
            case 1:
                return QString("Display Name");
            case 2:
                return QString("Type");
            case 3:
                return QString("Default Value");
            case 4:
                return QString("Description");
            }
        }
    }
    return QVariant();
}

bool FieldDefinitionsTableModel::insertRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    // Add new field definitions.
    for (int row = 0; row < rows; ++row)
    {
        QSharedPointer<Tome::FieldDefinition> fieldDefinition = QSharedPointer<Tome::FieldDefinition>::create();
        this->project->fieldDefinitionSets[0]->fieldDefinitions.insert(position, fieldDefinition);
    }

    endInsertRows();
    return true;
}

bool FieldDefinitionsTableModel::removeRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    // Remove field definitions.
    for (int row = 0; row < rows; ++row)
    {
        this->project->fieldDefinitionSets[0]->fieldDefinitions.removeAt(position);
    }

    endRemoveRows();
    return true;
}

void FieldDefinitionsTableModel::addFieldDefinition(const QString& id, const QString& displayName, const FieldType::FieldType& fieldType, const QString& defaultValue, const QString& description)
{
    int index = this->rowCount();

    // Insert row at the end.
    this->insertRows(index, 1, QModelIndex());

    // Update field definition.
    this->updateFieldDefinition(index, id, displayName, fieldType, defaultValue, description);
}

void FieldDefinitionsTableModel::updateFieldDefinition(const int index, const QString& id, const QString& displayName, const FieldType::FieldType& fieldType, const QString& defaultValue, const QString& description)
{
    // Get field definition.
    QVector<QSharedPointer<FieldDefinition> >& fieldDefinitions = this->project->fieldDefinitionSets[0]->fieldDefinitions;
    QSharedPointer<FieldDefinition> fieldDefinition = fieldDefinitions[index];

    // Set data.
    fieldDefinition->id = id;
    fieldDefinition->displayName = displayName;
    fieldDefinition->fieldType = fieldType;
    fieldDefinition->defaultValue = defaultValue;
    fieldDefinition->description = description;

    // Sort by display name.
    std::sort(fieldDefinitions.begin(), fieldDefinitions.end(), lessThanFieldDefinitions);

    // Update view.
    QModelIndex first = this->index(0, 0, QModelIndex());
    QModelIndex last = this->index(fieldDefinitions.size() - 1, 4, QModelIndex());

    emit(dataChanged(first, last));
}

void FieldDefinitionsTableModel::removeFieldDefinition(const int index)
{
    // Remove row.
    this->removeRows(index, 1, QModelIndex());
}