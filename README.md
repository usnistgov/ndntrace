# NDN-Trace

NDN-Trace is a measurement tool developed to retrieve certain information about an NDN network (RTTs, paths to a name, topology).

### Authors:
- Siham Khoussi
- Davide Pesavento
- Lotfi Benmohamed
- Abdella Battou

### Prerequisits
- Install the [ndn-cxx](https://github.com/named-data/ndn-cxx) library and its prerequisites. Please see Getting Started with ndn-cxx for how to install ndn-cxx.
- Install the NDN daemon [NFD](https://github.com/named-data/NFD). 
- Clone [Rapidjson](https://github.com/Tencent/rapidjson) directory into your home in a new folder named "include".

### Building ndn-cxx with the trace programs:

- Clone ndntrace into your home directory
- Copy the two programs "trace_daemon" and "trace_client" into ndn-cxx/examples
- Add the two nack source and header files into ndn-cxx/src/lp repository
- Copy the wscript from ndntrace into ndn-cxx to replace the old one (used to point out the location of rapidjson header files) or simply add these two lines to the script:

     >conf.add_os_flags('HOME')
   
     >if conf.env['HOME']:
   
     >conf.env.append_value('INCLUDES', [conf.env['HOME'][0] + '/include/'])  

- Inside ndn-cxx run the following commands:
    >./waf configure --with-examples
    
    >./waf
    
    >sudo ./waf install

### Building the NFD with the trace strategies:
- Add the trace strategy into your NFD/daemon/fw directory 
- Copy the content of (Single_path_tracing.txt) into your your default strategy used by your NFD in the begining of the "afterreceiveInterest" trigger
- Build the daemon once more by running the following commands:
    >./waf configure
    
    >./waf
    
    >sudo ./waf install

### Usage:
    - Set an alias to call ./trace_daemon or ./trace_client from ndn-cxx/build/examples to trace_daemon and trace_client respectively.
    - Run trace_daemon on every NDN node.
    - On the tracing node run the client:
    
    > trace_client -n <NAME> [-s|-m] [-p|-c|-a] where:
    
      - -n specifies the traced name prefix. This is the only mandatory parameter.
      - -s and -m choose between single-path and multi-path tracing.
      - -p|-c indicates the type of tracing session: -p to trace a producer application, -c to find a cached copy.



