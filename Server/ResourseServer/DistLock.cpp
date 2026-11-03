#include "DistLock.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <thread>

static std::string getUuid() {
    return boost::uuids::to_string(boost::uuids::random_generator()());
}

DistLock& DistLock::Inst()
{
    static DistLock lock;
    return lock;
}

std::string DistLock::acquireLock(redisContext* context, const std::string& lockName, const int& lockTimeout, const int& acquireTimeout){
    std::string lockKey = "lock_" + lockName;
    std::string identifier = getUuid();
    auto endTime = std::chrono::system_clock::now() + std::chrono::seconds(acquireTimeout);

    while (std::chrono::system_clock::now() < endTime) {
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d",
            lockKey, identifier, lockTimeout);
        if (reply != nullptr) {
            if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
                freeReplyObject(reply);
                return identifier;
            }
            freeReplyObject(reply);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return std::string();
}

bool DistLock::releaseLock(redisContext* context, const std::string& lockName, const std::string& identifier){
    std::string lockKey = "lock_" + lockName;
    const char* luaScript = "if redis.call('get', KEYS[1]) == ARGV[1] then \
                                return redis.call('del', KEYS[1])\
                            else \
                                return 0\
                            end";
    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", luaScript,
        lockKey, identifier);
    if (reply != nullptr) {
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
            freeReplyObject(reply);
            return true;
        }
        freeReplyObject(reply);
    }
    return false;
}
