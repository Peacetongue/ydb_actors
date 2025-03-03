#include "actors.h"
#include "events.h"
#include <library/cpp/actors/core/actor_bootstrapped.h>
#include <library/cpp/actors/core/hfunc.h>
#include <typeinfo>

static auto ShouldContinue = std::make_shared<TProgramShouldContinue>();

/*
Вам нужно написать реализацию TReadActor, TMaximumPrimeDevisorActor, TWriteActor
*/

/*
Требования к TReadActor:
1. Рекомендуется отнаследовать этот актор от NActors::TActorBootstrapped
2. В Boostrap этот актор отправляет себе NActors::TEvents::TEvWakeup
3. После получения этого сообщения считывается новое int64_t значение из strm
4. После этого порождается новый TMaximumPrimeDevisorActor который занимается вычислениями
5. Далее актор посылает себе сообщение NActors::TEvents::TEvWakeup чтобы не блокировать поток этим актором
6. Актор дожидается завершения всех TMaximumPrimeDevisorActor через TEvents::TEvDone
7. Когда чтение из файла завершено и получены подтверждения от всех TMaximumPrimeDevisorActor,
этот актор отправляет сообщение NActors::TEvents::TEvPoisonPill в TWriteActor

TReadActor
    Bootstrap:
        send(self, NActors::TEvents::TEvWakeup)

    NActors::TEvents::TEvWakeup:
        if read(strm) -> value:
            register(TMaximumPrimeDevisorActor(value, self, receipment))
            send(self, NActors::TEvents::TEvWakeup)
        else:
            ...

    TEvents::TEvDone:
        if Finish:
            send(receipment, NActors::TEvents::TEvPoisonPill)
        else:
            ...
*/
class TReadActor: public NActors::TActorBootstrapped<TReadActor>{
    const NActors::TActorId writerId;
    bool end;
    int count;

public:
    TReadActor(const NActors::TActorId writerId) //constructor
            : writerId(writerId), end(false)
    {}

    void Bootstrap() { //entry point
        Become(&TReadActor::StateFunc); //start cond
        Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>()); // send message to itself to start finding the largest simple divisor
    }

    STRICT_STFUNC(StateFunc, { // check cond
        cFunc(NActors::TEvents::TEvWakeup::EventType, HandleWakeup); // init fun when TEvWakeup type message is received
        cFunc(TEvents::TEvDone::EventType, HandleDone); // init fun when TEvDone type message is received
    });

    void HandleWakeup() {
        int64_t value;
        if (std::cin >> value) {
            Register(CreateTMaximumPrimeDevisorActor(SelfId(), writerId, value).Release()); //create new actor
            count++;
            Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
        } else {
            end = true;
            if(count == 0) { //actors were not created
                Send(writerId, std::make_unique<NActors::TEvents::TEvPoisonPill>()); //send event, complete actor work
                PassAway();
            }
        }
    }

    void HandleDone(){ //when actors done
        count--;
        if(end && count == 0){
            Send(writerId, std::make_unique<NActors::TEvents::TEvPoisonPill>());
            PassAway();
        }
    }
};

/*
Требования к TMaximumPrimeDevisorActor:
1. Рекомендуется отнаследовать этот актор от NActors::TActorBootstrapped
2. В конструкторе этот актор принимает:
 - значение для которого нужно вычислить простое число
 - ActorId отправителя (ReadActor)
 - ActorId получателя (WriteActor)
2. В Boostrap этот актор отправляет себе NActors::TEvents::TEvWakeup по вызову которого происходит вызов Handler для вычислений
3. Вычисления нельзя проводить больше 10 миллисекунд
4. По истечении этого времени нужно сохранить текущее состояние вычислений в акторе и отправить себе NActors::TEvents::TEvWakeup
5. Когда результат вычислен он посылается в TWriteActor c использованием сообщения TEvWriteValueRequest
6. Далее отправляет ReadActor сообщение TEvents::TEvDone
7. Завершает свою работу

TMaximumPrimeDevisorActor
    Bootstrap:
        send(self, NActors::TEvents::TEvWakeup)

    NActors::TEvents::TEvWakeup:
        calculate
        if > 10 ms:
            Send(SelfId(), NActors::TEvents::TEvWakeup)
        else:
            Send(WriteActor, TEvents::TEvWriteValueRequest)
            Send(ReadActor, TEvents::TEvDone)
            PassAway()
*/

