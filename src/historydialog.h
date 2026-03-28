#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

#include "playerstatsstore.h"

class HistoryDialog : public QDialog
{
public:
	explicit HistoryDialog(const QString &userName, const PlayerRecord &record, QWidget *parent = 0);
};

#endif // HISTORYDIALOG_H
