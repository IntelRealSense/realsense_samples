// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <cstring>

inline int64_t GetCurrentTimeInMiliSecods()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

class float_stats
{
private:
    std::unique_ptr<float[]> m_pfIntervalsInMiliSecondsFrontBuffer;
    std::unique_ptr<float[]> m_pfCallbackIntervalsInMiliSecondsFrontBuffer;
    const int m_iSize;
    int m_iCount;
    float m_fPreviousTimestamp;
    std::string m_collectionType;
    int m_frameCounter;
    uint64_t m_droppedFrames;
    uint64_t m_junkTimestampFrameCount;
    const std::string m_latencyString;
    const int64_t m_delta;
    float m_fPreviousCallbackTime;
    std::unique_ptr<float[]> m_pfIntervalsInMiliSecondsBackBuffer;
    std::unique_ptr<float[]> m_pfCallbackIntervalsInMiliSecondsBackBuffer;
    float*  m_pfIntervalsInMiliSecondsCurrentBuffer;
    float*  m_pfCallbackIntervalsInMiliSecondsCurrentBuffer;
    int const m_iCycle;
    std::unique_ptr<uint64_t []> m_meanTimestampFrameCount;
    std::unique_ptr<uint64_t []> m_meanCallbackFrameCount;
    const int m_iMaxLatency;


    std::vector<float> m_timestampStdDevs;
    std::vector<float> m_callbackStdDevs;

    std::vector<float> m_timestampMeans;
    std::vector<float> m_callbackMeans;
    std::thread 	   m_calculatetionThread;

    static float mean(const float* pfIntervalsInMiliSeconds, const int iCount, uint64_t *meanFrameCounts, const int iMaxLatency)
    {
        float fSumInMillis = 0;
        for (int i = 0; i < iCount; ++i)
        {
            fSumInMillis += pfIntervalsInMiliSeconds[i];
        }
        const float fMean = fSumInMillis / iCount;

        for (int i = 0; i < iCount; ++i)
        {
            int iLatency = static_cast<int>(pfIntervalsInMiliSeconds[i] - fMean + 0.5f);
            const int iIndex = (iLatency > iMaxLatency) ? (2 *  iMaxLatency) : (iLatency < (-iMaxLatency))? 0: (iLatency + iMaxLatency);
            meanFrameCounts[iIndex]++;
        }
        return fMean;
    }

    static float std_deviation(const float* pfIntervalsInMiliSeconds, const int iCount, const float fMean)
    {
        double std_dev = 0.0;
        for (int i = 0; i < iCount; ++i)
        {
            std_dev += (fMean - pfIntervalsInMiliSeconds[i]) * (fMean - pfIntervalsInMiliSeconds[i]);
        }
        if(iCount)
        {
            std_dev /= iCount;
        }
        return static_cast<float>(sqrt(std_dev));
    }

    static float average(const std::vector<float>& vals, const float val)
    {
        double sum = 0.0F;
        for(std::vector<float>::const_iterator itr = vals.begin(); itr != vals.end(); ++itr)
        {
            sum += (*itr);
        }
        int size  = static_cast<int>(vals.size());
        if(val != 0)
        {
            ++size;
            sum += val;
        }
        if (size!= 0)
            sum = sum/size;
        return sum;
    }

    static void calculate_stat(const float *pfIntervalsInMiliSeconds, const int iCount, float &fMean, float &fStdDev, uint64_t *fMeanFrameCounts, const int iMaxLatency)
    {
        fMean   = mean(pfIntervalsInMiliSeconds, iCount, fMeanFrameCounts, iMaxLatency);
        fStdDev = std_deviation(pfIntervalsInMiliSeconds, iCount, fMean);
    }

    void calculate_stats(float &fMean, float &fStdDev, float &fLatencyMean, float &fLatencyStdDev)
    {
        calculate_stat(m_pfIntervalsInMiliSecondsCurrentBuffer, m_iCount, fMean, fStdDev, m_meanTimestampFrameCount.get(), m_iMaxLatency);
        calculate_stat(m_pfCallbackIntervalsInMiliSecondsCurrentBuffer, m_iCount, fLatencyMean, fLatencyStdDev, m_meanCallbackFrameCount.get(), m_iMaxLatency);

        fMean = average(m_timestampMeans, fMean);
        fStdDev = average(m_timestampStdDevs, fStdDev);
        fLatencyMean = average(m_callbackMeans, fLatencyMean);
        fLatencyStdDev = average(m_callbackStdDevs, fLatencyStdDev);
    }

