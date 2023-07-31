#pragma once

#if _DEBUG
#include <chrono>
#include <iostream>
#include <format>

class stopwatch {
	std::chrono::high_resolution_clock::time_point start;
	inline static unsigned long count = 0;
	inline static double sum = 0.0;
public:
	stopwatch() : start(std::chrono::high_resolution_clock::now()) {}
	~stopwatch() {
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::format_to(std::ostreambuf_iterator{ std::cout }, "{} us", duration);
		if (count) {
			sum += duration;
			std::format_to(std::ostreambuf_iterator{ std::cout }, " ave: {}", sum / count);
		}
		count++;
		std::cout << std::endl;
	}
};

#if !_DEBUG
#pragma message("stopwatch は有効です")

#endif

#else

class stopwatch {};

#endif
