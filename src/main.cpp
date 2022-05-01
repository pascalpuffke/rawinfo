#include <cxxopts.hpp>
#ifdef __unix__
#include <sys/ioctl.h>
#endif
#include <filesystem>
#include <libraw/libraw.h>
#include <numeric>

#include "Data.h"
#include "FormatUtils.h"
#include "TimeUtils.h"

// Used as a backup for non-Unix platforms, otherwise current terminal columns
#define TERM_WIDTH 80

struct CommandlineOptions {
    bool silent { false };
    bool showCamera { true };
    bool showLens { true };
    bool showSize { true };
    bool showTimestamp { true };
    bool showSoftware { true };
    bool showCameraType { true };
    bool showQuality { true };
};

namespace fs = std::filesystem;
namespace chrono = std::chrono;

using fmt::print, fmt::fg, fmt::bg, fmt::color;
using Clock = chrono::system_clock;

constexpr bool stringEmpty(const char* str)
{
    return str == nullptr || str[0] == '\0';
}

void printMetadata(const LibRaw& rawData, const CommandlineOptions& options)
{
    auto imgdata = rawData.imgdata;
    auto sizes = imgdata.sizes;
    auto meta = imgdata.idata;
    // why is this called 'other'? iso, shutter, aperture etc. seem like important metadata, don't they?
    auto other = imgdata.other;
    auto lensInfo = imgdata.lens;
    auto lens = FormatUtils::formatLens(lensInfo);
    auto shooting = imgdata.shootinginfo;

    if (options.showCamera) {
        print(fg(color::cyan), "\tCamera: ");
        print("{} {} ", meta.make, meta.model);
        print(fg(color::gray), "@");
        print(" ISO {} {}\n", other.iso_speed, FormatUtils::formatShutterSpeed(other.shutter));

        if (!stringEmpty(shooting.BodySerial)) {
            print(fg(color::cyan), "\t\tBody serial: ");
            print("{}\n", shooting.BodySerial);
        }
    }

    if (options.showLens) {
        print(fg(color::cyan), "\tLens: ");
        print("{} (id={}) ", lens, lensInfo.makernotes.LensID);
        print(fg(color::gray), "@");
        print(" {}mm {}\n", other.focal_len, FormatUtils::formatAperture(other.aperture));

        if (!stringEmpty(lensInfo.LensSerial)) {
            print(fg(color::cyan), "\t\tLens serial: ");
            print("{}\n", lensInfo.LensSerial);
        }
    }

    if (options.showSize) {
        print(fg(color::cyan), "\tSize: ");
        print("{} {}x{} ", FormatUtils::formatResolution(sizes.width * sizes.height), sizes.width, sizes.height);
        print(fg(color::gray), "(raw: {}x{})\n", sizes.raw_width, sizes.raw_height);
    }

    if (options.showTimestamp) {
        print(fg(color::cyan), "\tTimestamp: ");
        print("{}", TimeUtils::formatISO8601(other.timestamp));
        print(fg(color::gray), " ({})\n", TimeUtils::formatTimeSince(other.timestamp));
    }

    if (options.showSoftware && !stringEmpty(meta.software)) {
        print(fg(color::cyan), "\tSoftware: ");
        print("{}\n", meta.software);
    }

    switch (meta.maker_index) {
    case LIBRAW_CAMERAMAKER_Sony: {
        auto sony = imgdata.makernotes.sony;

        if (options.showCameraType) {
            print(fg(color::cyan), "\tCamera type: ");
            switch (sony.CameraType) {
            case LIBRAW_SONY_DSC:
                print("Sony DSC point-and-shoot\n");
                break;
            case LIBRAW_SONY_DSLR:
                print("Sony DSLR\n");
                break;
            case LIBRAW_SONY_NEX:
                print("Sony NEX mirrorless\n");
                break;
            case LIBRAW_SONY_SLT:
                print("Sony SLT DSLT\n");
                break;
            case LIBRAW_SONY_ILCE:
                print("Sony ILCE E-mount mirrorless\n");
                break;
            case LIBRAW_SONY_ILCA:
                print("Sony ILCA A-mount DSLT\n");
                break;
            default:
                print("Unknown type ({})\n", sony.CameraType);
                break;
            }
        }

        if (options.showQuality) {
            print(fg(color::cyan), "\tQuality: ");
            switch (sony.Quality) {
            case 0:
            case 6:
                print("(Uncompressed) RAW\n");
                break;
            case 7:
            case 8:
                print("Compressed RAW\n");
                break;
            default:
                print("Unknown quality ({})\n", sony.Quality);
                break;
            }
        }

        break;
    }
    case LIBRAW_CAMERAMAKER_Canon: {
        auto canon = imgdata.makernotes.canon;

        if (options.showQuality) {
            print(fg(color::cyan), "\tQuality: ");
            switch (canon.Quality) {
            case 1:
                print("Economy\n");
                break;
            case 2:
                print("Normal\n");
                break;
            case 3:
                print("Fine\n");
                break;
            case 4:
                print("RAW\n");
                break;
            case 5:
                print("Superfine\n");
                break;
            case 7:
                print("CRAW\n");
                break;
            case 130:
                print("Normal Movie\n");
                break;
            case 131:
                print("CRM StandardRaw\n");
                break;
            default:
                print("Unknown quality ({})\n", canon.Quality);
                break;
            }
        }

        break;
    }
    default: {
        UNREACHABLE();
    }
    }
}

