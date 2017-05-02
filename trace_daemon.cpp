
#include "face.hpp"
//#include "nfdidcollector.hpp"
#include "security/validator-null.hpp"
#include "security/key-chain.hpp"
#include <ndn-cxx/mgmt/nfd/controller.hpp>
#include <ndn-cxx/mgmt/nfd/fib-entry.hpp>
#include <map>
#include <chrono>
#include <ctime>
#include <stdio.h>

namespace ndn {

    namespace examples {

        using namespace std::chrono;
        using ndn::nfd::Controller;

        typedef std::chrono::high_resolution_clock Clock;

        class Tracker : noncopyable
        {

            public:
            void
            runp()
            {
                a_face.setInterestFilter("/Trace",
                bind(&Tracker::onInterest, this, _1, _2),
                RegisterPrefixSuccessCallback(),
                bind(&Tracker::onRegisterFailed, this, _1, _2));
                a_face.processEvents();
            }



            private:


            int number(){
                static std::uniform_int_distribution<uint32_t> distribution;
                 return distribution(getRandomGenerator());
            }

            static std::mt19937&
                getRandomGenerator()
{
  static std::mt19937 rng{std::random_device{}()};
  return rng;
}




            void
            onRegisterFailed(const Name& prefix, const std::string& reason)
            {
                std::cerr << "ERROR: Failed to register prefix \""
                << prefix << "\" in local hub's daemon (" << reason << ")"
                << std::endl;
                a_face.shutdown();
            }


