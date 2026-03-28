#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include <QtWidgets/QMainWindow>
#include "ui_backgammon.h"
#include "types.h"
class QMouseEvent;
class QGraphicsScene;
class judgeWinner;

class Backgammon : public QMainWindow
{
	Q_OBJECT

public:
	Backgammon(QWidget *parent = 0);
	~Backgammon();
	void DrawBoard();   //ª≠∆Â≈Ãµƒœﬂ
	bool IsBoardClean();  //≈–∂œ∆Â≈Ã «∑Ò «ø’µƒ
	void mousePressEvent(QMouseEvent * event);
	void CleanBoard();  

public slots:
	void slotStartBtnClicked();

private:
	Ui::BackgammonClass ui;
	ePiece m_arrBoard[15][15];  //∆Â≈Ã
	QGraphicsScene *m_pGraphicsScene;
	bool m_bStarted;
	judgeWinner* m_pJugdeWinner;

	int m_nDeep;
};

#endif // BACKGAMMON_H
