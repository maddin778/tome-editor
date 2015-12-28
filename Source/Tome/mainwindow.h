#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

#include "Fields/fielddefinitionswindow.h"
#include "Fields/fieldvaluewindow.h"
#include "Projects/newprojectwindow.h"
#include "Projects/project.h"
#include "Records/recordsitemmodel.h"
#include "Records/recordtablemodel.h"
#include "Records/recordwindow.h"
#include "Help/aboutwindow.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void on_actionAbout_triggered();
        void on_actionExit_triggered();
        void on_actionField_Definions_triggered();

        void on_actionNew_Project_triggered();
        void on_actionOpen_Project_triggered();
        void on_actionSave_Project_triggered();

        void on_actionNew_Record_triggered();
        void on_actionEdit_Record_triggered();
        void on_actionRemove_Record_triggered();

        void on_treeView_doubleClicked(const QModelIndex &index);
        void on_tableView_doubleClicked(const QModelIndex &index);

        void treeViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    private:
        static const QString FieldDefinitionFileExtension;
        static const QString ProjectFileExtension;
        static const QString RecordFileExtension;

        Ui::MainWindow *ui;

        QSharedPointer<Tome::Project> project;

        AboutWindow *aboutWindow;
        FieldDefinitionsWindow* fieldDefinitionsWindow;
        FieldValueWindow* fieldValueWindow;
        NewProjectWindow *newProjectWindow;
        RecordWindow* recordWindow;

        QSharedPointer<Tome::RecordsItemModel> recordsViewModel;
        QSharedPointer<Tome::RecordTableModel> recordViewModel;

        void createNewProject(const QString& name, const QString& path);
        QString getSelectedRecordDisplayName() const;
        void saveProject(QSharedPointer<Tome::Project> project);
        void setProject(QSharedPointer<Tome::Project> project);
        void showWindow(QWidget* widget);
        void updateMenus();
};

#endif // MAINWINDOW_H
