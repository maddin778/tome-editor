#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdexcept>

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>
#include <QXmlStreamWriter>

#include "Fields/fielddefinitionsetserializer.h"
#include "Projects/project.h"
#include "Projects/projectserializer.h"
#include "Records/recordsetserializer.h"
#include "Util/pathutils.h"

using namespace Tome;

const QString MainWindow::FieldDefinitionFileExtension = ".tfields";
const QString MainWindow::ProjectFileExtension = ".tproj";
const QString MainWindow::RecordFileExtension = ".tdata";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    aboutWindow(0),
    fieldDefinitionsWindow(0),
    fieldValueWindow(0),
    newProjectWindow(0),
    recordWindow(0)
{
    ui->setupUi(this);

    // Maximize window.
    this->showMaximized();

    // Can't access some functionality until project created or loaded.
    this->updateMenus();

    // Setup record field table model.
    RecordTableModel* model = new RecordTableModel(this);
    this->recordViewModel = QSharedPointer<RecordTableModel>(model);
    this->ui->tableView->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    if (!this->aboutWindow)
    {
        this->aboutWindow = new AboutWindow(this);
    }

    this->showWindow(this->aboutWindow);
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionField_Definions_triggered()
{
    if (!this->fieldDefinitionsWindow)
    {
        this->fieldDefinitionsWindow = new FieldDefinitionsWindow(this->project, this);
    }

    this->showWindow(this->fieldDefinitionsWindow);
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

        this->createNewProject(projectName, projectPath);
    }
}

void MainWindow::on_actionOpen_Project_triggered()
{
    // Open file browser dialog.
    const QString& projectFileName = QFileDialog::getOpenFileName(this,
                                                          tr("Open Project"),
                                                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                          "Tome Project Files (*.tproj)");

    if (projectFileName.count() <= 0)
    {
        return;
    }

    // Open project file.
    QSharedPointer<QFile> projectFile = QSharedPointer<QFile>::create(projectFileName);
    QSharedPointer<QFileInfo> projectFileInfo = QSharedPointer<QFileInfo>::create(projectFileName);
    const QString projectPath = projectFileInfo->path();

    if (projectFile->open(QIODevice::ReadOnly))
    {
        // Load project from file.
        QSharedPointer<ProjectSerializer> projectSerializer =
                QSharedPointer<ProjectSerializer>::create();
        QSharedPointer<Project> project = QSharedPointer<Project>::create();
        project->path = projectPath;

        try
        {
             projectSerializer->deserialize(projectFile, project);
        }
        catch (const std::runtime_error& e)
        {
            QMessageBox::critical(
                        this,
                        tr("Unable to open project"),
                        tr("File could not be read:\r\n") + e.what(),
                        QMessageBox::Close,
                        QMessageBox::Close);
            return;
        }

        // Load field definition files.
        QSharedPointer<FieldDefinitionSetSerializer> fieldDefinitionSerializer =
                QSharedPointer<FieldDefinitionSetSerializer>::create();

        for (QVector<QSharedPointer<FieldDefinitionSet> >::iterator it = project->fieldDefinitionSets.begin();
             it != project->fieldDefinitionSets.end();
             ++it)
        {
            QSharedPointer<FieldDefinitionSet> fieldDefinitionSet = *it;

            // Open field definition file.
            const QString fullFieldDefinitionSetPath = combinePaths(projectPath, fieldDefinitionSet->name + FieldDefinitionFileExtension);
            QSharedPointer<QFile> fieldDefinitionFile = QSharedPointer<QFile>::create(fullFieldDefinitionSetPath);

            if (fieldDefinitionFile->open(QIODevice::ReadOnly))
            {
                try
                {
                     fieldDefinitionSerializer->deserialize(fieldDefinitionFile, fieldDefinitionSet);
                }
                catch (const std::runtime_error& e)
                {
                    QMessageBox::critical(
                                this,
                                tr("Unable to open project"),
                                tr("File could not be read:\r\n") + e.what(),
                                QMessageBox::Close,
                                QMessageBox::Close);
                    return;
                }
            }
            else
            {
                QMessageBox::critical(
                            this,
                            tr("Unable to open project"),
                            tr("File could not be read:\r\n") + fullFieldDefinitionSetPath,
                            QMessageBox::Close,
                            QMessageBox::Close);
                return;
            }
        }

        // Load record files.
        QSharedPointer<RecordSetSerializer> recordSetSerializer =
                QSharedPointer<RecordSetSerializer>::create();

        for (QVector<QSharedPointer<RecordSet> >::iterator it = project->recordSets.begin();
             it != project->recordSets.end();
             ++it)
        {
            QSharedPointer<RecordSet> recordSet = *it;

            // Open record file.
            const QString fullRecordSetPath = combinePaths(projectPath, recordSet->name + RecordFileExtension);
            QSharedPointer<QFile> recordFile = QSharedPointer<QFile>::create(fullRecordSetPath);

            if (recordFile->open(QIODevice::ReadOnly))
            {
                try
                {
                     recordSetSerializer->deserialize(recordFile, recordSet);
                }
                catch (const std::runtime_error& e)
                {
                    QMessageBox::critical(
                                this,
                                tr("Unable to open project"),
                                tr("File could not be read:\r\n") + e.what(),
                                QMessageBox::Close,
                                QMessageBox::Close);
                    return;
                }
            }
            else
            {
                QMessageBox::critical(
                            this,
                            tr("Unable to open project"),
                            tr("File could not be read:\r\n") + fullRecordSetPath,
                            QMessageBox::Close,
                            QMessageBox::Close);
                return;
            }
        }

        // Set project reference.
        this->setProject(project);
    }
    else
    {
        QMessageBox::critical(
                    this,
                    tr("Unable to open project"),
                    tr("File could not be read:\r\n") + projectFileName,
                    QMessageBox::Close,
                    QMessageBox::Close);
        return;
    }
}

