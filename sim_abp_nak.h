#pragma once
#include "events.h"
#include "sender.h"
#include "receiver.h"
#include "rng.h"
class SIM_ABP_NAK {
    public:
    double to;
    int header_length;
    int packet_length;
    int link_rate;
    int num_success;
    double propagation_delay;
    double bit_error_rate;

    Sender* sender;
    Receiver* receiver;

    double tc;
    int current_success;
    Event* timeout;
    EventList* events;
    RNG* rng;

    SIM_ABP_NAK(double to, int header_length, int packet_length, int link_rate, int num_success, double propagation_delay, double bit_error_rate){
        this->to = to;
        this->header_length = header_length;
        this->packet_length = packet_length;
        this->link_rate = link_rate;
        this->num_success = num_success;
        this->propagation_delay = propagation_delay;
        this->bit_error_rate = bit_error_rate;

        this->tc = 0.0;

        this->sender = new Sender();
        this->receiver = new Receiver();

        this->current_success = 0;
        this->timeout = new Event(2*to, TIMEOUT, 0, false);
        this->events = new EventList();
        this->rng = new RNG();
    };

    double start(){
        this->send();
        while(this->current_success < this->num_success){
            this->process_next_event();
        }

      return this->current_success * this->packet_length / this->tc;
    };

    void send(){
        this->events->put_and_sort(new Event(this->tc, SEND_DATA, this->sender->SN, false));
    };

    void process_next_event(){
        Event* event = this->timeout;
        if(this->events->get_head_time() <= event->time){
            event = this->events->pop_head()->event;
        }

        this->tc = event->time;
        int num_errors;
        //cout<<(*event)<<endl<<flush;
        switch(event->event_type){
            case TIMEOUT:
            this->send();
            break;
            case SEND_DATA:
            //schedule DATA_DEPARTURE
            this->events->put_and_sort(new Event(this->tc + ((this->packet_length + this->header_length)/(double)this->link_rate), DATA_DEPARTURE, this->sender->SN, false));
            //change timeout time
            this->timeout->time = this->tc + ((this->packet_length + this->header_length)/(double)this->link_rate) + to;
            break;
            case DATA_DEPARTURE:
            //determine if frame gets through channel error free
            num_errors = 0;
            for(int i = 0;i<(this->packet_length + this->header_length);i++){
                if(this->rng->get_next() < this->bit_error_rate){
                    num_errors += 1;
                }
            }
            if(num_errors < 1){
                this->events->put_and_sort(new Event(this->tc + this->propagation_delay, DATA_ARRIVAL, event->value, false));
                } else if (num_errors < 5){
                this->events->put_and_sort(new Event(this->tc + this->propagation_delay, DATA_ARRIVAL, event->value, true));
                } else {
                //event dropped
            }
            break;
            case DATA_ARRIVAL:
            if(event->value == this->receiver->NEXT_EXPECTED_FRAME && !event->error){
                this->receiver->NEXT_EXPECTED_FRAME = 1 - this->receiver->NEXT_EXPECTED_FRAME;
            }
            this->events->put_and_sort(new Event(this->tc + (this->header_length/(double)this->link_rate), ACK_DEPARTURE, this->receiver->NEXT_EXPECTED_FRAME, false));
            break;
            case ACK_DEPARTURE:
            num_errors = 0;
            for(int i = 0;i<this->header_length;i++){
                if(this->rng->get_next() < this->bit_error_rate){
                    num_errors += 1;
                }
            }
            if(num_errors < 1){
                this->events->put_and_sort(new Event(this->tc + this->propagation_delay, ACK_ARRIVAL, event->value, false));
                } else if (num_errors < 5){
                this->events->put_and_sort(new Event(this->tc + this->propagation_delay, ACK_ARRIVAL, event->value, true));
                } else {
                //event dropped
            }
            break;
            case ACK_ARRIVAL:
            if(event->value == this->sender->NEXT_EXPECTED_ACK && !event->error){
                //empty buffer, increment SN and NEXT_EXPECTED_ACK
                this->sender->SN = 1-this->sender->SN;
                this->sender->NEXT_EXPECTED_ACK = 1 - this->sender->NEXT_EXPECTED_ACK;
                this->current_success += 1;
                //cout<<this->current_success<<endl<<flush;
                if(this->current_success < this->num_success){
                    this->send();
                }
                } else {
                  this->send();
                //error, wait for time out
            }
            break;
        }
    };
};
