const crypto = require('crypto');
const fs = require('fs');

var encrypt = ((val,key,iv) => {
  let cipher = crypto.createCipheriv('aes-256-CBC',  Buffer.from(key), Buffer.from(iv));
  let encrypted = cipher.update(val);
  encrypted = Buffer.concat([encrypted, cipher.final()]);

  return encrypted.toString('hex');
});

var decrypt = ((encrypted,key,iv) => {
  let decipher = crypto.createDecipheriv('aes-256-CBC', Buffer.from(key), Buffer.from(iv));
  let encryptedText = Buffer.from(encrypted, 'hex');
  let decrypted = decipher.update(encryptedText);
  decrypted = Buffer.concat([decrypted, decipher.final()]);
  return decrypted.toString();
});

var encryptFile = ((file,key,iv) => {
    const readStream = fs.createReadStream('./cheat.dll');

});




module.exports = {
    encrypt: encrypt,
    decrypt: decrypt
}
