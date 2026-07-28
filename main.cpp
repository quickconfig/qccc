#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <libqcc.h>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace colors {
constexpr std::string_view rosewater = "\033[38;2;245;224;220m";
constexpr std::string_view flamingo = "\033[38;2;242;205;205m";
constexpr std::string_view pink = "\033[38;2;245;194;231m";
constexpr std::string_view mauve = "\033[38;2;203;166;247m";
constexpr std::string_view red = "\033[38;2;243;139;168m";
constexpr std::string_view maroon = "\033[38;2;235;160;172m";
constexpr std::string_view peach = "\033[38;2;250;179;135m";
constexpr std::string_view yellow = "\033[38;2;249;226;175m";
constexpr std::string_view green = "\033[38;2;166;227;161m";
constexpr std::string_view teal = "\033[38;2;148;226;213m";
constexpr std::string_view sky = "\033[38;2;137;220;235m";
constexpr std::string_view sapphire = "\033[38;2;116;199;236m";
constexpr std::string_view blue = "\033[38;2;137;180;250m";
constexpr std::string_view lavender = "\033[38;2;180;190;254m";

constexpr std::string_view text = "\033[38;2;205;214;244m";
constexpr std::string_view subtext0 = "\033[38;2;166;173;200m";
constexpr std::string_view subtext1 = "\033[38;2;186;194;222m";
constexpr std::string_view overlay0 = "\033[38;2;108;112;134m";
constexpr std::string_view overlay1 = "\033[38;2;127;132;156m";
constexpr std::string_view surface0 = "\033[38;2;49;50;68m";
constexpr std::string_view surface1 = "\033[38;2;69;71;90m";

constexpr std::string_view reset = "\033[0m";
constexpr std::string_view bold = "\033[1m";
} // namespace colors

namespace symbols {
constexpr std::string_view check = "✓";
constexpr std::string_view cross = "✗";
constexpr std::string_view pipe = "│";
constexpr std::string_view dash = "─";
constexpr std::string_view arrow = "→";
} // namespace symbols

namespace meta {
constexpr std::string_view app_name = "QuickConfigCompilerCLI";
constexpr std::string_view version = "26.2.1";
constexpr std::string_view library = "libqcc";
constexpr std::string_view language = "QuickConfig";
} // namespace meta

struct SourceCoord {
  std::size_t line{1};
  std::size_t column{1};
};

struct DiagnosticError {
  std::string msg;
  SourceCoord start;
  SourceCoord end;
};

struct FileIOResult {
  bool success{false};
  std::string content_or_error;
};

struct FileIOResultBin {
  bool success{false};
  std::vector<std::uint8_t> content_or_error;
};

FileIOResultBin read_file_to_bin(const fs::path &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return {false, {}};
  }

  std::streamsize size = file.tellg();
  if (size < 0) {
    return {false, {}};
  }

  file.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> buffer(size);
  if (size > 0 && !file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    return {false, {}};
  }

  return {true, std::move(buffer)};
}

FileIOResult read_file(const fs::path &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return {false, "Cannot open file"};
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  return {true, ss.str()};
}

bool write_file(const fs::path &path, std::string_view content) {
  std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }
  file << content;
  return true;
}

bool write_file(const fs::path &path, std::vector<std::uint8_t> content) {
  std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }
  file.write(reinterpret_cast<const char *>(content.data()), content.size());
  return !file.bad();
}

std::vector<std::string_view> split_lines(std::string_view source) {
  std::vector<std::string_view> lines;
  std::size_t start = 0;
  while (start < source.size()) {
    std::size_t end = source.find('\n', start);
    if (end == std::string_view::npos) {
      lines.push_back(source.substr(start));
      break;
    }
    std::size_t len = end - start;
    if (len > 0 && source[end - 1] == '\r') {
      lines.push_back(source.substr(start, len - 1));
    } else {
      lines.push_back(source.substr(start, len));
    }
    start = end + 1;
  }
  return lines;
}

void print_banner() {
  std::cout << colors::bold << colors::mauve << meta::app_name << colors::reset
            << "\n";
}

