
const mysql = require('mysql2');
const bcrypt = require('bcrypt');
const utils = require('./utils');

const pool = mysql.createPool({
    host: "127.0.0.1",
    user: "root",
    password: "",
    database: "mainsoftware",
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});

const GetMember = username => {
    return new Promise((resolve, reject) => {
            pool.query(`SELECT * FROM users WHERE username=?`, [username], (err, results, fields) => {
                if(err) return reject(err);
                if(results.length >= 1) {
                    var member = {
                        username: false,
                        password: false,
                        hwid: false
                    }
                    member.username = results[0].username;
                    member.password = results[0].password;
                    member.hwid = results[0].hwid;
                    resolve(member);
                } else {
                    reject("Unknown user!");
                }
            });
    });
};


const AuthenticateMember = (username, password, hwid) => {
    return new Promise((resolve, reject) => {
        GetMember(username).then(member => {
            if(!member.banned) {
                var pwdhash = member.password.replace("$2x$", "$2b$").replace("$2y$", "$2b$");
                bcrypt.compare(password, pwdhash, (err, correct) => {
                    if(correct) {
                        if(member.hwid == "unset"){
                          SetHWID(username, hwid);
                        }
                        else if(member.hwid != hwid){
                          reject("Invalid HWID");
                        }
                        resolve();
                    } else {
                        reject("Invalid credentials");
                    }
                });
            } else {
                reject("banned")
            }
        }).catch(err => {
            console.log("Invalid creds");
            reject("Invalid credentials");
        });
    });
};

const AuthenticateUnknown = (hwid) => {
    return new Promise((resolve, reject) => {
          pool.query(`SELECT * FROM users WHERE hwid=?`, [hwid], (err, results, fields) => {
              if(err) return reject(err);
              if(results.length >= 1) {
                  if(results[0].banned == "Yes"){
                    reject("banned");
                  }
                  resolve(results[0].username);
              } else {
                  resolve(false);
              }
          });
    });
};

const SetHWID = (username,hwid) => {
  pool.query(`UPDATE users SET hwid=? WHERE username=?`, [hwid,username], (err, results, fields) => {
      if (err) throw err;
  });
}

const CheckSubscription = (username,subid) => {
    return new Promise((resolve, reject) => {
          pool.query(`SELECT * FROM users INNER JOIN subscriptions on users.id=subscriptions.ownerID WHERE users.username=? AND subscriptions.subscriptionID=?`, [username,subid], (err, results, fields) => {
              if(err) return reject(err);
              var subState = {
                active: false,
                daysRemaining: false
              }
              if(results.length >= 1) {
                  var date = results[0].StartDate;
                  var length = results[0].Length;
                  date.setDate(date.getDate() + length);
                  var now = new Date();

                  if(now<date){
                    subState.active = true;
                    subState.daysRemaining = Math.round((date-now)/(1000*60*60*24));
                    resolve(subState);
                  }
                  else{
                    resolve(subState);
                  }
              } else {
                  resolve(subState);
              }
          });
    });
};

module.exports = {
    GetMember: GetMember,
    AuthenticateMember: AuthenticateMember,
    AuthenticateUnknown: AuthenticateUnknown,
    CheckSubscription: CheckSubscription
}
