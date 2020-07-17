#include "utilities.h"
#include <qdebug.h>

float sind(float angleDegree)
{
    float angleradians = (float) (angleDegree * (M_PI / 180.0f));
    return (float) (sin(angleradians));
}

float cosd(float angleDegree)
{
    float angleradians = (float) (angleDegree * (M_PI/180.0f));
    return (float) (cos(angleradians));
}


QVector<int> secondsToHoursMinSeconds(int totalSeconds){
    QVector<int> time(3); // hours minutes seconds
    time[0] = totalSeconds/3600; // hours
    time[1] = (totalSeconds % 3600) / 60; // minutes
    time[2] = totalSeconds % 60; // seconds

    return time;
}

QString secondsToStringHoursMinSeconds(int totalSeconds){

    QString timeText;
    int hours = totalSeconds/3600; // hours
    int minutes = (totalSeconds % 3600) / 60; // minutes
    int seconds = totalSeconds % 60; // seconds

    timeText = padTime(hours) + ":" + padTime(minutes) + ":" + padTime(seconds);
    return timeText;
}

QString padTime(int time){
    if(time < 10){
        return "0" + QString::number(time);
    }
    return QString::number(time);
}



