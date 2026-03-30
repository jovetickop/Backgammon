#include "soundmanager.h"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QByteArray>
#include <QSettings>
#include <cmath>

SoundManager::SoundManager(QObject *parent)
    : QObject(parent)
{
    QSettings settings;
    m_bEnabled = settings.value("sound/enabled", true).toBool();
}

void SoundManager::setEnabled(bool enabled)
{
    m_bEnabled = enabled;
    QSettings settings;
    settings.setValue("sound/enabled", enabled);
}

void SoundManager::playPlace()
{
    // 落子：清脆短促高音（800 Hz，80ms）
    playTone(800, 80, 0.4f);
}

void SoundManager::playUndo()
{
    // 悔棋：低沉短音（350 Hz，120ms）
    playTone(350, 120, 0.35f);
}

void SoundManager::playWin()
{
    // 胜利：两段上升音（600 Hz + 900 Hz）
    playTone(600, 150, 0.5f);
    playTone(900, 200, 0.5f);
}

void SoundManager::playTone(int frequencyHz, int durationMs, float volume)
{
    if (!m_bEnabled)
        return;

    // 配置音频格式：44100 Hz 单声道 16-bit
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    // 生成 PCM 正弦波数据
    const int sampleCount = format.sampleRate() * durationMs / 1000;
    QByteArray pcmData;
    pcmData.resize(sampleCount * 2); // 16-bit = 2 bytes/sample
    qint16 *samples = reinterpret_cast<qint16 *>(pcmData.data());

    for (int i = 0; i < sampleCount; ++i)
    {
        // 使用衰减包络避免爆音
        float envelope = 1.0f - static_cast<float>(i) / sampleCount;
        float sample = volume * envelope
            * std::sin(2.0f * M_PI * frequencyHz * i / format.sampleRate());
        samples[i] = static_cast<qint16>(sample * 32767);
    }

    // 通过 QBuffer 驱动 QAudioOutput 播放
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(pcmData);
    buffer->open(QIODevice::ReadOnly);

    QAudioOutput *output = new QAudioOutput(format, this);
    // 播放完成后自动销毁
    connect(output, &QAudioOutput::stateChanged,
            [output, buffer](QAudio::State state)
            {
                if (state == QAudio::IdleState || state == QAudio::StoppedState)
                {
                    output->stop();
                    buffer->close();
                    output->deleteLater();
                    buffer->deleteLater();
                }
            });
    output->start(buffer);
}
