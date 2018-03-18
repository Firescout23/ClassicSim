#ifndef ENCOUNTERSTART_H
#define ENCOUNTERSTART_H

#include "Event.h"

class Character;

class EncounterStart: public Event {
public:
    EncounterStart(Character*);

    virtual void act(void) override;
protected:
private:
    Character* pchar;
};

#endif // ENCOUNTERSTART_H