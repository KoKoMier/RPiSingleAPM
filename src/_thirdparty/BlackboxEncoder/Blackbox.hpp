#pragma once

#include <iostream>

#include <queue>
#include <vector>
#define BlackboxHeader "H Product:Blackbox flight data recorder by Nicholas Sherlock\n"
#define BlackboxVerion "H Data version: 2\n";
//
#define BlackboxIInt "H I interval:";
#define BlackboxPInt "H P interval:";
//
#define BlackboxIName "H Field I name:"
#define BlackboxISigned "H Field I signed:"
#define BlackboxIPredictor "H Field I predictor:"
#define BlackboxIEncoding "H Field I encoding:"
#define BlackboxPName "H Field P name:"
#define BlackboxPSigned "H Field P signed:"
#define BlackboxPPredictor "H Field P predictor:"
#define BlackboxPEncoding "H Field P encoding:"
//
#define BlackboxFirmwareType "H Firmware type:"
//
struct BlackboxList
{
    const char *FrameName;
    unsigned int FrameSigned;
    unsigned int FramePredictor;
    unsigned int FrameEncoder;
};

struct BlackboxHeaderInfo
{
    int IInterval;
    const char *PInterval;

    std::string FirmwareType;
    std::vector<BlackboxList> BlackBoxDataIInfo;
    std::vector<BlackboxList> BlackBoxDataPInfo;
    std::vector<std::string> BlackBoxCustom;
};

class BlackboxEncoder
{
public:
    std::string FullBlackboxHeader;
    inline BlackboxEncoder(BlackboxHeaderInfo);
    inline std::vector<uint8_t> BlackboxPIPush(std::vector<int> data);

private:
    int PINTNUM = 1;
    int PINTDEM = 1;
    std::vector<int> PredictDataLast;
    BlackboxHeaderInfo HeaderInfo;

    inline static std::vector<uint8_t> dataEncode(int value, int type)
    {
        switch (type)
        {
        case 0:
            return unsignedBtyeEncode(zigzagEncode(value));
            break;

        case 1:
            return unsignedBtyeEncode(value);
        default:
            return {};
            break;
        }
    }

    inline std::vector<int> dataPredict(std::vector<int> data, std::vector<BlackboxList> type)
    {
        if (data.size() == type.size())
        {
            for (size_t i = 0; i < data.size(); i++)
            {
                switch (type[i].FramePredictor)
                {
                case 0:
                    data[i] = data[i];
                    break;

                case 1:
                {
                    if (PredictDataLast.size() == data.size())
                    {
                        int tmpdata = data[i];
                        data[i] = data[i] - PredictDataLast[i];
                        PredictDataLast[i] = tmpdata;
                    }
                }
                default:
                    break;
                }
                //==//
                if (PredictDataLast.size() != data.size())
                {
                    PredictDataLast.push_back(data[i]);
                }
            }
        }

        return data;
    }

    inline static unsigned int zigzagEncode(int value)
    {
        return (value << 1) ^ (value >> 31);
    }

    inline static std::vector<uint8_t> unsignedBtyeEncode(int value)
    {
        std::vector<uint8_t> data;
        while (value > 127)
        {
            data.push_back(((uint8_t)(value | 0x80)));
            value >>= 7;
        }
        data.push_back(value);
        return data;
    }
};

