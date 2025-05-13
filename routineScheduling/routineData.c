// Copyright 2025
// Rights Reserved
// Benjamin Smith
// 05/12/2025
//
// Creates structures needed to store routine data

#include <stdlib.h>
#include "routineData.h"

Coordinates homeCoords = {0.0f, 0.0f};

// #define MAX_TASKS 10
Task task_list[MAX_TASKS];
int task_count = 0;


const char *const weekday_names[NUM_DAYS] = {
    "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday", "Sunday"
};

Day day_list[NUM_DAYS] = { 0 };
int day_count = NUM_DAYS;

// // A single task entry
// typedef struct {
//     char name[32];   // Fixed size for simplicity
//     int time;        // Duration of task
// } Task;

// // Travel destination
// typedef struct {
//     char name[32];
//     char coords[32];     // Can store coordinates as "lat,long" string
//     int travelTime;      // Could be set via GMap API using home and coords
// } Destination;

// // A single day's plan
// typedef struct {
//     Task order[10];      // Max 10 tasks, accessed sequentially
//     Destination dest;    // Destination for that day
//     char name[16];       // Name of the day or plan
//     int destArrival;     // Time to be at destination
// } Day;
