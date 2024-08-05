#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <winrt/base.h>

#include <iostream>
#include <vector>

#include "util.hpp"
#include "wgc.hpp"

using winrt::com_ptr;

// struct VectorBuffer : IMFMediaBuffer {
//     HRESULT Lock(BYTE**, DWORD*, DWORD*) override { return 0; }
//     HRESULT Unlock() override { return 0; }
//     HRESULT GetCurrentLength(DWORD*) override { return 0; }
//     HRESULT SetCurrentLength(DWORD) override { return 0; }
//     HRESULT GetMaxLength(DWORD*) override { return 0; }
//     HRESULT QueryInterface(REFIID, void**) override { return 0; }
//     ULONG AddRef() override { return 0; };
//     ULONG Release() override { return 0; };
// };

std::vector<uint8_t> BGRAToYUY2(unsigned int width, unsigned int height,
                                std::vector<uint8_t> bgra) {
    std::vector<uint8_t> yuy2(width * height * 2);
    size_t j = 0;
    double R0, G0, B0, R1, G1, B1, Y0, Y1, U, V;
    for (size_t i = 0; i < bgra.size(); i += 8) {
        B0 = bgra.at(i);
        G0 = bgra.at(i + 1);
        R0 = bgra.at(i + 2);  // skip alpha byte
        B1 = bgra.at(i + 4);
        G1 = bgra.at(i + 5);
        R1 = bgra.at(i + 6);

        Y0 = 0.299 * R0 + 0.587 * G0 + 0.114 * B0;
        Y1 = 0.299 * R1 + 0.587 * G1 + 0.114 * B1;
        U = 0.492 * (B0 - Y0) + 128.0;
        V = 0.877 * (R0 - Y0) + 128.0;

        yuy2.at(j) = static_cast<uint8_t>(Y0);
        yuy2.at(j + 2) = static_cast<uint8_t>(Y1);
        yuy2.at(j + 1) = U > 255.0 ? 255 : U < 0.0 ? 0 : static_cast<uint8_t>(U);
        yuy2.at(j + 3) = V > 255.0 ? 255 : V < 0.0 ? 0 : static_cast<uint8_t>(V);

        j += 4;
    }
    return yuy2;
}

// https://learn.microsoft.com/en-us/windows/win32/medfound/sink-writer
// https://stackoverflow.com/a/51278029
void WriteToFile(const std::wstring file, const unsigned int fps, std::vector<Frame> frames) {
    if (frames.empty()) return;
    Frame referenceFrame = frames.at(0);
    std::cout << std::format("{}, {}, {}, {}\n", referenceFrame.width, referenceFrame.height,
                             referenceFrame.timestamp.count(), referenceFrame.data.size());
    const int64_t sampleDuration = 10'000'000 / fps;
    const unsigned int bitrate =
        static_cast<unsigned int>(referenceFrame.width * referenceFrame.height * fps * 0.1);
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
    HandleError(hr, "Failed calling inputFormat->SetGUID(Major)!");
    hr = inputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
    HandleError(hr, "Failed calling inputFormat->SetGUID(Subtype)!");
    hr = inputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    HandleError(hr, "Failed calling inputFormat->SetUINT32(Interlace)!");
    hr = inputFormat->SetUINT32(
        MF_MT_DEFAULT_STRIDE,
        static_cast<unsigned int>(referenceFrame.data.size() / referenceFrame.height));
    HandleError(hr, "Failed calling inputFormat->SetUINT32(Stride)!");
    hr = MFSetAttributeSize(inputFormat.get(), MF_MT_FRAME_SIZE, referenceFrame.width,
                            referenceFrame.height);
    HandleError(hr, "Failed calling MFSetAttributeSize(Input)!");
    hr = MFSetAttributeRatio(inputFormat.get(), MF_MT_FRAME_RATE, fps, 1);
    HandleError(hr, "Failed calling MFSetAttributeRatio(Input)!");

    // Config output format
    hr = outputFormat->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    HandleError(hr, "Failed calling outputFormat->SetGUID(Major)!");
    hr = outputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    HandleError(hr, "Failed calling outputFormat->SetGUID(Subtype)!");
    hr = outputFormat->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
    HandleError(hr, "Failed calling outputFormat->SetUINT32(Bitrate)!");
    hr = outputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    HandleError(hr, "Failed calling outputFormat->SetUINT32(Interlace)!");
    hr = MFSetAttributeSize(outputFormat.get(), MF_MT_FRAME_SIZE, referenceFrame.width,
                            referenceFrame.height);
    HandleError(hr, "Failed calling MFSetAttributeSize(Output)!");
    hr = MFSetAttributeRatio(outputFormat.get(), MF_MT_FRAME_RATE, fps, 1);
    HandleError(hr, "Failed calling MFSetAttributeRatio(Output)!");

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
        size_t frameSize = frame.data.size();
        uint8_t* data = nullptr;
        hr = buffer->Lock(&data, NULL, NULL);
        HandleError(hr, "Failed calling buffer->Lock!");
        std::memcpy(data, frame.data.data(), frameSize);  //* slow?
        hr = buffer->SetCurrentLength(static_cast<DWORD>(frameSize));
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