BlackboxEncoder::BlackboxEncoder(BlackboxHeaderInfo Info)
{
    FullBlackboxHeader = "";
    FullBlackboxHeader += BlackboxHeader;
    FullBlackboxHeader += BlackboxVerion;
    FullBlackboxHeader += BlackboxIInt;
    FullBlackboxHeader += std::to_string(Info.IInterval);
    FullBlackboxHeader += "\n";
    FullBlackboxHeader += BlackboxPInt;
    FullBlackboxHeader += Info.PInterval;
    FullBlackboxHeader += "\n";

    std::string PINTER = Info.PInterval;
    std::string PINTERNUM = PINTER.substr(0, PINTER.find('/'));
    std::string PINTERDEM = PINTER.substr(PINTERNUM.size() + 1, PINTER.find('\0'));
    PINTNUM = std::atoi(PINTERNUM.c_str());
    PINTDEM = std::atoi(PINTERDEM.c_str());

    FullBlackboxHeader += BlackboxIName;
    for (auto sts : Info.BlackBoxDataIInfo)
    {
        FullBlackboxHeader += sts.FrameName;
        FullBlackboxHeader += ",";
    }
    FullBlackboxHeader += "\n";

    FullBlackboxHeader += BlackboxISigned;
    for (auto sts : Info.BlackBoxDataIInfo)
    {
        FullBlackboxHeader += std::to_string(sts.FrameSigned);
        FullBlackboxHeader += ",";
    }
    FullBlackboxHeader += "\n";

    FullBlackboxHeader += BlackboxIPredictor;
    for (auto sts : Info.BlackBoxDataIInfo)
    {
        FullBlackboxHeader += std::to_string(sts.FramePredictor);
        FullBlackboxHeader += ",";
    }
    FullBlackboxHeader += "\n";

    FullBlackboxHeader += BlackboxIEncoding;
    for (auto sts : Info.BlackBoxDataIInfo)
    {
        FullBlackboxHeader += std::to_string(sts.FrameEncoder);
        FullBlackboxHeader += ",";
    }
    FullBlackboxHeader += "\n";
    //=======================P FRAME INFO=============================//
    if (Info.BlackBoxDataPInfo.size() > 0)
    {
        FullBlackboxHeader += BlackboxPName;
        for (auto sts : Info.BlackBoxDataPInfo)
        {
            FullBlackboxHeader += sts.FrameName;
            FullBlackboxHeader += ",";
        }
        FullBlackboxHeader += "\n";

        FullBlackboxHeader += BlackboxPSigned;
        for (auto sts : Info.BlackBoxDataPInfo)
        {
            FullBlackboxHeader += std::to_string(sts.FrameSigned);
            FullBlackboxHeader += ",";
        }
        FullBlackboxHeader += "\n";

        FullBlackboxHeader += BlackboxPPredictor;
        for (auto sts : Info.BlackBoxDataPInfo)
        {
            FullBlackboxHeader += std::to_string(sts.FramePredictor);
            FullBlackboxHeader += ",";
        }
        FullBlackboxHeader += "\n";

        FullBlackboxHeader += BlackboxPEncoding;
        for (auto sts : Info.BlackBoxDataPInfo)
        {
            FullBlackboxHeader += std::to_string(sts.FrameEncoder);
            FullBlackboxHeader += ",";
        }
        FullBlackboxHeader += "\n";
    }

    FullBlackboxHeader += BlackboxFirmwareType;
    FullBlackboxHeader += Info.FirmwareType;
    FullBlackboxHeader += "\n";

    HeaderInfo = Info;

    std::cout << FullBlackboxHeader << "\n";
};

std::vector<uint8_t> BlackboxEncoder::BlackboxPIPush(std::vector<int> data)
{
    std::vector<uint8_t> FrameBuffer;
    //
    char Head = 'I';
    if (data[0] % HeaderInfo.IInterval == 0)
        Head = 'I';
    else if ((data[0] % HeaderInfo.IInterval + PINTNUM - 1) % PINTDEM < PINTNUM)
        Head = 'P';
    else
        return {}; // No log free, size 0;

    FrameBuffer.push_back((uint8_t)Head);
    //
    if (Head == 'I' && HeaderInfo.BlackBoxDataIInfo.size() > 0)
    {
        std::vector<int> predicted = dataPredict(data, HeaderInfo.BlackBoxDataIInfo);
        for (size_t i = 0; i < data.size(); i++)
        {
            std::vector<uint8_t> tmpBuffer = dataEncode(predicted[i], HeaderInfo.BlackBoxDataIInfo[i].FrameEncoder);
            FrameBuffer.insert(FrameBuffer.end(), tmpBuffer.begin(), tmpBuffer.end());
        }
    }
    else if (Head == 'P' && HeaderInfo.BlackBoxDataPInfo.size() > 0)
    {
        std::vector<int> predicted = dataPredict(data, HeaderInfo.BlackBoxDataPInfo);
        for (size_t i = 0; i < data.size(); i++)
        {
            std::vector<uint8_t> tmpBuffer = dataEncode(predicted[i], HeaderInfo.BlackBoxDataPInfo[i].FrameEncoder);
            FrameBuffer.insert(FrameBuffer.end(), tmpBuffer.begin(), tmpBuffer.end());
        }
    }
    else
    {
        return {};
    }

    return FrameBuffer;
}