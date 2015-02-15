/*
 *   app.js
 *
 *   Author: Daniel Alner
 *
 */

//requirements
var express = require('express');
var ffi = require('ffi');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var say = require('say')



//library location of the shared objects
var LIBLOC = '../api_wrapper/apiwrapper';

// core actions
var actionList = {};

// battery level to emit on heartbeat
var BatteryLevel = -1;
/************************************
* Function calls to native code:    *
*                                   *
*                                   *
************************************/
// these core actions must be updated
// to reflect the page builds
// on the rme editor (these are suggested 
// page number selections)
function setCoreActions() {
  actionList.stand        = 2;
//page 3 is walk ready
  actionList.sit          = 4;
  actionList.sitshtdown   = 6;
  actionList.wave         = 10;
  actionList.handshake    = 11;
  actionList.excite       = 12;
  actionList.thanks       = 13;
  actionList.superhero    = 14;
  actionList.dance        = 15;
  actionList.nod          = 16;
}

/**************************************
***              Web Service         **
****************************************/
app.use(express.static(__dirname + '/www'));


// serve up the html page on connection to ip address and port #
app.get('/', function(req, res){
  res.sendfile('index.html');
});
/**************************************
***              General/Options     **
****************************************/
var general = ffi.Library(LIBLOC, {
    // initialize servos
    'InitializeJS': ['bool', []],
    // off all servos
    'ServoShutdownJS' : ['void', [] ],
    // on all servos
    'ServoStartupJS' : ['void', [] ]
});
/******* REST API ***************/
//http://<ipaddress>/Options/GetName
//Still working this part out, UNCOMPLETED
app.get('/options/getname', function (req, res) {

});
//http://<ipaddress>/Diagnostics/Initialize
//Still working this part out, UNCOMPLETED
app.get('/diagnostics/initialize', function (req, res) {

});

/**************************************
***                  Actions         **
****************************************/
var actions = ffi.Library(LIBLOC, {
    // call page numbers
    'PlayActionJS' : ['int', ['int'] ]
});
/******* REST API ***************/
//http://<ipaddress>/Action/Call/name
//Call action by name.
app.get('/action/call/:name', function (req, res) {
    console.log(req);
});
//http://<ipaddress>/Action/Add/name
//Add action by name. Must be unique action names. System action names already exist, such as Stand, Sit, Wave, Exite...etc
app.get('action/add/:name', function (req, res) {
    console.log(req);
});
//http://<ipaddress>/Action/Position/name?position?server
//Still being determined how to do, UNCOMPLETED
app.get('action/position/:name:position:server', function (req, res) {
    console.log(req);
});
/**************************************
***                   Walking        **
****************************************/
var walk = ffi.Library(LIBLOC, {
    // turn walking on/off
    'WalkJS' : ['void', ['bool']],

    // walking location xy
    'WalkingJS' : ['void',['int', 'int']]

    // set walking gait
    //TBD

    // tune walking
    //TBD

    // walking speed set
    //TBD
});
/******* REST API ***************/
//http://<ipaddress>/Walk/bool
//Turn walk on or off (use true/false, on/off)
app.get('walk/:onOff', function (req, res){

});
//http://<ipaddress>/Walk/Position/x?y
//Walk position values must exists within -255 to 255, the larger the values the faster the walk stride (double check)
app.get('walk/position/:x:y', function (req, res){

});

