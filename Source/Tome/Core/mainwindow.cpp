#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdexcept>

#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>
#include <QXmlStreamWriter>

#include "../Features/Export/Controller/exportcontroller.h"
#include "../Features/Fields/Controller/fielddefinitionsetserializer.h"
#include "../Features/Projects/Model/project.h"
#include "../Features/Projects/Controller/projectserializer.h"
#include "../Features/Records/Controller/recordsetserializer.h"
#include "../Features/Settings/Controller/settingscontroller.h"
#include "../Features/Types/Model/builtintype.h"
#include "../Util/listutils.h"
#include "../Util/pathutils.h"
#include "../Util/stringutils.h"

using namespace Tome;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    controller(new Controller()),
    aboutWindow(0),
    componentsWindow(0),
    customTypesWindow(0),
    fieldDefinitionsWindow(0),
    fieldValueWindow(0),
    newProjectWindow(0),
    recordWindow(0)
{
    ui->setupUi(this);

    // Add record tree.
    this->treeWidget = new RecordTreeWidget();
    this->treeWidget->setDragEnabled(true);
    this->treeWidget->viewport()->setAcceptDrops(true);
    this->treeWidget->setDropIndicatorShown(true);
    this->treeWidget->setHeaderHidden(true);

    this->ui->splitter->addWidget(this->treeWidget);

    // Add record table.
    this->tableWidget = new QTableWidget();
    this->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->tableWidget->verticalHeader()->setVisible(false);

    this->ui->splitter->addWidget(this->tableWidget);

    // Connect signals.
    connect(
                this->ui->menuExport,
                SIGNAL(triggered(QAction*)),
                SLOT(exportRecords(QAction*))
                );

    connect(
                this->ui->menuRecent_Projects,
                SIGNAL(triggered(QAction*)),
                SLOT(openRecentProject(QAction*))
                );

    connect(
                this->tableWidget,
                SIGNAL(doubleClicked(const QModelIndex &)),
                SLOT(tableWidgetDoubleClicked(const QModelIndex &))
                );

    connect(
                this->treeWidget,
                SIGNAL(doubleClicked(const QModelIndex &)),
                SLOT(treeWidgetDoubleClicked(const QModelIndex &))
                );

    connect(
                this->treeWidget->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                SLOT(treeWidgetSelectionChanged(const QItemSelection &, const QItemSelection &))
                );

    connect(
                this->treeWidget,
                SIGNAL(recordReparented(const QString&, const QString&)),
                SLOT(treeWidgetRecordReparented(const QString&, const QString&))
                );

    // Maximize window.
    this->showMaximized();

    // Set window title.
    this->updateWindowTitle();

    // Can't access some functionality until project created or loaded.
    this->updateMenus();
    this->updateRecentProjects();

    // Setup view.
    this->resetFields();
}