void MainWindow::on_actionSave_Project_triggered()
{
    this->saveProject(this->project);
}

void MainWindow::on_actionNew_Record_triggered()
{
    if (!this->recordWindow)
    {
        this->recordWindow = new RecordWindow(this);
    }

    // Add fields.
    this->recordWindow->clearRecordFields();

    for (QVector<QSharedPointer<FieldDefinitionSet> >::iterator itFieldDefinitionSet = this->project->fieldDefinitionSets.begin();
         itFieldDefinitionSet != this->project->fieldDefinitionSets.end();
         ++itFieldDefinitionSet)
    {
        QSharedPointer<FieldDefinitionSet> fieldDefinitionSet = *itFieldDefinitionSet;

        for (QVector<QSharedPointer<FieldDefinition> >::iterator itFieldDefinition = fieldDefinitionSet->fieldDefinitions.begin();
             itFieldDefinition != fieldDefinitionSet->fieldDefinitions.end();
             ++itFieldDefinition)
        {
            QSharedPointer<FieldDefinition> fieldDefinition = *itFieldDefinition;
            this->recordWindow->setRecordField(fieldDefinition->id, false);
        }
    }

    int result = this->recordWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Add new record.
        const QString& recordId = this->recordWindow->getRecordId();
        const QString& recordDisplayName = this->recordWindow->getRecordDisplayName();

        this->recordsViewModel->addRecord(
                recordId,
                recordDisplayName);

        // Select new record.
        const QStandardItem* recordItem = this->recordsViewModel->findItem(recordDisplayName);
        const QModelIndex index = this->recordsViewModel->indexFromItem(recordItem);
        this->ui->treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

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
                QSharedPointer<FieldDefinition> field = this->project->getFieldDefinition(fieldId);
                this->recordViewModel->addRecordField(fieldId, field->defaultValue);
            }
        }
    }
}

