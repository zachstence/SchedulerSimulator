//
// Created by zach on 3/18/19.
//

#ifndef CS4328_PROJECT1_EVENTQUEUE_H
#define CS4328_PROJECT1_EVENTQUEUE_H

#include <set>
#include "Process.h"

using std::set;

enum EventType {
    ARRIVAL, DEPARTURE, TIMEOUT, QUERY
};

struct Event {
    double time;
    Process* process;
    EventType type;

    bool operator<(const Event& other) const {
        return this->time < other.time;
    }
};

class EventQueue {
private:
    set<Event> events;

public:
    void scheduleEvent(double time, Process* p, EventType type) {
        events.insert({time, p, type});
    }

    bool unscheduleDeparture(int id) {
        for (Event event : events) {
            if (event.process->getId() == id) {
                events.erase(event);
                return true;
            }
        }
        return false;
    }

    /**
     * Gets the next event and removes it from the EventQueue.
     * @return The next event.
     */
    Event getEvent() {
        Event event = *events.begin();
        events.erase(events.begin());
        return event;
    }

    bool empty() {
        return events.empty();
    }
};


#endif //CS4328_PROJECT1_EVENTQUEUE_H
