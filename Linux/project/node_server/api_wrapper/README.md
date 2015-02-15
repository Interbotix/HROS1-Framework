#C++ to Javascript API Wrapper
API wrapper uses ffi to connect the low level cpp code to javascript using a shared object. 


#Getting Started
Use make to build necessary files

Connect to robot (via ssh-wifi or screen-uart) and open terminal window
```bash
cd /path/to/api_wrapper
```
then make
```bash
$ make
```
or to clean directory & make
```bash
$ make clean && make
```

This generates a .so file which is used for the ffi connection, where node_server references to. 
