#pragma once

// This is a snippet you can put at the top of all of your SKSE plugins!

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/pattern_formatter.h"

namespace logger = SKSE::log;

class formatter_flag : public spdlog::custom_flag_formatter
{
    public:
    void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override
    {
        size_t longestFileName = "OcclusionHandler.cpp"s.size();
        size_t maxDigitsInLineNumber = 5;
        size_t digitsInLineNumber = std::to_string(msg.source.line).size();

        std::string filepath(msg.source.filename);
        std::string filename(filepath.substr(filepath.rfind("\\") + 1));

        longestFileName -= filename.size() - (maxDigitsInLineNumber - digitsInLineNumber);

        std::string whitespace(longestFileName, ' ');
        dest.append(whitespace.data(), whitespace.data() + whitespace.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<formatter_flag>();
    }
};

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::trace);
	auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<formatter_flag>('*').set_pattern("[%H:%M:%S.%e][%s:%#]%*%v");
    spdlog::set_formatter(std::move(formatter));
    //spdlog::set_pattern("[%H:%M:%S.%e] %16s:%-5# | %v"); // %<x>s: x = # of characters in longest file name//https://github.com/gabime/spdlog/wiki/Custom-formatting
}




