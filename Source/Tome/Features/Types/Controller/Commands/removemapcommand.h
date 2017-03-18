#ifndef REMOVEMAPCOMMAND_H
#define REMOVEMAPCOMMAND_H

#include <QString>
#include <QUndoCommand>

namespace Tome
{
    class TypesController;

    class RemoveMapCommand : public QUndoCommand, public QObject
    {
        public:
            RemoveMapCommand(TypesController& typesController,
                             const QString& name);

    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;

    private:
        TypesController& typesController;

        const QString name;

        QString keyType;
        QString valueType;
        QString customTypeSetName;
    };
}

#endif // REMOVEMAPCOMMAND_H
