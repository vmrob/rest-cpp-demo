#include <server_https.hpp>
#include <client_https.hpp>

typedef SimpleWeb::Server<SimpleWeb::HTTPS> HttpsServer;
typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

std::unique_ptr<HttpsServer> SetupServer(int port, int numThreads);
void CreateDefaultEndpoints(HttpsServer* server);
void CreateEchoEndpoint(HttpsServer* server);
void PostHelloWorld(const char* address, const char* endpoint);

std::unique_ptr<HttpsServer> SetupServer(int port, int numThreads) {
    const char* serverPubKeyPath     = "server.crt";
    const char* serverPrivateKeyPath = "server.key";

    auto server = std::make_unique<HttpsServer>(port, numThreads, serverPubKeyPath, serverPrivateKeyPath);

    CreateDefaultEndpoints(server.get());
    CreateEchoEndpoint(server.get());

    return server;
}

void CreateDefaultEndpoints(HttpsServer* server) {
    auto invalidRequest = [](HttpsServer::Response& response, std::shared_ptr<HttpsServer::Request> request) {
        std::string content = "Could not open path " + request->path;
        response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.size() << "\r\n\r\n" << content;
    };

    server->default_resource["POST"] = invalidRequest;
    server->default_resource["GET"]  = invalidRequest;
}

void CreateEchoEndpoint(HttpsServer* server) {
    server->resource["^/echo$"]["POST"] = [](HttpsServer::Response& response, std::shared_ptr<HttpsServer::Request> request) {
        std::stringstream content;
        content << request->content.rdbuf();
        std::string out = content.str();
        response << "HTTP/1.1 200 OK\r\nContent-Length: " << out.size() << "\r\n\r\n" << out;
    };
}

void PostHelloWorld(const char* address, const char* endpoint) {
    HttpsClient client(address, false);

    std::stringstream ss("hello world!");
    auto r3 = client.request("POST", endpoint, ss);
    std::cout << r3->content.rdbuf() << std::endl;
}

int main(int argc, const char* argv[]) {

    const unsigned int numThreads = [&]{
        if (argc == 1) {
            return std::thread::hardware_concurrency();
        }
        return (unsigned int) std::atoi(argv[1]);
    }();

    const int port = 8080;

    auto server = SetupServer(port, numThreads);

    std::thread serverThread([&server](){
        server->start();
    });

    std::cout << "listening on port " << port << " with " << numThreads << " threads..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    PostHelloWorld("localhost:8080", "/echo");

    serverThread.join();

    return 0;
}