    void print_stat(const std::string& type, const float fMean, const float fStdDev)
    {
        std::cout << type << ":" << "\tmean = " <<  fMean << "ms\t std dev = " << fStdDev << "+-ms" << "\n";
    }

    template <class T>
    void print_stat(const std::string& type, T *values, const int iSize)
    {
        std::cout << type << "[time, frame count]"<< std::endl;
        int i = 0;
        for(; i < iSize; ++i)
            if(values[i])
                break;
        int endLimit = iSize - 1;
        for(; endLimit > i; --endLimit)
            if(values[endLimit])
                break;

        for(; i <= endLimit; ++i)
        {
            std::cout << "[" << i-m_iMaxLatency <<", " << values[i] << "];\t";
        }
        std::cout << std::endl;
    }

    void print_stats()
    {
        std::cout << "---------------------------------------------------------------------------------------------\n";
        std::cout << m_collectionType << ": samples collected = " << m_iCount + (m_iSize * m_timestampMeans.size()) <<
                  "\t dropped frames = " <<  m_droppedFrames	<<
                  "\tjunk time stamp frames = " <<  m_junkTimestampFrameCount << "\n\t\t";

        float fMean = 0.0f, fStdDev = 0.0f, fLatencyMean = 0.0f, fLatencyStdDev = 0.0f;
        calculate_stats(fMean, fStdDev, fLatencyMean, fLatencyStdDev);

        print_stat("time stamp", fMean, fStdDev);
        std::cout << "\t\t";
        print_stat(m_latencyString, fLatencyMean, fLatencyStdDev);
        std::cout << "\t\t";
        print_stat("time stamp array", m_meanTimestampFrameCount.get(), (m_iMaxLatency * 2) + 1);
        std::cout << "\t\t";
        print_stat("callback array", m_meanCallbackFrameCount.get(), (m_iMaxLatency * 2) + 1);
        std::cout << "---------------------------------------------------------------------------------------------\n";
    }

    void calculate_live_thread(float* pfIntervalsInMiliSecondsBuffer, float* pfCallbackIntervalsInMiliSecondsBuffer, int count)
    {
        float fMean = 0.0f, fStdDev = 0.0f, fLatencyMean = 0.0f, fLatencyStdDev = 0.0f;

        calculate_stat(pfIntervalsInMiliSecondsBuffer, count, fMean, fStdDev, m_meanTimestampFrameCount.get(),m_iMaxLatency);
        calculate_stat(pfCallbackIntervalsInMiliSecondsBuffer, count, fLatencyMean, fLatencyStdDev, m_meanCallbackFrameCount.get(), m_iMaxLatency);
        m_timestampStdDevs.push_back(fStdDev);
        m_callbackStdDevs.push_back(fLatencyStdDev);

        m_timestampMeans.push_back(fMean);
        m_callbackMeans.push_back(fLatencyMean);
    }

    void Switch()
    {
        float * pfIntervalsInMiliSecondsPreviousBuffer = m_pfIntervalsInMiliSecondsCurrentBuffer;
        float * pfCallbackIntervalsInMiliSecondspreviousBuffer = m_pfCallbackIntervalsInMiliSecondsCurrentBuffer;

        if(m_pfIntervalsInMiliSecondsCurrentBuffer == m_pfIntervalsInMiliSecondsFrontBuffer.get())
        {
            m_pfIntervalsInMiliSecondsCurrentBuffer = m_pfIntervalsInMiliSecondsBackBuffer.get();
            m_pfCallbackIntervalsInMiliSecondsCurrentBuffer = m_pfCallbackIntervalsInMiliSecondsBackBuffer.get();
        }
        else
        {
            m_pfIntervalsInMiliSecondsCurrentBuffer = m_pfIntervalsInMiliSecondsFrontBuffer.get();
            m_pfCallbackIntervalsInMiliSecondsCurrentBuffer = m_pfCallbackIntervalsInMiliSecondsFrontBuffer.get();
        }

        if(m_calculatetionThread.joinable())
            m_calculatetionThread.join();
        m_calculatetionThread = std::thread(&float_stats::calculate_live_thread, this, pfIntervalsInMiliSecondsPreviousBuffer, pfCallbackIntervalsInMiliSecondspreviousBuffer, m_iCount);
        m_iCount = 0;
    }

public:
    float_stats(float_stats&) = delete;
    float_stats& operator=(float_stats&) = delete;
    float_stats(const std::string collectionType, const int iMaxExpectedLatency, int iSize = 20000) : m_pfIntervalsInMiliSecondsFrontBuffer(new float[iSize]),
        m_pfCallbackIntervalsInMiliSecondsFrontBuffer(new float[iSize]),
        m_iSize(iSize), m_iCount(0), m_fPreviousTimestamp(0.0f), m_collectionType(collectionType),
        m_frameCounter(-1), m_droppedFrames(0), m_junkTimestampFrameCount(0),  m_latencyString("callback latency"),
        m_delta(GetCurrentTimeInMiliSecods()), m_fPreviousCallbackTime(0.0f),
        m_pfIntervalsInMiliSecondsBackBuffer(new float[iSize]),
        m_pfCallbackIntervalsInMiliSecondsBackBuffer(new float[iSize]),
        m_pfIntervalsInMiliSecondsCurrentBuffer(nullptr),
        m_pfCallbackIntervalsInMiliSecondsCurrentBuffer(nullptr),
        m_iCycle(4096),
        m_meanTimestampFrameCount(new uint64_t[(2 * iMaxExpectedLatency) + 1]),
        m_meanCallbackFrameCount(new uint64_t[(2 * iMaxExpectedLatency) + 1]),
        m_iMaxLatency(iMaxExpectedLatency)

