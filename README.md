ThreadPool
==========

A simple C++17 Thread Pool implementation.

Basic usage:
```c++
// create thread pool with 4 worker threads
ThreadPool pool(4);

// enqueue and store future (it's optional to store the future)
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;

```
