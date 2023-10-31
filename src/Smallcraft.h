#ifndef SMALLCRAFT_H
#define SMALLCRAFT_H
bool HijackEvent(uint32 eventId, EventMap &oldMap, EventMap &newMap, std::chrono::duration<int64_t, std::milli> newTime = Milliseconds::max(), bool cancelOriginal = true);

#endif // SMALLCRAFT_H