void print_help() {
  print_banner();
  std::cout << "\n"
            << colors::sky << "Usage\n"
            << colors::reset << "    " << colors::text << "qccc"
            << colors::subtext0 << " <command> [options]\n\n"
            << colors::sky << "Commands\n"
            << colors::reset << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "compile") << colors::subtext0
            << " Compile a target .qcf input file\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "decompile") << colors::subtext0
            << " Decompile a compiled asset file\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "fmt") << colors::subtext0
            << " Format file or directory of .qcf assets\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "export") << colors::subtext0
            << " Export a target .qcf input file. Options json/toml\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "decompile") << colors::subtext0
            << " Decompile a compiled asset file\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "version") << colors::subtext0
            << " Display compiler application version\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::mauve) + "help") << colors::subtext0
            << " Show this help documentation\n\n"
            << colors::sky << "Options\n"
            << colors::reset << "    " << std::left << std::setw(16)
            << (std::string(colors::peach) + "-C, --compile")
            << colors::subtext0 << " Alias for compile command\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::peach) + "--decompile") << colors::subtext0
            << " Alias for decompile command\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::peach) + "--export") << colors::subtext0
            << " Alias for export command\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::peach) + "-V, --version")
            << colors::subtext0 << " Alias for version command\n"
            << "    " << std::left << std::setw(16)
            << (std::string(colors::peach) + "-h, --help") << colors::subtext0
            << " Alias for help command\n\n";
}

void print_version() {
  print_banner();
  std::cout << "\n"
            << std::left << std::setw(12) << colors::subtext1 << "Version "
            << colors::teal << meta::version << colors::reset << "\n"
            << std::left << std::setw(12) << colors::subtext1 << "Library "
            << colors::sapphire << meta::library << ": " << libqcc::VERSION
            << colors::reset << "\n"
            << std::left << std::setw(12) << colors::subtext1 << "Language "
            << colors::green << meta::language << colors::reset << "\n";
}

void print_error_header(std::string_view label, std::string_view label_color,
                        std::string_view message) {
  std::cerr << colors::bold << label_color << label << ":" << colors::reset
            << "\n"
            << colors::text << message << colors::reset << "\n\n";
}

void print_error(std::string_view message, std::string_view detail = "",
                 std::string_view hint = "") {
  print_error_header("error", colors::red, message);
  if (!detail.empty()) {
    std::cerr << colors::text << detail << colors::reset << "\n\n";
  }
  if (!hint.empty()) {
    std::cerr << colors::subtext0 << "Try\n"
              << colors::reset << "    " << colors::mauve << hint
              << colors::reset << "\n\n";
  }
}

void print_warning(std::string_view message) {
  print_error_header("warning", colors::yellow, message);
}

void print_note(std::string_view message) {
  print_error_header("note", colors::sky, message);
}

void print_info(std::string_view label, std::string_view value) {
  std::cout << colors::subtext1 << std::left << std::setw(10) << label
            << colors::text << value << colors::reset << "\n";
}

void render_source_snippet(std::string_view source, const DiagnosticError &err,
                           const fs::path &filepath) {
  auto lines = split_lines(source);
  if (lines.empty())
    return;

  std::size_t start_line = (err.start.line > 0) ? err.start.line : 1;
  std::size_t end_line = (err.end.line > 0) ? err.end.line : start_line;

  std::size_t display_start = (start_line > 1) ? start_line - 1 : 1;
  std::size_t display_end = std::min(lines.size(), end_line + 1);

  std::size_t max_line_num = display_end;
  std::size_t gutter_width = std::to_string(max_line_num).length();

  std::cerr << std::string(gutter_width, ' ') << " " << colors::blue
            << symbols::arrow << colors::reset << " " << colors::subtext0
            << filepath.string() << ":" << err.start.line << ":"
            << err.start.column << colors::reset << "\n\n";

  for (std::size_t line_idx = display_start; line_idx <= display_end;
       ++line_idx) {
    std::cerr << colors::overlay0 << std::right << std::setw(gutter_width)
              << line_idx << " " << colors::surface1 << symbols::pipe << " "
              << colors::reset;

    std::string_view line_str = lines[line_idx - 1];

    if (line_idx < start_line || line_idx > end_line) {
      std::cerr << colors::mauve << line_str << colors::reset << "\n";
      continue;
    }

    std::size_t col_start_0 = (line_idx == start_line && err.start.column > 0)
                                  ? err.start.column - 1
                                  : 0;
    std::size_t col_end_0 = (line_idx == end_line && err.end.column > 0)
                                ? err.end.column - 1
                                : line_str.size();

    col_start_0 = std::min(col_start_0, line_str.size());
    col_end_0 = std::min(col_end_0, line_str.size());

    if (col_start_0 > col_end_0) {
      std::swap(col_start_0, col_end_0);
    }

    std::string_view prefix = line_str.substr(0, col_start_0);
    std::string_view highlighted =
        line_str.substr(col_start_0, col_end_0 - col_start_0);
    std::string_view suffix = line_str.substr(col_end_0);

    std::cerr << colors::mauve << prefix << colors::red << highlighted
              << colors::mauve << suffix << colors::reset << "\n";
  }
  std::cerr << "\n";
}

