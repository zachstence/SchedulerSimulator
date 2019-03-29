/**
 * @author zachstence / zms22
 * @since 3/29/2019
 */

#ifndef CS4328_PROJECT1_EVENTQUEUE_H
#define CS4328_PROJECT1_EVENTQUEUE_H

#include <set>
#include "Process.h"

using std::set;

/**
 * Enumeration of event types
 * Arrival: A processes' arrival.
 * Departure: A process' departure.
 * Timeout: An event that occurs in round robin simulation after each quantum.
 * Query: An event that occurs in order to capture real-time statistics about the simulation.
 */
enum EventType {
    ARRIVAL, DEPARTURE, TIMEOUT, QUERY
};

/**
 * A class holding information event information such as the time it occurs, the process it corresponds to, and the
 * type of the event.
 */
class Event {
public:

    Event(double time, Process* process, EventType type) {
        this->time = time;
        this->process = process;
        this->type = type;
    }

    double getTime() { return this->time; }
    Process* getProcess() { return this->process; }
    EventType getType() { return this->type; }

    /**
     * Function for comparing two events. Events are compared based on their time (unless their times are equal, then
     * event type is used as an arbitrary tie-breaker).
     * @param other The event to compare this event to.
     * @return True if this event is less than other, otherwise false.
     */
    bool operator<(const Event& other) const {
        if (this->time != other.time)
            return this->time < other.time;
        else
            return this->type < other.type;
    }

private:
    double time;
    Process* process;
    EventType type;

};

/**
 * A class to hold events in a queue-like structure ordered by the time they will occur.
 */
class EventQueue {

public:
    /**
     * Creates an Event and adds it to the EventQueue.
     * @param time The time the event occurs.
     * @param p The process the event corresponds to.
     * @param type The type of the event.
     */
    void scheduleEvent(double time, Process* p, EventType type) {
        Event e (time, p, type);
        events.insert(e);
    }

    /**
     * Unschedules (removes) a departure event (if it exists) from the EventQueue.
     * @param id The departure's corresponding process' ID.
     * @return True if the departure was successfully removed from the EventQueue, false if the departure wasn't present
     * in the EventQueue to begin with.
     */
    bool unscheduleDeparture(int id) {
        for (Event event : events) {
            if (event.getType() == DEPARTURE && event.getProcess()->getId() == id) {
                events.erase(event);
                return true;
            }
        }
        return false;
    }

    /**
     * Gets the next event (the one with the soonest time) and removes it from the EventQueue.
     * @return The next event.
     */
    Event getEvent() {
        Event event = *events.begin();
        events.erase(events.begin());
        return event;
    }

    /**
     * Determines whether or not the EventQueue is empty.
     * @return True if the EventQueue is empty (has no events), false otherwise.
     */
    bool empty() {
        return events.empty();
    }

private:
    set<Event> events;

};


#endif //CS4328_PROJECT1_EVENTQUEUE_H
