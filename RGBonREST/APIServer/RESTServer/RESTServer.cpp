//
// @file : RESTServer.cpp
// @author : Gooday2die (Isu Kim) @ dev.gooday2die@gmail.com
// @brief : A file that implements all member functions for class RESTServer
//

#include "RESTServer.h"


/**
 * A constructor member function for class RESTServer.
 * This initializes all listeners in REST API using http_listener.open, http_listener.support.
 */
RESTServer::RESTServer() {
    ConfigReader configReader = ConfigReader(); // Read Config file.
    this->configValues = configReader.getConfigValues();
    string tmpAddr = "http://" + configValues.ip + ":" + std::to_string(configValues.port);
    cout << "[+] Starting server at " << tmpAddr << endl;

    this->baseAddress = wstring(tmpAddr.begin(), tmpAddr.end()); // The base URL for API itself.

    this->initListeners(); // Init all http_listener instances.
    this->activateListeners(); // Activate and call .open and .support for all http_listeners.

    this->serverThread = new thread(&RESTServer::startServer, this); // start server thread
    this->serverThread->join(); // join thread
}

/**
 * A destructor member function that destroys class RESTServer.
 * This closes all instances and deletes instances that were generated by activateListeners().
 * Then it will exit(0).
 */
RESTServer::~RESTServer() {
    this->exitFlag = true;
    for (auto const& x : this->endpoints) {
        x.second->listener->close(); // close listener
        delete(x.second->listener); // delete http_listener instance
        delete(x.second); // delete EndPoint instance
    }
    cout << "[+] Stopped server. Press any key to exit" << endl;
    system("pause");
}

/**
 * A member function for class RESTServer that activates all http_listener instances.
 * This member function will do following processes:
 * 1. Generate a http_listener instance.
 * 2. http_listener::open and http_listener::wait http_listener.
 * 3. http_listener::support
 */
void RESTServer::activateListeners() {
    for (auto const &x : this->endpoints) {
        EndPoint* curEndPoint = x.second;
        http_listener* listener;
        listener = new http_listener(curEndPoint->uri);
        listener->open().wait();
        listener->support(curEndPoint->method, curEndPoint->handler);
        curEndPoint->listener = listener;
    }
}

/**
 * A member function for class RESTServer that initializes endpoints for http_listener instances.
 */
void RESTServer::initListeners() {
    this->endpoints.insert(pair<int, EndPoint*>(EndPoints::ConnectionCheck, generateEndPoint(this->baseAddress + U("/general/connection"), methods::GET, RequestHandler::General::connection)));
    this->endpoints.insert(pair<int, EndPoint*>(EndPoints::StopServer, generateEndPoint(this->baseAddress + U("/general/stop_server"), methods::DEL, [this](const http_request &request) {
        this->exitFlag = true;
    })));
}

/**
 * A member function for class RESTServer that generates EndPoint for each endpoints
 * @param argUri : wstring object that represents the endpoint URI
 * @param argMethod : http::method object that represents the method for this endpoint
 * @param argHandler : the function that handles request for this endpoint
 * @return returns a pointer to EndPoint object.
 */
EndPoint* RESTServer::generateEndPoint(const wstring& argUri, const method& argMethod,
                                       const function<void(http_request)>& argHandler){
    EndPoint* newEndPoint;
    newEndPoint = new EndPoint;
    newEndPoint->uri = argUri;
    newEndPoint->method = argMethod;
    newEndPoint->handler = argHandler;
    newEndPoint->listener = nullptr;

    return newEndPoint;
}

/**
 * A member function that starts server.
 * This actually is just a loop that keeps server alive.
 * This member function will check exitFlag and see if it should stop running. When it hit exitFlag, it will call
 * destructor and delete current object.
 */
void RESTServer::startServer() {
    while (!this->exitFlag);
    cout << "[+] Stopping server..." << endl;
    delete this;
}