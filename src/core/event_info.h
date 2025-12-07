#ifndef EVENT_INFO_H_INCLUDED
#define EVENT_INFO_H_INCLUDED

#include <span>
#include <vector>
#include <CL/opencl.hpp>

struct event_info
{
	cl::Event event;
	const char* name;
	enum type_t {
		kernel,
		fill_buffer,
		write_buffer,
		copy_buffer,
		read_buffer
	} type;

	event_info(const char* n, type_t t, cl::Event e)
        : event(std::move(e)), name(n), type(t) {}

    // Delete copy
    event_info(const event_info&) = delete;
    event_info& operator=(const event_info&) = delete;

    // Default move
    event_info(event_info&&) = default;
    event_info& operator=(event_info&&) = default;
};

struct EventLog {
    std::vector<event_info> events;

	// Add multiple events (rvalue, move)
    void add(std::vector<event_info>&& evts) {
		events.insert(events.end(),
					std::make_move_iterator(evts.begin()),
					std::make_move_iterator(evts.end()));
	}

	// Add a single event (by value, moved)
    void add(event_info ev) {
        events.push_back(std::move(ev));
    }

    void reserve(std::size_t n) {
        events.reserve(n);
    }

    auto& get() { return events; }
    const auto& get() const { return events; }
};


#endif // EVENT_INFO_H_INCLUDED
