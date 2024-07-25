

// https://learn.microsoft.com/en-us/windows/win32/medfound/sink-writer

// namespace winrt {
// using namespace winrt::Windows::Media::MediaProperties;
// using namespace winrt::Windows::Media::Core;
// using namespace winrt::Windows::Media::Transcoding;
// using namespace winrt::Windows::Storage;
// using namespace winrt::Windows::Storage::Streams;
// }

// std::vector<winrt::MediaStreamSample> frames{};
// int currentFrame = 0;

// void OnSampleRequested(winrt::MediaStreamSource source,
//                        winrt::MediaStreamSourceSampleRequestedEventArgs request) {
//     auto frame = (currentFrame < frames.size()) ? frames.at(currentFrame) : NULL;
//     request.Request().Sample(frame);
//     ++currentFrame;
// }

// void WriteToFile(int width, int height) {
//     winrt::MediaEncodingProfile outputFormat =
//         winrt::MediaEncodingProfile::CreateMp4(winrt::VideoEncodingQuality::HD1080p);
//     winrt::VideoEncodingProperties inputProperties =
//         winrt::VideoEncodingProperties::CreateUncompressed(winrt::MediaEncodingSubtypes::Bgra8(),
//                                                            width, height);
//     winrt::VideoStreamDescriptor descriptor{inputProperties};
//     winrt::MediaStreamSource source{descriptor};
//     winrt::MediaTranscoder transcoder{};
//     source.SampleRequested(&OnSampleRequested);

//     int length = GetCurrentDirectoryA(0, NULL);
//     std::wstring filePath(length, NULL);
//     GetCurrentDirectoryW(length, filePath.data());
//     filePath += L"\\out.mp4";
//     std::wcout << filePath << "\n";
//     winrt::IRandomAccessStream* file;
//     HRESULT hr = CreateRandomAccessStreamOnFile(
//         filePath.data(), static_cast<unsigned long>(winrt::FileAccessMode::ReadWrite),
//         winrt::guid_of<winrt::IRandomAccessStream>(), (void**)&file);
//     HandleError(hr, "Couldn't CreateRandomAccessStreamOnFile!");

//     auto result =
//         transcoder.PrepareMediaStreamSourceTranscodeAsync(source, *file, outputFormat).get();
//     result.TranscodeAsync().get();

//     auto start = std::chrono::system_clock::now();
//     std::cout << "Writing video file took "
//               << std::chrono::duration_cast<std::chrono::milliseconds>(
//                      std::chrono::system_clock::now() - start)
//                          .count() /
//                      1000.0
//               << " seconds.\n";
// }