/**************************************
***                   Head Motion    **
****************************************/
var head = ffi.Library(LIBLOC, {
    // head pan (double) tilt (double) motion
    // 'MoveHeadByAngleJS' : ['void', ['double', 'double']]
});
/******* REST API ***************/
//http://<ipaddress>/Head/Position/angle?degree
//Still working this part out, UNCOMPLETED
app.get('head/position/:angle:degree', function(req, res){

});
/**************************************
***                   Diagnostic     **
****************************************/
var diagnostics = ffi.Library(LIBLOC, {
    // check all servos and return failing servo or return 0
    'CheckServosJS' : [ 'int', []],
    // check battery and return volt value or if below value, sit robot
    'BatteryVoltLevelJS' : [ 'int', [] ]
    // check servo heat value and return or sit if above value
    // maybe merge this into check servos?

    // check wifi connectivity and values

    // give alert of any other issues and return
});
/***                Battery       **/
  function getBatteryVoltLevel(){
    // from here, this returns a voltage, possible change to percent
    BatteryLevel = diagnostics.BatteryVoltLevelJS();
  }
  /***                Servos check -- not yet implemented      
  function checkServos(){
    var servosValues = 0;
    servosValues = diagnostics.CheckServosJS();
    socket.emit('servovalues', servosValues);
  }**/
/******* REST API ***************/
//http://<ipaddress>/Diagnostics/check
//Still working this part out, UNCOMPLETED
app.get('/diagnostics/check', function (req, res){

});
//http://<ipaddress>/Diagnostics/Battery
//Still working this part out, UNCOMPLETED
app.get('/diagnostics/battery', function (req, res){

});
/**************************************
***                   Sensors        **
****************************************/
var sensors = ffi.Library(LIBLOC,{
    // add sensor
    // flesh this out a bit more

});


/**************************************
***        Connection/Disconnection  **
****************************************/
io.on('connection',function(socket) {
  console.log("device connected");
  // start battery emit to client
  setInterval(function(){
      io.emit('batterylevel', BatteryLevel);
    },2000);
  /***                Initialize       **/
  socket.on('initialize', function () {
    setCoreActions();
    if(general.InitializeJS())
      actions.PlayActionJS(actionList.sit);  // sit robot
  });

  /***                Servo Shutdown       **/
  socket.on('servoshutdown', function () {
    general.ServoShutdownJS();
  });

  /***                Servo Startup       **/
  socket.on('servostartup', function () {
    general.ServoStartupJS();
  });

  /***                disconnect       **/
  socket.on('disconnect',function(){
      console.log('disconnecting');
      //notify app and sit robot
      actions.PlayActionJS(actionList.sit);  // sit robot
      console.log('disconnected');
  });

  /***                action       **/
  socket.on('action',function(action){
    var pageNumb = action.trim();
    console.log('loading page', pageNumb);
    try {
      // if they pass a paramter of name rather than page number
      if(typeof pageNumb === 'string')
        pageNumb = actionList[pageNumb];

      actions.PlayActionJS(pageNumb);  // this currently holds the thread on play
      console.log('played page', pageNumb);
    } catch (err) {
      console.log('failed to load page');
    }
  });

  /***                Walk           **/
  socket.on('walktoggle',function (onOff) {
    console.log('walk toggle');
    if(typeof onOff === 'boolean')
      walk.WalkJS(onOff);
    else
      console.log('must be boolean parameter');
  });

  /***                walking       **/
  socket.on('walking', function (walkCoords) {
    if(typeof parseInt(walkCoords.x) === 'number' && typeof parseInt(walkCoords.y) === 'number') {
      console.log(walkCoords.x, walkCoords.y);
      walk.WalkingJS(parseInt(walkCoords.x), parseInt(walkCoords.y));
    }
    else
      console.log('must give int x and y coords');
  });
  
  /***                Head Motion       **/
  socket.on('headposition', function (pan, tilt) {
    head.MoveHeadByAngleJS (pan, tilt);
  });
  /***                speech       **/
  // speech module only works on linux (currently untested)
  socket.on('speech', function(string){
    say.speak(null, string);
  });
});

http.listen(2114,function(){
  console.log("listening on port 2114");
  // start the battery and servo monitoring on server start
  setInterval(getBatteryVoltLevel, 1000);  // check the heartbeat every 1 second (up for change)
  // setInterval(checkServos(), 5000);  // check the heartbeat every 5 second (up for change)
  
  setCoreActions();
    if(general.InitializeJS()) {
      actions.PlayActionJS(actionList.sit);  // sit robot
      console.log("successful robot init");
    }
    else
      console.log("error initializing robot");
});