void render_compile_error(const DiagnosticError &err, std::string_view source,
                          const fs::path &filepath) {
  print_error_header("error", colors::red, err.msg);
  render_source_snippet(source, err, filepath);
}

DiagnosticError make_diagnostic(const libqcc::errors::compile_time_error &err) {
  return DiagnosticError{
      .msg = err.msg,
      .start = {static_cast<std::size_t>(err.coord_start.line),
                static_cast<std::size_t>(err.coord_start.column)},
      .end = {static_cast<std::size_t>(err.coord_end.line),
              static_cast<std::size_t>(err.coord_end.column)}};
}

int compile_command(const fs::path &filepath) {
  auto timer_start = std::chrono::high_resolution_clock::now();

  if (!fs::exists(filepath)) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  auto file_res = read_file(filepath);
  if (!file_res.success) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  try {
    auto compiled = libqcc::deep_compile(file_res.content_or_error);

    fs::path out_path = filepath;
    out_path.replace_extension(".qbin");

    if (!write_file(out_path, libqcc::compiler::compile_to_bin(compiled))) {
      print_error("Failed to write output binary file", out_path.string(), "");
      return 1;
    }

    auto timer_end = std::chrono::high_resolution_clock::now();
    double elapsed_ms =
        std::chrono::duration<double, std::milli>(timer_end - timer_start)
            .count();

    std::cout << colors::bold << colors::green << symbols::check
              << " Compilation successful" << colors::reset << "\n\n";
    print_info("Input", filepath.string());
    print_info("Output", out_path.string());

    std::ostringstream time_ss;
    time_ss << std::fixed << std::setprecision(1) << elapsed_ms << " ms";
    print_info("Time", time_ss.str());

    return 0;
  } catch (const libqcc::errors::compile_time_error &err) {
    render_compile_error(make_diagnostic(err), file_res.content_or_error,
                         filepath);
    return 1;
  } catch (...) {
    print_error("Internal compiler error", filepath.string(), "");
    return 1;
  }
}

int decompile_command(const fs::path &filepath) {
  auto timer_start = std::chrono::high_resolution_clock::now();

  if (!fs::exists(filepath)) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  auto file_res = read_file_to_bin(filepath);
  if (!file_res.success) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  try {
    auto decompiled =
        libqcc::compiler::de_compile_to_ast(file_res.content_or_error);

    if (!decompiled.success) {
      print_error("Failed to decompiled source file",
                  std::to_string(decompiled.index), decompiled.error);
      return 1;
    }

    fs::path out_path = filepath;
    out_path.replace_extension(".qcf");

    if (!write_file(out_path, libqcc::format(decompiled.result))) {
      print_error("Failed to write decompiled source file", out_path.string(),
                  "");
      return 1;
    }

    auto timer_end = std::chrono::high_resolution_clock::now();
    double elapsed_ms =
        std::chrono::duration<double, std::milli>(timer_end - timer_start)
            .count();

    std::cout << colors::bold << colors::green << symbols::check
              << " Decompilation successful" << colors::reset << "\n\n";
    print_info("Input", filepath.string());
    print_info("Output", out_path.string());

    std::ostringstream time_ss;
    time_ss << std::fixed << std::setprecision(1) << elapsed_ms << " ms";
    print_info("Time", time_ss.str());

    return 0;
  } catch (...) {
    print_error("Internal unexpected error", filepath.string(), "");
    return 1;
  }
}

int format_file(const fs::path &filepath, bool verbose = true) {
  if (!fs::exists(filepath)) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  auto file_res = read_file(filepath);
  if (!file_res.success) {
    print_error("Cannot open file", filepath.string(), "");
    return 1;
  }

  try {
    std::string formatted = libqcc::format(file_res.content_or_error);
    if (!write_file(filepath, formatted)) {
      print_error("Failed to write file", filepath.string(), "");
      return 1;
    }

    if (verbose) {
      std::cout << colors::green << symbols::check << colors::text
                << " Formatted " << filepath.string() << colors::reset << "\n";
    }
    return 0;
  } catch (const libqcc::errors::compile_time_error &err) {
    render_compile_error(make_diagnostic(err), file_res.content_or_error,
                         filepath);
    return 1;
  } catch (...) {
    print_error("Failed to format file", filepath.string(), "");
    return 1;
  }
}