            void
            onInterest(const InterestFilter& filter, const Interest& interest)
            {
                Ti_a = steady_clock::now(); // time of a request arrivals
                sso[interest.getNonce()] = steady_clock::now();
                m_over[interest.getNonce()]=0;


                // loop check
                for(std::vector<uint32_t>::iterator it=loop.begin() ; it !=loop.end(); ++it){
                        uint32_t nonce = *it;
                        if (interest.getNonce() == nonce){
                        return;}
                        else{
                        loop.push_back(interest.getNonce());
                        }
                return;}


                // Store the request for the nonce

                request[interest.getNonce()] = interest.getName().toUri();


                // Extract name from interest

                int s = interest.getName().size();
                int k = (-1)*s;
                int i;


                std:: string p2 = interest.getName().at(k+2).toUri();
                //std::cout << "Parameter p2 " << p2 << std::endl;
               //

                //Parsing parameters
                if (interest.getName().at(k+1).toUri() == "M"){ // Could be from User (no face_id) or from another daemon >> what name??

                    //std::cout << "Request for M" << std::endl;

                    // Get the name to trace
                    const ndn::Name c = interest.getName().at(k+3).toUri();
                    const ndn::Name nx = c.toUri() ;
                    ndn::Name v = nx.toUri();

                    // Different names according to where the interest is coming from
                    std::size_t found4 = interest.getName().at(-1).toUri().find("Key-UID");

                    if (found4!=std::string::npos){// user interest

                        key[interest.getNonce()]= interest.getName().at(-1).toUri();

                            for(i=k+4; i< -1; i++){

                                v = v.toUri() + "/" + interest.getName().at(i).toUri();

                            }

                    }else{// another daemon's interest

                    key[interest.getNonce()]= interest.getName().at(-2).toUri();

                            for(i=k+4; i< -2; i++){

                                v = v.toUri() + "/" + interest.getName().at(i).toUri();

                            }
                    }

                    //std::cout << "The name to trace just to make sure is " << v << std::endl;


                    // Query the Fib manager

                    Face face;
                    KeyChain x_keyChain;
                    ndn::nfd::Controller cont(face, x_keyChain);
                    const ndn::nfd::CommandOptions& options = {};
                    const ndn::Name& name = v;
                    //std::cout << "The nonce " << interest.getNonce() << std::endl;
                    query(cont, name, interest.getNonce(), options); // should store the vector for the corresponding request (nonce)
                    face.processEvents();

                    // is this a forking point or a regular node getting a multipath request
                    if (mhops[interest.getNonce()].size()>1){

                       // std::cout << "This is indeed a forking point. this interest will be M processed" << std::endl;
                        mflag[interest.getNonce()]=true; //setting the multipath flag
                    }


                    mscount[interest.getNonce()]= 0; // initialization of the mscounter

                        Ttemp = steady_clock::now();
                        steady_clock::duration ts = Ttemp - Ti_a;

                        double xv = double(ts.count()) * steady_clock::period::num / steady_clock::period::den;

                    // Create N corresponding interests

                    for(int i=0; i<mhops[interest.getNonce()].size(); i++){
                        //for(int i=0; i<m_nexthops.size(); i++){

                        //for (std::vector<uint64_t>::iterator it=m_nexthops.begin(); it!=m_nexthops.end(); ++it){

                        std::string x =std::to_string(mhops[interest.getNonce()][i]); // getting the face id into a string

                        ndn::Interest I;
                        std::string aa = "Key-TID" + std::to_string(number());
                        ndn::Name n = "/Trace/M/"+ p2 + v.toUri()+ "/" + aa + "/" + x ;  // attaching the face id as x
                        I.setName(n);
                        I.setNonce(interest.getNonce());
                        I.setInterestLifetime(ndn::time::milliseconds(10000));
                        I.setMustBeFresh(true);
                        //std::cout << "interest " << i << " is " << I << std::endl;


                        m_mapi[interest.getNonce()][I.getName().at(-2).toUri()] = steady_clock::now();// better record the timestamp here
                        s_face.expressInterest(I,
                        bind(&Tracker::onData, this,  _1, _2),
                        bind(&Tracker::onNack, this, _1, _2),
                        bind(&Tracker::onTimeout, this, _1));

//                        s_face.processEvents();
                    }
                        s_face.processEvents();

                    // Part two: collection of Data and Nacks

                    if (mscount[interest.getNonce()] == mhops[interest.getNonce()].size()){

                        // if multiple nexthops add <child>><node name=" "> some of arrivals </node></child>

                        std::string collector;



                 for(std::vector<node>::iterator it = nodes[interest.getNonce()].begin(); it != nodes[interest.getNonce()].end(); ++it){

                        node y =*it;

std::string u = "{"+m_nfdId.toUri()+"(*)"+  std::to_string(y.delay) +"(**)"+std::to_string(mean+(m_over[interest.getNonce()]-y.delay)+xv)+"}!";

                 std::string uu = "<child><node name=\""+u+"\">"+ y.arr + "</node></child>";

                        collector = collector + uu;

                        }

                        //////////////////////////////////////////////////////////////

                        //if (mhops[interest.getNonce()].size()>1){
                        //collector = "<node name=\" \" >" +  collector + "</node>";
                        //}

                        // Create Data packetild
                        shared_ptr<Data> data = make_shared<Data>();
                        data->setName(request[interest.getNonce()]);
                        data->setFreshnessPeriod(time::seconds(0));
                        data->setContent(reinterpret_cast<const uint8_t*>(collector.c_str()), collector.size());

                        //std::cout << "data name " << request[interest.getNonce()] <<std::endl;
                        //std::cout << "multipath data is " << std::string(reinterpret_cast<const char*>(data->getContent().value()), data->getContent().value_size()) << std::endl;

                        // Sign Data packet with default identity
                        m_keyChain.sign(*data);

                        // Send the data through the incoming face
                        a_face.put(*data);

                    }

                    return;

                }else{

                    // Enter single path section

                    const ndn::Name c = interest.getName().at(k+3).toUri();
                    const ndn::Name nx = c.toUri() ;
                    ndn::Name v = nx.toUri();

                    for(i=k+4; i< -1; i++){

                        v = v.toUri() + "/" + interest.getName().at(i).toUri();  // The name to lookup

                    }


                    // Removing last key id and adding a new one

                    //std::string sid = std::to_string(number());
                    //std::cout << "Number? " << number()+1 <<std::endl;

                    ndn::Interest I;
                    ndn::Name n = "/Trace/S/"+p2+ v.toUri()+ "/Key-TID" + std::to_string(number()) ;
                    //std::cout << "aa " << aa <<std::endl;
                    //std::cout << "N name" << n <<std::endl;


                    I.setName(n);
                    I.setNonce(interest.getNonce());

                    I.setInterestLifetime(interest.getInterestLifetime());
                    I.setMustBeFresh(true);
                    //std::cout << " interest received  " <<  interest.getName() << std::endl;
                    //std::cout << " interest name  " <<  I.getName() << std::endl;

                    // Express a new interest (as a consumer)  -->  we care about the Nack and timeout only -->  No data expected

                    seo[interest.getNonce()] = steady_clock::now();

                    smapi[interest.getNonce()] = steady_clock::now();

                    s_face.expressInterest(I,
                    bind(&Tracker::onData, this,  _1, _2),
                    bind(&Tracker::onNack, this, _1, _2),
                    bind(&Tracker::onTimeout, this, _1));



                    // processEvents will block until the requested data received or timeout occurs
                    s_face.processEvents();

                    if (GN == true) {
                        nackprocessing(interest);

                    }

                    if (GD == true) {
                        dataprocessing(interest);
                    }

                }//end else

            }

