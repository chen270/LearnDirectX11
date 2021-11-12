#ifndef __CHILITIMER_H__
#define __CHILITIMER_H__

#include <chrono>

class ChiliTimer
{
public:
	ChiliTimer() noexcept;
	float Mark() noexcept;
	float Peek() const noexcept;
private:
	std::chrono::steady_clock::time_point last;
};

#endif //__CHILITIMER_H__