#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <winrt/base.h>

#include <vector>

#include "util.hpp"
#include "wgc.hpp"

using winrt::com_ptr;

std::vector<Frame> BGRAToYUY2(std::vector<Frame> frames) {
    for (auto frame : frames) {
        std::vector<uint8_t> dest{};
        dest.resize(frame.width * frame.height * 2);
        double R0, G0, B0, R1, G1, B1, Y0, Y1, U, V;
        size_t j = 0;
        for (size_t i = 0; i < frame.data.size(); i += 8) {
            B0 = frame.data.at(i);
            G0 = frame.data.at(i + 1);
            R0 = frame.data.at(i + 2);  // skip alpha byte
            B1 = frame.data.at(i + 4);
            G1 = frame.data.at(i + 5);
            R1 = frame.data.at(i + 6);

            Y0 = 0.299 * R0 + 0.587 * G0 + 0.114 * B0;
            Y1 = 0.299 * R1 + 0.587 * G1 + 0.114 * B1;
            U = 0.492 * (B0 - Y0) + 128.0;
            V = 0.877 * (R0 - Y0) + 128.0;

            dest.at(j) = static_cast<uint8_t>(Y0);
            dest.at(j + 2) = static_cast<uint8_t>(Y1);
            dest.at(j + 1) = U > 255.0 ? 255 : U < 0.0 ? 0 : static_cast<uint8_t>(U);
            dest.at(j + 3) = V > 255.0 ? 255 : V < 0.0 ? 0 : static_cast<uint8_t>(V);

            j += 4;
        }
        // frame.data = dest;
        FILE* file;
        fopen_s(&file, "pic.yuy2", "w");
        std::fwrite(dest.data(), sizeof(uint8_t), dest.size(), file);
        std::fclose(file);
        fopen_s(&file, "pic.bgra", "w");
        std::fwrite(frame.data.data(), sizeof(uint8_t), frame.data.size(), file);
        std::fclose(file);
        exit(0);
    }
    return frames;
}

// https://learn.microsoft.com/en-us/windows/win32/medfound/sink-writer
// https://learn.microsoft.com/en-us/windows/win32/medfound/colorconverter
// https://stackoverflow.com/a/51278029
// compute shader
// test with our format first to see if it even works
void WriteToFile(const std::wstring file, const int fps, std::vector<Frame> frames) {
    if (frames.empty()) return;
    const int64_t sampleDuration = 10'000'000 / fps;
    const unsigned int bitrate = 800'000;
    frames = BGRAToYUY2(frames);
    Frame referenceFrame = frames.at(0);
    com_ptr<IMFSinkWriter> sinkWriter;
    com_ptr<IMFMediaType> inputFormat;
    com_ptr<IMFMediaType> outputFormat;
    com_ptr<IMFSample> sample;
    com_ptr<IMFMediaBuffer> buffer;
    unsigned long videoStreamIndex;
    HRESULT hr = MFStartup(MF_VERSION);
    HandleError(hr, "Failed calling MFStartup!");
    hr = MFCreateSinkWriterFromURL(file.data(), NULL, NULL, sinkWriter.put());
    HandleError(hr, "Failed calling MFCreateSinkWriterFromURL!");
    hr = MFCreateMediaType(inputFormat.put());
    HandleError(hr, "Failed calling MFCreateMediaType(Input)!");
    hr = MFCreateMediaType(outputFormat.put());
    HandleError(hr, "Failed calling MFCreateMediaType(Output)!");
    hr = MFCreateSample(sample.put());
    HandleError(hr, "Failed calling MFCreateSample!");
    hr = MFCreateMemoryBuffer(static_cast<DWORD>(referenceFrame.data.size()), buffer.put());
    HandleError(hr, "Failed calling MFCreateMemoryBuffer!");

    // Config input format
    hr = inputFormat->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = inputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    hr = inputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    hr = MFSetAttributeSize(inputFormat.get(), MF_MT_FRAME_SIZE, referenceFrame.width,
                            referenceFrame.height);
    hr = MFSetAttributeRatio(inputFormat.get(), MF_MT_FRAME_RATE, fps, 1);

    // Config output format
    hr = outputFormat->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = outputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    hr = outputFormat->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
    hr = outputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(outputFormat.get(), MF_MT_FRAME_SIZE, referenceFrame.width,
                       referenceFrame.height);
    MFSetAttributeRatio(outputFormat.get(), MF_MT_FRAME_RATE, fps, 1);

    hr = sample->AddBuffer(buffer.get());
    HandleError(hr, "Failed calling sample->AddBuffer!");
    hr = sample->SetSampleDuration(sampleDuration);
    HandleError(hr, "Failed calling sample->SetSampleDuration!");
    hr = sinkWriter->AddStream(outputFormat.get(), &videoStreamIndex);
    HandleError(hr, "Failed calling sinkWriter->AddStream!");
    hr = sinkWriter->SetInputMediaType(videoStreamIndex, inputFormat.get(), NULL);
    HandleError(hr, "Failed calling sinkWriter->SetInputMediaType!");
    hr = sinkWriter->BeginWriting();
    HandleError(hr, "Failed calling sinkWriter->BeginWriting!");
    for (auto frame : frames) {
        uint8_t* data;
        hr = buffer->Lock(&data, NULL, NULL);
        HandleError(hr, "Failed calling buffer->Lock!");
        data = frame.data.data();
        hr = buffer->SetCurrentLength(static_cast<DWORD>(frame.data.size()));
        HandleError(hr, "Failed calling buffer->SetCurrentLength!");
        hr = buffer->Unlock();
        HandleError(hr, "Failed calling buffer->Unlock!");
        hr = sample->SetSampleTime(frame.timestamp.count());
        HandleError(hr, "Failed calling sample->SetSampleTime!");
        hr = sinkWriter->WriteSample(videoStreamIndex, sample.get());
        HandleError(hr, "Failed calling sinkWriter->WriteSample!");
    }
    hr = sinkWriter->Finalize();
    HandleError(hr, "Failed calling sinkWriter->Finalize!");
    hr = MFShutdown();
    HandleError(hr, "Failed calling MFShutdown!");
}