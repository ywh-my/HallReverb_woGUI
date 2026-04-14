#ifndef MINIMAL_DR_WAV_H
#define MINIMAL_DR_WAV_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

typedef uint64_t drwav_uint64;

typedef enum
{
    drwav_container_riff = 0
} drwav_container;

enum
{
    DR_WAVE_FORMAT_PCM = 0x0001,
    DR_WAVE_FORMAT_IEEE_FLOAT = 0x0003
};

typedef struct
{
    drwav_container container;
    unsigned short format;
    unsigned short channels;
    unsigned int sampleRate;
    unsigned short bitsPerSample;
} drwav_data_format;

typedef struct
{
    FILE* file;
    drwav_data_format format;
    drwav_uint64 dataChunkDataSizeInBytes;
} drwav;

float* drwav_open_file_and_read_pcm_frames_f32(
    const char* filename,
    unsigned int* channelsOut,
    unsigned int* sampleRateOut,
    drwav_uint64* totalFrameCountOut,
    const void* allocationCallbacks);

drwav* drwav_open_file_write(
    const char* filename,
    const drwav_data_format* format,
    const void* allocationCallbacks);

drwav_uint64 drwav_write_pcm_frames(drwav* wav, drwav_uint64 frameCount, const void* data);
void drwav_close(drwav* wav);
void drwav_free(void* memory, const void* allocationCallbacks);

#ifdef DR_WAV_IMPLEMENTATION

