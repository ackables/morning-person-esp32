#ifndef ROUTINEDATA_H
#define ROUTINEDATA_H

#include <stdlib.h>

#define MAX_TASKS 10


typedef struct {
    float lat;
    float lon;
} Coordinates;

extern Coordinates homeCoords;

typedef enum {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    NUM_DAYS
} Weekday;

extern const char *const weekday_names[NUM_DAYS];

// A single task entry
typedef struct {
    int id;          // id for day order reference
    char name[32];   // Fixed size for simplicity
    int time;        // Duration of task
} Task;

// Travel destination
typedef struct {
    Coordinates coords;     // Can store coordinates
    int travelTime;      // Could be set via GMap API using home and coords
} Destination;

// A single day's plan
typedef struct {
    Weekday weekday;
    int task_id[MAX_TASKS];      // Max 10 task ids, accessed sequentially
    int task_count;       // number of tasks for that day
    Destination dest;    // Destination for that day
    char name[16];       // Name of the day or plan
    int destArrival;     // Time to be at destination
} Day;

extern Day day_list[NUM_DAYS];
extern int day_count;

extern Task  task_list[MAX_TASKS];
extern int   task_count;

#endif