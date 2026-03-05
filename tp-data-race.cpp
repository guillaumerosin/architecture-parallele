#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <functional>
#include <atomic>
//-----------------------------------

#define N 1000*1000*10
#define W 4
#define CLOCK std::chrono::steady_clock
//-----------------------------------

class SpinLock3 {
	std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
	void lock() {
		int spins = 64;
		int backoff = 1;

		//CPU burn
		for (int i = 0; i < spins; ++i) {
			if (!flag.test_and_set(std::memory_order_acquire))
				return;
		}

		//CPU friendly spin
		for (int i = 0; i < 10; ++i) {
			if (!flag.test_and_set(std::memory_order_acquire))
				return;
			std::this_thread::yield();
		}

		//CPU sleep
		while (flag.test_and_set(std::memory_order_acquire)) {
			std::this_thread::sleep_for(
				std::chrono::microseconds(backoff));
			backoff = std::min(backoff * 2, 64);
		}
	}

	void unlock() {
		flag.clear(std::memory_order_release);
	}
};
//-----------------------------------

//resssources
uint64_t x = 0;
std::atomic<uint64_t> xa = 0;
//-----------------------------------


//Verrous
//mutex
//spinlock
//-----------------------------------

void worker(int n)
{
	for (int i = 0; i < n; ++i)
	{
		x++;
	}
}

void worker_yield(int n)
{
	for (int i = 0; i < n; ++i)
	{
		x++;
		std::this_thread::yield();
	}
}

void worker_mutex(int n)
{
	for (int i = 0; i < n; ++i)
	{
		//MUTEX LOCK
		x++;
		//MUTEX UNLOCK
	}
}

void worker_mutex_guard(int n)
{
	for (int i = 0; i < n; ++i)
	{
		//MUTEX LOCK GUARD
		x++;
	}
}

void worker_mutex_all(int n)
{
	//MUTEX LOCK
	for (int i = 0; i < n; ++i)
	{
		x++;
	}
	//MUTEX UNLOCK
}

void worker_mutex_guard_all(int n)
{
	//MUTEX LOCK GUARD
	for (int i = 0; i < n; ++i)
	{
		x++;
	}
}

void worker_atomic(int n)
{
	for (int i = 0; i < n; ++i)
	{
		//ATOMIC ADD
	}
}

void worker_spinlock(int n)
{
	for (int i = 0; i < n; ++i)
	{
		//SPINLOCK LOCK
		x++;
		//SPINLOCK UNLOCK
	}
}


void worker_spinlock_all(int n)
{
	//SPINLOCK LOCK
	for (int i = 0; i < n; ++i)
	{
		x++;
	}
	//SPINLOCK UNLOCK
}

void launch_threads(std::function<void(int)> f, int n, int n_threads, std::string name, bool atomic = false)
{
	x = 0;
	xa.store(0);

	CLOCK::time_point start = CLOCK::now();
	
	std::vector<std::thread> workers;
	for (int i = 0; i < n_threads; ++i) workers.emplace_back(f,n);
	for (int i = 0; i < n_threads; ++i) workers[i].join();
	
	CLOCK::time_point stop = CLOCK::now();
	CLOCK::duration duration = stop - start;

	if (atomic) x = xa;

	std::cout << x << " (" << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " us) - " << name << std::endl;

}

void launch_benchmark(int n, int w)
{
	std::cout << std::endl << "--- N = " << n << " and Threads = " << w << std::endl;
	launch_threads(worker, n * w, 1, "sequential (Thread)");
	launch_threads(worker, n, w, "datarace (no bug if no luck)");
	launch_threads(worker_yield, n, w, "forced datarace (no bug if no luck)");
	launch_threads(worker_atomic, n, w, "atomic (works only with a very simple case)", true);
	launch_threads(worker_mutex, n, w, "mutex");
	launch_threads(worker_mutex_guard, n, w, "mutex lock guard");
	launch_threads(worker_spinlock, n, w, "spinlock");
	launch_threads(worker_mutex_all, n, w, "mutex all (nearly sequential execution path)");
	launch_threads(worker_mutex_guard_all, n, w, "mutex lock guard all (nearly sequential execution path)");
	launch_threads(worker_spinlock_all, n, w, "spinlock all (nearly sequential execution path)");
}

int main()
{

	//-------------  PARALLEL PROCESSING STEP 1 
	int n = N;
	int w = W;
	launch_benchmark(n, w);

	//-------------  PARALLEL PROCESSING STEP 2 
	n = N / 10;
	w = W * 10;
	launch_benchmark(n, w);

	//------------- SEQUENTIAL PROCESSING
	CLOCK::time_point start = CLOCK::now();
	x = 0;
	worker(N * W);
	CLOCK::time_point stop = CLOCK::now();
	CLOCK::duration duration = stop - start;
	std::cout << std::endl;
	std::cout << x << " (" << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " us) - sequential (MAIN)" << std::endl;


	return 0;
}