void populateArrays(Data* data, const fs::path& path, const CommandlineOptions& options)
{
    ASSERT_MSG(exists(path), "File does not exist");
    LibRaw raw {};
    ASSERT_MSG(raw.open_file(path.c_str()) == LIBRAW_SUCCESS, path.c_str());

    auto imgdata = raw.imgdata;
    auto sizes = imgdata.sizes;
    auto meta = imgdata.idata;
    auto other = imgdata.other;
    auto lensInfo = imgdata.lens;
    auto lens = FormatUtils::formatLens(lensInfo);

    data->timestamps.push_back(other.timestamp);
    data->iso_speeds.push_back(other.iso_speed);
    data->shutter_speeds.push_back(other.shutter);
    data->focal_lengths.push_back(other.focal_len);
    data->aperture_values.push_back(other.aperture);
    data->resolutions.push_back(static_cast<float>(sizes.width * sizes.height));
    data->lenses[lens]++;
    data->cameras[fmt::format("{} {}", meta.make, meta.model)]++;

    if (!options.silent)
        printMetadata(raw, options);
}

std::vector<fs::path> findRawFiles(Data* data, const std::vector<std::string>& directories)
{
    std::vector<fs::path> files {};
    std::size_t count { 0 };
    for (auto& directory : directories) {
        for (auto& entry : fs::recursive_directory_iterator(directory)) {
            if (!entry.is_regular_file())
                continue;

            const auto& path = entry.path();
            if (path.filename().string().at(0) == '.')
                continue;
            if (auto extension = path.extension(); extension == ".ARW" || extension == ".CR2" || extension == ".CR3") {
                files.push_back(path);
                ++count;
            }
        }
    }
    data->reserveCapacity(count);

    return files;
}

int getTerminalWidth()
{
#ifdef __unix__
    struct winsize ws { };
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col;
#else
    return TERM_WIDTH;
#endif
}

