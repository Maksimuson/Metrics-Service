#include <httplib.h>
#include <iostream>

int main() {
    httplib::Server svr;

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain");
    });

    std::cout << "API started on port 8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}