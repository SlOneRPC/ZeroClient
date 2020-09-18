const net = require('net');
const aesCBC = require('./crypto');
const utils = require('./Utils');
const crypto = require('crypto');
const fs = require('fs');
const ENC_KEY = "XSEZ1ZiXpwonxSLIbwyoOwBnJOX9mM1n" //public Key
const IV = "byOPz5oNOIGvk1bC"; // public iv

var myserver = net.createServer();

myserver.listen(8001,() => {
    console.log('Started listening on port 8001');

    myserver.on('connection', (connection)=> {
      var client = {
          alive: true,
          handshaked: false,
          authenticated: false,
          remoteAddress: connection.remoteAddress.split("::ffff:")[1],

          key: false,
          iv: false,

          login: false,
          username: false,
          HWinfo: false
      }

      console.log(`Incoming connection from ${client.remoteAddress}`);

      connection.on('data', function(data){
          var incomingMessage = "";
          if(!client.handshaked){
            incomingMessage = aesCBC.decrypt(data.toString(), ENC_KEY, IV);
          } else {
            incomingMessage = aesCBC.decrypt(data.toString(), client.key, client.iv);
          }

          if(incomingMessage === "Ban"){
              console.log(`Banned user`);
              //close connection
          }

          if(!client.handshaked){
            if(incomingMessage === "Test"){
              client.key = utils.generateString(32);
              client.iv = utils.generateString(16);

              connection.write(aesCBC.encrypt(client.key + ';' + client.iv, ENC_KEY, IV));
              client.handshaked = true;
            }
          } else if(!client.authenticated) {
              //TODO: check hwid is valid
              client.HWinfo = incomingMessage;
              console.log("Connection " + client.remoteAddress + " authenticated! With HWID - " + incomingMessage);
              connection.write(aesCBC.encrypt("SUCCESS_AUTHENTICATION", client.key, client.iv));
              client.authenticated = true;
          } else if(!client.login){
              var userpwd = incomingMessage.split(";");
              //TODO: proper username/password checks
              if(userpwd.length == 2 && userpwd[0] === "test" && userpwd[1] === "test"){
                  connection.write(aesCBC.encrypt("SUCCESS_LOGIN", client.key, client.iv));
                  client.login = true;
              }
              else{
                connection.write(aesCBC.encrypt("FAILED_LOGIN", client.key, client.iv));
              }
          }
          else if(incomingMessage == "REQUEST_DLL"){
            var stats = fs.statSync("cheat.dll")
            connection.write(aesCBC.encrypt(stats["size"].toString(), client.key, client.iv));

            const filestream =  fs.createReadStream("cheat.dll");
            console.log("Sending DLL");

            //connection.pipe(filestream);
            filestream.pipe(connection);
          }

    });
  })
})
