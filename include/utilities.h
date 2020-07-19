#ifndef UTILITIES_H
#define UTILITIES_H

#include <math.h>
#include <QVector>
#include <QString>

#define M_PI 3.14159265

float sind(float angleDegree);

float cosd(float angleDegree);

QVector<int> secondsToHoursMinSeconds(int totalSeconds);

QString secondsToStringHoursMinSeconds(int totalSeconds);

QString padTime(int time);

#endif // UTILITIES_H