    {
        m_pfIntervalsInMiliSecondsCurrentBuffer = m_pfIntervalsInMiliSecondsFrontBuffer.get();
        m_pfCallbackIntervalsInMiliSecondsCurrentBuffer = m_pfCallbackIntervalsInMiliSecondsFrontBuffer.get();
        memset(m_meanTimestampFrameCount.get(), 0, sizeof(uint64_t) * (2 * iMaxExpectedLatency + 1));
        memset(m_meanCallbackFrameCount.get(), 0,  sizeof(uint64_t) * (2 * iMaxExpectedLatency + 1));
    }

    ~float_stats()
    {
        if(m_calculatetionThread.joinable())
            m_calculatetionThread.join();
        print_stats();
    }

    void add_frame_with_junk_time_stamp()
    {
        ++m_junkTimestampFrameCount;
    }

    void add_sample(const float fTimestampInMilisecods, const int iFrameCounter)
    {
        const float callbackTime = static_cast<float>(GetCurrentTimeInMiliSecods() - m_delta);
        if(m_iCount == m_iSize)
            Switch();
        if (/*m_iCount < m_iSize && */m_frameCounter != -1)
        {
            const int expectedFrameCounter = (m_frameCounter + 1) % m_iCycle;
            const int iFrameGap = (iFrameCounter % m_iCycle) - expectedFrameCounter + 1;
            m_droppedFrames = m_droppedFrames + (iFrameGap - 1);

            m_pfIntervalsInMiliSecondsCurrentBuffer[m_iCount] = (fTimestampInMilisecods - m_fPreviousTimestamp) / iFrameGap;
            m_pfCallbackIntervalsInMiliSecondsCurrentBuffer[m_iCount] = (callbackTime - m_fPreviousCallbackTime) / iFrameGap;
            ++m_iCount;
        }
        m_fPreviousTimestamp = fTimestampInMilisecods;
        m_frameCounter = (iFrameCounter % m_iCycle);
        m_fPreviousCallbackTime = callbackTime;
    }

    void add_samples(const float fMinTimestampInMilisecods, const float fMaxTimestampInMilisecods, const int count)
    {
        if(count > 0)
        {
            const float callbackTime = static_cast<float>(GetCurrentTimeInMiliSecods() - m_delta);
            if(m_iCount + count >= m_iSize)
                Switch();
            if (m_frameCounter != -1)
            {
                //callbackTime is for MaxTimestamp
                float increament = (fMaxTimestampInMilisecods - fMinTimestampInMilisecods) / count;
                float currentTimestamp = fMinTimestampInMilisecods;
                float callbackIncreament = (callbackTime - m_fPreviousCallbackTime) / count;
                float currentcallbackTime = callbackTime - ((count - 1)* callbackIncreament) ;
                for(int i = 0; i < count; ++i)
                {
                    m_pfIntervalsInMiliSecondsCurrentBuffer[m_iCount] = (currentTimestamp - m_fPreviousTimestamp);
                    m_pfCallbackIntervalsInMiliSecondsCurrentBuffer[m_iCount] = (currentcallbackTime - m_fPreviousCallbackTime);
                    m_fPreviousTimestamp = currentTimestamp;
                    m_fPreviousCallbackTime = currentcallbackTime ;
                    ++m_iCount;
                    currentTimestamp+=increament;
                    currentcallbackTime+=callbackIncreament;
                }
            }
            ++m_frameCounter;
            m_fPreviousTimestamp = fMaxTimestampInMilisecods;
            m_fPreviousCallbackTime = callbackTime;
        }
    }
};

