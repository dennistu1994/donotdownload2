#pragma once
#include "events.h"
#include "sender.h"
#include "receiver.h"
class SIM_ABP {
    public:
        double to;
        int packet_length;
        int link_rate;
        int num_success;

        Sender* sender;
        Receiver* receiver;

        double tc;
        int current_success;
        Event* timeout;
        EventList* events;

        ES(double to, int packet_length, int link_rate, int num_success){
            this->to = to;
            this->packet_length = packet_length;
            this->link_rate = link_rate;
            this->num_success = num_success;            

            this->tc = 0.0;
            
            this->sender = new Sender();
            this->receiver = new Receiver();            

            this->current_success = 0;
            this->events = new EventList(); 
        };

        void start(){
            this->send();
            this->process_next_event();
        };

        void send(){
            Event* event = new Event(this->tc, SEND_DATA, this->sender->SN);
            this->events->put(event);
        };

        void process_next_event(){
            Event* next = this->timeout;
            if(this->events->get_head_time() < next->time){
                next = this->events->pop_head();
            }
            
            this->tc = next->time;

            switch(next->type){
                case TIMEOUT:
                    break;
                case SEND_DATA:
                    //schedule DATA_DEPARTURE
                    this->events->put(new Event(tc + (this->packet_length/(double)this->link_rate)));
                    break;
                case DATA_DEPARTURE:
                    break;
                case DATA_ARRIVAL:
                    break;
                case ACK_DEPARTURE:
                    break;
                case ACK_ARRIVAL:
                    break;
            }
        };
}
