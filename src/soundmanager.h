#pragma once

#include <QObject>
#include <QSettings>

// 音效管理器：运行时合成简单音调，无需外部音频文件。
// 支持落子、悔棋、胜利三种音效，以及全局开关。
class SoundManager : public QObject
{
    Q_OBJECT

public:
    explicit SoundManager(QObject *parent = nullptr);

    // 播放落子音效
    void playPlace();
    // 播放悔棋音效
    void playUndo();
    // 播放胜利音效
    void playWin();

    // 音效是否启用
    bool isEnabled() const { return m_bEnabled; }
    // 设置音效开关，并持久化到 QSettings
    void setEnabled(bool enabled);

private:
    // 合成并播放指定频率和时长的正弦波音调
    void playTone(int frequencyHz, int durationMs, float volume = 0.5f);

    bool m_bEnabled;
};
