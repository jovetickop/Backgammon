#include "backgammon.h"

#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QFontMetrics>
#include <QIntValidator>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPen>
#include <QPushButton>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QShowEvent>
#include <QThread>
#include <QFileDialog>
#include "sgf_serializer.h"
#include <QVBoxLayout>
#include <QTransform>
#include <cmath>
#include <tuple>

#include "AiWorker.h"
#include "historydialog.h"
#include "replaydialog.h"
#include "soundmanager.h"
#include "domain/aggregates/game_board.h"
#include "resultdialog.h"
#include "winratechart.h"

using namespace std;

namespace
{
	const int kBoardSize = 15;
	const int kBoardCenter = 7;
	const int kGridSize = 52;
	const int kPieceSize = 44;
	const int kPieceRadius = kPieceSize / 2;
	const int kLineMin = -kBoardCenter * kGridSize;
	const int kLineMax = kBoardCenter * kGridSize;
	const int kScenePadding = kGridSize;

	int BoardToScene(int index)
	{
		return (index - kBoardCenter) * kGridSize;
	}

	int SceneToBoard(double pos)
	{
		return static_cast<int>(std::lround(pos / kGridSize)) + kBoardCenter;
	}

	// 将启发式评估分数映射为 AI 胜率(0~100)。
	// 使用 S 型曲线(tanh)而非对数压缩，确保：
	// - 活四/冲四等关键棋型能产生接近 0%/100% 的极端胜率
	// - 散子堆积不会轻易掩盖对方的高价值棋型
	double ScoreToAiWinRate(int score)
	{
		// 缩放因子：控制曲线陡峭程度。
		// OPEN_FOUR(1000000) 应映射到 ~5% 以下，所以 k 取较小值让大分数快速饱和。
		const double k = 0.00005;
		const double rate = std::tanh(score * k);
		// tanh 输出 [-1, 1]，映射到 [2, 98] 避免极端值。
		return 50.0 + rate * 48.0;
	}

	// 将 ePiece 数组转换为 GameBoard 供 DDD 层使用
	game_core::GameBoard ToGameBoard(const ePiece arrBoard[15][15])
	{
		game_core::GameBoard board;
		for (int i = 0; i < 15; ++i)
			for (int j = 0; j < 15; ++j)
				if (arrBoard[i][j] != NONE) {
					game_core::Piece p = (arrBoard[i][j] == WHITE) ? game_core::Piece::White : game_core::Piece::Black;
					board.placePiece(game_core::Position(i, j), p);
				}
		return board;
	}

	QString StarterText(bool playerStarts)
	{
		return playerStarts
			? QString::fromUtf8("\u6211\u5148\u624B\uFF08\u9ED1\u68CB\uFF09")
			: QString::fromUtf8("AI \u5148\u624B\uFF08\u767D\u68CB\uFF09");
	}

	void ShowAiLogicDialog(QWidget *parent, int searchDepth)
	{
		QDialog dialog(parent);
		dialog.setModal(true);
		dialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		dialog.setWindowTitle(QString::fromUtf8("AI \u8FD0\u884C\u903B\u8F91"));
		dialog.setFixedSize(640, 620);
		dialog.setStyleSheet(
			"* { font-family: \"Source Han Sans CN\", \"Noto Sans CJK SC\", \"Microsoft YaHei\", sans-serif; }"
			"QDialog {"
			"background-color: rgb(236, 241, 247);"
			"}"
			"QFrame#card {"
			"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
			"stop:0 rgba(255,255,255,248), stop:1 rgba(229,236,244,248));"
			"border: 1px solid rgba(255,255,255,220);"
			"border-radius: 24px;"
			"}"
			"QLabel#title {"
			"color: rgb(41, 52, 70);"
			"font-size: 30px;"
			"font-weight: 800;"
			"}"
			"QLabel#subtitle {"
			"color: rgb(96, 108, 124);"
			"font-size: 17px;"
			"}"
			"QLabel#section {"
			"background-color: rgba(255,255,255,188);"
			"border: 1px solid rgba(221,228,236,220);"
			"border-radius: 16px;"
			"padding: 16px 18px;"
			"color: rgb(56, 69, 89);"
			"font-size: 18px;"
			"font-weight: 600;"
			"}"
			"QPushButton {"
			"min-height: 48px;"
			"padding: 10px 22px;"
			"border: none;"
			"border-radius: 14px;"
			"font-size: 20px;"
			"font-weight: 700;"
			"color: white;"
			"background-color: rgb(63, 84, 117);"
			"}");

		QVBoxLayout *rootLayout = new QVBoxLayout(&dialog);
		rootLayout->setContentsMargins(18, 18, 18, 18);

		QFrame *card = new QFrame(&dialog);
		card->setObjectName("card");
		rootLayout->addWidget(card);

		QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(&dialog);
		shadow->setBlurRadius(34);
		shadow->setOffset(0, 12);
		shadow->setColor(QColor(65, 82, 106, 80));
		card->setGraphicsEffect(shadow);

		QVBoxLayout *cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(28, 28, 28, 24);
		cardLayout->setSpacing(12);

		QLabel *title = new QLabel(QString::fromUtf8("AI \u51B3\u7B56\u8BF4\u660E"), card);
		title->setObjectName("title");
		cardLayout->addWidget(title);

		QLabel *subtitle = new QLabel(
			QString::fromUtf8("\u5F53\u524D\u641C\u7D22\u6DF1\u5EA6\uFF1A%1 \u5C42\u3002\u6BCF\u5C42\u8868\u793A\u4E00\u6B21\u8F6E\u6D41\uFF08AI\u548C\u73A9\u5BB6\u5404\u4E00\u6B65\uFF09\uFF0C\u6DF1\u5EA6\u8D8A\u5927 AI \u601D\u8003\u8D8A\u6DF1\uFF0C\u4F46\u4E5F\u8D8A\u6162\u3002").arg(searchDepth),
			card);
		subtitle->setObjectName("subtitle");
		subtitle->setWordWrap(true);
		cardLayout->addWidget(subtitle);

		QLabel *logicOne = new QLabel(
			QString::fromUtf8("1. \u7D27\u6025\u5224\u65AD\uFF08\u4F18\u5148\u7EA7\u6700\u9AD8\uFF09\n"
				"AI \u6BCF\u6B21\u843D\u5B50\u524D\u4F1A\u5148\u68C0\u67E5\u7D27\u6025\u60C5\u51B5\uFF1A"
				"\u81EA\u5DF1\u80FD\u5426\u76F4\u63A5\u4E94\u8FDE\u83B7\u80DC\uFF1F\u5BF9\u624B\u662F\u5426\u5DF2\u7ECF\u6709\u51B2\u56DB\u6216\u6D3B\u56DB\u9700\u8981\u5C01\u5835\uFF1F"
				"\u8FD9\u4E9B\u4F1A\u8DF3\u8FC7\u641C\u7D22\u76F4\u63A5\u505A\u51FA\u53CD\u5E94\u3002"),
			card);
		logicOne->setObjectName("section");
		logicOne->setWordWrap(true);
		cardLayout->addWidget(logicOne);

		QLabel *logicTwo = new QLabel(
			QString::fromUtf8("2. \u6D3B\u4E09\u5FC5\u6740\u68C0\u6D4B\n"
				"\u5982\u679C\u5BF9\u624B\u5728\u591A\u4E2A\u65B9\u5411\u540C\u65F6\u5F62\u6210\u6D3B\u4E09\uFF08\u4E09\u5B50\u8FDE\u7EBF\u4E14\u4E24\u7AEF\u5F00\u53E3\uFF09\uFF0C"
				"AI \u4F1A\u8BC6\u522B\u8FD9\u79CD\u201C\u53CC\u6D3B\u4E09\u201D\u5FC5\u6740\u5C40\u9762\uFF0C"
				"\u5E76\u4F18\u5148\u9009\u62E9\u80FD\u540C\u65F6\u62E6\u622A\u591A\u4E2A\u65B9\u5411\u7684\u4F4D\u7F6E\u3002"),
			card);
		logicTwo->setObjectName("section");
		logicTwo->setWordWrap(true);
		cardLayout->addWidget(logicTwo);

		QLabel *logicThree = new QLabel(
			QString::fromUtf8("3. \u5019\u9009\u70B9\u751F\u6210\n"
				"\u53EA\u8003\u8651\u5DF2\u6709\u68CB\u5B50\u516B\u90BB\u57DF\u9644\u8FD1\u7684\u7A7A\u4F4D\uFF0C"
				"\u907F\u514D\u5728\u79BB\u6218\u573A\u5F88\u8FDC\u7684\u4F4D\u7F6E\u6D6A\u8D39\u65F6\u95F4\u3002"),
			card);
		logicThree->setObjectName("section");
		logicThree->setWordWrap(true);
		cardLayout->addWidget(logicThree);

		QLabel *logicFour = new QLabel(
			QString::fromUtf8("4. \u8BC4\u4F30\u6392\u5E8F\n"
				"\u5BF9\u6BCF\u4E2A\u5019\u9009\u70B9\u8BA1\u7B97\u8FDB\u653B\u4EF7\u503C\u548C\u9632\u5B88\u4EF7\u503C\uFF0C"
				"\u7EFC\u5408\u6392\u5E8F\u540E\u53EA\u641C\u7D22\u524D N \u4E2A\u6700\u6709\u4EF7\u503C\u7684\u70B9\uFF0C"
				"\u8BA9 alpha-beta \u526A\u679D\u6548\u7387\u63D0\u5347\u6570\u500D\u3002"),
			card);
		logicFour->setObjectName("section");
		logicFour->setWordWrap(true);
		cardLayout->addWidget(logicFour);

		QLabel *logicFive = new QLabel(
			QString::fromUtf8("5. \u6781\u5927\u6781\u5C0F\u641C\u7D22 + Alpha-Beta \u526A\u679D\n"
				"AI \u843D\u5B50\u65F6\u5C3D\u91CF\u8BA9\u5C40\u9762\u5206\u6570\u66F4\u9AD8\uFF0C"
				"\u73A9\u5BB6\u56DE\u5408\u5219\u5047\u8BBE\u4F60\u4F1A\u9009\u62E9\u6700\u538B\u5236 AI \u7684\u5E94\u624B\u3002"
				"\u641C\u7D22\u4E2D\u63D0\u524D\u622A\u65AD\u4E0D\u53EF\u80FD\u66F4\u4F18\u7684\u5206\u652F\uFF0C\u63D0\u9AD8\u601D\u8003\u901F\u5EA6\u3002"),
			card);
		logicFive->setObjectName("section");
		logicFive->setWordWrap(true);
		cardLayout->addWidget(logicFive);

		QLabel *logicSix = new QLabel(
			QString::fromUtf8("6. \u5C40\u9762\u8BC4\u4F30\n"
				"\u8BC4\u4F30\u5668\u7EDF\u8BA1\u6D3B\u4E00\u3001\u6D3B\u4E8C\u3001\u6D3B\u4E09\u3001\u6D3B\u56DB\u3001\u51B2\u56DB\u3001\u4E94\u8FDE\u7B49\u68CB\u578B\u5206\u503C\uFF0C"
				"\u7528\u201C\u767D\u65B9\u5206 - \u9ED1\u65B9\u5206\u201D\u5F97\u5230\u5C40\u9762\u4F18\u52BF\uFF0C"
				"\u901A\u8FC7 S \u578B\u66F2\u7EBF\u6620\u5C04\u4E3A\u53F3\u4FA7\u80DC\u7387\u56FE\u7684\u767E\u5206\u6BD4\u3002"),
			card);
		logicSix->setObjectName("section");
		logicSix->setWordWrap(true);
		cardLayout->addWidget(logicSix);

		QLabel *logicSeven = new QLabel(
			QString::fromUtf8("7. \u968F\u673A\u9009\u70B9\n"
				"\u5982\u679C\u591A\u4E2A\u5019\u9009\u70B9\u540C\u5206\uFF0CAI \u4F1A\u968F\u673A\u9009\u62E9\u5176\u4E2D\u4E00\u4E2A\uFF0C"
				"\u907F\u514D\u6BCF\u5C40\u90FD\u8D70\u5B8C\u5168\u76F8\u540C\u7684\u56FA\u5B9A\u5957\u8DEF\u3002"),
			card);
		logicSeven->setObjectName("section");
		logicSeven->setWordWrap(true);
		cardLayout->addWidget(logicSeven);

		cardLayout->addStretch(1);

		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
		QPushButton *okButton = buttonBox->addButton(QString::fromUtf8("\u77E5\u9053\u4E86"), QDialogButtonBox::AcceptRole);
		QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
		cardLayout->addWidget(buttonBox);

		dialog.exec();
	}
}

