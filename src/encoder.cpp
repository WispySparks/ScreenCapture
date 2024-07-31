#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <winrt/base.h>

#include <vector>

#include "util.hpp"
#include "wgc.hpp"

using winrt::com_ptr;

// https://learn.microsoft.com/en-us/windows/win32/medfound/sink-writer
// https://stackoverflow.com/a/51278029
void WriteToFile(const std::wstring file, const int fps, std::vector<Frame> frames) {
    if (frames.size() == 0) return;
    const int64_t sampleDuration = 10'000'000 / fps;
    const unsigned int bitrate = 800'000;
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