#pragma once

#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <exception>
#include <mutex>

#include <fmt/format.h>
#include <fmt/color.h>

#define EXPLO_LOG_LEVEL_ERROR 0
#define EXPLO_LOG_LEVEL_WARN  1
#define EXPLO_LOG_LEVEL_INFO  2
#define EXPLO_LOG_LEVEL_DEBUG 3

#define EXPLO_LOG_LEVEL EXPLO_LOG_LEVEL_DEBUG

namespace explo
{
	namespace internal
	{
		inline void log_level_prompt(int log_level)
		{
			switch (log_level)
			{
			case EXPLO_LOG_LEVEL_ERROR: fmt::print(fmt::fg(fmt::color::indian_red), "ERROR");   break;
			case EXPLO_LOG_LEVEL_WARN:  fmt::print(fmt::fg(fmt::color::yellow), "WARN"); break;
			case EXPLO_LOG_LEVEL_INFO:  fmt::print(fmt::fg(fmt::color::white), "INFO");  break;
			case EXPLO_LOG_LEVEL_DEBUG: fmt::print(fmt::fg(fmt::color::gray), "DEBUG");  break;
			default:
				throw std::runtime_error("Illegal argument");
			}
		}
	} // namespace internal

	template<typename... _args_t>
	void log(int log_level, char const* context, char const* format_string, _args_t&&... args)
	{
		static std::mutex s_mutex;
		std::lock_guard<std::mutex> lock(s_mutex);

		// [explo]
		fmt::print(fmt::fg(fmt::color::aqua), "[explo] ");

		// [context]
		if (context && strcmp(context, ""))
		{
			uint32_t context_color = (std::hash<std::string>{}(context) % (0xEEEEEE - 0x707070)) + 0x707070;
			fmt::text_style context_style = fmt::fg(static_cast<fmt::color>(context_color));

			fmt::print(context_style, "[");
			fmt::print(context_style, context);
			fmt::print(context_style, "] ");
		}

		// LOG_LEVEL:
		internal::log_level_prompt(log_level);
		fmt::print(": ");

		// Formatted message
		fmt::print<_args_t...>(format_string, std::forward<_args_t>(args)...);
		fmt::print("\n");

		fflush(stdout);
	}
} // namespace explo

#if EXPLO_LOG_LEVEL >= EXPLO_LOG_LEVEL_ERROR
#   define LOG_E(context, format, ...)  explo::log(EXPLO_LOG_LEVEL_ERROR, context, format, __VA_ARGS__)
#else
#	define LOG_E(context, format, ...)
#endif

#if EXPLO_LOG_LEVEL >= EXPLO_LOG_LEVEL_WARN
#   define LOG_W(context, format, ...) explo::log(EXPLO_LOG_LEVEL_WARN, context, format, __VA_ARGS__)
#else
#   define LOG_W(context, format, ...)
#endif

#if EXPLO_LOG_LEVEL >= EXPLO_LOG_LEVEL_INFO
#   define LOG_I(context, format, ...) explo::log(EXPLO_LOG_LEVEL_INFO, context, format, __VA_ARGS__)
#else
#	define LOG_I(context, format, ...)
#endif

#if EXPLO_LOG_LEVEL >= EXPLO_LOG_LEVEL_DEBUG
#   define LOG_D(context, format, ...) explo::log(EXPLO_LOG_LEVEL_DEBUG, context, format, __VA_ARGS__)
#else
#   define LOG_D(context, format, ...)
#endif