Backgammon::Backgammon(PlayerStatsStore *statsStore, const PlayerRecord &playerRecord, QWidget *parent)
	: QMainWindow(parent)
	, m_pGraphicsScene(0)
	, m_pLastAiPiece(0)
	, m_pWinRateChart(0)
	, m_pStatsStore(statsStore)
	, m_playerRecord(playerRecord)
	, m_sCurrentUser(playerRecord.displayName)
	, m_bStarted(false)
	, m_bPlayerStarts(playerRecord.preferredStarter == "player")
	, m_nPlayerWins(playerRecord.wins)
	, m_nAiWins(playerRecord.losses)
	, m_nFinishedGames(playerRecord.games.isEmpty() ? playerRecord.wins + playerRecord.losses : playerRecord.games.size())
	, m_nMoveCount(0)
	, m_nPlayerWinRate(50.0)
	, m_nAiWinRate(50.0)
	, m_nDeep(8)
	, m_nPreferredDeep(8)
	, m_pThinkToggleBtn(0)
	, m_bShowTop10(false)
	, m_pHintBtn(0)
	, m_bHintVisible(false)
	, m_bPvPMode(false)
	, m_nUndoCount(0)
	, m_nMaxUndoCount(999)
	, m_pAiThread(nullptr)
	, m_pAiWorker(nullptr)
	, m_bAiThinking(false)
	, m_pSoundManager(nullptr)
	, m_pSoundToggleBtn(nullptr)
{
	ui.setupUi(this);

	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
			m_arrBoard[i][j] = NONE;
	}

	m_pGraphicsScene = new QGraphicsScene;
	m_pGraphicsScene->setSceneRect(kLineMin - kScenePadding, kLineMin - kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding);

	ui.starterComboBox->addItem(QString::fromUtf8("\u6211\u5148\u624B\uFF08\u9ED1\u68CB\uFF09"), "player");
	ui.starterComboBox->addItem(QString::fromUtf8("AI \u5148\u624B\uFF08\u767D\u68CB\uFF09"), "ai");
	ui.starterComboBox->addItem(QString::fromUtf8("\u53CC\u4EBA\u5BF9\u6218"), "pvp");
	const int starterIndex = m_bPlayerStarts ? 0 : 1;
	ui.starterComboBox->setCurrentIndex(starterIndex);

	// 难度设置（搜索步数）- 可编辑下拉框支持自定义步数
	ui.difficultyComboBox->setEditable(true);
	ui.difficultyComboBox->addItem(QString::fromUtf8("2 \u6B65"), 2);
	ui.difficultyComboBox->addItem(QString::fromUtf8("4 \u6B65"), 4);
	ui.difficultyComboBox->addItem(QString::fromUtf8("6 \u6B65"), 6);
	ui.difficultyComboBox->addItem(QString::fromUtf8("8 \u6B65"), 8);
	ui.difficultyComboBox->addItem(QString::fromUtf8("10 \u6B65"), 10);
	ui.difficultyComboBox->addItem(QString::fromUtf8("12 \u6B65"), 12);
	ui.difficultyComboBox->addItem(QString::fromUtf8("14 \u6B65"), 14);
	ui.difficultyComboBox->setCurrentIndex(3); // 默认8步
	ui.difficultyComboBox->setCurrentText(QString::fromUtf8("8 \u6B65"));
	// 设置输入验证：只允许1-20的正整数
	QIntValidator *validator = new QIntValidator(1, 50, this);
	ui.difficultyComboBox->lineEdit()->setValidator(validator);
	ui.difficultyComboBox->lineEdit()->setPlaceholderText(QString::fromUtf8("\u8F93\u5165\u81EA\u5B9A\u4E49\u6B65\u6570 (1-50)"));

	// 样式 - 复用 starterComboBox 样式，整体字体加大
	setStyleSheet(
		"* { font-family: \"Source Han Sans CN\", \"Noto Sans CJK SC\", \"Microsoft YaHei\", sans-serif; }"
		"QMainWindow#BackgammonClass {"
		"background: qradialgradient(cx:0.2, cy:0.15, radius:1.25, stop:0 rgba(255,255,255,230), stop:1 rgba(203,217,230,230));"
		"}"
		"QWidget#left_widget {"
		"background-color: rgba(255,255,255,112);"
		"border: 1px solid rgba(255,255,255,195);"
		"border-radius: 20px;"
		"}"
		"QPushButton#startButton, QPushButton#historyButton {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 12px;"
		"padding: 14px 18px;"
		"font-size: 24px;"
		"font-weight: 600;"
		"}"
		"QPushButton#startButton:hover, QPushButton#historyButton:hover {"
		"background-color: rgba(255,255,255,228);"
		"}"
		"QPushButton#startButton:pressed, QPushButton#historyButton:pressed {"
		"background-color: rgba(236,242,249,236);"
		"}"
		"QComboBox#starterComboBox, QComboBox#difficultyComboBox {"
		"min-height: 56px;"
		"padding: 0 16px;"
		"font-size: 22px;"
		"font-weight: 600;"
		"border: 1px solid rgba(190,203,220,220);"
		"border-radius: 14px;"
		"background-color: rgba(255,255,255,210);"
		"color: rgb(47, 58, 76);"
		"}"
		"QComboBox#starterComboBox::drop-down, QComboBox#difficultyComboBox::drop-down {"
		"width: 40px;"
		"border: none;"
		"}"
		"QPushButton#aiInfoButton {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 34px;"
		"font-size: 32px;"
		"font-weight: 700;"
		"color: rgb(63, 84, 117);"
		"}"
		"QPushButton#aiInfoButton:hover {"
		"background-color: rgba(255,255,255,232);"
		"}"
		"QPushButton#aiInfoButton:pressed {"
		"background-color: rgba(236,242,249,236);"
		"}"
		"QLabel#starterTitleLabel,"
		"QLabel#difficultyTitleLabel,"
		"QLabel#currentUserTitleLabel,"
		"QLabel#historyTitleLabel,"
		"QLabel#statsTitleLabel {"
		"color: rgb(58,70,90);"
		"font-size: 26px;"
		"font-weight: 700;"
		"padding-top: 8px;"
		"}"
		"QLabel#currentUserLabel,"
		"QLabel#historyGamesLabel, QLabel#historyPlayerRateLabel, QLabel#historyAiRateLabel,"
		"QLabel#gamesLabel, QLabel#playerRateLabel, QLabel#aiRateLabel {"
		"color: rgb(74,86,105);"
		"background-color: rgba(255,255,255,148);"
		"border: 1px solid rgba(255,255,255,198);"
		"border-radius: 12px;"
		"padding: 14px 16px;"
		"font-size: 22px;"
		"font-weight: 600;"
		"}"
		"QWidget#chartHost {"
		"background-color: rgba(255,255,255,128);"
		"border: 1px solid rgba(255,255,255,195);"
		"border-radius: 16px;"
		"}"
		"QGraphicsView#graphicsView {"
		"background: rgba(255,255,255,78);"
		"border: 1px solid rgba(255,255,255,175);"
		"border-radius: 18px;"
		"}"
		"QPushButton#thinkToggleBtn {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 12px;"
		"padding: 14px 18px;"
		"font-size: 24px;"
		"font-weight: 600;"
		"color: rgb(47, 58, 76);"
		"}"
		"QPushButton#thinkToggleBtn:hover {"
		"background-color: rgba(255,255,255,228);"
		"}"
		"QPushButton#thinkToggleBtn:checked {"
		"background-color: rgba(63,84,117,210);"
		"color: white;"
		"}"
		"QPushButton#thinkToggleBtn:pressed {"
		"background-color: rgba(236,242,249,236);"
		"}"
		"QPushButton#hintButton {"
		"min-height: 68px;"
		"padding: 0 16px;"
		"font-size: 24px;"
		"font-weight: 600;"
		"color: rgb(47, 58, 76);"
		"}"
		"QPushButton#hintButton:hover {"
		"background-color: rgba(255,255,255,228);"
		"}"
		"QPushButton#hintButton:checked {"
		"background-color: rgba(34,139,87,210);"
		"color: white;"
		"}"
		"QPushButton#hintButton:pressed {"
		"background-color: rgba(28,120,74,236);"
		"}");

	// 创建"AI 思考"按钮并插入到左侧面板按钮行（row 0）。
	// 重新排列：col0=start, col1=history, col2=thinkToggle, col3=undo, col4=aiInfo，共5列。
	m_pThinkToggleBtn = new QPushButton(QString::fromUtf8("AI \u601D\u8003"), ui.left_widget);
	m_pThinkToggleBtn->setObjectName("thinkToggleBtn");
	m_pThinkToggleBtn->setCheckable(true);
	m_pThinkToggleBtn->setMinimumHeight(68);
	QGridLayout *leftLayout = qobject_cast<QGridLayout *>(ui.left_widget->layout());
	if (leftLayout)
	{
		// 移走 undoButton 和 aiInfoButton，重新排列为 col2=thinkToggle, col3=undo, col4=aiInfo。
		leftLayout->removeWidget(ui.undoButton);
		leftLayout->removeWidget(ui.aiInfoButton);
		leftLayout->addWidget(m_pThinkToggleBtn, 0, 2);
		leftLayout->addWidget(ui.undoButton, 0, 3);
		leftLayout->addWidget(ui.aiInfoButton, 0, 4);
	}
	connect(m_pThinkToggleBtn, &QPushButton::toggled, this, &Backgammon::slotThinkToggleClicked);

	// 创建「提示」按钮并插入到左侧面板按钮行 col5。
	m_pHintBtn = new QPushButton(QString::fromUtf8("\u63D0\u793A"), ui.left_widget);
	m_pHintBtn->setObjectName("hintButton");
	m_pHintBtn->setCheckable(true);
	m_pHintBtn->setMinimumHeight(68);
	if (leftLayout)
		leftLayout->addWidget(m_pHintBtn, 0, 5);
	connect(m_pHintBtn, &QPushButton::toggled, this, &Backgammon::slotHintBtnClicked);

	// 创建音效管理器
	m_pSoundManager = new SoundManager(this);

	// 创建「音效」开关按钮并插入到左侧面板按钮行 col6
	m_pSoundToggleBtn = new QPushButton(QString::fromUtf8("\u97F3\u6548"), ui.left_widget);
	m_pSoundToggleBtn->setObjectName("soundToggleBtn");
	m_pSoundToggleBtn->setCheckable(true);
	m_pSoundToggleBtn->setChecked(m_pSoundManager->isEnabled());
	m_pSoundToggleBtn->setMinimumHeight(68);
	if (leftLayout)
		leftLayout->addWidget(m_pSoundToggleBtn, 0, 6);
	connect(m_pSoundToggleBtn, &QPushButton::toggled, this, &Backgammon::slotSoundToggleClicked);

	// 棋盘背景：浅木纹径向渐变，中心偏亮、边缘微暗，保持明亮清爽感。
	QRadialGradient boardBg(kBoardCenter * kGridSize + 180, kBoardCenter * kGridSize - 120, 600);
	boardBg.setColorAt(0.0, QColor(245, 222, 179));
	boardBg.setColorAt(0.5, QColor(238, 213, 168));
	boardBg.setColorAt(1.0, QColor(225, 200, 155));
	ui.graphicsView->setBackgroundBrush(QBrush(boardBg));
	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);
	ui.graphicsView->setScene(m_pGraphicsScene);
	ui.graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	ui.graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);

	QGraphicsDropShadowEffect *leftShadow = new QGraphicsDropShadowEffect(this);
	leftShadow->setBlurRadius(34);
	leftShadow->setOffset(0, 10);
	leftShadow->setColor(QColor(74, 91, 116, 88));
	ui.left_widget->setGraphicsEffect(leftShadow);

	QGraphicsDropShadowEffect *boardShadow = new QGraphicsDropShadowEffect(this);
	boardShadow->setBlurRadius(26);
	boardShadow->setOffset(0, 8);
	boardShadow->setColor(QColor(63, 82, 105, 76));
	ui.graphicsView->setGraphicsEffect(boardShadow);

	QVBoxLayout *chartLayout = new QVBoxLayout(ui.chartHost);
	chartLayout->setContentsMargins(10, 10, 10, 10);
	chartLayout->setSpacing(0);

	m_pWinRateChart = new WinRateChart(ui.chartHost);
	chartLayout->addWidget(m_pWinRateChart);

	DrawBoard();
	UpdateStatsPanel();

	connect(ui.startButton, SIGNAL(clicked()), this, SLOT(slotStartBtnClicked()));
	connect(ui.historyButton, SIGNAL(clicked()), this, SLOT(slotHistoryBtnClicked()));
	connect(ui.undoButton, SIGNAL(clicked()), this, SLOT(slotUndoBtnClicked()));
	connect(ui.starterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotStarterChanged(int)));
	connect(ui.difficultyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDifficultyChanged(int)));
	connect(ui.difficultyComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(slotDifficultyTextChanged(QString)));
	connect(ui.aiInfoButton, &QPushButton::clicked, this, [this]() { ShowAiLogicDialog(this, m_nDeep); });
	connect(ui.exportSgfButton, &QPushButton::clicked, this, &Backgammon::slotExportSgfClicked);
	connect(ui.importSgfButton, &QPushButton::clicked, this, &Backgammon::slotImportSgfClicked);
	ui.startButton->setChecked(false);

	// 初始化 AI 异步计算线程
	m_pAiThread = new QThread(this);
	m_pAiWorker = new AiWorker();
	m_pAiWorker->moveToThread(m_pAiThread);
	connect(m_pAiThread, &QThread::finished, m_pAiWorker, &QObject::deleteLater);
	connect(m_pAiWorker, &AiWorker::MoveReady, this, &Backgammon::slotAiMoveReady);
	m_pAiThread->start();

	// 恢复上次退出时保存的窗口状态和难度设置。
	QSettings settings("Backgammon", "Backgammon");
	if (settings.contains("windowGeometry"))
		restoreGeometry(settings.value("windowGeometry").toByteArray());
	const int savedDepth = settings.value("difficulty", 8).toInt();
	SetDifficulty(savedDepth);
	// 同步界面上的难度下拉框显示。
	const QString depthText = QString::number(savedDepth) + QString::fromUtf8(" \u6B65");
	ui.difficultyComboBox->setCurrentText(depthText);
}