int format_directory(const fs::path &dirpath) {
  auto timer_start = std::chrono::high_resolution_clock::now();

  if (!fs::exists(dirpath)) {
    print_error("Directory does not exist", dirpath.string(), "");
    return 1;
  }

  if (!fs::is_directory(dirpath)) {
    print_error("Path is not a directory", dirpath.string(), "");
    return 1;
  }

  std::size_t file_count = 0;
  int overall_status = 0;

  try {
    for (const auto &entry : fs::directory_iterator(dirpath)) {
      if (entry.is_regular_file() && entry.path().extension() == ".qcf") {
        int status = format_file(entry.path(), false);
        if (status == 0) {
          std::cout << colors::green << symbols::check << colors::text << " "
                    << entry.path().string() << colors::reset << "\n";
          file_count++;
        } else {
          overall_status = status;
        }
      }
    }
  } catch (const fs::filesystem_error &e) {
    print_error("Filesystem error", e.what(), "");
    return 1;
  }

  auto timer_end = std::chrono::high_resolution_clock::now();
  double elapsed_ms =
      std::chrono::duration<double, std::milli>(timer_end - timer_start)
          .count();

  if (overall_status == 0) {
    std::cout << "\n"
              << colors::bold << colors::green << symbols::check << colors::text
              << " Formatted " << file_count << " files in "
              << static_cast<long long>(elapsed_ms) << " ms" << colors::reset
              << "\n";
  }

  return overall_status;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    print_banner();
    std::cout << "\n"
              << colors::text << "No command supplied.\n"
              << colors::subtext0 << "Run\n"
              << colors::reset << "    " << colors::mauve << "qccc --help"
              << colors::reset << "\n";
    return 0;
  }

  std::vector<std::string_view> args;
  args.reserve(argc - 1);
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  const std::string_view cmd = args[0];

  if (cmd == "--help" || cmd == "-h" || cmd == "help") {
    print_help();
    return 0;
  }

  if (cmd == "--version" || cmd == "-V" || cmd == "version") {
    print_version();
    return 0;
  }

  if (cmd == "compile" || cmd == "-C" || cmd == "--compile") {
    if (args.size() < 2) {
      print_error("Missing target file for compilation", "",
                  "qccc compile <file.qcf>");
      return 1;
    }
    return compile_command(args[1]);
  }

  if (cmd == "decompile" || cmd == "--decompile") {
    if (args.size() < 2) {
      print_error("Missing target binary for decompilation", "",
                  "qccc decompile <file.qbin>");
      return 1;
    }
    return decompile_command(args[1]);
  }

  if (cmd == "fmt") {
    if (args.size() == 1) {
      return format_directory(".");
    }

    fs::path target_path(args[1]);

    if (args.size() == 2) {
      if (fs::is_directory(target_path)) {
        return format_directory(target_path);
      }
      return format_file(target_path, true);
    }

    print_error("Invalid arguments for format command", "",
                "qccc fmt <path> [options]");
    return 1;
  }

  if (cmd == "export" || cmd == "--export") {
    if (args.size() != 3) {
      print_error("Invalid argument length for export command.", "",
                  "qcc export <option> <filepath>");
      return 1;
    }

    auto opt = args[1];
    auto filepath = args[2];

    auto code = read_file(filepath);

    if (!code.success) {
      print_error("Cannot read target file.", code.content_or_error);
      return 1;
    }

    try {
      auto compiled = libqcc::deep_compile(code.content_or_error);
      auto serialized = libqcc::serializer::serialize(compiled);

      if (opt == "json") {
        std::cout << libqcc::serializer::export_to_json(serialized);
        return 0;
      } else if (opt == "toml") {
        std::cout << libqcc::serializer::export_to_toml(serialized);
        return 0;
      } else {
        print_error("Invalid option selected.", "",
                    "Available 'json' and 'toml'.");
        return 1;
      }
    } catch (const libqcc::errors::compile_time_error &err) {
      render_compile_error(make_diagnostic(err), code.content_or_error,
                           filepath);
      return 1;
    } catch (...) {
      print_error("Internal compiler error", "", "");
      return 1;
    }
  }

  print_error("Unknown command", std::string(cmd), "qccc --help");
  return 1;
}
