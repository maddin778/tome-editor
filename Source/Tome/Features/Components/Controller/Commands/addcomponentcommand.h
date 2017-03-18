#ifndef ADDCOMPONENTCOMMAND_H
#define ADDCOMPONENTCOMMAND_H

#include <QUndoCommand>

#include "../../Model/component.h"

namespace Tome
{
    class ComponentsController;

    class AddComponentCommand : public QUndoCommand, public QObject
    {
        public:
            AddComponentCommand(ComponentsController& componentsController, const QString& componentName, const QString& componentSetName);

            virtual void undo() Q_DECL_OVERRIDE;
            virtual void redo() Q_DECL_OVERRIDE;

        private:
            ComponentsController& componentsController;

            const QString componentName;
            const QString componentSetName;

            Component component;
    };
}

#endif // ADDCOMPONENTCOMMAND_H