Backgammon::~Backgammon()
{
	if (m_pAiThread) {
		m_pAiThread->quit();
		m_pAiThread->wait();
	}
}

void Backgammon::closeEvent(QCloseEvent *event)
{
	// 退出时持久化窗口几何信息和难度设置。
	QSettings settings("Backgammon", "Backgammon");
	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("difficulty", m_nDeep);
	QMainWindow::closeEvent(event);
}

void Backgammon::DrawBoard()
{
	// 棋盘外框：深色边框带阴影，模拟木质边框的立体感。
	QPen borderPen(QColor(60, 40, 20, 200));
	borderPen.setWidth(3);
	const int borderOffset = 8;
	m_pGraphicsScene->addRect(
		kLineMin - borderOffset, kLineMin - borderOffset,
		(kLineMax - kLineMin) + 2 * borderOffset,
		(kLineMax - kLineMin) + 2 * borderOffset,
		borderPen, QBrush(QColor(140, 100, 55, 60)));

	// 网格线：深色主线+浅色偏移线模拟刻痕凹凸效果。
	QPen gridLine(QColor(60, 40, 18, 160));
	gridLine.setWidth(1);
	// 浅色偏移线（模拟光线从刻痕侧面反射）。
	QPen gridHighlight(QColor(120, 90, 50, 80));
	gridHighlight.setWidth(1);

	for (int i = 0; i < kBoardSize; ++i)
	{
		const int coord = BoardToScene(i);
		// 主网格线。
		m_pGraphicsScene->addLine(kLineMin, coord, kLineMax, coord, gridLine);
		m_pGraphicsScene->addLine(coord, kLineMin, coord, kLineMax, gridLine);
		// 凹陷高光线（向右下偏移1像素）。
		m_pGraphicsScene->addLine(kLineMin + 1, coord + 1, kLineMax + 1, coord + 1, gridHighlight);
		m_pGraphicsScene->addLine(coord + 1, kLineMin + 1, coord + 1, kLineMax + 1, gridHighlight);
	}

	// 星位点（天元和四个角星）：径向渐变模拟立体圆点。
	const int starPoints[][2] = {
		{7, 7},   // 天元
		{3, 3}, {3, 11}, {11, 3}, {11, 11}  // 四角星位
	};
	const int starRadius = 5;
	for (int i = 0; i < 5; ++i)
	{
		const int cx = BoardToScene(starPoints[i][0]);
		const int cy = BoardToScene(starPoints[i][1]);
		// 径向渐变：左上高光、右下暗影。
		QRadialGradient starGrad(cx - 1, cy - 1, starRadius);
		starGrad.setColorAt(0.0, QColor(80, 55, 25));
		starGrad.setColorAt(1.0, QColor(50, 35, 15));
		m_pGraphicsScene->addEllipse(
			cx - starRadius, cy - starRadius,
			starRadius * 2, starRadius * 2,
			QPen(Qt::NoPen), QBrush(starGrad));
	}
}