template<int MAX_LIMIT = 30>
class fps_counter
{
public:
    int m_frameCount;
    int64_t m_previousTime;
    float m_fCurrentFPS;
    std::string m_type;
    int64_t m_timeSumInterval;
    uint64_t m_frameCounter;

    fps_counter(const std::string& type): m_frameCount(0), m_previousTime(0), m_fCurrentFPS(0.0f), m_type (type), m_timeSumInterval(0), m_frameCounter(0)
    {

    }

    void add_sample()
    {
        add_samples(1);
    }

    void add_samples(const int count)
    {
        if(count > 0)
        {
            m_frameCounter += count;
            const auto currrenTime = GetCurrentTimeInMiliSecods();
            if(m_previousTime == 0)
            {
                m_previousTime = currrenTime;
                return;
            }

            m_timeSumInterval += (currrenTime - m_previousTime);
            m_previousTime = currrenTime;
            m_frameCount += count;

            if(m_frameCount >= MAX_LIMIT)
            {
                m_fCurrentFPS = (m_frameCount * 1000.0f) /m_timeSumInterval;
                m_timeSumInterval = 0;
                m_frameCount = 0;
            }
        }
    }

    inline float get_fps() const
    {
        return m_fCurrentFPS;
    }

    uint64_t get_frame_count() const
    {
        return m_frameCounter;
    }

    void print_fps()
    {
        std::cout << m_type << " fps:"<< get_fps() << std::endl;
    }

    ~fps_counter()
    {
    }
};


class stream_stats
{
private:
    fps_counter<> m_depthFPS;
    fps_counter<> m_fisheyeFPS;

    fps_counter<125> m_acclerometerFPS;
    fps_counter<100> m_gyroscopeFPS;

public:
    stream_stats(): m_depthFPS("depth"), m_fisheyeFPS("fisheye"), m_acclerometerFPS("accelerometer"), m_gyroscopeFPS("gyroscope")
    {
    }

    void add_depth_samples(int count)
    {
        add_samples(m_depthFPS, count);
    }

    void add_fisheye_samples(int count)
    {
        add_samples(m_fisheyeFPS, count);
    }

    void add_acceleromter_samples(int count)
    {
        add_samples(m_acclerometerFPS, count);
    }
    void add_gyroscope_samples(int count)
    {
        return add_samples(m_gyroscopeFPS, count);
    }

    float get_gyroscope_fps() const
    {
        return get_fps(m_gyroscopeFPS);
    }

    float get_depth_fps() const
    {
        return get_fps(m_depthFPS);
    }

    float get_fisheye_fps() const
    {
        return get_fps(m_fisheyeFPS);
    }

    float get_acceleromter_fps() const
    {
        return get_fps(m_acclerometerFPS);
    }

    uint64_t get_fisheye_frame_count() const
    {
        return get_frame_count(m_fisheyeFPS);
    }

    uint64_t  get_depth_frame_count() const
    {
        return get_frame_count(m_depthFPS);
    }

    uint64_t get_gyroscope_frame_count() const
    {
        return get_frame_count(m_gyroscopeFPS);
    }

    uint64_t  get_acceleromter_frame_count() const
    {
        return get_frame_count(m_acclerometerFPS);
    }

    ~stream_stats()
    {
    }

private:
    template <int N>
    static void add_samples(fps_counter<N> &fpsCounter, int count)
    {
        fpsCounter.add_samples(count);
    }

    template <int N>
    static float get_fps(const fps_counter<N> &fpsCounter)
    {
        return fpsCounter.get_fps();
    }

    template <int N>
    static uint64_t get_frame_count(const fps_counter<N> &fpsCounter)
    {
        return fpsCounter.get_frame_count();
    }
};

inline void print_stream_fps_stats(const std::string& type, const stream_stats& streamStats)
{
    std::cout << type << " fps (fisheye, depth, accelerometer, gyroscope)" << "("<<
              streamStats.get_fisheye_fps() << ", " <<
              streamStats.get_depth_fps() << ", " <<
              streamStats.get_acceleromter_fps() << ", " <<
              streamStats.get_gyroscope_fps() <<
              ")" << std::endl;
}

inline void print_stream_count_stats(const std::string& type, const stream_stats& streamStats)
{
    std::cout << type << " frame count (fisheye, depth, accelerometer, gyroscope)" << "("<<
              streamStats.get_fisheye_frame_count() << ", " <<
              streamStats.get_depth_frame_count() << ", " <<
              streamStats.get_acceleromter_frame_count() << ", " <<
              streamStats.get_gyroscope_frame_count() <<
              ")" << std::endl;
}