class TMaximumPrimeDevisorActor: public NActors::TActorBootstrapped<TMaximumPrimeDevisorActor> {
    const NActors::TActorId readerId;
    const NActors::TActorId writerId;
    int64_t value;
    int64_t current;
    int64_t max;

public:
    TMaximumPrimeDevisorActor(const NActors::TActorId readerId, const NActors::TActorId writerId, const int64_t value)
            : readerId(readerId), writerId(writerId), value(value), current(2), max(1) {}

    void Bootstrap() {
        Become(&TMaximumPrimeDevisorActor::StateFunc);
        Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
    }

    STRICT_STFUNC(StateFunc,
    {
        cFunc(NActors::TEvents::TEvWakeup::EventType, HandleWakeup);
    });

    void HandleWakeup() { //calculating the max divisor
        auto startTime = std::chrono::system_clock::now();
        while (value > 1 && std::chrono::system_clock::now() - startTime < TDuration::MilliSeconds(10)) { // check 10 milsec and val>1
            while (value % current == 0) {
                max = current;
                value /= current;
            }
            current++;
        }

        if (value > 1) { //check that val is simple
            Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
        } else {
            if (current - 1 > max) { //check "is max is max?"
                max = current - 1;
            }
            Send(writerId, std::make_unique<TEvents::TEvWriteValueRequest>(max));
            Send(readerId, std::make_unique<TEvents::TEvDone>());
            PassAway();
        }
    };
};


/*
Требования к TWriteActor:
1. Рекомендуется отнаследовать этот актор от NActors::TActor
2. Этот актор получает два типа сообщений NActors::TEvents::TEvPoisonPill::EventType и TEvents::TEvWriteValueRequest
2. В случае TEvents::TEvWriteValueRequest он принимает результат посчитанный в TMaximumPrimeDevisorActor и прибавляет его к локальной сумме
4. В случае NActors::TEvents::TEvPoisonPill::EventType актор выводит в Cout посчитанную локальнкую сумму, проставляет ShouldStop и завершает свое выполнение через PassAway

TWriteActor
    TEvents::TEvWriteValueRequest ev:
        Sum += ev->Value

    NActors::TEvents::TEvPoisonPill::EventType:
        Cout << Sum << Endl;
        ShouldStop()
        PassAway()
*/

    class TWriteActor : public NActors::TActorBootstrapped<TWriteActor> {
        int64_t sum = 0;
    public:
        TWriteActor() {}

        void Bootstrap() {
            Become(&TWriteActor::StateFunc);
            Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
        }

        STRICT_STFUNC(StateFunc,
        {
            hFunc(TEvents::TEvWriteValueRequest, HandleWriteValueRequest)
            cFunc(NActors::TEvents::TEvPoisonPill::EventType, HandlePoisonPill);
        });

        void HandleWriteValueRequest(TEvents::TEvWriteValueRequest::TPtr &event) {
            int64_t value = (*event->Get()).value;
            sum += value;
        }

        void HandlePoisonPill() { //ending
            std::cout << sum << std::endl; //show sum
            ShouldContinue->ShouldStop(); //set flag
            PassAway(); //end
        }
    };


    class TSelfPingActor : public NActors::TActorBootstrapped<TSelfPingActor> {
        TDuration Latency;
        TInstant LastTime;

    public:
        TSelfPingActor(const TDuration &latency)
                : Latency(latency) {}

        void Bootstrap() {
            LastTime = TInstant::Now();
            Become(&TSelfPingActor::StateFunc);
            Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
        }

        STRICT_STFUNC(StateFunc,
        {
            cFunc(NActors::TEvents::TEvWakeup::EventType, HandleWakeup);
        });

        void HandleWakeup() {
            auto now = TInstant::Now();
            TDuration delta = now - LastTime;
            Y_VERIFY(delta <= Latency, "Latency too big");
            LastTime = now;
            Send(SelfId(), std::make_unique<NActors::TEvents::TEvWakeup>());
        }
    };

    THolder <NActors::IActor> CreateSelfPingActor(const TDuration &latency) {
        return MakeHolder<TSelfPingActor>(latency);
    }

    std::shared_ptr <TProgramShouldContinue> GetProgramShouldContinue() {
        return ShouldContinue;
    }

    THolder <NActors::IActor> CreateTReadActor(const NActors::TActorId writerId) {
        return MakeHolder<TReadActor>(writerId);
    }

    THolder <NActors::IActor>CreateTMaximumPrimeDevisorActor(const NActors::TActorIdentity readerId, const NActors::TActorId writerId,
                                    int64_t value) {
        return MakeHolder<TMaximumPrimeDevisorActor>(readerId, writerId, value);
    }

    THolder <NActors::IActor> CreateTWriteActor() {
        return MakeHolder<TWriteActor>();
    }