# NDN-Trace

NDN-Trace is a measurement tool developed to retrieve certain information about an NDN network (RTTs, paths to a name, topology).
The paper that describes the protocol in details was published in [ACM ICN 2017](http://conferences.sigcomm.org/acm-icn/2017/).    

  [Link to access the paper](https://github.com/named-data/ndn-cxx)


### Authors:
- Siham Khoussi     siham.khoussi@nist.gov
- Davide Pesavento
- Lotfi Benmohamed
- Abdella Battou

### Prerequisits
- Install VERSION = "0.5.0" of [ndn-cxx](https://github.com/named-data/ndn-cxx) library and its prerequisites. Please see Getting Started with ndn-cxx for how to install ndn-cxx. If you are using a different version of ndn-cxx, use the following command (inside ndn-cxx) to get the required version:

     > git checkout tags/ndn-cxx-0.5.0
    
- Install the NDN daemon [NFD](https://github.com/named-data/NFD), VERSION = "0.5.0". If you are using a different version of NFD, use the following command (inside NFD) to get the required version:

     > git checkout tags/NFD-0.5.0
     
- Clone [Rapidjson](https://github.com/Tencent/rapidjson) into your home in a new folder named "include".


### Building ndn-cxx with the trace programs:

- Clone ndntrace into your home directory.
- From ndntrace/patches, apply the patch ws.patch inside ndn-cxx.
- Copy the two programs "trace_daemon" and "trace_client" into ndn-cxx/examples.
- From ndntrace/patches, apply the patch nackcpp.patch and nackhpp.patch inside ndn-cxx/src/lp.
- To build ndn-cxx with the new changes run the following commands:

    >./waf configure --with-examples
    
    >./waf
    
    >sudo ./waf install

### Building the NFD with the trace strategy components:

The steps below provide guidance to integrate Trace changes into the ASF strategy as well as thebest route strategy:

- Open NFD directory
- From ndntrace/patches, apply the patches bestroutecpp.patch bestroutehpp.patch inside NFD/daemon/fw. To include NdnTrace changes to the best route strategy.
- From ndntrace/patches, apply the patches asfcpp.patch asfhpp.patch inside NFD/daemon/fw. To include NdnTrace changes to the ASF strategy.
- Build the daemon once more to include the changes by running the following commands:

    >./waf configure
    
    >./waf
    
    >sudo ./waf install

To use NdnTrace on different/customized strategies, follow these steps:

- Change the field "YourDefaultStrategyHere" from strategy_components.hpp into the name of your strategy. Then copy it's content into it.
- Copy the content of strategy_component.cpp into your own strategy and call "Trace" function from within "afterReceiveInterest".
- Initialize "m_forwarder" in the constructer of your strategy class.
- Build the NFD with the above changes:

    >./waf configure
    
    >./waf
    
    >sudo ./waf install

### Usage:
    - Set an alias to call ./trace_daemon or ./trace_client from ndn-cxx/build/examples to trace_daemon and trace_client respectively.
    - Run trace_daemon on every NDN node.
    - On the tracing node run the client:
    
    > trace_client -n <NAME> [-s|-m] [-p|-c] where:
    
      - -n specifies the traced name prefix. This is the only mandatory parameter.
      - -s and -m choose between single-path and multi-path tracing.
      - -p|-c indicates the type of tracing session: -p to trace a producer application, -c to find a cached copy.



