#define SIO_TLS

#include <sio_client.h>

#include <condition_variable>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "rapidjson/document.h"
#include <string>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#define MAIN_FUNC int main(int argc ,const char* args[])

using namespace sio;
using namespace std;
using namespace rapidjson;

std::mutex _lock;

std::condition_variable_any _cond;
bool connect_finish = false;

class connection_listener{
    sio::client &handler;
public:
    connection_listener(sio::client& h) :handler(h){}
    void on_connected(){
        _lock.lock();
        _cond.notify_all();
        connect_finish = true;
        _lock.unlock();
    }
    void on_close(client::close_reason const& reason){
        exit(0);
    }
    void on_fail(){
        exit(0);
    }
};
socket::ptr current_socket;
// This function is the Socket.io notification listener
void bind_events(){
    current_socket->on("message", sio::socket::event_listener_aux([&](string const& name, message::ptr const& data, bool isAck, message::list &ack_resp){
        _lock.lock();
        string jsonData = data->get_string();

        // Parse JSON string for "action". Action: "start" for receive video call notification. "end" for end call.
        Document d;
        d.Parse<0>(jsonData.c_str());
        Value& actionValue = d["action"];

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        actionValue.Accept(writer);

        string actionString = buffer.GetString();

        if ( actionString.front() == '"' ) {
            actionString.erase( 0, 1 );
            actionString.erase( actionString.size() - 1 );
        }

        std::cout << "New Incoming Notification: " + actionString << std::endl;
        std::cout << jsonData << std::endl;

        if (actionString == "start"){ // Start Call notification
            Document p;
            p.Parse<0>(jsonData.c_str());
            Value& payloadValue = p["payload"];
            StringBuffer bufferPayload;
            Writer<StringBuffer> writerPayload(bufferPayload);
            payloadValue.Accept(writerPayload);
            string payloadString = bufferPayload.GetString();
            std::cout << payloadString << std::endl;

            Document n;
            n.Parse<0>(payloadString.c_str());
            Value& nameValue = n["caller_full_name"];

            StringBuffer bufferName;
            Writer<StringBuffer> writerName(bufferName);
            nameValue.Accept(writerName);

            string nameString = bufferName.GetString(); //Name of Caller for Video Call Notification Modal
            if ( nameString.front() == '"' ) {
                nameString.erase( 0, 1 );
                nameString.erase( nameString.size() - 1 );
            }
            std::cout << nameString << std::endl;

            //Start LG System Notification for video call notification modal.
            //Pass "jsonData" JSON string to the Independa app
        } else if (actionString == "end"){ // End Video call notification
            //If Independa app is inactive and LG notification is active.
            // Stop any video call notification in progress. Caller has already ended the call.
        } else if (actionString == "new_photo"){ // New Photo notification
            //Start LG System Notification for new photo notification.
        } else if (actionString == "new_message"){ // New Message notification
            //Start LG System Notification for new message notification.
        }

        _lock.unlock();
    }));
}
MAIN_FUNC{
    sio::client h;
    connection_listener l(h);
    h.set_open_listener(std::bind(&connection_listener::on_connected, &l));
    h.set_close_listener(std::bind(&connection_listener::on_close, &l,std::placeholders::_1));
    h.set_fail_listener(std::bind(&connection_listener::on_fail, &l));
    std::map<std::string, std::string> query;
    query["token"] = "275c1627-224c-4b22-930e-e913176a22d7"; //Token passed in from Independa app. Hardcoded for now.
    h.connect("https://dev-socket.independa.com", query); //Socket.io test server
    _lock.lock();
    if (!connect_finish){
        _cond.wait(_lock);
    }
    _lock.unlock();

    current_socket = h.socket();
//    string msg = "{channel:'independa'}";
//    current_socket->emit("connect", msg);
    bind_events();
    for (std::string line; std::getline(std::cin, line);) {}

    h.sync_close();
    h.clear_con_listeners();
    return 0;
}