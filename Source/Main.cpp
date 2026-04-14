#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define DR_WAV_IMPLEMENTATION
#include "third_party/dr_wav.h"

#include "HallReverb.h"

namespace
{
using ParameterSetter = std::function<void(HallReverb&, float)>;

struct CommandLineOptions
{
    float frameDurationMs = 40.0f;
};

void printUsage(const char* programName)
{
    std::cout
        << "Usage:\n"
        << "  " << programName << " <input.wav> <output.wav> [options]\n\n"
        << "Options:\n"
        << "  --dry <value>\n"
        << "  --early-level <value>\n"
        << "  --early-send <value>\n"
        << "  --late-level <value>\n"
        << "  --early-output-hpf <value>\n"
        << "  --early-output-lpf <value>\n"
        << "  --early-room-size <value>\n"
        << "  --early-stereo-width <value>\n"
        << "  --late-ap-feedback <value>\n"
        << "  --late-crossover-high <value>\n"
        << "  --late-crossover-low <value>\n"
        << "  --late-decay <value>\n"
        << "  --late-decay-factor-high <value>\n"
        << "  --late-decay-factor-low <value>\n"
        << "  --late-diffusion <value>\n"
        << "  --late-lfo1-freq <value>\n"
        << "  --late-lfo2-freq <value>\n"
        << "  --late-lfo-factor <value>\n"
        << "  --late-output-hpf <value>\n"
        << "  --late-output-lpf <value>\n"
        << "  --late-predelay <value>\n"
        << "  --late-room-size <value>\n"
        << "  --late-spin <value>\n"
        << "  --late-spin-factor <value>\n"
        << "  --late-stereo-width <value>\n"
        << "  --late-wander <value>\n"
        << "  --frame-ms <value>\n\n"
        << "Example:\n"
        << "  " << programName
        << " input.wav output.wav --dry 0.8 --late-level 0.25 --late-decay 0.6 --frame-ms 40\n";
}

float parseFloatArgument(const std::string& optionName, const std::string& valueText)
{
    size_t parsedLength = 0;
    const float value = std::stof(valueText, &parsedLength);
    if (parsedLength != valueText.size())
    {
        throw std::runtime_error("Invalid numeric value for " + optionName + ": " + valueText);
    }

    return value;
}

const std::unordered_map<std::string, ParameterSetter>& getParameterSetters()
{
    static const std::unordered_map<std::string, ParameterSetter> setters{
        {"--dry", [](HallReverb& reverb, float value) { reverb.setDryLevel(value); }},
        {"--early-level", [](HallReverb& reverb, float value) { reverb.setEarlyLevel(value); }},
        {"--early-send", [](HallReverb& reverb, float value) { reverb.setEarlySendLevel(value); }},
        {"--late-level", [](HallReverb& reverb, float value) { reverb.setLateLevel(value); }},
        {"--early-output-hpf", [](HallReverb& reverb, float value) { reverb.setEarlyOutputHPF(value); }},
        {"--early-output-lpf", [](HallReverb& reverb, float value) { reverb.setEarlyOutputLPF(value); }},
        {"--early-room-size", [](HallReverb& reverb, float value) { reverb.setEarlyRoomSize(value); }},
        {"--early-stereo-width", [](HallReverb& reverb, float value) { reverb.setEarlyStereoWidth(value); }},
        {"--late-ap-feedback", [](HallReverb& reverb, float value) { reverb.setLateApFeedback(value); }},
        {"--late-crossover-high", [](HallReverb& reverb, float value) { reverb.setLateCrossOverFreqHigh(value); }},
        {"--late-crossover-low", [](HallReverb& reverb, float value) { reverb.setLateCrossOverFreqLow(value); }},
        {"--late-decay", [](HallReverb& reverb, float value) { reverb.setLateDecay(value); }},
        {"--late-decay-factor-high", [](HallReverb& reverb, float value) { reverb.setLateDecayFactorHigh(value); }},
        {"--late-decay-factor-low", [](HallReverb& reverb, float value) { reverb.setLateDecayFactorLow(value); }},
        {"--late-diffusion", [](HallReverb& reverb, float value) { reverb.setLateDiffusion(value); }},
        {"--late-lfo1-freq", [](HallReverb& reverb, float value) { reverb.setLateLFO1Freq(value); }},
        {"--late-lfo2-freq", [](HallReverb& reverb, float value) { reverb.setLateLFO2Freq(value); }},
        {"--late-lfo-factor", [](HallReverb& reverb, float value) { reverb.setLateLFOFactor(value); }},
        {"--late-output-hpf", [](HallReverb& reverb, float value) { reverb.setLateOutputHPF(value); }},
        {"--late-output-lpf", [](HallReverb& reverb, float value) { reverb.setLateOutputLPF(value); }},
        {"--late-predelay", [](HallReverb& reverb, float value) { reverb.setLatePredelay(value); }},
        {"--late-room-size", [](HallReverb& reverb, float value) { reverb.setLateRoomSize(value); }},
        {"--late-spin", [](HallReverb& reverb, float value) { reverb.setLateSpin(value); }},
        {"--late-spin-factor", [](HallReverb& reverb, float value) { reverb.setLateSpinFactor(value); }},
        {"--late-stereo-width", [](HallReverb& reverb, float value) { reverb.setLateStereoWidth(value); }},
        {"--late-wander", [](HallReverb& reverb, float value) { reverb.setLateWander(value); }},
    };

    return setters;
}

CommandLineOptions applyCommandLineParameters(HallReverb& reverb, int argc, char* argv[])
{
    const auto& setters = getParameterSetters();
    CommandLineOptions options;

    for (int argumentIndex = 3; argumentIndex < argc; ++argumentIndex)
    {
        const std::string optionName = argv[argumentIndex];
        if (argumentIndex + 1 >= argc)
        {
            throw std::runtime_error("Missing value for option: " + optionName);
        }

        const float value = parseFloatArgument(optionName, argv[++argumentIndex]);

        if (optionName == "--frame-ms")
        {
            if (value <= 0.0f)
            {
                throw std::runtime_error("Frame size must be greater than 0 ms.");
            }
            options.frameDurationMs = value;
            continue;
        }

        const auto setterIt = setters.find(optionName);
        if (setterIt == setters.end())
        {
            throw std::runtime_error("Unknown option: " + optionName);
        }

        setterIt->second(reverb, value);
    }

    return options;
}
}

