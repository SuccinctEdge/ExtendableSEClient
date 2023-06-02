///usr/include
// Created by weiqin xu on 19/11/2020.
//

#include "TCPReceiver.hpp"


std::vector<std::string> app_buff;
bool interrupt_flag = false;
std::thread receiver;
std::thread query_thread;
std::mutex *lock_tcp;
std::vector<std::vector<std::string>> query = {};
std::vector<std::string> signal = {};
std::vector<std::string> query_buffer = {};
//std::string url = "localhost";
const char* url = "192.168.1.22";

std::string client_id = "testid";
// Harcode to test
long long time_reception;
struct mosquitto *mosq;
struct mosquitto *mosq_query;
std::regex regex("\\,");
bool isServer = false;
bool started_receiving = false;
bool data_waiting = false;
std::unordered_map<std::string, std::vector<std::vector<std::string>>> map;

std::vector<std::string> split(const std::string& input, const std::string& regex) {
    std::regex re(regex);
    std::sregex_token_iterator
            first{input.begin(), input.end(), re, -1},
            last;
    return {first, last};
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
    char buf[1024];
    char topic_buf[1024];
    memcpy(buf, message->payload, 1024 * sizeof(char));
    memcpy(topic_buf, message->topic, 1024 * sizeof(char));
    std::string str = buf;
    std::string topic = topic_buf;

    str.erase(0,1);
    str.erase(str.end()-1,str.end());
    std::vector<std::string> out(
            std::sregex_token_iterator(str.begin(), str.end(), regex, -1),
            std::sregex_token_iterator()
    );

    lock_tcp->lock();
    //std::cout << "Received stuff" << std::endl;
    if(map.find(topic) == map.end())
        map[topic] = {};

    map[topic].push_back(out);
    data_waiting = true;
    lock_tcp->unlock();
}

void receive_query(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
    char buf[1024];
    memcpy(buf, message->payload, 1024 * sizeof(char));
    std::string tmp = buf;
    if(tmp.rfind("__start__",0) == 0){
        int pos = tmp.find('|');
        if(pos == -1){
            started_receiving = true;
        }
        else{
            tmp = tmp.substr(pos + 1);
            std::vector<std::string> ids(
                    std::sregex_token_iterator(tmp.begin(), tmp.end(), regex, -1),
                    std::sregex_token_iterator()
            );
            for(auto id : ids){
                if(id == client_id) {
                    started_receiving = true;
                    break;
                }
            }
        }
    }

    else if(started_receiving && tmp.find("__signal__") != std::string::npos){
            size_t start_clause = tmp.find('(') + 1;
            size_t end_clause = tmp.find(')');
            signal.push_back(tmp.substr(start_clause, end_clause - start_clause));
    }
    else if(started_receiving && tmp == "__end__"){
        lock_tcp->lock();
        std::vector<std::string> copy = {};
        std::copy(query_buffer.begin(), query_buffer.end(), std::back_inserter(copy));
        query.push_back(copy);
        lock_tcp->unlock();
        query_buffer.clear();
    }
    else if(started_receiving){
        query_buffer.push_back(tmp);
    }
}

void tcpReceiver(){

    mosq = mosquitto_new(NULL, true, NULL);
    mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
    mosquitto_connect(mosq, url, 1883, 1000);
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_subscribe_v5(mosq, NULL, "test", 2, 0, NULL);

    mosq_query = mosquitto_new(NULL, true, NULL);
    mosquitto_int_option(mosq_query, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
    mosquitto_connect(mosq_query, url, 1883, 1000);
    mosquitto_message_callback_set(mosq_query, receive_query);
    mosquitto_subscribe_v5(mosq_query, NULL, "query", 2, 0, NULL);
    mosquitto_loop_forever(mosq_query, -1, 1);
}

void mosquitto_looping(){
     mosq = mosquitto_new(NULL, true, NULL);
     mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
     mosquitto_connect(mosq, url, 1883, 1000);
     mosquitto_message_callback_set(mosq, on_message);

     for(int i = 1; i <= 5; i++){
         mosquitto_subscribe_v5(mosq, NULL, ("Topic" + std::to_string(i)).c_str(), 2, 0, NULL);
     }


    std::cout << "Mosquitto loop running" << std::endl;
    mosquitto_loop_forever(mosq, 60000, 1);
}

void report_anomaly(std::string anom){
    struct mosquitto *mosq_send = mosquitto_new(NULL, true, NULL);
    mosquitto_connect(mosq_send, url, 1883, 1000);
    mosquitto_publish_v5(mosq_send, NULL, "anomaly", anom.length(), anom.c_str(), 0, false, NULL);
}

std::tuple<std::string,std::vector<std::string>> getQueries(){
    while(query.size() == 0){
        usleep(1000);
    }
    auto tmp = query[0];
    auto sign = signal[0];
    query.erase(query.begin());
    signal.erase(signal.begin());
    return std::make_tuple(sign,tmp);
}

std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> getCSV(){
    lock_tcp->lock();
    // No data has been received yet
    if(!data_waiting){
        lock_tcp->unlock();
        return {};
    }
    data_waiting = false;
    std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> tmp = {};

    // Creates a tuple for every sensor
    for(auto kv : map){
        std::string topic;
        std::vector<std::vector<std::string>> content;
        std::tie(topic, content) = kv;
        tmp.push_back(std::tuple<std::string, std::vector<std::vector<std::string>>>{topic, content});

        // Reset column names for next rows
        map[topic] = {};
    }


    lock_tcp->unlock();
    return tmp;
}


// Lance la Thread qui reçoit les données par TCP
void run_tcp_receiver(bool server, std::mutex *mutex) {
    isServer = server;
    lock_tcp = mutex;
    mosquitto_lib_init();
    receiver = std::thread (tcpReceiver);
    query_thread = std::thread (mosquitto_looping);
}

// Arrête la thread TCP
void stop_tcp_receiver(){
    mosquitto_disconnect(mosq);
    receiver.join();
}

std::string erase_head(std::string str){
    //std::cout << "current str:" << str << " ";
    while(str.size()>0 && !(str.front() >= '0' && str.front() <= '9'))
        str.erase(str.begin());

    while(str.size()>0 && !(str.back() >= '0' && str.back() <= '9'))
        str.erase(str.end()-1);
    //std::cout << str << " ";
    return str;
}