int main(int argc, char** argv)
{
    auto options = cxxopts::Options {
        "rawinfo",
        "Command-line frontend for libraw"
    };
    // clang-format off
    options.add_options()
        ("h,help", "Show this help")
        ("s,silent", "Don't show per-file info")
        ("showCamera", "Show camera name, image ISO and shutter speed", cxxopts::value<bool>()->default_value("true"))
        ("showLens", "Show lens name, image focal length and aperture", cxxopts::value<bool>()->default_value("true"))
        ("showSize", "Show image size (dimensions) and resolution", cxxopts::value<bool>()->default_value("true"))
        ("showTimestamp", "Show image timestamp", cxxopts::value<bool>()->default_value("true"))
        ("showSoftware", "Show camera software version", cxxopts::value<bool>()->default_value("true"))
        ("showCameraType", "Show camera type", cxxopts::value<bool>()->default_value("true"))
        ("showQuality", "Show image quality setting", cxxopts::value<bool>()->default_value("true"))
        ("d,directories", "Directories to search for raw files", cxxopts::value<std::vector<std::string>>()->default_value("."))
        ("f,files", "Raw files to process", cxxopts::value<std::vector<std::string>>()->default_value(""));

    // clang-format on
    options.custom_help("[-s] [-f <files>] [-d <directories>] [OPTION...]\n\nNOTE: Use --show(...)=false to disable boolean options with a default value of true.");
    options.set_width(getTerminalWidth());

    if (argc == 1) {
        print(fg(color::red), "No arguments provided\n");
        print("{}\n", options.help());
        return 1;
    }

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        print("{}\n", options.help());
        return 0;
    }

    auto commandLineOptions = CommandlineOptions {
        .silent = result.count("silent") != 0,
        .showCamera = result["showCamera"].as<bool>(),
        .showLens = result["showLens"].as<bool>(),
        .showSize = result["showSize"].as<bool>(),
        .showTimestamp = result["showTimestamp"].as<bool>(),
        .showSoftware = result["showSoftware"].as<bool>(),
        .showCameraType = result["showCameraType"].as<bool>(),
        .showQuality = result["showQuality"].as<bool>(),
    };
    auto data = std::make_unique<Data>();
    auto files = findRawFiles(data.get(), result["directories"].as<std::vector<std::string>>());

    for (auto& file : result["files"].as<std::vector<std::string>>()) {
        files.emplace_back(file);
    }
    ASSERT_MSG(!files.empty(), "No raw files found");

    auto start = Clock::now();

    for (auto& file : files) {
        if (file.empty())
            continue;

        if (!commandLineOptions.silent) {
            print(bg(color::dark_slate_gray) | fg(color::white), "Metadata for image ");
            print(bg(color::dark_slate_gray) | fg(color::light_green), "{}", file.filename().string());
            print(bg(color::dark_slate_gray) | fg(color::white), ":");
            print("\n");
        }
        populateArrays(data.get(), file, commandLineOptions);
    }

    // Do not print summary if we're processing a single file.
    if (files.size() == 1)
        return 0;

    if (!commandLineOptions.silent)
        print("\n");

    auto time = chrono::duration_cast<chrono::milliseconds>(Clock::now() - start);
    auto printVector = [](std::vector<float>& vec, std::string_view name, const std::function<std::string(float)>& format) {
        print(fg(color::cyan), "{}: ", name);

        std::sort(ITERATORS(vec));

        auto min = *std::min_element(ITERATORS(vec));
        auto max = *std::max_element(ITERATORS(vec));
        auto avg = std::accumulate(ITERATORS(vec), 0.0) / vec.size();
        auto median = vec[vec.size() / 2];

        print(fg(color::gray), "min: ");
        print("{} ", format(min));
        print(fg(color::gray), "max: ");
        print("{} ", format(max));
        print(fg(color::gray), "avg: ");
        print("{} ", format(static_cast<float>(avg)));
        print(fg(color::gray), "median: ");
        print("{}\n", format(median));
    };

    data->assertNotEmpty();
    data->assertEqualSizes();

    std::sort(ITERATORS(data->timestamps));
    print("Analyzed {} photos in {}ms ({:.02f}ms/photo)\n", data->iso_speeds.size(), time.count(), time.count() / static_cast<float>(data->iso_speeds.size()));
    print(fg(color::cyan), "Time frame: ");
    print("{} ", TimeUtils::formatISO8601(data->timestamps.front()));
    print(fg(color::gray), "-");
    print(" {} ", TimeUtils::formatISO8601(data->timestamps.back()));
    print(fg(color::gray), "({})\n", TimeUtils::formatTimeSpan(data->timestamps.front(), data->timestamps.back()));
    printVector(data->iso_speeds, "ISO speeds", FormatUtils::formatISO);
    printVector(data->shutter_speeds, "Shutter speeds", FormatUtils::formatShutterSpeed);
    printVector(data->focal_lengths, "Focal lengths", FormatUtils::formatFocalLength);
    printVector(data->aperture_values, "Aperture", FormatUtils::formatAperture);
    printVector(data->resolutions, "Resolutions", FormatUtils::formatResolution);

    for (auto& [lens, count] : data->lenses) {
        print(fg(color::cyan), "Lens '{}': ", lens);
        if (count == 1)
            print("1 photo\n");
        else
            print("{} photos\n", count);
    }

    for (auto& [camera, count] : data->cameras) {
        print(fg(color::cyan), "Camera '{}': ", camera);
        if (count == 1)
            print("1 photo\n");
        else
            print("{} photos\n", count);
    }

    return 0;
}