MainWindow::~MainWindow()
{
    delete this->ui;

    delete this->controller;

    delete this->treeWidget;
    delete this->tableWidget;

    delete this->aboutWindow;
    delete this->componentsWindow;
    delete this->customTypesWindow;
    delete this->fieldDefinitionsWindow;
    delete this->fieldValueWindow;
    delete this->newProjectWindow;
    delete this->recordWindow;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionField_Definions_triggered()
{
    if (!this->fieldDefinitionsWindow)
    {
        this->fieldDefinitionsWindow = new FieldDefinitionsWindow(
                    this->controller->getFieldDefinitionsController(),
                    this->controller->getComponentsController(),
                    this->controller->getRecordsController(),
                    this->controller->getTypesController(),
                    this);
    }

    this->showWindow(this->fieldDefinitionsWindow);
}


void MainWindow::on_actionManage_Components_triggered()
{
    if (!this->componentsWindow)
    {
        this->componentsWindow = new ComponentsWindow(this->controller->getComponentsController(), this);
    }

    this->showWindow(this->componentsWindow);
}

void MainWindow::on_actionManage_Custom_Types_triggered()
{
    if (!this->customTypesWindow)
    {
        this->customTypesWindow = new CustomTypesWindow(this->controller->getTypesController(), this);
    }

    this->showWindow(this->customTypesWindow);
}

void MainWindow::on_actionNew_Project_triggered()
{
    if (!this->newProjectWindow)
    {
        this->newProjectWindow = new NewProjectWindow(this);
    }

    int result = this->newProjectWindow->exec();

    if (result == QDialog::Accepted)
    {
        const QString& projectName = this->newProjectWindow->getProjectName();
        const QString& projectPath = this->newProjectWindow->getProjectPath();

        try
        {
            this->controller->createProject(projectName, projectPath);
            this->onProjectChanged();
        }
        catch (std::runtime_error& e)
        {
            QMessageBox::critical(
                        this,
                        tr("Unable to create project"),
                        e.what(),
                        QMessageBox::Close,
                        QMessageBox::Close);
        }
    }
}

void MainWindow::on_actionOpen_Project_triggered()
{
    // Open file browser dialog.
    const QString& projectFileName = QFileDialog::getOpenFileName(this,
                                                                  tr("Open Project"),
                                                                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                                  "Tome Project Files (*.tproj)");

    this->openProject(projectFileName);
}

void MainWindow::on_actionSave_Project_triggered()
{
    try
    {
         this->controller->saveProject();
    }
    catch (std::runtime_error& e)
    {
        QMessageBox::critical(
                    this,
                    tr("Unable to create project"),
                    e.what(),
                    QMessageBox::Close,
                    QMessageBox::Close);
    }
}

void MainWindow::on_actionNew_Record_triggered()
{
    if (!this->recordWindow)
    {
        this->recordWindow = new RecordWindow(this);
    }

    // Add fields.
    this->recordWindow->clearRecordFields();

    const FieldDefinitionSetList& fieldDefinitionSetList =
            this->controller->getFieldDefinitionsController().getFieldDefinitionSets();

    for (int i = 0; i < fieldDefinitionSetList.size(); ++i)
    {
        const FieldDefinitionSet& fieldDefinitionSet = fieldDefinitionSetList[i];

        for (int j = 0; j < fieldDefinitionSet.fieldDefinitions.size(); ++j)
        {
            const FieldDefinition& fieldDefinition = fieldDefinitionSet.fieldDefinitions[j];
            this->recordWindow->setRecordField(fieldDefinition.id, fieldDefinition.component, false);
        }
    }

    int result = this->recordWindow->exec();

    if (result == QDialog::Accepted)
    {
        const QString& recordId = this->recordWindow->getRecordId();
        const QString& recordDisplayName = this->recordWindow->getRecordDisplayName();

        // Update model.
        const Record& record = this->controller->getRecordsController().addRecord(recordId, recordDisplayName);

        // Update view.
        int index = this->controller->getRecordsController().indexOf(record);
        QTreeWidgetItem* newItem = new RecordTreeWidgetItem(recordId, recordDisplayName, QString());
        this->treeWidget->insertTopLevelItem(index, newItem);

        // Select new record.
        this->treeWidget->setCurrentItem(newItem);

        // Add record fields.
        const QMap<QString, bool> recordFields = this->recordWindow->getRecordFields();

        for (QMap<QString, bool>::const_iterator it = recordFields.begin();
             it != recordFields.end();
             ++it)
        {
            const QString& fieldId = it.key();
            const bool fieldEnabled = it.value();

            if (fieldEnabled)
            {
                this->addRecordField(fieldId);
            }
        }

        // Update view.
        this->refreshRecordTable();
    }
}

void MainWindow::on_actionEdit_Record_triggered()
{
    const QString& id = getSelectedRecordId();

    if (id.isEmpty())
    {
        return;
    }

    // Get selected record.
    const Record& record =
            this->controller->getRecordsController().getRecord(id);

    // Show window.
    if (!this->recordWindow)
    {
        this->recordWindow = new RecordWindow(this);
    }

    // Update view.
    this->recordWindow->setRecordId(record.id);
    this->recordWindow->setRecordDisplayName(record.displayName);

    this->recordWindow->clearRecordFields();

    const FieldDefinitionSetList& fieldDefinitionSets =
            this->controller->getFieldDefinitionsController().getFieldDefinitionSets();

    for (int i = 0; i < fieldDefinitionSets.size(); ++i)
    {
        const FieldDefinitionSet& fieldDefinitionSet = fieldDefinitionSets[i];

        for (int j = 0; j < fieldDefinitionSet.fieldDefinitions.size(); ++j)
        {
            const FieldDefinition& fieldDefinition = fieldDefinitionSet.fieldDefinitions[j];

            // Check if record contains field.
            bool fieldEnabled = record.fieldValues.contains(fieldDefinition.id);

            // Add to view.
            this->recordWindow->setRecordField(fieldDefinition.id, fieldDefinition.component, fieldEnabled);
        }
    }

    int result = this->recordWindow->exec();

    if (result == QDialog::Accepted)
    {
        const QString recordId = this->recordWindow->getRecordId();
        const QString recordDisplayName = this->recordWindow->getRecordDisplayName();

        // Update record.
        this->updateRecord(recordId, recordDisplayName);

        // Update record fields.
        const QMap<QString, bool> recordFields = this->recordWindow->getRecordFields();

        for (QMap<QString, bool>::const_iterator it = recordFields.begin();
             it != recordFields.end();
             ++it)
        {
            const QString& fieldId = it.key();

            const bool fieldWasEnabled = record.fieldValues.contains(fieldId);
            const bool fieldIsEnabled = it.value();

            if (fieldIsEnabled && !fieldWasEnabled)
            {
                this->addRecordField(fieldId);
            }
            else if (fieldWasEnabled && !fieldIsEnabled)
            {
                this->removeRecordField(fieldId);
            }
        }

        // Update view.
        this->refreshRecordTable();
    }
}

void MainWindow::on_actionRemove_Record_triggered()
{
    RecordTreeWidgetItem* recordItem = this->getSelectedRecordItem();

    if (recordItem == 0)
    {
        return;
    }

    // Update model.
    this->controller->getRecordsController().removeRecord(recordItem->getId());

    // Update view.
    if (recordItem->parent() != 0)
    {
        recordItem->parent()->removeChild(recordItem);
    }
    else
    {
        int index = this->treeWidget->indexOfTopLevelItem(recordItem);
        this->treeWidget->takeTopLevelItem(index);
    }

    delete recordItem;
}

void MainWindow::on_actionAbout_triggered()
{
    if (!this->aboutWindow)
    {
        this->aboutWindow = new AboutWindow(this);
    }

    this->showWindow(this->aboutWindow);
}

void MainWindow::on_actionManual_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/npruehs/tome-editor/wiki/Introduction"));
}

