#define COUNTDOWN_DONE_MSG 1

struct TimerBlock {
    EXCHANGE e;
    uint32_t CountDown;
} timerblocks[20];

void TimerIRQ(void)
{
    uint8_t i;

    for (i = 0; i < 20; i++)
        if (timerblocks[i].CountDown > 0) {
            timerblocks[i].CountDown--;
            if (timerblocks[i].CountDown == 0)
                SendMessage(timerblocks[i].e, COUNTDOWN_DONE_MESSAGE);
        }
}

void Sleep(uint32_t delay)
{
    struct TimerBlock *t;

    if ((t = findTimerBlock()) == nil)
        return;
    t->CountDown = delay;
    WaitForMessageFrom(t->e = getCrntExch());
}
