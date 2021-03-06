#pragma once
#include <iostream>
#include <limits>
using namespace std;
enum EventType {TIMEOUT, SEND_DATA, DATA_DEPARTURE, DATA_ARRIVAL, ACK_DEPARTURE, ACK_ARRIVAL};
class Event{
  public:
    double time;
    EventType event_type;
    int value;
    bool error;

    Event(double time, EventType event_type, int value, bool error){
        this->event_type = event_type;
        this->time = time;
        this->value = value;
        this->error = error;
    };

    bool operator<(const Event& other) const {
        if(this->time == other.time){
            if(other.event_type == DATA_DEPARTURE){
              return false;
            } else if(other.event_type == SEND_DATA){
              return this->event_type == ACK_ARRIVAL;
            } else {
              return true;
            }
        } else {
          return (this->time < other.time);
        }
    };

    friend std::ostream& operator<<(std::ostream & Str, const Event& event);
};

class EventNode{
  public:
    Event* event;
    EventNode* prev = NULL;
    EventNode* next = NULL;

    EventNode(Event& event){
      this->event = &event;
    };

    //move towards the head of the list
    void move_forward(){
      if(this->prev != NULL){
        Event* temp = this->event;
        this->event = this->prev->event;
        this->prev->event = temp;
      }
    };
};

class EventList{
  public:
    EventNode* head = NULL;
    EventNode* tail = NULL;
    int size = 0;

    void put(Event* event){
      EventNode* node = new EventNode(*event);
      if(this->size == 0){
        this->head = node;
        this->tail = node;
      } else {
        this->tail->next = node;
        node->prev = this->tail;
        this->tail = node;
      }
      this->size++;
    };

    void put_and_sort(Event* event){
      this->put(event);
      EventNode* temp = this->tail;
      while(temp->prev!=NULL && *(temp->event)<*(temp->prev->event)){
        temp->move_forward();
        temp = temp->prev;
      }
    };

    void put(EventList* event_list){
      if(event_list == NULL || event_list->size < 1){
        return;
      }
      if(this->size == 0){
        this->head = event_list->head;
        this->tail = event_list->tail;
      } else {
        this->tail->next = event_list->head;
        event_list->head->prev = this->tail;
        this->tail = event_list->tail;
      }
      this->size += event_list->size;
    };

    void sort_by_time(){
      if(this->size < 2){
        return;
      } else {
        EventNode* temp = this->head->next;
        EventNode* next = temp->next;
        while(true){
          while(temp->prev!=NULL && *(temp->event)<*(temp->prev->event)){
            temp->move_forward();
            temp = temp->prev;
          }
          temp = next;
          if(temp == NULL){
            break;
          }
          next = temp->next;
        }
      }
    };

    EventNode* pop_head(){
      if(this->size < 1){
        return NULL;
      }
      EventNode* result = this->head;
      this->head = result->next;
      result->next = NULL;
      if(this->head != NULL){
        this->head->prev = NULL;
      } else {
        this->tail = NULL;
      }
      this->size--;
      return result;
    };

    double get_head_time(){
      if(this->size > 0){
        return this->head->event->time;
      } else {
        return numeric_limits<double>::max();
      }
    };

    EventList* clone(){
      EventList* result = new EventList();
      EventNode* temp = this->head;
      while (temp != NULL){
        result->put(temp->event);
        temp = temp->next;
      }
      return result;
    };
};

std::ostream & operator<<(std::ostream & Str, const Event& event) {
  switch (event.event_type){
    case TIMEOUT:
      Str << "TIMEOUT";
      break;
    case SEND_DATA:
      Str << "SEND_DATA";
      break;
    case DATA_DEPARTURE:
      Str << "DATA_DEPARTURE";
      break;
    case DATA_ARRIVAL:
      Str << "DATA_ARRIVAL";
      break;
    case ACK_DEPARTURE:
      Str << "ACK_DEPARTURE";
      break;
    case ACK_ARRIVAL:
      Str << "ACK_ARRIVAL";
      break;
    default:
      Str << "UNKNOWN";
  }
  Str<<'('<<event.value<<')'<<'('<<event.time<<')';
}

std::ostream & operator<<(std::ostream & Str, const EventList& list) {
  EventNode* temp = list.head;
  while (temp != NULL){
    Str << *(temp->event);
    temp = temp->next;
    if(temp!=NULL){
      Str << ", ";
    }
  }
  Str << '[' << list.size << ']';
  return Str;
}