void Backgammon::slotStartBtnClicked()
{
	if (ui.startButton->isChecked())
	{
		ui.startButton->setText(QString::fromUtf8("\u6E05\u9664"));
		ui.starterComboBox->setEnabled(false);
		ui.difficultyComboBox->setEnabled(false);
		m_bStarted = true;
		m_nUndoCount = 0;
		// 游戏开始时启用悔棋按钮（玩家落子后才真正有棋可悔，但提前启用保持界面一致）。
		ui.undoButton->setEnabled(true);
		ResetWinRateEstimate();
		// PvP 模式下黑方始终先手，无需 AI 开局落子。
		if (!m_bPlayerStarts && !m_bPvPMode)
			PlaceAiOpeningMove();
		return;
	}

	if (IsBoardClean())
	{
		FinishRoundCleanup();
		return;
	}

	QMessageBox::StandardButton btn = QMessageBox::warning(
		this,
		QString::fromUtf8("\u8B66\u544A"),
		QString::fromUtf8("\u786E\u5B9A\u6E05\u9664\u68CB\u76D8\uFF1F"),
		QMessageBox::Yes | QMessageBox::No);
	if (btn == QMessageBox::Yes)
	{
		FinishRoundCleanup();
		return;
	}

	ui.startButton->setChecked(true);
}

void Backgammon::slotStarterChanged(int)
{
	const QString pref = CurrentStarterPreference();
	m_bPvPMode = (pref == "pvp");
	m_bPlayerStarts = (pref == "player");
	m_playerRecord.preferredStarter = pref;
	// PvP 模式下隐藏难度选择，PvE 模式下显示。
	ui.difficultyComboBox->setVisible(!m_bPvPMode);
	PersistPlayerRecord();
	UpdateStatsPanel();
}

void Backgammon::slotDifficultyChanged(int index)
{
	const int depth = ui.difficultyComboBox->itemData(index).toInt();
	SetDifficulty(depth);
}

int Backgammon::CurrentDifficulty() const
{
	return m_nDeep;
}

void Backgammon::SetDifficulty(int depth)
{
	m_nDeep = depth;
	m_nPreferredDeep = depth;
}

void Backgammon::slotDifficultyTextChanged(const QString &text)
{
	// 从文本中提取数字
	const QString numbers = text.trimmed();
	bool ok = false;
	int depth = numbers.toInt(&ok);

	// 验证输入：1-20之间的正整数
	if (ok && depth >= 1 && depth <= 50)
	{
		SetDifficulty(depth);
	}
}

