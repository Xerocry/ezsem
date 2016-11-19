#include <iostream>
#include <sched.h>
#include <unistd.h>

int main() {
	int min, max;

	// SCHED_FIFO

	min = sched_get_priority_min(SCHED_FIFO);
	max = sched_get_priority_max(SCHED_FIFO);

	std::cout << "SCHED_FIFO [" << min << ", " << max << "]" << std::endl;

	// SCHED_RR

	min = sched_get_priority_min(SCHED_RR);
	max = sched_get_priority_max(SCHED_RR);

	std::cout << "SCHED_RR [" << min << ", " << max << "]" << std::endl;

	// SCHED_OTHER

	min = sched_get_priority_min(SCHED_OTHER);
	max = sched_get_priority_max(SCHED_OTHER);

	std::cout << "SCHED_OTHER [" << min << ", " << max << "]" << std::endl;

	// Получение текущей политики планирования
	int policy = sched_getscheduler(0);

	std::cout << "Current policy: " << ((policy == SCHED_FIFO) ? "SCHED_FIFO" : ((policy == SCHED_RR) ? "SCHED_RR": "SCHED_OTHER")) << std::endl;

	
	struct sched_param schedParam;

	// Получение текущего приоритета
	if(sched_getparam(0, &schedParam) != 0) {
		// Выводим сообщение об ошибке
		std::cerr << "It's impossible to get param." << std::endl;
		return 0x1;
	}

	std::cout << "Current priority: " << schedParam.sched_priority << std::endl;

	return 0x0;
}
