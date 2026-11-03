const config_module = require('./config')
const Redis = require('ioredis')

// 创建客户端示例
const RedisCli = new Redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_passwd
});

/**
 * 监听错误信息
 */
RedisCli.on("error", function(err){
    console.log('RedisCli connect error');
    RedisCli.quit();
})

/**
 * 根据key获取value
 * @param {*} key 
 * @returns 
 */
async function GetRedis(key){
    try{
        const result = await RedisCli.get(key);
        if(result == null){
            console.log('result:', '<'+result+'>', 'This key cannot be find...');
            return null;
        }else{
            console.log('result:', '<'+result+'>', 'Get key success! ....');
            return result;
        }
    }catch(error){
        console.log("GetRedis error is ", error);
        return null;
    }
}

/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key){
    try{
        const result = await RedisCli.exists(key);
        if(result == 0){
            console.log('result:<','<'+result+'>','This key is null...');
            return null;
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is ', error);
        return null
    }
}

/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key,value, exptime){
    try{
        // 原子操作：SET key value EX seconds
        const result = await RedisCli.set(key, value, 'EX', exptime);
        if (result === 'OK') {
            console.log(`SetRedisExpire success: ${key} will expire in ${exptime}s`);
            return true;
        } else {
            console.log(`SetRedisExpire failed: ${result}`);
            return false;
        }
    }catch(error){
        console.log('SetRedisExpire error is', error);
        return false;
    }
}
/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}
module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}