void MainWindow::on_actionReport_a_Bug_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/npruehs/tome-editor/issues/new"));
}

void MainWindow::on_actionReleases_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/npruehs/tome-editor/releases"));
}

void MainWindow::exportRecords(QAction* exportAction)
{
    // Get export template.
    QString exportTemplateName = exportAction->text();
    const RecordExportTemplate& exportTemplate =
            this->controller->getExportController().getRecordExportTemplate(exportTemplateName);

    // Build export file name suggestion.
    const QString suggestedFileName = this->controller->getProjectName() + exportTemplate.fileExtension;
    const QString suggestedFilePath = combinePaths(this->controller->getProjectPath(), suggestedFileName);
    const QString filter = exportTemplateName + " (*" + exportTemplate.fileExtension + ")";

    // Show file dialog.
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Export Records"),
                                                    suggestedFilePath,
                                                    filter);

    if (filePath.isEmpty())
    {
        return;
    }

    // Export records.
    try
    {
        this->controller->getExportController().exportRecords(exportTemplate, filePath);
    }
    catch (std::runtime_error& e)
    {
        QMessageBox::critical(
                    this,
                    tr("Unable to export records"),
                    e.what(),
                    QMessageBox::Close,
                    QMessageBox::Close);
    }
}

void MainWindow::openRecentProject(QAction* recentProjectAction)
{
    QString path = recentProjectAction->text();
    this->openProject(path);
}