int main(int argc, char* argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help")
    {
        printUsage(argv[0]);
        return 0;
    }

    if (argc < 3)
    {
        printUsage(argv[0]);
        return 1;
    }

    try
    {
        unsigned int channels = 0;
        unsigned int sampleRate = 0;
        drwav_uint64 totalFrameCount = 0;

        float* sampleData = drwav_open_file_and_read_pcm_frames_f32(
            argv[1], &channels, &sampleRate, &totalFrameCount, nullptr);
        if (sampleData == nullptr)
        {
            throw std::runtime_error("Failed to open or decode the input WAV file.");
        }

        if (channels != 1 && channels != 2)
        {
            drwav_free(sampleData, nullptr);
            throw std::runtime_error("Only mono or stereo WAV files are supported.");
        }

        std::vector<float> leftIn(static_cast<size_t>(totalFrameCount));
        std::vector<float> rightIn(static_cast<size_t>(totalFrameCount));

        for (drwav_uint64 frameIndex = 0; frameIndex < totalFrameCount; ++frameIndex)
        {
            const drwav_uint64 baseIndex = frameIndex * channels;
            leftIn[static_cast<size_t>(frameIndex)] = sampleData[baseIndex];
            rightIn[static_cast<size_t>(frameIndex)] =
                (channels == 1) ? sampleData[baseIndex] : sampleData[baseIndex + 1];
        }

        drwav_free(sampleData, nullptr);

        HallReverb reverb;
        reverb.setSampleRate(static_cast<float>(sampleRate));
        const CommandLineOptions options = applyCommandLineParameters(reverb, argc, argv);

        std::vector<float> leftOut(static_cast<size_t>(totalFrameCount));
        std::vector<float> rightOut(static_cast<size_t>(totalFrameCount));

        const int frameSizeInSamples = std::max(
            1,
            static_cast<int>((options.frameDurationMs * static_cast<float>(sampleRate)) / 1000.0f));

        for (drwav_uint64 frameOffset = 0; frameOffset < totalFrameCount; frameOffset += static_cast<drwav_uint64>(frameSizeInSamples))
        {
            const drwav_uint64 framesRemaining = totalFrameCount - frameOffset;
            const int framesThisBlock = static_cast<int>(
                std::min<drwav_uint64>(static_cast<drwav_uint64>(frameSizeInSamples), framesRemaining));

            reverb.process(
                leftIn.data() + frameOffset,
                rightIn.data() + frameOffset,
                leftOut.data() + frameOffset,
                rightOut.data() + frameOffset,
                framesThisBlock);
        }

        std::vector<float> interleavedOutput(static_cast<size_t>(totalFrameCount) * 2);
        for (drwav_uint64 frameIndex = 0; frameIndex < totalFrameCount; ++frameIndex)
        {
            const size_t outputIndex = static_cast<size_t>(frameIndex) * 2;
            interleavedOutput[outputIndex] = leftOut[static_cast<size_t>(frameIndex)];
            interleavedOutput[outputIndex + 1] = rightOut[static_cast<size_t>(frameIndex)];
        }

        drwav_data_format outputFormat{};
        outputFormat.container = drwav_container_riff;
        outputFormat.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        outputFormat.channels = 2;
        outputFormat.sampleRate = sampleRate;
        outputFormat.bitsPerSample = 32;

        drwav* wavOut = drwav_open_file_write(argv[2], &outputFormat, nullptr);
        if (wavOut == nullptr)
        {
            throw std::runtime_error("Failed to open the output WAV file for writing.");
        }

        const drwav_uint64 writtenFrames =
            drwav_write_pcm_frames(wavOut, totalFrameCount, interleavedOutput.data());
        drwav_close(wavOut);

        if (writtenFrames != totalFrameCount)
        {
            throw std::runtime_error("Failed to write all output frames.");
        }

        std::cout << "Processed " << totalFrameCount << " frames at "
                  << sampleRate << " Hz using " << frameSizeInSamples
                  << " samples per streaming block (" << options.frameDurationMs
                  << " ms) and wrote " << argv[2] << '\n';
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}