void Backgammon::slotHistoryBtnClicked()
{
	HistoryDialog dialog(m_sCurrentUser, m_playerRecord, this);
	// 连接删除信号：用户在历史对话框中删除选中记录后，更新内存中的 PlayerRecord 并持久化。
	connect(&dialog, &HistoryDialog::deletedIndices, this,
		[this](const QVector<int> &indices)
		{
			if (indices.isEmpty())
				return;
			// 按索引降序排列，从后往前删除，避免索引偏移。
			QVector<int> sorted = indices;
			std::sort(sorted.begin(), sorted.end(), std::greater<int>());
			for (int idx : sorted)
			{
				if (idx >= 0 && idx < m_playerRecord.games.size())
				{
					// 同步更新胜负计数。
					if (m_playerRecord.games[idx].playerWon)
						--m_nPlayerWins;
					else
						--m_nAiWins;
					m_playerRecord.games.remove(idx);
				}
			}
			m_nFinishedGames = m_playerRecord.games.size();
			// 重新计算并持久化。
			PersistPlayerRecord();
			UpdateStatsPanel();
		});
	// 连接回放信号：用户选择回放某局时，打开 ReplayDialog 展示棋谱。
	connect(&dialog, &HistoryDialog::replayRequested, this,
		[this](int gameIndex)
		{
			if (gameIndex < 0 || gameIndex >= m_playerRecord.games.size())
				return;
			ReplayDialog *replayDlg = new ReplayDialog(m_playerRecord.games[gameIndex], this);
			replayDlg->setAttribute(Qt::WA_DeleteOnClose);
			replayDlg->exec();
		});
	dialog.exec();
}
void Backgammon::slotUndoBtnClicked()
{
	// 检查是否有棋可悔
	if (m_currentGameMoves.isEmpty())
	{
		QMessageBox::information(this,
			QString::fromUtf8("提示"),
			QString::fromUtf8("当前没有可悔的棋步！"));
		return;
	}

	// PvE 模式一次撤销玩家和 AI 各一步，PvP 模式每次撤销一步。
	const int stepsToUndo = m_bPvPMode ? 1 : 2;

	for (int i = 0; i < stepsToUndo && !m_currentGameMoves.isEmpty(); ++i)
	{
		const MoveRecord lastMove = m_currentGameMoves.back();
		m_currentGameMoves.pop_back();

		// 同步撤销胜率历史记录。
		if (!m_playerRateHistory.isEmpty())
			m_playerRateHistory.pop_back();
		if (!m_aiRateHistory.isEmpty())
			m_aiRateHistory.pop_back();

		m_arrBoard[lastMove.row][lastMove.col] = NONE;

		// 从场景中找到并删除对应位置的棋子图形项。
		const QList<QGraphicsItem*> items = m_pGraphicsScene->items();
		for (QGraphicsItem* item : items)
		{
			if (item->type() != QGraphicsEllipseItem::Type)
				continue;
			QGraphicsEllipseItem* ellipseItem = static_cast<QGraphicsEllipseItem*>(item);
			const QRectF rect = ellipseItem->rect();
			const int boardRow = SceneToBoard(rect.x() + rect.width() / 2.0);
			const int boardCol = SceneToBoard(rect.y() + rect.height() / 2.0);
			if (boardRow == lastMove.row && boardCol == lastMove.col)
			{
				if (ellipseItem == m_pLastAiPiece)
					m_pLastAiPiece = nullptr;
				m_pGraphicsScene->removeItem(item);
				delete item;
				break;
			}
		}
	}

	m_nMoveCount = m_currentGameMoves.size();

	if (m_playerRateHistory.isEmpty())
	{
		m_nPlayerWinRate = 50.0;
		m_nAiWinRate = 50.0;
	}
	else
	{
		m_nPlayerWinRate = m_playerRateHistory.back();
		m_nAiWinRate = 100.0 - m_nPlayerWinRate;
	}

	// 无可悔棋步时禁用悔棋按钮。
	ui.undoButton->setEnabled(!m_currentGameMoves.isEmpty());
	// 悔棋音效
	if (m_pSoundManager) m_pSoundManager->playUndo();
}

bool Backgammon::IsBoardClean()
{
	return m_pGraphicsScene->items().size() <= (kBoardSize * 2);
}

void Backgammon::showEvent(QShowEvent *event)
{
	QMainWindow::showEvent(event);
	UpdateBoardView();
}

void Backgammon::resizeEvent(QResizeEvent *event)
{
	QMainWindow::resizeEvent(event);
	UpdateBoardView();
}