            void
            nackprocessing(const Interest& interest){

                //double elapsedn_secs = double(Tn_a - Ti_a) / CLOCKS_PER_SEC;

                steady_clock::duration time_span = smapr[interest.getNonce()] - smapi[interest.getNonce()];

                steady_clock::duration time_span2 = seo[interest.getNonce()] - sso[interest.getNonce()];

                double extra = double(time_span2.count()) * steady_clock::period::num / steady_clock::period::den;
                double elapsedn_secs = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;

                //double elapsedn_secs = double(smapr[interest.getNonce()] - smapi[interest.getNonce()]) / CLOCKS_PER_SEC;


                // Create new name, based on Interest's name
                Name dataName(interest.getName());

                // Fake content
                //static const std::string seperator = "#**#";
                std::string s = "(*)";
                std::string idsp = "<" + m_nfdId.toUri() + s + std::to_string(elapsedn_secs) + "(**)" + std::to_string(mean+extra)+ ">!";


                // Create Data packet
                shared_ptr<Data> data = make_shared<Data>();
                data->setName(dataName);
                data->setFreshnessPeriod(time::seconds(0));
                //std::cout << "N" << std::endl;
                //std::cout << "Test: " << idsp << std::endl;
                data->setContent(reinterpret_cast<const uint8_t*>(idsp.c_str()), idsp.size());

                //std::cout << "******" << std::endl;

                // Sign Data packet with default identity
                m_keyChain.sign(*data);

                // Send the data through the incoming face
                a_face.put(*data);
            }
            void
            dataprocessing(const Interest& interest){

                //std::cout << "D " << std::endl;
                //double elapsedd_secs = double(Td_a - Ti_a) / CLOCKS_PER_SEC;

                steady_clock::duration time_span = smapr[interest.getNonce()] - smapi[interest.getNonce()];

                double elapsedd_secs = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;

                steady_clock::duration time_span2 = seo[interest.getNonce()] - sso[interest.getNonce()];

                double extra = double(time_span2.count()) * steady_clock::period::num / steady_clock::period::den;

                //double elapsedd_secs = double(]) / CLOCKS_PER_SEC;

                std::string s = "(*)";
                std::string idsp = "<" + m_nfdId.toUri() + s + std::to_string(elapsedd_secs) + "(**)" + std::to_string(mean+extra) + ">!";

                //std::cout << "data_ a  " << data_a << std::endl;

                //std::cout << "idsp  " << idsp << std::endl;

                data_d = idsp + data_a;

                //std::cout << "Combined " << data_d << std::endl;

                shared_ptr<Data> data = make_shared<Data>();
                Name dataName(interest.getName());
                data->setName(dataName);
                data->setFreshnessPeriod(time::seconds(10)); // to be adjusted to the parameter in the user

                data->setContent(reinterpret_cast<const uint8_t*>(data_d.c_str()), data_d.size());
                //std::cout << "What's actually being sent : " << std::string(reinterpret_cast<const char*>(data->getContent().value()), data->getContent().value_size()) << std::endl;

                m_keyChain.sign(*data);
                a_face.put(*data);
            }


