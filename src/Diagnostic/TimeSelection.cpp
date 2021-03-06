#include "TimeSelection.h"
#include "PyTools.h"
#include <math.h>

using namespace std;


// Constructor
TimeSelection::TimeSelection(PyObject* timeSelection, string name)
{
    
    start   = 0.;
    end     = std::numeric_limits<double>::max();
    period  = 1.;
    repeat  = 1;
    spacing = 1.;
    
    // If the selection is an int
    if( PyNumber_Check(timeSelection) ) {
        
        // Interpret as the period
        if( !PyTools::convert(timeSelection, period) )
            ERROR(name << ": time selection must be a number or a list of numbers");
        // If zero, no output, ever
        if( !period ) {
            start = std::numeric_limits<double>::max();
            return;
        }
        
    }
    // If the selection is a list or tuple
    else if( PyTuple_Check(timeSelection) || PyList_Check(timeSelection) ) {
        
        PyObject* seq = PySequence_Fast(timeSelection, "expected a sequence");
        int nitems = PySequence_Size(seq);
        
        if ( nitems<1 )
            ERROR(name << ": time selection must have at least one argument");
        if ( nitems>5 )
            ERROR(name << ": time selection must not have more than 5 arguments");
        
        // Extract all values
        vector<double> items (nitems);
        for( int i=0; i<nitems; i++ )
            if (!PyTools::convert(PySequence_Fast_GET_ITEM(seq, i), items[i]))
                ERROR(name << ": time selection must be a list of numbers");
        
        // Depending on the number of values, they mean different things (see doc)
        if        ( nitems==1 ) {
            if(items[0]) period  = items[0];
        } else if ( nitems==2 ) {
            if(items[0]) start   = (int)round(items[0]);
            if(items[1]) period  = items[1];
        } else if ( nitems==3 ) {
            if(items[0]) start   = (int)round(items[0]);
            if(items[1]) end     = (int)round(items[1]);
            if(items[2]) period  = items[2];
        } else if ( nitems==4 ) {
            if(items[0]) start   = (int)round(items[0]);
            if(items[1]) end     = (int)round(items[1]);
            if(items[2]) period  = items[2];
            if(items[3]) repeat  = (int)round(items[3]);
        } else if ( nitems==5 ) {
            if(items[0]) start   = (int)round(items[0]);
            if(items[1]) end     = (int)round(items[1]);
            if(items[2]) period  = items[2];
            if(items[3]) repeat  = (int)round(items[3]);
            if(items[4]) spacing = items[4];
        }
        
        Py_DECREF(seq);
        
    } else {
        
        ERROR(name << ": time selection (`every`) must be a number or a list of numbers");
        
    }
    
    // Calculate the smallest interval
    SmallestInterval = (repeat<=1) ? ((int)period) : ((int)spacing);
    // Calculate the group width
    groupWidth = (repeat-1)*spacing + 1.;
    
    // Verify a few things
    if( period<1. )
        ERROR(name << ": time selection's period must be >= 1.");
    if( repeat<1 )
        ERROR(name << ": time selection's repeat must be >= 1");
    if( spacing<1. )
        ERROR(name << ": time selection's spacing must be >= 1.");
    if( groupWidth > period )
        ERROR(name << ": time selection must have repeat*spacing<period");
    
}


// Empty time selection
TimeSelection::TimeSelection()
{
    start   = std::numeric_limits<double>::max();
    end     = std::numeric_limits<double>::max();
    period  = 1.;
    repeat  = 1;
    spacing = 1.;
    groupWidth   = 0.;
    TheTimeIsNow = false;
    NextTime     = std::numeric_limits<int>::max();
    PreviousTime = std::numeric_limits<int>::max();
}

// Cloning Constructor
TimeSelection::TimeSelection(TimeSelection * timeSelection)
{
    start        = timeSelection->start       ;
    end          = timeSelection->end         ;
    period       = timeSelection->period      ;
    repeat       = timeSelection->repeat      ;
    spacing      = timeSelection->spacing     ;
    groupWidth   = timeSelection->groupWidth  ;
    TheTimeIsNow = timeSelection->TheTimeIsNow;
    NextTime     = timeSelection->NextTime    ;
    PreviousTime = timeSelection->PreviousTime;
}


// Tell whether the current timestep is within the selection
bool TimeSelection::theTimeIsNow(int timestep)
{
    TheTimeIsNow = false;
    // In selection if inside the start/end bounds
    if( timestep>=start && timestep<=end ) {
        // Calculate the number of timesteps since the start
        double t = (double)timestep - start + 0.5;
        // Calculate the time from the previous period
        t -= floor(t/period)*period;
        // If within group, calculate the time to the previous repeat
        if( t < groupWidth ) t -= floor(t/spacing)*spacing;
        // The time is now if closest repeat it within 0.5
        if( t < 1. ) TheTimeIsNow = true;
    }
    return TheTimeIsNow;
}


// Tell what is the next timestep within the selection
// Returns the same timestep if already within the selection
int TimeSelection::nextTime(int timestep)
{
    if( timestep<=start ) { 
        NextTime = start;
    } else if( timestep>end ) { 
        NextTime = std::numeric_limits<int>::max();
    } else {
        double t = (double)(timestep)-start + 0.5; // number of timesteps since the start
        double p = floor(t/period)*period; // previous period
        double T = t - p; // time to the previous period
        
        // If within group
        if( T < groupWidth ) {
            double r = floor(T/spacing)*spacing; // previous repeat
            if( T-r < 1. ) { NextTime = timestep; } // return current timestep if good
            else { NextTime = (int) (start + round(p + r + spacing)); } // otherwise, return next repeat
        // If after group, return next group's beginning
        } else {
            NextTime = (int) (start + round(p+period));
        }
    }
    return NextTime;
}


// Tell what is the previous timestep within the selection
// Returns the same timestep if already within the selection
int TimeSelection::previousTime(int timestep)
{
    if( timestep<start ) {
        PreviousTime = std::numeric_limits<int>::min();
    } else if( timestep>=end ) {
        PreviousTime = end;
    } else {
        double t = (double)(timestep)-start + 0.5; // number of timesteps since the start
        double p = floor(t/period)*period; // previous period
        double T = t - p; // time to the previous period
        
        // If within group
        if( T < groupWidth ) {
            PreviousTime = (int) (start + round(p + floor(T/spacing)*spacing)); // return previous good timestep
        // If after group, return end of that group
        } else {
            PreviousTime = (int) (start + round(p + groupWidth - 1));
        }
    }
    return PreviousTime;
}