namespace drwav_detail
{
static inline uint16_t read_u16(FILE* file)
{
    unsigned char bytes[2];
    if (fread(bytes, 1, 2, file) != 2) return 0;
    return static_cast<uint16_t>(bytes[0] | (bytes[1] << 8));
}

static inline uint32_t read_u32(FILE* file)
{
    unsigned char bytes[4];
    if (fread(bytes, 1, 4, file) != 4) return 0;
    return static_cast<uint32_t>(bytes[0]
        | (bytes[1] << 8)
        | (bytes[2] << 16)
        | (bytes[3] << 24));
}

static inline void write_u16(FILE* file, uint16_t value)
{
    const unsigned char bytes[2] = {
        static_cast<unsigned char>(value & 0xFF),
        static_cast<unsigned char>((value >> 8) & 0xFF)};
    fwrite(bytes, 1, 2, file);
}

static inline void write_u32(FILE* file, uint32_t value)
{
    const unsigned char bytes[4] = {
        static_cast<unsigned char>(value & 0xFF),
        static_cast<unsigned char>((value >> 8) & 0xFF),
        static_cast<unsigned char>((value >> 16) & 0xFF),
        static_cast<unsigned char>((value >> 24) & 0xFF)};
    fwrite(bytes, 1, 4, file);
}

static inline float pcm16_to_f32(int16_t sample)
{
    return static_cast<float>(sample) / 32768.0f;
}

static inline float pcm24_to_f32(const unsigned char* bytes)
{
    int32_t sample = (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
    if ((sample & 0x00800000) != 0)
    {
        sample |= ~0x00FFFFFF;
    }
    return static_cast<float>(sample) / 8388608.0f;
}

static inline float pcm32_to_f32(int32_t sample)
{
    return static_cast<float>(sample) / 2147483648.0f;
}
}

inline float* drwav_open_file_and_read_pcm_frames_f32(
    const char* filename,
    unsigned int* channelsOut,
    unsigned int* sampleRateOut,
    drwav_uint64* totalFrameCountOut,
    const void*)
{
    FILE* file = std::fopen(filename, "rb");
    if (file == nullptr)
    {
        return nullptr;
    }

    char riff[4];
    char wave[4];
    if (std::fread(riff, 1, 4, file) != 4 || std::memcmp(riff, "RIFF", 4) != 0)
    {
        std::fclose(file);
        return nullptr;
    }

    (void)drwav_detail::read_u32(file);
    if (std::fread(wave, 1, 4, file) != 4 || std::memcmp(wave, "WAVE", 4) != 0)
    {
        std::fclose(file);
        return nullptr;
    }

    uint16_t audioFormat = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    uint32_t dataSize = 0;
    long dataOffset = 0;

    while (!std::feof(file))
    {
        char chunkId[4];
        if (std::fread(chunkId, 1, 4, file) != 4)
        {
            break;
        }

        const uint32_t chunkSize = drwav_detail::read_u32(file);

        if (std::memcmp(chunkId, "fmt ", 4) == 0)
        {
            audioFormat = drwav_detail::read_u16(file);
            channels = drwav_detail::read_u16(file);
            sampleRate = drwav_detail::read_u32(file);
            (void)drwav_detail::read_u32(file);
            (void)drwav_detail::read_u16(file);
            bitsPerSample = drwav_detail::read_u16(file);

            const long remainingBytes = static_cast<long>(chunkSize) - 16;
            if (remainingBytes > 0)
            {
                std::fseek(file, remainingBytes, SEEK_CUR);
            }
        }
        else if (std::memcmp(chunkId, "data", 4) == 0)
        {
            dataSize = chunkSize;
            dataOffset = std::ftell(file);
            std::fseek(file, chunkSize, SEEK_CUR);
        }
        else
        {
            std::fseek(file, chunkSize, SEEK_CUR);
        }

        if ((chunkSize & 1U) != 0)
        {
            std::fseek(file, 1, SEEK_CUR);
        }
    }

    if (channels == 0 || sampleRate == 0 || bitsPerSample == 0 || dataSize == 0 || dataOffset == 0)
    {
        std::fclose(file);
        return nullptr;
    }

    const uint32_t bytesPerFrame = static_cast<uint32_t>(channels) * (bitsPerSample / 8);
    if (bytesPerFrame == 0)
    {
        std::fclose(file);
        return nullptr;
    }

    const drwav_uint64 totalFrames = dataSize / bytesPerFrame;
    float* samples = static_cast<float*>(
        std::malloc(static_cast<size_t>(totalFrames * channels) * sizeof(float)));
    if (samples == nullptr)
    {
        std::fclose(file);
        return nullptr;
    }

    std::fseek(file, dataOffset, SEEK_SET);
    for (drwav_uint64 frame = 0; frame < totalFrames; ++frame)
    {
        for (uint16_t channel = 0; channel < channels; ++channel)
        {
            float sample = 0.0f;
            if (audioFormat == DR_WAVE_FORMAT_IEEE_FLOAT && bitsPerSample == 32)
            {
                if (std::fread(&sample, sizeof(float), 1, file) != 1)
                {
                    std::free(samples);
                    std::fclose(file);
                    return nullptr;
                }
            }
            else if (audioFormat == DR_WAVE_FORMAT_PCM && bitsPerSample == 16)
            {
                const int16_t pcm = static_cast<int16_t>(drwav_detail::read_u16(file));
                sample = drwav_detail::pcm16_to_f32(pcm);
            }
            else if (audioFormat == DR_WAVE_FORMAT_PCM && bitsPerSample == 24)
            {
                unsigned char bytes[3];
                if (std::fread(bytes, 1, 3, file) != 3)
                {
                    std::free(samples);
                    std::fclose(file);
                    return nullptr;
                }
                sample = drwav_detail::pcm24_to_f32(bytes);
            }
            else if (audioFormat == DR_WAVE_FORMAT_PCM && bitsPerSample == 32)
            {
                const int32_t pcm = static_cast<int32_t>(drwav_detail::read_u32(file));
                sample = drwav_detail::pcm32_to_f32(pcm);
            }
            else
            {
                std::free(samples);
                std::fclose(file);
                return nullptr;
            }

            samples[static_cast<size_t>(frame * channels + channel)] = sample;
        }
    }

    std::fclose(file);

    if (channelsOut != nullptr) *channelsOut = channels;
    if (sampleRateOut != nullptr) *sampleRateOut = sampleRate;
    if (totalFrameCountOut != nullptr) *totalFrameCountOut = totalFrames;

    return samples;
}

inline drwav* drwav_open_file_write(
    const char* filename,
    const drwav_data_format* format,
    const void*)
{
    if (format == nullptr)
    {
        return nullptr;
    }

    FILE* file = std::fopen(filename, "wb");
    if (file == nullptr)
    {
        return nullptr;
    }

    drwav* wav = static_cast<drwav*>(std::malloc(sizeof(drwav)));
    if (wav == nullptr)
    {
        std::fclose(file);
        return nullptr;
    }

    wav->file = file;
    wav->format = *format;
    wav->dataChunkDataSizeInBytes = 0;

    std::fwrite("RIFF", 1, 4, file);
    drwav_detail::write_u32(file, 0);
    std::fwrite("WAVE", 1, 4, file);

    std::fwrite("fmt ", 1, 4, file);
    drwav_detail::write_u32(file, 16);
    drwav_detail::write_u16(file, format->format);
    drwav_detail::write_u16(file, format->channels);
    drwav_detail::write_u32(file, format->sampleRate);
    const uint32_t blockAlign = static_cast<uint32_t>(format->channels * (format->bitsPerSample / 8));
    const uint32_t byteRate = format->sampleRate * blockAlign;
    drwav_detail::write_u32(file, byteRate);
    drwav_detail::write_u16(file, static_cast<uint16_t>(blockAlign));
    drwav_detail::write_u16(file, format->bitsPerSample);

    std::fwrite("data", 1, 4, file);
    drwav_detail::write_u32(file, 0);

    return wav;
}

inline drwav_uint64 drwav_write_pcm_frames(drwav* wav, drwav_uint64 frameCount, const void* data)
{
    if (wav == nullptr || wav->file == nullptr || data == nullptr)
    {
        return 0;
    }

    const size_t bytesPerFrame =
        static_cast<size_t>(wav->format.channels) * (wav->format.bitsPerSample / 8);
    const size_t bytesToWrite = static_cast<size_t>(frameCount) * bytesPerFrame;
    const size_t bytesWritten = std::fwrite(data, 1, bytesToWrite, wav->file);
    wav->dataChunkDataSizeInBytes += bytesWritten;
    return bytesPerFrame == 0 ? 0 : static_cast<drwav_uint64>(bytesWritten / bytesPerFrame);
}

inline void drwav_close(drwav* wav)
{
    if (wav == nullptr)
    {
        return;
    }

    if (wav->file != nullptr)
    {
        const long fileSize = std::ftell(wav->file);
        const uint32_t riffChunkSize = static_cast<uint32_t>(fileSize - 8);
        const uint32_t dataChunkSize = static_cast<uint32_t>(wav->dataChunkDataSizeInBytes);

        std::fseek(wav->file, 4, SEEK_SET);
        drwav_detail::write_u32(wav->file, riffChunkSize);
        std::fseek(wav->file, 40, SEEK_SET);
        drwav_detail::write_u32(wav->file, dataChunkSize);
        std::fclose(wav->file);
    }

    std::free(wav);
}

inline void drwav_free(void* memory, const void*)
{
    std::free(memory);
}

#endif

#endif
