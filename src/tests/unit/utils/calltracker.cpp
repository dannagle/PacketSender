//
// Created by Tomas Gallucci on 6/10/26.
//

#include "calltracker.h"

#include <QString>

void CallTracker::recordCall(const QString& methodName) const
{
    callSequence.push_back(methodName);
    callCounts[methodName]++;
}

const std::vector<QString>& CallTracker::getCallSequence() const
{
    return callSequence;
}

bool CallTracker::wasMethodCalled(const QString& methodName) const
{
    return std::find(callSequence.begin(), callSequence.end(), methodName)
           != callSequence.end();
}

int CallTracker::getCallCount(const QString& methodName) const
{
    return callCounts.value(methodName, 0);
}

void CallTracker::clearCallSequence() const
{
    callSequence.clear();
}
