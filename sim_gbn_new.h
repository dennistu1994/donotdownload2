#pragma once
#include "events.h"
#include "sender_gbn.h"
#include "receiver.h"
#include <limits>
#include "rng.h"
class SIM_GBN {
    public:
    double to;
    int header_length;
    int packet_length;
    int link_rate;
    int num_success;
    double propagation_delay;
    double bit_error_rate;
    int n;
    int mod;

    Sender* sender;
    Receiver* receiver;

    double tc;
    int current_success;
    Event* timeout;
    EventList* events;
    EventList* buffer;
    RNG* rng;

    SIM_GBN(double to, int header_length, int packet_length, int link_rate, int num_success, double propagation_delay, double bit_error_rate, int n){
        this->to = to;
        this->header_length = header_length;
        this->packet_length = packet_length;
        this->link_rate = link_rate;
        this->num_success = num_success;
        this->propagation_delay = propagation_delay;
        this->bit_error_rate = bit_error_rate;
        this->n = n;
        this->mod = n+1;

        this->tc = 0.0;

        this->sender = new Sender();
        this->receiver = new Receiver();

        this->current_success = 0;
        this->timeout = new Event(std::numeric_limits<double>::max(), TIMEOUT, 0, false);
        this->events = new EventList();
        this->buffer = new EventList();

        this->rng = new RNG();
    };

    void start(){
        this->send();
        this->timeout->time = this->tc + ((this->packet_length + this->header_length)/(double)this->link_rate) + this->to;
        int milestone = 500;
        int temp = 500;
        while(this->current_success < this->num_success){
            this->process_next_event();
            if(this->current_success >= temp){
                cout<<temp<<", "<<(*this->events)<<endl;
                temp += milestone;
            };
        }

        cout<<"transferred "<<this->current_success * this->packet_length <<" bits in " << this->tc << " seconds"<<endl;
        cout<<"throughput is "<<this->current_success * this->packet_length / this->tc <<" bits/second"<<endl;
    };

    void send(){
        this->events->put_and_sort(new Event(this->tc, SEND_DATA, this->sender->SN, false));
        this->sender->SN  = (this->sender->SN + 1) % this->mod;
    };

    void update_timeout(){
      this->timeout->time = this->buffer->get_head_time()+ ((this->packet_length + this->header_length)/(double)this->link_rate) + this->to;
    };

    void process_next_event(){
        Event* event = this->timeout;
        if(this->events->get_head_time() == event->time){
            if(this->events->head->event->event_type != SEND_DATA){
                event = this->events->pop_head()->event;
            }
        } else if(this->events->get_head_time() < event->time){
            event = this->events->pop_head()->event;
        }

        if(event->time > this->tc){
          this->tc = event->time;
        }
        int num_errors;
        cout<<(*event)<<endl<<flush;
        switch(event->event_type){
            case TIMEOUT:
                //go back n
                this->timeout->time = this->tc + ((this->packet_length + this->header_length)/(double)this->link_rate) + this->to;

                this->sender->SN = this->buffer->head->event->value;
                if(this->sender->IN_FLIGHT == this->n){
                  //next will be sent
                  this->send();
                }
                this->sender->IN_FLIGHT = 0;
                //delete old eventlist if time allows
                this->buffer = new EventList();
            break;
            case SEND_DATA:
                //schedule DATA_DEPARTURE
                this->tc = event->time + ((this->packet_length + this->header_length)/(double)this->link_rate);
                this->sender->IN_FLIGHT += 1;
                this->buffer->put(event);
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
                if(this->sender->IN_FLIGHT < this->n){
                    this->send();
                }
                break;
            case DATA_ARRIVAL:
                if(event->value == this->receiver->NEXT_EXPECTED_FRAME && !event->error){
                    this->receiver->NEXT_EXPECTED_FRAME = (this->receiver->NEXT_EXPECTED_FRAME + 1) % this->mod;
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
                if(event->value != this->sender->BAD_ACK && !event->error){
                    int bulk = event->value - this->sender->BAD_ACK;
                    if(bulk < 0){
                        bulk += this->mod;
                    }
                    this->sender->BAD_ACK = (this->sender->BAD_ACK + bulk) % this->mod;
                    this->sender->IN_FLIGHT -= bulk;
                    this->current_success += bulk;
                    for(int j = 0; j<bulk;j++){
                        this->buffer->pop_head();
                    }
                    this->update_timeout();
                    //cout<<this->current_success<<endl<<flush;
                    if(this->current_success < this->num_success){
                        this->send();
                    }
                } else {
                    //error, wait for time out
                }
                break;
        }
        //getchar();

    };
};
