const const_module = require('./const')
const emailModule = require('./email')
const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const {v4:uuidv4} = require('uuid')
const redis_module = require('./redis')

/**
 * GetVarifyCode grpc响应获取验证码的服务
 * @param {*} call 为grpc请求 
 * @param {*} callback 为grpc回调
 * @returns 
 */
async function GetVerifyCode(call, callback){
    console.log('email is ', call.request.email);
    try{
        let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);
        console.log("query_res is ", query_res)
        uniqueId = query_res;
        if(query_res == null){
            uniqueId = uuidv4();
            if(uniqueId.length > 6){
                uniqueId = uniqueId.substring(0, 6);
            }
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,300)
            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
                });
                return;
            }
        }

        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请五分钟内完成注册'

        // 发送邮件
        let mailOptions_ = {
            from: 'wechat204621@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };

        let send_res = await emailModule.SendEmail(mailOptions_);
        console.log('send res is', send_res)

        callback(null, {email: call.request.email, 
            error: const_module.Errors.Success
        });
    }catch(error){
        console.log('catch error is ', error)
        callback(null, {email:call.request.email,
            error:const_module.Errors.Exception
        })
    }
}

function main(){
    let server = new grpc.Server()
    server.addService(message_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        console.log(`gRPC 服务器已启动`);
    });
}

main()