            void
            onData(const Interest& interest, const Data& data)
            {
                Td_a = steady_clock::now();
                smapr[interest.getNonce()]=steady_clock::now();  // for single path data timestamp
                int s = interest.getName().size();
                int k = (-1)*s;

                if (interest.getName().at(k+1).toUri() == "M"){  // Trace/M/name/Key-Tid/face_id

                    m_mapr[interest.getNonce()][interest.getName().at(-2).toUri()] = Td_a;

                    /// XML /////////

                     steady_clock::duration time_span = m_mapr[interest.getNonce()][interest.getName().at(-2).toUri()] - m_mapi[interest.getNonce()][interest.getName().at(-2).toUri()];

                     double e = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;
                     if (e>m_over[interest.getNonce()]){
                        m_over[interest.getNonce()]=e;
                     }


                   node n = {e, std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size())};

                   nodes[interest.getNonce()].push_back(n);
                   mscount[interest.getNonce()]++;

                 return;
                 }else{

                    GD =  true;
                    data_a = std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
                }
            }

            void
            onNack(const Interest& interest, const lp::Nack& nack)
            {
                Tn_a = steady_clock::now();

                int s = interest.getName().size();
                int k = (-1)*s;

                if (interest.getName().at(k+1).toUri() == "M"){

                    m_mapr[interest.getNonce()][interest.getName().at(-2).toUri()] = Tn_a;

                    steady_clock::duration time_span = m_mapr[interest.getNonce()][interest.getName().at(-2).toUri()] - m_mapi[interest.getNonce()][interest.getName().at(-2).toUri()];

                     double e = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;

                     if (e>m_over[interest.getNonce()]){
                        m_over[interest.getNonce()]=e;
                        }

                     node n = {e, ""};
                     nodes[interest.getNonce()].push_back(n);
                     mscount[interest.getNonce()]++;

                        return;
                    }else{

                        smapr[interest.getNonce()]=steady_clock::now();
                       if ((nack.getReason()== lp::NackReason::PRODUCER_LOCAL)||(nack.getReason()== lp::NackReason::CACHE_LOCAL)){ // expected nack could alsp be No route / prohibited / not supported ...
                            GN = true;
                        }

                    }
                }



                void
                onTimeout(const Interest& interest)
                {
                    std::cout << "Timeout " << interest << std::endl;
                }


                // init part --> in charge of retrieving the NFD ID

                public:
                void
                getid()
                {
                    Interest interest(Name("ndn:/localhost/nfd/fib/list"));
                    interest.setInterestLifetime(time::milliseconds(10000));
                    interest.setMustBeFresh(true);

                    l_face.expressInterest(interest,
                    bind(&Tracker::onId, this,  _1, _2),
                    bind(&Tracker::onNId, this, _1, _2),
                    bind(&Tracker::onTId, this, _1));

                    std::cout << "Looking for ID " << interest << std::endl;

                    // processEvents will block until the requested data received or timeout occurs
                    l_face.processEvents();
                }


                private:

                void
                onNId(const Interest& interest, const lp::Nack& nack)
                {
                    std::cout << "Could not retrieve the NFD ID, received Nack with reason " << nack.getReason()
                    << " for interest " << interest << std::endl;
                    }

                void
                onTId(const Interest& interest)
                {
                    std::cout << "Request for the NFD ID timed out " << interest << std::endl;
                    }


                void
                onId(const Interest& interest, const Data& data)
                {
                    const ndn::Signature& sig = data.getSignature();
                    const ndn::KeyLocator& kl = sig.getKeyLocator();
                    m_nfdId = kl.getName();
                    std::cout<< "(NFDId): " << m_nfdId.toUri() << std::endl;
                }


                void
                query(Controller& controller, const ndn::Name& name, uint32_t nonce, const ndn::nfd::CommandOptions& options = {})
                {
                    parameters param = {name, nonce};
                    //param.name = name;
                    //param.nonce = nonce;

                    controller.fetch<ndn::nfd::FibDataset>(

                    [this, param] (const std::vector<ndn::nfd::FibEntry>& result) {

                        for (size_t prefixLen = param.name.size(); prefixLen > 0; --prefixLen) {

                            for (const ndn::nfd::FibEntry& item : result) {

                                if (item.getPrefix().size() == prefixLen && item.getPrefix().isPrefixOf(param.name)) {

                                    mhops[param.nonce].clear();

                                    for (const ndn::nfd::NextHopRecord& nh : item.getNextHopRecords()) {

                                        mhops[param.nonce].push_back(nh.getFaceId()); // Adding a ((once, vector) couple to the map
                                    }

                                    //std::cout << " " <<'\n';

                                    return;

                                }
                            }
                        }

                    },
                    bind([]{ std::cout << "To do something here\n";}),
                    options);
                }


