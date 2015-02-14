#Node Server



##Gettings Started
Dependencies: (pre-installed)
* node.js http://nodejs.org
* python v 2.7
* make
* GCC

Connect to robot (via ssh-wifi or screen-uart) and open terminal window
```bash
cd /path/to/api/wrapper
```
1. Run:
```bash
$ npm install
```
or
```bash
$ sudo npm install
```
* this installs socket.io, ffi, and node-gyp, grunt

If you want to build the minified version of the node server, dev dependencies are required

```bash
$ npm install --dev
```
or
```bash
$ sudo npm install --dev
```

```bash
$ grunt
```

##Run node server
###Run from cli
*  navigate to build folder
*  run node server
```bash
$ node node-app.min.js
```

###Run node server on boot
*  NUC Robot
	*  Use upstart to start node server


*  Edison
	*  Use init.d to start node server

(See autoboot scripts in init.d files)
