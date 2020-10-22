const net = require('net');
const aesCBC = require('./crypto');
const utils = require('./Utils');
const crypto = require('crypto');
const dbrequest = require('./queries');
const fs = require('fs');
var stream = require('stream');
const ENC_KEY = "XSEZ1ZiXpwonxSLIbwyoOwBnJOX9mM1n" //public Key
const IV = "byOPz5oNOIGvk1bC"; // public iv

var myserver = net.createServer();

myserver.listen(8001,() => {
    console.log('Started listening on port 8001');

    myserver.on('connection', (connection)=> {
      var client = {
          handshaked: false,
          authenticated: false,
          remoteAddress: connection.remoteAddress.split("::ffff:")[1],

          login: false,
          username: false,
          HWinfo: false,

          key: false,
          iv: false,

          csgo: false,
          gta5: false,

          csgoLength: 0,
          gtaLength: 0
      }

      console.log(`Incoming connection from ${client.remoteAddress}`);

      connection.on('data', function(data){
          var incomingMessage = "";
          if(!client.handshaked){
            incomingMessage = aesCBC.decrypt(data.toString(), ENC_KEY, IV); //use public key and iv
          } else {
            incomingMessage = aesCBC.decrypt(data.toString(), client.key, client.iv); //use client key and iv
          }

          if(incomingMessage === "Ban"){
              console.log(`Banned user`);
              //TODO write to db
              //TODO close connection
          }

          if(!client.handshaked){
            if(incomingMessage === "Test"){
              client.key = utils.generateString(32);
              client.iv = utils.generateString(16);

              connection.write(aesCBC.encrypt(client.key + ';' + client.iv, ENC_KEY, IV));
              client.handshaked = true;
            }
          } else if(!client.authenticated) {
              client.HWinfo = incomingMessage;

              dbrequest.AuthenticateUnknown(client.HWinfo).then(result=>{
                connection.write(aesCBC.encrypt("SUCCESS_AUTHENTICATION", client.key, client.iv));
                client.authenticated = true;
                if(result != false){
                  client.username = result;
                  console.log("User " + client.username + " authenticated! With HWID - " + incomingMessage);
                }
                else{
                  console.log("Connection " + client.remoteAddress + " authenticated! With HWID - " + incomingMessage);
                }
              }).catch(err => {
                    connection.write(aesCBC.encrypt("REJECTED_AUTHENTICATION", client.key, client.iv));
              });

          } else if(!client.login){
              var userpwd = incomingMessage.split(";");
              if(userpwd.length>1){
                  dbrequest.AuthenticateMember(userpwd[0], userpwd[1], client.HWinfo).then(result=>{
                    connection.write(aesCBC.encrypt("SUCCESS_LOGIN", client.key, client.iv));
                    client.login = true;
                    client.username = userpwd[0];
                  }).catch(err => {
                        connection.write(aesCBC.encrypt(err, client.key, client.iv));
                  });
            } else {
              connection.write(aesCBC.encrypt("Missing value", client.key, client.iv));
            }
          } else if(incomingMessage == "REQUEST_SUBS"){
            var output = "";
            dbrequest.CheckSubscription(client.username, 0).then(result=>{
              if(result.active){
                  client.csgo = true;
                  output="CSGO," + result.daysRemaining;
                  client.csgoLength = result.daysRemaining;
              }
              dbrequest.CheckSubscription(client.username, 1).then(result=>{
                if(result.active){
                  client.gta5 = true;
                  output+="|Grand Theft Auto 5,"+result.daysRemaining;
                  client.gtaLength = result.daysRemaining;
                }
                connection.write(aesCBC.encrypt(output, client.key, client.iv));
              });
            });
          } else if(incomingMessage == "REQUEST_DLL"){
            if(client.csgo){
              const filestream = fs.readFileSync("cheat.dll");
              const encryptedDLL = aesCBC.encryptFile(filestream, client.key, client.iv);

              //write dll size in bytes to the socket
              connection.write(aesCBC.encrypt(Buffer.byteLength(encryptedDLL).toString(), client.key, client.iv));

              console.log("Sending CSGO DLL to " + client.username);
              var bufferStream = new stream.PassThrough();
              bufferStream.end( encryptedDLL );
              //send the dll
              bufferStream.pipe(connection);
            }
          }

    });
    connection.on('error', function(msg){
        console.log("A connection closed unexpectly!");
    });
  })
})
