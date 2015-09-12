#include <client_https.hpp>
#include <chrono>
#include <memory>
#include <future>
#include <iostream>

typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

size_t ClientStressThread(const char* hostname, size_t trials) {

    HttpsClient client(hostname, false);

    std::stringstream ss("hello world!");

    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < trials; ++i) {
        auto response = client.request("POST", "/echo", ss);
        // std::cout << response->content.rdbuf() << std::endl;
        ss.seekg(0, std::ios::beg);
    }

    auto stop = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
}

int main(int argc, const char* argv[]) {

    const char* hostname = [&]{
        if (argc == 1) {
            std::cout << "usage: stress_client hostname:port [threads] [requests]" << std::endl;
            std::exit(1);
        }
        return argv[1];
    }();

    const unsigned int numThreads = [&]{
        if (argc == 3) {
            return (unsigned int) std::atoi(argv[2]);
        }
        return std::thread::hardware_concurrency();
    }();

    const unsigned int requestsPerThread = [&]{
        if (argc == 4) {
            return std::atoi(argv[3]);
        }
        return 10000;
    }();

    std::vector<std::future<size_t>> futures;

    std::cout << "sending " << requestsPerThread * numThreads << " requests across " << numThreads << " threads to " << hostname << "..." << std::endl;

    size_t totalMicroseconds = 0;
    size_t totalRequests = requestsPerThread * numThreads;

    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, ClientStressThread, hostname, requestsPerThread));
    }

    std::cout << "waiting for results..." << std::endl;

    for (auto&& f : futures) {
        totalMicroseconds += f.get();
    }

    auto stop = std::chrono::steady_clock::now();

    auto totalTime = stop - start;
    double totalTimeSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count() / 1000.0;
    auto requestsPerSecond = totalRequests / (double)totalTimeSeconds;
    auto averageMicrosecondsPerRequest = totalMicroseconds / (double) totalRequests;

    std::cout << "total time (s): " << totalTimeSeconds << std::endl;
    std::cout << "requests per second: " << requestsPerSecond << std::endl;
    std::cout << "time per request (us): " << averageMicrosecondsPerRequest << std::endl;

    return 0;
}