void MainWindow::on_actionEdit_Record_triggered()
{
    const QString& displayName = getSelectedRecordDisplayName();

    if (displayName.isEmpty())
    {
        return;
    }

    // Get selected record.
    QSharedPointer<Record> record = this->project->getRecordByDisplayName(displayName);

    if (record == 0)
    {
        return;
    }

    // Show window.
    if (!this->recordWindow)
    {
        this->recordWindow = new RecordWindow(this);
    }

    // Update view.
    this->recordWindow->setRecordId(record->id);
    this->recordWindow->setRecordDisplayName(record->displayName);

    this->recordWindow->clearRecordFields();

    for (QVector<QSharedPointer<FieldDefinitionSet> >::iterator itFieldDefinitionSet = this->project->fieldDefinitionSets.begin();
         itFieldDefinitionSet != this->project->fieldDefinitionSets.end();
         ++itFieldDefinitionSet)
    {
        QSharedPointer<FieldDefinitionSet> fieldDefinitionSet = *itFieldDefinitionSet;

        for (QVector<QSharedPointer<FieldDefinition> >::iterator itFieldDefinition = fieldDefinitionSet->fieldDefinitions.begin();
             itFieldDefinition != fieldDefinitionSet->fieldDefinitions.end();
             ++itFieldDefinition)
        {
            QSharedPointer<FieldDefinition> fieldDefinition = *itFieldDefinition;

            // Check if record contains field.
            bool fieldEnabled = record->fieldValues.contains(fieldDefinition->id);

            // Add to view.
            this->recordWindow->setRecordField(fieldDefinition->id, fieldEnabled);
        }
    }

    int result = this->recordWindow->exec();

    if (result == QDialog::Accepted)
    {
        const QString recordId = this->recordWindow->getRecordId();
        const QString recordDisplayName = this->recordWindow->getRecordDisplayName();

        // Update record.
        this->recordsViewModel->updateRecord
                (record->id,
                 recordId,
                 recordDisplayName);

        // Update record fields.
        const QMap<QString, bool> recordFields = this->recordWindow->getRecordFields();

        for (QMap<QString, bool>::const_iterator it = recordFields.begin();
             it != recordFields.end();
             ++it)
        {
            const QString& fieldId = it.key();

            const bool fieldWasEnabled = record->fieldValues.contains(fieldId);
            const bool fieldIsEnabled = it.value();

            if (fieldIsEnabled && !fieldWasEnabled)
            {
                QSharedPointer<FieldDefinition> field = this->project->getFieldDefinition(fieldId);
                this->recordViewModel->addRecordField(fieldId, field->defaultValue);
            }
            else if (fieldWasEnabled && !fieldIsEnabled)
            {
                this->recordViewModel->removeRecordField(fieldId);
            }
        }
    }
}

void MainWindow::on_actionRemove_Record_triggered()
{
    const QString& displayName = getSelectedRecordDisplayName();

    if (displayName.isEmpty())
    {
        return;
    }

    this->recordsViewModel->removeRecord(displayName);
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    this->on_actionEdit_Record_triggered();
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    // Get current field data.
    const QString& fieldId = this->recordViewModel->getFieldId(index);
    const QString& fieldValue = this->recordViewModel->getFieldValue(index);

    QSharedPointer<FieldDefinition> fieldDefinition =
            this->project->getFieldDefinition(fieldId);

    // Show window.
    if (!this->fieldValueWindow)
    {
        this->fieldValueWindow = new FieldValueWindow(this);
    }

    // Update view.
    this->fieldValueWindow->setFieldDisplayName(fieldDefinition->displayName);
    this->fieldValueWindow->setFieldType(fieldDefinition->fieldType);
    this->fieldValueWindow->setFieldValue(fieldValue);
    this->fieldValueWindow->setFieldDescription(fieldDefinition->description);

    int result = this->fieldValueWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update field.
        this->recordViewModel->setFieldValue(
                    fieldId,
                    this->fieldValueWindow->getFieldValue());
    }
}

void MainWindow::treeViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);

    QModelIndex selectedIndex = selected.first().indexes().first();

    if (!selectedIndex.isValid())
    {
        return;
    }

    const QString& displayName = selectedIndex.data(Qt::DisplayRole).toString();

    if (displayName.isEmpty())
    {
        return;
    }

    // Get selected record.
    QSharedPointer<Record> record = this->project->getRecordByDisplayName(displayName);

    if (record == 0)
    {
        return;
    }

    // Update field table.
    this->recordViewModel->setRecord(record);
}

void MainWindow::createNewProject(const QString &projectName, const QString &projectPath)
{
    // Create new project.
    QSharedPointer<Project> newProject = QSharedPointer<Project>::create();
    newProject->name = projectName;
    newProject->path = projectPath;

    // Create field definition set.
    QSharedPointer<FieldDefinitionSet> fieldDefinitionSet =
            QSharedPointer<FieldDefinitionSet>::create();
    fieldDefinitionSet->name = projectName;
    newProject->fieldDefinitionSets.push_back(fieldDefinitionSet);

    // Create record set.
    QSharedPointer<RecordSet> recordSet =
            QSharedPointer<RecordSet>::create();
    recordSet->name = projectName;
    newProject->recordSets.push_back(recordSet);

    // Write project files.
    this->saveProject(newProject);

    // Set project reference.
    this->setProject(newProject);
}