                public:
                float estimate(){



                    Ts = steady_clock::now(); // to estimate name creation and all

                    shared_ptr<Data> data = make_shared<Data>();

                    Name n = "/Test";
                    data->setName(n);
                    data->setFreshnessPeriod(time::seconds(10));
                    data->setContent(reinterpret_cast<const uint8_t*>(data_d.c_str()), data_d.size());

                    //Ts = steady_clock::now();  just the signature
                    m_keyChain.sign(*data);
                    To = steady_clock::now();

                    //std::cout << "Signature estimation " << elapsedd_secs << std::endl;

                    steady_clock::duration time_span = To - Ts;
                    float nseconds = float(time_span.count()) * steady_clock::period::num / steady_clock::period::den;

                    //float elapsedd_secs = float(To - Ts) / CLOCKS_PER_SEC;
                    return nseconds;

                }


                private:
                struct parameters{
                    const ndn::Name& name;
                    uint32_t nonce;
                };
                Face a_face;  // with a being the first arrived request
                Face s_face;  // with s being the sending face
                Face l_face;
                KeyChain m_keyChain;
                steady_clock::time_point Ti_a;
                steady_clock::time_point Tn_a;
                steady_clock::time_point Td_a;
                steady_clock::time_point Ttemp;

                steady_clock::time_point Ts;
                steady_clock::time_point To;
                std::string data_a;
                std::string data_d;

                //std::map<std::string, clock_t> mapi; //interest sending time
                //std::map<std::string, clock_t> mapr; // Nack or Dt

                int mcounter = 0;

                std::map<uint32_t, std::vector<uint64_t>> mhops; // map for multipath next hops (nonce, list of nexthops)
                std::map<uint32_t, int> mscount; // map for multipath counter of the faces, satisfied or nor?!
                std::map<uint32_t, bool> mflag; // flag that indicates if a request is multipath or not per request (nonce, flag)
                std::map<uint32_t, int> m_counter; // counter that shows if per request, we retrieved an equal number of replies to the number of the vailable faces for it
                std::map<uint32_t, std::map<std::string,steady_clock::time_point>> m_mapi; // map of multipath interests (nonce, (Key-ID, Ti))
                std::map<uint32_t, std::map<std::string,steady_clock::time_point>> m_mapr; // map of multipath replies (nonce, (Key-ID, Tr))

                std::map<uint32_t, steady_clock::time_point> smapr; //single path reply
                std::map<uint32_t, steady_clock::time_point> smapi; //single path request

                std::map<uint32_t, std::string> mcollect;
                std::map<uint32_t, std::map<std::string,std::string>> m_data;
                std::map<uint32_t, std::string> request;
                std::map<uint32_t, std::string> key;
                std::map<uint32_t, steady_clock::time_point> sso;
                std::map<uint32_t, steady_clock::time_point> seo;
                std::map<uint32_t, double> m_over;
                std::vector<uint32_t> loop;
                public:
                bool GN;
                bool GD;
                float mean = 0;
                Name m_nfdId;

                struct node{
                        double delay;
                        std::string arr;
                        };

                std::map<uint32_t, std::vector<node>> nodes;

            };

       } // namespace examples
    } // namespace ndn



    int
    main(int argc, char** argv)
    {
        static bool wasExecuted = false;

        ndn::examples::Tracker track;

        float sum =0;

        for (int i = 0; i <20; i++){   // average of 20 run

        sum = sum + track.estimate();

        std::cout << track.estimate() <<std::endl;
        }

        track.mean = sum/20;

        std::cout << "Average of " << std::to_string(track.mean)<< std::endl;


        if (!wasExecuted){

            try {
                track.getid();
            }
            catch (const std::exception& e) {
                std::cerr << "ERROR: " << e.what() << std::endl;
            }

            wasExecuted = true;

        }


        track.GN = false;
        track.GD = false;


        try {

            track.runp();
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }



        return 0;
    }