void MainWindow::revertFieldValue()
{
    // Get record to revert field of.
    QString recordId = this->getSelectedRecordId();

    // Get field to revert.
    QModelIndexList selectedIndexes = this->tableWidget->selectionModel()->selectedRows(0);

    if (selectedIndexes.empty())
    {
        return;
    }

    const RecordFieldValueMap fieldValues =
            this->controller->getRecordsController().getRecordFieldValues(recordId);
    const QString fieldId = fieldValues.keys()[selectedIndexes.first().row()];

    // Update view.
    QVariant inheritedValue = this->controller->getRecordsController().getInheritedFieldValue(recordId, fieldId);

    if (inheritedValue != QVariant())
    {
        if (this->fieldValueWindow != 0)
        {
            this->fieldValueWindow->setFieldValue(inheritedValue);
        }
    }
}

void MainWindow::tableWidgetDoubleClicked(const QModelIndex &index)
{
    QString id = this->getSelectedRecordId();
    const RecordFieldValueMap fieldValues =
            this->controller->getRecordsController().getRecordFieldValues(id);

    // Get current field data.
    const QString fieldId = fieldValues.keys()[index.row()];
    const QVariant fieldValue = fieldValues[fieldId];

    const FieldDefinition& field =
            this->controller->getFieldDefinitionsController().getFieldDefinition(fieldId);

    // Prepare window.
    if (!this->fieldValueWindow)
    {
        this->fieldValueWindow = new FieldValueWindow
                (this->controller->getRecordsController(),
                 this->controller->getTypesController(),
                 this);

        connect(
                    this->fieldValueWindow,
                    SIGNAL(revert()),
                    SLOT(revertFieldValue())
                    );
    }

    // Update view.
    this->fieldValueWindow->setFieldDisplayName(field.displayName);
    this->fieldValueWindow->setFieldDescription(field.description);
    this->fieldValueWindow->setFieldType(field.fieldType);
    this->fieldValueWindow->setFieldValue(fieldValue);

    // Show window.
    int result = this->fieldValueWindow->exec();

    if (result == QDialog::Accepted)
    {
        QVariant fieldValue = this->fieldValueWindow->getFieldValue();

        // Update model.
        this->controller->getRecordsController().updateRecordFieldValue(id, fieldId, fieldValue);

        // Update view.
        this->updateRecordRow(index.row());
    }
}

void MainWindow::treeWidgetDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    this->on_actionEdit_Record_triggered();
}

void MainWindow::treeWidgetRecordReparented(const QString& recordId, const QString& newParentId)
{
    // Check if allowed.
    if (recordId == newParentId ||
            this->controller->getRecordsController().isAncestorOf(recordId, newParentId))
    {
        return;
    }

    // Update model.
    this->controller->getRecordsController().reparentRecord(recordId, newParentId);

    // Update view.
    this->resetRecords();
    this->refreshRecordTree();
}

void MainWindow::treeWidgetSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    const QString& id = this->getSelectedRecordId();

    if (id.isEmpty() || !this->controller->getRecordsController().hasRecord(id))
    {
        // Clear table.
        this->tableWidget->setRowCount(0);
        return;
    }

    // Get selected record.
    const RecordFieldValueMap fieldValues =
            this->controller->getRecordsController().getRecordFieldValues(id);

    // Update field table.
    this->tableWidget->setRowCount(fieldValues.size());
    this->refreshRecordTable();
}

void MainWindow::addRecordField(const QString& fieldId)
{
    QString id = this->getSelectedRecordId();
    const Record& record =
            this->controller->getRecordsController().getRecord(id);

    int index = findInsertionIndex(record.fieldValues.keys(), fieldId, qStringLessThanLowerCase);

    // Update model.
    this->controller->getRecordsController().addRecordField(record.id, fieldId);

    // Update view.
    this->tableWidget->insertRow(index);
}

void MainWindow::resetRecords()
{
    this->treeWidget->setColumnCount(1);

    while (this->treeWidget->topLevelItemCount() > 0)
    {
        this->treeWidget->takeTopLevelItem(0);
    }
}

