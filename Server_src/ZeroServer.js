const net = require('net');
const aesCBC = require('./crypto');
const utils = require('./Utils');
const crypto = require('crypto');
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
          iv: false
      }

      console.log(`Incoming connection from ${client.remoteAddress}`);

      connection.on('data', function(data){
          var incomingMessage = "";
          if(!client.handshaked){
            incomingMessage = aesCBC.decrypt(data.toString(), ENC_KEY, IV);
          } else {
            incomingMessage = aesCBC.decrypt(data.toString(), client.key, client.iv);
          }


          if(!client.handshaked){
            if(incomingMessage === "Test"){
              client.key = utils.generateString(32);
              client.iv = utils.generateString(16);

              connection.write(aesCBC.encrypt(client.key + ';' + client.iv, ENC_KEY, IV));
              client.handshaked = true;
            }
          } else if(!client.authenticated) {

          }
    });
  })
})
