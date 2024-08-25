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
// refactor encoder into a class
// https://learn.microsoft.com/en-us/windows/win32/medfound/video-processor-mft
// make faster!
void WriteToFile(const std::wstring file, unsigned int outputWidth, unsigned int outputHeight,
                 const unsigned int fps, std::vector<Frame> frames) {
    if (frames.empty()) return;
    Frame referenceFrame = frames.front();
    const int64_t sampleDuration = 10'000'000 / fps;
    const unsigned int bitrate =
        static_cast<unsigned int>(referenceFrame.width * referenceFrame.height * fps * 0.1);
    const int imageWidthBytes = referenceFrame.width * 4;
    const int bufferSize = imageWidthBytes * referenceFrame.height;
    // Crop to make sure output dimensions are even for H.264
    outputWidth = outputWidth % 2 == 0 ? outputWidth : outputWidth - 1;
    outputHeight = outputHeight % 2 == 0 ? outputHeight : outputHeight - 1;
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
    hr = MFCreateMemoryBuffer(bufferSize, buffer.put());
    HandleError(hr, "Failed calling MFCreateMemoryBuffer!");

    // Config input format
    hr = inputFormat->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    HandleError(hr, "Failed calling inputFormat->SetGUID(Major)!");
    hr = inputFormat->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);  // = B8G8R8A8
    HandleError(hr, "Failed calling inputFormat->SetGUID(Subtype)!");
    hr = inputFormat->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    HandleError(hr, "Failed calling inputFormat->SetUINT32(Interlace)!");
    hr = inputFormat->SetUINT32(MF_MT_DEFAULT_STRIDE, imageWidthBytes);
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
    hr = MFSetAttributeSize(outputFormat.get(), MF_MT_FRAME_SIZE, outputWidth, outputHeight);
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
        int stride = static_cast<int>(frame.data.size() / frame.height);
        uint8_t* frameData = frame.data.data();
        uint8_t* bufferData = nullptr;
        hr = buffer->Lock(&bufferData, NULL, NULL);
        HandleError(hr, "Failed calling buffer->Lock!");
        for (unsigned int i = 0; i < frame.height; i++) {  // Copy row by row skipping padding bytes
            std::memcpy(bufferData, frameData, imageWidthBytes);  //* slow?
            frameData += stride;
            bufferData += imageWidthBytes;
        }
        hr = buffer->SetCurrentLength(bufferSize);
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