QString MainWindow::getSelectedRecordId() const
{
    RecordTreeWidgetItem* recordTreeItem = this->getSelectedRecordItem();

    if (recordTreeItem == 0)
    {
        return QString();
    }

    return recordTreeItem->getId();
}

RecordTreeWidgetItem* MainWindow::getSelectedRecordItem() const
{
    QList<QTreeWidgetItem*> selectedItems = this->treeWidget->selectedItems();

    if (selectedItems.empty())
    {
        return 0;
    }

    return static_cast<RecordTreeWidgetItem*>(selectedItems.first());
}

void MainWindow::openProject(QString path)
{
    if (path.isEmpty())
    {
        return;
    }

    try
    {
        this->controller->openProject(path);
        this->onProjectChanged();
    }
    catch (std::runtime_error& e)
    {
        this->updateRecentProjects();

        QMessageBox::critical(
                    this,
                    tr("Unable to open project"),
                    e.what(),
                    QMessageBox::Close,
                    QMessageBox::Close);
    }
}

void MainWindow::removeRecordField(const QString& fieldId)
{
    QString id = this->getSelectedRecordId();
    const Record& record =
            this->controller->getRecordsController().getRecord(id);

    // Update model.
    this->controller->getRecordsController().removeRecordField(record.id, fieldId);

    // Update view.
    int index = findInsertionIndex(record.fieldValues.keys(), fieldId, qStringLessThanLowerCase);
    this->tableWidget->removeRow(index);
}

void MainWindow::onProjectChanged()
{
    // Enable project-specific buttons.
    this->updateMenus();

    // Setup tree view.
    this->resetRecords();
    this->resetFields();

    this->refreshRecordTree();

    // Setup record exports.
    this->ui->menuExport->clear();

    const RecordExportTemplateMap& recordExportTemplateMap =
            this->controller->getExportController().getRecordExportTemplates();

    for (RecordExportTemplateMap::const_iterator it = recordExportTemplateMap.begin();
         it != recordExportTemplateMap.end();
         ++it)
    {
        QAction* exportAction = new QAction(it.key(), this);
        this->ui->menuExport->addAction(exportAction);
    }

    // Update title.
    this->updateWindowTitle();

    // Update recent projects.
    this->updateRecentProjects();
}

void MainWindow::refreshRecordTree()
{
    // Create record tree items.
    QMap<QString, RecordTreeWidgetItem*> recordItems;

    const RecordSetList& recordSetList = this->controller->getRecordsController().getRecordSets();

    for (int i = 0; i < recordSetList.size(); ++i)
    {
        const RecordSet& recordSet = recordSetList[i];

        for (int j = 0; j < recordSet.records.size(); ++j)
        {
            const Record& record = recordSet.records[j];
            RecordTreeWidgetItem* recordItem =
                    new RecordTreeWidgetItem(record.id, record.displayName, record.parentId);
            recordItems.insert(record.id, recordItem);
        }
    }

    // Build hierarchy and prepare item list for tree widget.
    QList<QTreeWidgetItem* > items;

    for (QMap<QString, RecordTreeWidgetItem*>::iterator it = recordItems.begin();
         it != recordItems.end();
         ++it)
    {
        RecordTreeWidgetItem* recordItem = it.value();
        QString recordItemParentId = recordItem->getParentId();
        if (!recordItemParentId.isEmpty() && recordItems.contains(recordItemParentId))
        {
            RecordTreeWidgetItem* recordParent = recordItems[recordItemParentId];
            recordParent->addChild(recordItem);
        }

        items.append(recordItem);
    }

    // Fill tree widget.
    this->treeWidget->insertTopLevelItems(0, items);
    this->treeWidget->expandAll();
}

void MainWindow::refreshRecordTable()
{
    for (int i = 0; i < this->tableWidget->rowCount(); ++i)
    {
        this->updateRecordRow(i);
    }
}

void MainWindow::resetFields()
{
    this->tableWidget->clear();

    this->tableWidget->setRowCount(0);
    this->tableWidget->setColumnCount(2);

    QStringList headers;
    headers << tr("Field");
    headers << tr("Value");
    this->tableWidget->setHorizontalHeaderLabels(headers);
}

