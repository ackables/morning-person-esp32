#ifndef __ROUTINEDATA_H
#define __ROUTINEDATA_H

#include <stdlib.h>

#define MAX_TASKS 10


typedef struct {
    float lat;
    float lon;
} Coordinates;

extern Coordinates homeCoords;

// Enum for days of the week
enum Weekdays { Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday };

// A single task entry
typedef struct {
    char name[32];   // Fixed size for simplicity
    int time;        // Duration of task
} Task;

// Travel destination
typedef struct {
    char name[32];
    char coords[32];     // Can store coordinates as "lat,long" string
    int travelTime;      // Could be set via GMap API using home and coords
} Destination;

// A single day's plan
typedef struct {
    Task order[10];      // Max 10 tasks, accessed sequentially
    Destination dest;    // Destination for that day
    char name[16];       // Name of the day or plan
    int destArrival;     // Time to be at destination
} Day;


#endif