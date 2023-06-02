#ifndef SUCCINCTEDGE_CATA_TCPRECEIVER_HPP
#define SUCCINCTEDGE_CATA_TCPRECEIVER_HPP

// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <regex>
#include <mutex>
#include <fstream>
#include <mosquitto.h>
#include <mqtt_protocol.h>

#define PORT 25005

//using namespace std;

extern std::vector<std::string> app_buff;
extern bool interrupt_flag;
extern std::thread receiver;
extern std::vector<std::string> columns;
extern std::mutex *lock_tcp;
extern long long time_reception;

std::vector<std::string> split(const std::string& input, const std::string& regex);
std::tuple<std::string,std::vector<std::string>> getQueries();

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void transmit_query(std::vector<std::string> str, std::vector<std::string> ids);
void receive_query(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void mosquitto_looping();

void report_anomaly(std::string anom);
void tcpReceiver();

std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> getCSV();

// Lance la Thread qui reçoit les données par TCP
void run_tcp_receiver(bool server, std::mutex *mutex);

// Arrête la thread TCP
void stop_tcp_receiver();

std::string erase_head(std::string str);



#endif //SUCCINCTEDGE_CATA_TCPRECEIVER_HPP



// Server side C/C++ program to demonstrate Socket programming