void MainWindow::showWindow(QWidget* widget)
{
    widget->show();
    widget->raise();
    widget->activateWindow();
}

void MainWindow::updateMenus()
{
    bool projectLoaded = this->controller->isProjectLoaded();

    this->ui->actionSave_Project->setEnabled(projectLoaded);

    this->ui->actionField_Definions->setEnabled(projectLoaded);
    this->ui->actionManage_Components->setEnabled(projectLoaded);
    this->ui->actionManage_Custom_Types->setEnabled(projectLoaded);

    this->ui->actionNew_Record->setEnabled(projectLoaded);
    this->ui->actionEdit_Record->setEnabled(projectLoaded);
    this->ui->actionRemove_Record->setEnabled(projectLoaded);
}

void MainWindow::updateRecentProjects()
{
    // Clear menu.
    this->ui->menuRecent_Projects->clear();

    // Add recent projects.
    const QStringList& recentProjects = this->controller->getSettingsController().getRecentProjects();
    for (int i = 0; i < recentProjects.size(); ++i)
    {
        QString path = recentProjects.at(i);
        QAction* action = new QAction(path, this);
        this->ui->menuRecent_Projects->addAction(action);
    }
}

void MainWindow::updateRecord(const QString& id, const QString& displayName)
{
    QString selectedRecordId = this->getSelectedRecordId();
    const Record& record =
            this->controller->getRecordsController().getRecord(selectedRecordId);

    bool needsSorting = record.displayName != displayName;

    // Update model.
    this->controller->getRecordsController().updateRecord(record.id, id, displayName);

    // Update view.
    QModelIndexList selectedIndexes = this->treeWidget->selectionModel()->selectedIndexes();

    if (selectedIndexes.empty())
    {
        return;
    }

    QModelIndex currentIndex = selectedIndexes.first();

    if (!currentIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* item = this->treeWidget->topLevelItem(currentIndex.row());
    RecordTreeWidgetItem* recordItem = static_cast<RecordTreeWidgetItem*>(item);
    recordItem->setId(id);
    recordItem->setDisplayName(displayName);

    // Sort by display name.
    if (needsSorting)
    {
        this->treeWidget->sortItems(0, Qt::AscendingOrder);
    }
}

void MainWindow::updateRecordRow(int i)
{
    // Get selected record.
    QString id = this->getSelectedRecordId();

    // Get record field values.
    const RecordFieldValueMap fieldValues =
            this->controller->getRecordsController().getRecordFieldValues(id);

    // Get selected record field key and value.
    QString key = fieldValues.keys()[i];
    QVariant value = fieldValues[key];

    // Get selected record field type.
    const FieldDefinition& field =
            this->controller->getFieldDefinitionsController().getFieldDefinition(key);

    QString keyString = field.displayName;
    QString valueString = value.toString();

    if (this->controller->getTypesController().isCustomType(field.fieldType))
    {
        const CustomType& customType = this->controller->getTypesController().getCustomType(field.fieldType);

        if (customType.isList())
        {
            valueString = toString(value.toList());
        }
    }

    // Show field and value.
    this->tableWidget->setItem(i, 0, new QTableWidgetItem(keyString));
    this->tableWidget->setItem(i, 1, new QTableWidgetItem(valueString));

    // Show field description as tooltip.
    this->tableWidget->item(i, 0)->setData(Qt::ToolTipRole, field.description);
    this->tableWidget->item(i, 1)->setData(Qt::ToolTipRole, field.description);

    // Show color preview.
    if (field.fieldType == BuiltInType::Color)
    {
        QColor color = value.value<QColor>();
        this->tableWidget->item(i, 1)->setData(Qt::DecorationRole, color);
    }
}

void MainWindow::updateWindowTitle()
{
    // Get application version.
    QString windowTitle = "Tome " + QApplication::instance()->applicationVersion();

    if (this->controller->isProjectLoaded())
    {
        // Add project name.
        windowTitle += " - " + this->controller->getFullProjectPath();
    }

    this->setWindowTitle(windowTitle);
}
