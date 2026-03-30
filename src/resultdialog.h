#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>

class ResultDialog : public QDialog
{
public:
	explicit ResultDialog(QWidget *parent, bool playerWon, int moveCount, int totalGames, int playerWins, int aiWins);
};

#endif // RESULTDIALOG_H