void Backgammon::mousePressEvent(QMouseEvent * event)
{
	if (!m_bStarted)
		return;

	QPoint viewPos = ui.graphicsView->mapFrom(this, event->pos());
	if (!ui.graphicsView->rect().contains(viewPos))
		return;

	QPointF scenePos = ui.graphicsView->mapToScene(viewPos);
	const int row = SceneToBoard(scenePos.x());
	const int col = SceneToBoard(scenePos.y());

	if (row >= kBoardSize || row < 0 || col >= kBoardSize || col < 0)
	{
		QMessageBox::warning(this, QString::fromUtf8("\u8B66\u544A"), QString::fromUtf8("\u65E0\u6CD5\u843D\u5B50\uFF01"));
		return;
	}

	if (m_arrBoard[row][col] != NONE)
	{
		QMessageBox::warning(this, QString::fromUtf8("\u8B66\u544A"), QString::fromUtf8("\u65E0\u6CD5\u843D\u5B50\uFF01"));
		return;
	}

	if (m_bPvPMode)
	{
		// PvP 模式：根据当前已落子数奇偶决定落黑/白子。
		const bool isBlackTurn = (m_currentGameMoves.size() % 2 == 0);
		if (isBlackTurn)
		{
			PlacePlayerMove(row, col);
			UpdateWinRateEstimate(WHITE);
			const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
			if (m_winDetector.checkWin(gboard, game_core::Piece::Black))
			{
				RecordGameResult(BLACK);
				ResultDialog(this, true, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
				FinishRoundCleanup();
				return;
			}
		}
		else
		{
			PlaceAiMove(row, col);
			UpdateWinRateEstimate(BLACK);
			const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
			if (m_winDetector.checkWin(gboard, game_core::Piece::White))
			{
				RecordGameResult(WHITE);
				ResultDialog(this, false, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
				FinishRoundCleanup();
				return;
			}
		}
		// PvP 模式刷新轮次提示。
		UpdateStatsPanel();
		return;
	}

	PlacePlayerMove(row, col);
	// 玩家刚走完，接下来轮到 AI 落子。
	UpdateWinRateEstimate(WHITE);

	const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
	if (m_winDetector.checkWin(gboard, game_core::Piece::Black))
	{
		RecordGameResult(BLACK);
		ResultDialog(this, true, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		FinishRoundCleanup();
		return;
	}

	// 通过 AiWorker 异步计算 AI 落子，禁止玩家在此期间操作
	m_bAiThinking = true;
	m_bStarted = false;
	m_pAiWorker->SetBoard(m_arrBoard, m_nDeep);
	QMetaObject::invokeMethod(m_pAiWorker, "Run", Qt::QueuedConnection);
}

void Backgammon::CleanBoard()
{
	QList<QGraphicsItem*> list = m_pGraphicsScene->items();
	while (!list.isEmpty())
	{
		m_pGraphicsScene->removeItem(list[0]);
		list.removeFirst();
	}

	DrawBoard();

	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
			m_arrBoard[i][j] = NONE;
	}

	ResetWinRateEstimate();
	UpdateBoardView();
}

void Backgammon::RecordGameResult(ePiece winner)
{
	if (winner != BLACK && winner != WHITE)
		return;

	GameRecord game;
	game.finishedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	game.playerWon = (winner == BLACK);
	game.playerStarted = m_bPlayerStarts;
	game.moveCount = m_currentGameMoves.size();
	game.moves = m_currentGameMoves;
	m_playerRecord.games.push_back(game);

	if (winner == BLACK)
		++m_nPlayerWins;
	else
		++m_nAiWins;

	m_nFinishedGames = m_playerRecord.games.size();
	PersistPlayerRecord();
	UpdateStatsPanel();
	// 胜负音效
	if (m_pSoundManager) m_pSoundManager->playWin();
}

void Backgammon::PersistPlayerRecord()
{
	if (!m_pStatsStore || m_sCurrentUser.trimmed().isEmpty())
		return;

	m_playerRecord.displayName = m_sCurrentUser;
	m_playerRecord.wins = m_nPlayerWins;
	m_playerRecord.losses = m_nAiWins;
	m_playerRecord.preferredStarter = CurrentStarterPreference();
	m_pStatsStore->SaveRecord(m_playerRecord);
	m_pStatsStore->Save();
}

void Backgammon::ResetWinRateEstimate()
{
	m_nMoveCount = 0;
	m_nPlayerWinRate = 50.0;
	m_nAiWinRate = 50.0;
	m_playerRateHistory.clear();
	m_aiRateHistory.clear();
	m_currentGameMoves.clear();
	m_pLastAiPiece = 0;
	ClearTop10Overlay();
	ClearHintOverlay();
	if (m_pHintBtn)
		m_pHintBtn->setChecked(false);
	UpdateStatsPanel();
}

void Backgammon::UpdateStatsPanel()
{
	ui.currentUserLabel->setText(
		QString::fromUtf8("%1\n\u9ED8\u8BA4\u5F00\u5C40\uFF1A%2")
			.arg(m_sCurrentUser)
			.arg(StarterText(m_bPlayerStarts)));
	ui.historyGamesLabel->setText(QString::fromUtf8("\u7D2F\u8BA1\u5C40\u6570 %1").arg(m_nFinishedGames));
	ui.historyPlayerRateLabel->setText(
		QString::fromUtf8("\u6211 %1 \u80DC | \u80DC\u7387 %2%")
			.arg(m_nPlayerWins)
			.arg(m_nFinishedGames == 0 ? 0 : qRound(m_nPlayerWins * 100.0 / m_nFinishedGames)));
	ui.historyAiRateLabel->setText(
		QString::fromUtf8("AI %1 \u80DC | \u80DC\u7387 %2%")
			.arg(m_nAiWins)
			.arg(m_nFinishedGames == 0 ? 0 : qRound(m_nAiWins * 100.0 / m_nFinishedGames)));
	ui.gamesLabel->setText(QString::fromUtf8("\u5F53\u524D\u624B\u6570 %1").arg(m_nMoveCount));
	ui.playerRateLabel->setText(QString::fromUtf8("\u6211\u7684\u9884\u4F30\u80DC\u7387 %1%").arg(qRound(m_nPlayerWinRate)));
	ui.aiRateLabel->setText(QString::fromUtf8("AI \u9884\u4F30\u80DC\u7387 %1%").arg(qRound(m_nAiWinRate)));
	ui.historyButton->setText(QString::fromUtf8("\u5386\u53F2\u5BF9\u5C40 (%1)").arg(m_playerRecord.games.size()));

	if (m_pWinRateChart)
		m_pWinRateChart->SetSeries(m_playerRateHistory, m_aiRateHistory);
}

void Backgammon::UpdateWinRateEstimate(ePiece nextPiece)
{
	const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);

	if (m_winDetector.checkWin(gboard, game_core::Piece::White))
	{
		m_nAiWinRate = 100.0;
		m_nPlayerWinRate = 0.0;
	}
	else if (m_winDetector.checkWin(gboard, game_core::Piece::Black))
	{
		m_nAiWinRate = 0.0;
		m_nPlayerWinRate = 100.0;
	}
	// 检查即将落子的轮次方是否已经有冲四/活四（一步即可五连）。
	else if (nextPiece == BLACK && m_boardEvaluator.hasWinningMove(gboard, game_core::Piece::Black))
	{
		m_nAiWinRate = 0.0;
		m_nPlayerWinRate = 100.0;
	}
	else if (nextPiece == WHITE && m_boardEvaluator.hasWinningMove(gboard, game_core::Piece::White))
	{
		m_nAiWinRate = 100.0;
		m_nPlayerWinRate = 0.0;
	}
	else
	{
		const int score = m_boardEvaluator.evaluate(gboard);
		m_nAiWinRate = ScoreToAiWinRate(score);
		m_nPlayerWinRate = 100.0 - m_nAiWinRate;
	}

	++m_nMoveCount;
	m_playerRateHistory.push_back(m_nPlayerWinRate);
	m_aiRateHistory.push_back(m_nAiWinRate);
	UpdateStatsPanel();

	// 如果 Top10 标记处于开启状态，每次落子后自动刷新标记。
	if (m_bShowTop10)
		ComputeAndShowTop10();
}

void Backgammon::UpdateBoardView()
{
	if (!ui.graphicsView || !ui.graphicsView->scene())
		return;

	if (ui.graphicsView->viewport()->width() <= 0 || ui.graphicsView->viewport()->height() <= 0)
		return;

	ui.graphicsView->resetTransform();
	ui.graphicsView->fitInView(ui.graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void Backgammon::SetLastAiPiece(QGraphicsEllipseItem *piece)
{
	// 恢复上一颗 AI 棋子的原始 3D 白色外观。
	if (m_pLastAiPiece)
	{
		QRectF rect = m_pLastAiPiece->rect();
		const qreal cx = rect.x() + rect.width() / 2.0 - 6;
		const qreal cy = rect.y() + rect.height() / 2.0 - 6;
		QRadialGradient whiteGrad(cx, cy, kPieceRadius);
		whiteGrad.setColorAt(0.0, QColor(255, 255, 255));
		whiteGrad.setColorAt(0.6, QColor(235, 235, 235));
		whiteGrad.setColorAt(0.85, QColor(200, 200, 200));
		whiteGrad.setColorAt(1.0, QColor(160, 160, 160));
		m_pLastAiPiece->setBrush(QBrush(whiteGrad));
		m_pLastAiPiece->setPen(QPen(QColor(140, 140, 140, 120), 1));
	}

	m_pLastAiPiece = piece;
	if (!m_pLastAiPiece)
		return;

	// 高亮样式：暖色径向渐变+金色边框，模拟光晕效果。
	QRectF rect = m_pLastAiPiece->rect();
	const qreal cx = rect.x() + rect.width() / 2.0 - 6;
	const qreal cy = rect.y() + rect.height() / 2.0 - 6;
	QRadialGradient hlGrad(cx, cy, kPieceRadius);
	hlGrad.setColorAt(0.0, QColor(255, 252, 240));
	hlGrad.setColorAt(0.5, QColor(250, 240, 210));
	hlGrad.setColorAt(0.85, QColor(230, 210, 160));
	hlGrad.setColorAt(1.0, QColor(200, 175, 120));
	m_pLastAiPiece->setBrush(QBrush(hlGrad));
	m_pLastAiPiece->setPen(QPen(QColor(210, 170, 80, 200), 2));
}

void Backgammon::AppendMove(int row, int col, ePiece piece)
{
	MoveRecord move;
	move.row = row;
	move.col = col;
	move.piece = piece;
	m_currentGameMoves.push_back(move);
}

void Backgammon::PlaceAiMove(int row, int col)
{
	m_arrBoard[row][col] = WHITE;
	const int x = BoardToScene(row) - kPieceRadius;
	const int y = BoardToScene(col) - kPieceRadius;
	// 白子 3D 效果：径向渐变模拟球体光照（左上高光→右下暗影）。
	const qreal cx = x + kPieceRadius - 6;
	const qreal cy = y + kPieceRadius - 6;
	QRadialGradient whiteGrad(cx, cy, kPieceRadius);
	whiteGrad.setColorAt(0.0, QColor(255, 255, 255));
	whiteGrad.setColorAt(0.6, QColor(235, 235, 235));
	whiteGrad.setColorAt(0.85, QColor(200, 200, 200));
	whiteGrad.setColorAt(1.0, QColor(160, 160, 160));
	QGraphicsEllipseItem *aiPiece = m_pGraphicsScene->addEllipse(
		x, y, kPieceSize, kPieceSize,
		QPen(QColor(140, 140, 140, 120), 1), QBrush(whiteGrad));
	SetLastAiPiece(aiPiece);
	AppendMove(row, col, WHITE);
	// 落子音效
	if (m_pSoundManager) m_pSoundManager->playPlace();
}

void Backgammon::PlacePlayerMove(int row, int col)
{
	// 落子后清除提示标记并重置按钮状态。
	ClearHintOverlay();
	if (m_pHintBtn && m_pHintBtn->isChecked())
		m_pHintBtn->setChecked(false);
	m_arrBoard[row][col] = BLACK;
	const int x = BoardToScene(row) - kPieceRadius;
	const int y = BoardToScene(col) - kPieceRadius;
	// 黑子 3D 效果：径向渐变模拟球体光照（左上微光→右下深黑）。
	const qreal cx = x + kPieceRadius - 6;
	const qreal cy = y + kPieceRadius - 6;
	QRadialGradient blackGrad(cx, cy, kPieceRadius);
	blackGrad.setColorAt(0.0, QColor(90, 90, 90));
	blackGrad.setColorAt(0.5, QColor(50, 50, 50));
	blackGrad.setColorAt(0.85, QColor(25, 25, 25));
	blackGrad.setColorAt(1.0, QColor(10, 10, 10));
	m_pGraphicsScene->addEllipse(
		x, y, kPieceSize, kPieceSize,
		QPen(QColor(5, 5, 5, 80), 1), QBrush(blackGrad));
	AppendMove(row, col, BLACK);
	// 落子音效
	if (m_pSoundManager) m_pSoundManager->playPlace();
}

void Backgammon::PlaceAiOpeningMove()
{
	PlaceAiMove(kBoardCenter, kBoardCenter);
	// 重置时不需指定轮次方（初始状态 50/50）。
	UpdateWinRateEstimate(NONE);
}

void Backgammon::FinishRoundCleanup()
{
	m_bStarted = false;
	ui.startButton->setText(QString::fromUtf8("\u5F00\u59CB"));
	ui.startButton->setChecked(false);
	ui.starterComboBox->setEnabled(true);
	ui.difficultyComboBox->setEnabled(true);
	// 对局结束后禁用悔棋按钮，并重置悔棋计数。
	ui.undoButton->setEnabled(false);
	m_nUndoCount = 0;
	m_currentGameMoves.clear();
	CleanBoard();
}

QString Backgammon::CurrentStarterPreference() const
{
	return ui.starterComboBox->currentData().toString();
}

void Backgammon::slotAiMoveReady(int row, int col)
{
	// AI 计算完成，在主线程落子并恢复输入
	m_bAiThinking = false;
	m_bStarted = true;

	PlaceAiMove(row, col);
	// AI 刚走完，接下来轮到玩家落子。
	UpdateWinRateEstimate(BLACK);

	const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
	if (m_winDetector.checkWin(gboard, game_core::Piece::White))
	{
		RecordGameResult(WHITE);
		ResultDialog(this, false, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		FinishRoundCleanup();
	}
}

void Backgammon::slotThinkToggleClicked()
{
	m_bShowTop10 = m_pThinkToggleBtn->isChecked();
	if (m_bShowTop10)
	{
		// 开启：计算并显示 Top10 候选点。
		ComputeAndShowTop10();
	}
	else
	{
		// 关闭：清除棋盘上的标记。
		ClearTop10Overlay();
	}
}

void Backgammon::ComputeAndShowTop10()
{
	// 先清除旧的标记。
	ClearTop10Overlay();

	// 如果没有评估器或棋盘为空（第一手AI天元除外），不显示。
	// 确定当前轮到谁走：偶数手数后轮到玩家（BLACK），奇数手数后轮到AI（WHITE）。
	const ePiece currentPiece = (m_nMoveCount % 2 == 0) ? BLACK : WHITE;

	// 收集所有候选空位（八邻域有棋子的空点）。
	QVector<std::tuple<int, int, int>> candidates; // {row, col, score}
	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
		{
			if (m_arrBoard[i][j] != NONE)
				continue;

			// 候选点筛选：八邻域至少有一枚棋子，或者棋盘为空（中心点）。
			bool hasNeighbor = false;
			for (int di = -1; di <= 1 && !hasNeighbor; ++di)
			{
				for (int dj = -1; dj <= 1 && !hasNeighbor; ++dj)
				{
					if (di == 0 && dj == 0) continue;
					int ni = i + di, nj = j + dj;
					if (ni >= 0 && ni < kBoardSize && nj >= 0 && nj < kBoardSize
						&& m_arrBoard[ni][nj] != NONE)
					{
						hasNeighbor = true;
					}
				}
			}
			if (!hasNeighbor && !(i == kBoardCenter && j == kBoardCenter && m_nMoveCount == 0))
				continue;

			// 计算综合评估分 = 己方增益 + 对手阻断价值
			const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
			const game_core::Piece gcur = (currentPiece == BLACK) ? game_core::Piece::Black : game_core::Piece::White;
			const game_core::Piece gopp = (currentPiece == BLACK) ? game_core::Piece::White : game_core::Piece::Black;
			const int attackScore = m_boardEvaluator.evaluateMove(gboard, game_core::Position(i, j), gcur);
			const ePiece opponent = (currentPiece == BLACK) ? WHITE : BLACK;
			const int defenseScore = m_boardEvaluator.evaluateMove(gboard, game_core::Position(i, j), gopp);
			const int totalScore = attackScore + defenseScore * 2;
			candidates.push_back({i, j, totalScore});
		}
	}

	// 按综合分降序排列，取前 10。
	std::sort(candidates.begin(), candidates.end(),
		[](const std::tuple<int, int, int> &a, const std::tuple<int, int, int> &b)
		{
			return std::get<2>(a) > std::get<2>(b);
		});

	const int topN = qMin(5, candidates.size());
	if (topN == 0)
		return;

	// 找出最高分和最低分（用于归一化颜色）。
	const int maxScore = std::get<2>(candidates[0]);
	const int minScore = std::get<2>(candidates[topN - 1]);
	const int scoreRange = maxScore - minScore;

	// 对每个 Top 候选点绘制标记：半透明圆 + 胜率。
	// 使用 ComputerMove.MaxMinSearch 深度搜索精确评估每个候选点的胜率。
	// 搜索深度 = 用户设置的步数 + 5，比正常AI落子多思考几步以获得更准确的胜率。
	const int thinkDepth = m_nDeep + 5;
	QVector<double> winRates(topN);
	for (int rank = 0; rank < topN; ++rank)
	{
		const int row = std::get<0>(candidates[rank]);
		const int col = std::get<1>(candidates[rank]);
		// 临时落子，用 AIEngine 搜索评估该点后续发展
		m_arrBoard[row][col] = currentPiece;
		game_core::GameBoard tempBoard = ToGameBoard(m_arrBoard);
		const game_core::Piece gcur = (currentPiece == WHITE) ? game_core::Piece::White : game_core::Piece::Black;
		m_aiEngine.setSearchDepth(thinkDepth - 1);
		m_aiEngine.calculateBestMove(tempBoard, gcur);
		const int boardScore = m_boardEvaluator.evaluate(tempBoard);
		m_arrBoard[row][col] = NONE;
		const int effectiveScore = (currentPiece == WHITE) ? boardScore : -boardScore;
		winRates[rank] = ScoreToAiWinRate(effectiveScore);
	}
	// 胜率归一化范围：胜率高 → normalizedRate 大 → 颜色深。
	double maxRate = winRates[0], minRate = winRates[0];
	for (int rank = 1; rank < topN; ++rank)
	{
		if (winRates[rank] > maxRate) maxRate = winRates[rank];
		if (winRates[rank] < minRate) minRate = winRates[rank];
	}
	const double rateRange = maxRate - minRate;

	for (int rank = 0; rank < topN; ++rank)
	{
		const int row = std::get<0>(candidates[rank]);
		const int col = std::get<1>(candidates[rank]);
		const double winRate = winRates[rank];

		// 用胜率归一化控制颜色深浅：胜率越高颜色越深。
		const double normalizedRate = (rateRange > 0)
			? (winRate - minRate) / rateRange
			: 1.0;

		// 绘制半透明圆形标记，大小与棋子一致，覆盖在交叉点上。
		const qreal cx = BoardToScene(row);
		const qreal cy = BoardToScene(col);
		const int markerRadius = kPieceRadius + 4;

		QGraphicsEllipseItem *marker = m_pGraphicsScene->addEllipse(
			cx - markerRadius, cy - markerRadius,
			markerRadius * 2, markerRadius * 2,
			QPen(Qt::NoPen),
			QBrush(QColor(209, 132, 47, static_cast<int>(60 + normalizedRate * 120))));
		m_top10Items.push_back(marker);

		// 胜率数值直接放在圆圈内部居中显示（不带百分号）。
		QString rateStr = QString::number(qRound(winRate));
		QGraphicsSimpleTextItem *rateText = m_pGraphicsScene->addSimpleText(rateStr);
		QFont rateFont;
		rateFont.setPixelSize(14);
		rateFont.setBold(true);
		rateText->setFont(rateFont);
		// 文字颜色：高分用深色确保可读，低分用浅色。
		const int textAlpha = static_cast<int>(180 + normalizedRate * 75);
		rateText->setBrush(QColor(255, 255, 255, textAlpha));
		// 以文本包围盒的中心为原点，再平移到交叉点(cx, cy)，实现精确视觉居中。
		QFontMetrics fm(rateFont);
		const qreal textW = fm.horizontalAdvance(rateStr);
		const qreal textH = fm.height();
		const qreal origX = textW / 2.0;
		const qreal origY = textH / 2.0 + fm.ascent() - textH; // ascent 上方偏移修正
		QTransform t;
		t.translate(cx - origX, cy - origY);
		rateText->setTransform(t);
		m_top10Items.push_back(rateText);
	}
}

void Backgammon::ClearTop10Overlay()
{
	// 从场景中移除并删除所有 Top10 标记图形项。
	for (QGraphicsItem *item : m_top10Items)
	{
		if (item)
		{
			m_pGraphicsScene->removeItem(item);
			delete item;
		}
	}
	m_top10Items.clear();
}

void Backgammon::ClearHintOverlay()
{
	// 从场景中移除并删除所有提示标记图形项。
	for (QGraphicsItem *item : m_hintItems)
	{
		if (item)
		{
			m_pGraphicsScene->removeItem(item);
			delete item;
		}
	}
	m_hintItems.clear();
	m_bHintVisible = false;
}

void Backgammon::slotHintBtnClicked(bool checked)
{
	if (checked)
		ComputeAndShowHint();
	else
		ClearHintOverlay();
}

void Backgammon::ComputeAndShowHint()
{
	ClearHintOverlay();

	// 提示从玩家视角计算：玩家执 BLACK，AI 执 WHITE（无论先后手）。
	// 若 PvP 模式则根据当前手数判断轮到哪方。
	const ePiece playerPiece = m_bPvPMode
		? ((m_nMoveCount % 2 == 0) ? BLACK : WHITE)
		: BLACK;
	const ePiece opponentPiece = (playerPiece == BLACK) ? WHITE : BLACK;

	// 收集八邻域内有棋子的空位作为候选点。
	QVector<std::tuple<int, int, int>> candidates;
	const game_core::GameBoard gboard = ToGameBoard(m_arrBoard);
	const game_core::Piece gplayer = (playerPiece == BLACK) ? game_core::Piece::Black : game_core::Piece::White;
	const game_core::Piece gopp = (opponentPiece == BLACK) ? game_core::Piece::Black : game_core::Piece::White;

	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
		{
			if (m_arrBoard[i][j] != NONE)
				continue;

			// 候选条件：八邻域有棋子，或棋盘为空时取中心。
			bool hasNeighbor = false;
			for (int di = -1; di <= 1 && !hasNeighbor; ++di)
				for (int dj = -1; dj <= 1 && !hasNeighbor; ++dj)
				{
					if (di == 0 && dj == 0) continue;
					int ni = i + di, nj = j + dj;
					if (ni >= 0 && ni < kBoardSize && nj >= 0 && nj < kBoardSize
						&& m_arrBoard[ni][nj] != NONE)
						hasNeighbor = true;
				}
			if (!hasNeighbor && !(i == kBoardCenter && j == kBoardCenter && m_nMoveCount == 0))
				continue;

			// 综合分 = 进攻价值 + 防守价值 * 2
			const int attack  = m_boardEvaluator.evaluateMove(gboard, game_core::Position(i, j), gplayer);
			const int defense = m_boardEvaluator.evaluateMove(gboard, game_core::Position(i, j), gopp);
			candidates.push_back({i, j, attack + defense * 2});
		}
	}

	if (candidates.isEmpty())
		return;

	// 取综合分最高的前 3 个。
	std::sort(candidates.begin(), candidates.end(),
		[](const std::tuple<int, int, int> &a, const std::tuple<int, int, int> &b)
		{ return std::get<2>(a) > std::get<2>(b); });

	const int topN = qMin(3, static_cast<int>(candidates.size()));

	// 绘制绿色圆形标记，排名越高圆形越不透明。
	for (int k = 0; k < topN; ++k)
	{
		const int row = std::get<0>(candidates[k]);
		const int col = std::get<1>(candidates[k]);
		const int cx = BoardToScene(col);
		const int cy = BoardToScene(row);

		// 透明度：第1名最深，第3名较浅。
		const int alpha = 200 - k * 50;
		const int markerRadius = 18 - k * 2;

		// 绿色填充圆，带白色边框。
		QGraphicsEllipseItem *marker = m_pGraphicsScene->addEllipse(
			cx - markerRadius, cy - markerRadius,
			markerRadius * 2, markerRadius * 2,
			QPen(QColor(255, 255, 255, alpha), 2),
			QBrush(QColor(50, 200, 80, alpha)));
		m_hintItems.push_back(marker);

		// 在圆内显示排名数字（1/2/3）。
		QGraphicsSimpleTextItem *rankText = m_pGraphicsScene->addSimpleText(QString::number(k + 1));
		QFont f;
		f.setPixelSize(13);
		f.setBold(true);
		rankText->setFont(f);
		rankText->setBrush(Qt::white);
		QFontMetrics fm(f);
		const qreal tw = fm.horizontalAdvance(QString::number(k + 1));
		const qreal th = fm.height();
		QTransform t;
		t.translate(cx - tw / 2.0, cy - th / 2.0);
		rankText->setTransform(t);
		m_hintItems.push_back(rankText);
	}
	m_bHintVisible = true;
}

void Backgammon::slotSoundToggleClicked(bool checked)
{
	// 将开关状态同步给音效管理器并持久化。
	if (m_pSoundManager)
		m_pSoundManager->setEnabled(checked);
}

void Backgammon::slotExportSgfClicked()
{
	// 若当前无对局记录则提示并返回。
	if (m_currentGameMoves.isEmpty()) {
		QMessageBox::information(this, tr("导出 SGF"), tr("当前没有可导出的对局记录。"));
		return;
	}

	// 弹出文件保存对话框
	QString filePath = QFileDialog::getSaveFileName(
		this,
		tr("导出 SGF 棋谱"),
		QString(),
		tr("SGF 文件 (*.sgf);;所有文件 (*)"));
	if (filePath.isEmpty())
		return;

	// 构建 GameRecord 用于导出
	GameRecord record;
	record.moves = m_currentGameMoves;
	record.moveCount = m_currentGameMoves.size();
	record.playerStarted = m_bPlayerStarts;
	record.finishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);

	if (!SgfSerializer::saveToFile(filePath, record, m_sCurrentUser)) {
		QMessageBox::warning(this, tr("导出失败"), tr("无法写入文件：%1").arg(filePath));
		return;
	}
	QMessageBox::information(this, tr("导出成功"), tr("棋谱已保存至：%1").arg(filePath));
}

void Backgammon::slotImportSgfClicked()
{
	// 弹出文件选择对话框
	QString filePath = QFileDialog::getOpenFileName(
		this,
		tr("导入 SGF 棋谱"),
		QString(),
		tr("SGF 文件 (*.sgf);;所有文件 (*)"));
	if (filePath.isEmpty())
		return;

	bool ok = false;
	GameRecord record = SgfSerializer::loadFromFile(filePath, ok);
	if (!ok || record.moves.isEmpty()) {
		QMessageBox::warning(this, tr("导入失败"), tr("无法解析 SGF 文件，请确认文件格式正确。"));
		return;
	}

	// 将导入的棋谱加载到当前游戏状态供回放
	CleanBoard();
	m_currentGameMoves = record.moves;
	m_bPlayerStarts = record.playerStarted;

	// 在棋盘上重现所有落子
	for (const MoveRecord &mv : m_currentGameMoves) {
		if (mv.row < 0 || mv.row >= 15 || mv.col < 0 || mv.col >= 15) continue;
		m_arrBoard[mv.row][mv.col] = mv.piece;
	}
	UpdateBoardView();

	QMessageBox::information(this,
		tr("导入成功"),
		tr("已加载 %1 步棋谱。").arg(record.moveCount));
}