QString MainWindow::getSelectedRecordDisplayName() const
{
    QModelIndex currentIndex = this->ui->treeView->selectionModel()->currentIndex();

    if (!currentIndex.isValid())
    {
        return QString();
    }

    return currentIndex.data(Qt::DisplayRole).toString();
}

void MainWindow::saveProject(QSharedPointer<Project> project)
{
    QString& projectName = project->name;
    QString& projectPath = project->path;

    QSharedPointer<ProjectSerializer> projectSerializer =
            QSharedPointer<ProjectSerializer>::create();

    // Build file name.
    const QString projectFileName = projectName + ProjectFileExtension;
    const QString fullProjectPath = combinePaths(projectPath, projectFileName);

    // Write project file.
    QSharedPointer<QFile> projectFile = QSharedPointer<QFile>::create(fullProjectPath);

    if (projectFile->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        projectSerializer->serialize(projectFile, project);
    }
    else
    {
        QMessageBox::critical(
                    this,
                    tr("Unable to create project"),
                    tr("Destination file could not be written:\r\n") + fullProjectPath,
                    QMessageBox::Close,
                    QMessageBox::Close);
        return;
    }

    // Write field definition sets.
    QSharedPointer<FieldDefinitionSetSerializer> fieldDefinitionSetSerializer =
            QSharedPointer<FieldDefinitionSetSerializer>::create();

    for (QVector<QSharedPointer<FieldDefinitionSet> >::iterator it = project->fieldDefinitionSets.begin();
         it != project->fieldDefinitionSets.end();
         ++it)
    {
        QSharedPointer<FieldDefinitionSet> fieldDefinitionSet = *it;

        // Build file name.
        const QString fieldDefinitionSetFileName = fieldDefinitionSet->name + FieldDefinitionFileExtension;
        const QString fullFieldDefinitionSetPath = combinePaths(projectPath, fieldDefinitionSetFileName);

        // Write file.
        QSharedPointer<QFile> fieldDefinitionSetFile = QSharedPointer<QFile>::create(fullFieldDefinitionSetPath);

        if (fieldDefinitionSetFile->open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            fieldDefinitionSetSerializer->serialize(fieldDefinitionSetFile, fieldDefinitionSet);
        }
        else
        {
            QMessageBox::critical(
                        this,
                        tr("Unable to create project"),
                        tr("Destination file could not be written:\r\n") + fullFieldDefinitionSetPath,
                        QMessageBox::Close,
                        QMessageBox::Close);
            return;
        }
    }


    // Write record sets.
    QSharedPointer<Tome::RecordSetSerializer> recordSetSerializer =
            QSharedPointer<Tome::RecordSetSerializer>::create();

    for (QVector<QSharedPointer<RecordSet> >::iterator it = project->recordSets.begin();
         it != project->recordSets.end();
         ++it)
    {
        QSharedPointer<RecordSet> recordSet = *it;

        // Build file name.
        const QString recordSetFileName = recordSet->name + RecordFileExtension;
        const QString fullRecordSetPath = Tome::combinePaths(projectPath, recordSetFileName);

        // Write file.
        QSharedPointer<QFile> recordSetFile = QSharedPointer<QFile>::create(fullRecordSetPath);

        if (recordSetFile->open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            recordSetSerializer->serialize(recordSetFile, recordSet);
        }
    }
}

void MainWindow::setProject(QSharedPointer<Project> project)
{
    this->project = project;

    // Enable project-specific buttons.
    this->updateMenus();

    // Setup tree view.
    RecordsItemModel* recordsViewModel = new RecordsItemModel(project);
    this->recordsViewModel = QSharedPointer<RecordsItemModel>(recordsViewModel);

    this->ui->treeView->setModel(recordsViewModel);
    this->ui->treeView->expandAll();

    // Listen for selection changes.
    connect(
      this->ui->treeView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      SLOT(treeViewSelectionChanged(const QItemSelection &, const QItemSelection &))
     );
}

void MainWindow::showWindow(QWidget* widget)
{
    widget->show();
    widget->raise();
    widget->activateWindow();
}

void MainWindow::updateMenus()
{
    bool projectLoaded = this->project != 0;

    this->ui->actionSave_Project->setEnabled(projectLoaded);
    this->ui->actionField_Definions->setEnabled(projectLoaded);
    this->ui->actionNew_Record->setEnabled(projectLoaded);
    this->ui->actionEdit_Record->setEnabled(projectLoaded);
    this->ui->actionRemove_Record->setEnabled(projectLoaded);
}
