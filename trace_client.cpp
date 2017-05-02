
// #include <ndn-cxx/face.hpp>
#include "face.hpp"
//#include "nfdidcollector.hpp"
#include "security/validator-null.hpp"
#include <stdio.h>
#include <map>
#include <chrono>
#include <ctime>
#include <ratio>
#include <iostream>
#include <fstream>
#include <vector>
#include "rapidxml.hpp"
#include <sstream>



// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

using std::string;
using namespace std::chrono;
using namespace rapidxml;


class User : noncopyable
{
public:
  void
  run(std::string name, int life, std::string p1, std::string p2)
  {
        std::string sid = std::to_string(number());
        std::string aa = "Key-UID" + sid;
        ndn::Name n = "/Trace/" + p1 + "/" + p2 + "/" + name + "/" + aa ;
        ndn::Interest interest;
        interest.setName(n);
        int x = life*1000;
        interest.setInterestLifetime(time::milliseconds(x));
        interest.setMustBeFresh(true);

        mi[interest.getNonce()] = steady_clock::now();

        m_face.expressInterest(interest,
                           bind(&User::onData, this,  _1, _2),
                           bind(&User::onNack, this, _1, _2),
                           bind(&User::onTimeout, this, _1));



        std::cout << "                 " << std::endl;
        std::cout << "Sending interest " << interest << std::endl;
        std::cout << "                 " << std::endl;
        //std::cout << "parameter " << name << " " << life << " "<< p1 << " " << p2 << std::endl;
        m_face.processEvents();

  }

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
private:
  void
  onData(const Interest& interest, const Data& data)
  {

int k =(-1)*interest.getName().size();

mr[interest.getNonce()]= steady_clock::now();

steady_clock::duration time_span = mr[interest.getNonce()] - mi[interest.getNonce()];

double nseconds = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;


if(interest.getName().at(k+1).toUri()=="S"){

//std::vector<node> collected = mcol[interest.getNonce()];  // vector will contain what was received for this request in the case of single path

        data_a = std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
        //std::cout << "*****Received Data:  " << data_a <<std::endl;
        std::string m;
        int n = count(data_a, "!") ;
        int i = 0;
        for ( i=0; i < n ; i++){
                data_a = aextract(data_a, "<");
                m = bextract(data_a, ">");

                node n ={ bextract(m, "(*)"), std::stof(aextract(m, "(*)")), std::stof(aextract(m, "(**)")) };
                mcol[interest.getNonce()].push_back(n);

        }

        // A simple check
        std::vector<node> collected = mcol[interest.getNonce()];
       std::cout << "       Localhost to: " << std::endl;
       std::cout << "                     " << std::endl;

       node a = collected[0];
       for (int index = 1; index < collected.size(); ++index){
                node  k = collected[index];
                float f = (a.delay-k.delay-sum(index, interest.getNonce()))*1000;
                std::string pp = collected[index].Id;
                pp = aextract(pp, "/KEY");
                pp = bextract(pp, "/ID-CERT");

                std::cout << "                 " <<index << "   " << pp << "   " << f << std::endl;
        }
        std::cout << "                     " << std::endl;
        std::cout << "                     " << std::endl;
return;
}


if (interest.getName().at(k+1).toUri()=="M"){



        data_a = std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());

        std::string g = "<?xmlversion=\"1.0\"encoding=\"utf-8\"?><Trace><node name=\" \">"+data_a+"></node></Trace>";

        //std::cout << g << std::endl;

        v[interest.getNonce()]=PrintPaths(g, v[interest.getNonce()]);

        std::vector<std::string> path = v[interest.getNonce()];  // get the corresponding vector of paths from the map of vectors

        // to check, we can print the paths:
        int k =0;
        for(std::vector<std::string>::iterator it = path.begin(); it != path.end(); ++it) {
                std::string o = *it;
             std::cout << "path  " <<  k+1 <<"  : " <<std::endl;
           //  std::cout << "  path  " << std::endl;
             k++;

                // Extract from every path (o)

                std::string m;
                int n = count(o, "}!") ;
                //std::cout << "  number of packets per path:  "  <<  n << std::endl;
                int i =0;
                std::vector<node> vn;
                for ( i=1; i <= n ; i++){
                        o = aextract(o, "{");
                        m = bextract(o, "}!");

                        node no ={bextract(m, "(*)"), std::stof(aextract(m, "(*)")), std::stof(aextract(m, "(**)")) };
                        vn.push_back(no);

                        //std::cout << "node " << no.Id << "  " << no.Sig << "  " << no.delay  << std::endl;

                  }

                // A simple check
                std::cout << "       Localhost to: " << std::endl;
                std::cout << "                     " << std::endl;


                node a = vn[0];
                for (int index = 1; index < vn.size(); ++index){
                                node  kk = vn[index];

                                        float f =0;
                                        int l =0;
                                        for (int l =1; l <=index; l++ ){
                                                node nn = vn[l];
                                                f = f + nn.Sig;
                                        }

                                        float ff = (a.delay-a.Sig-f)*1000;  // this is wrongs

                                        float c =0;
                                        if (ff <0){
                                         c = (a.delay-kk.delay)*1000;
                                         }else{
                                         c= ff;
                                         }

                                        std::string p = vn[index].Id;
                                        p = aextract(p, "/KEY");
                                        p = bextract(p, "/ID-CERT");

                                        std::cout << "                 " <<index << "   " << p << "   " << c << std::endl;



                 }

                std::cout << "                     " << std::endl;
                std::cout << "                     " << std::endl;
                vn.clear();


        }


}
}

  void
  onNack(const Interest& interest, const lp::Nack& nack)
  {
    std::cout << "received Nack with reason " << nack.getReason()
              << " for interest " << interest << std::endl;
  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << "Timeout " << interest << std::endl;
  }


std::string aextract(std::string x, std::string delim){

        return x.substr(x.find(delim) + delim.size());

}


float sum(int x, uint32_t nonce){

std::vector<node> v = mcol[nonce];

float f =0;

        for (int index =1; index <=x; index++ ){
                node n = v[index];
                f = f + n.Sig;
        }

return f;

}



std::string bextract(std::string const& s, std::string a)
{
    std::string::size_type pos = s.find(a);
    if (pos != std::string::npos)
    {
        return s.substr(0, pos);
    }
    else
    {
        return s;
    }
}

int count(const std::string& str, const std::string& sub)
{
        if (sub.length() == 0) return 0;
        int count = 0;
        for (size_t offset = str.find(sub); offset != std::string::npos;
        offset = str.find(sub, offset + sub.length()))
        {
                count++;
        }
        return count;
}



void
printer(xml_node<> * n){

std::cout << n->first_attribute("name")->value() <<std::endl; //print tag

}




std::vector<std::string>
recur (xml_node<> * x, string conca, std::vector<std::string> v){

        string tempo = conca;


        for (xml_node<> * node = x->first_node("child"); node; node = node->next_sibling())
        {
         //child level node = child tag

                for(xml_node<> * node_node = node->first_node("node"); node_node; node_node = node_node->next_sibling())
            {
                //node level node_node = node tag

                conca = conca + node_node->first_attribute("name")->value();

                //print only when there is no child, which means we are at the last node in the path
                if(NULL == node_node->first_node("child"))
                {


                        //std::cout << conca <<std::endl;
                        v.push_back(conca);
                        //s=s+"/"+conca;

                }

                v = recur(node_node,conca,v);

            }
                //reuse the old concat when moving to the next child of the same node
                conca = tempo;


        }
        conca="";
        return v;
}


std::vector<std::string>
PrintPaths(std::string content, std::vector<std::string> v){

        xml_document<> doc;
        xml_node<> * root_node;

        std::vector<char> buffer(content.begin(), content.end());
        buffer.push_back('\0');

        doc.parse<0>(&buffer[0]);

        root_node = doc.first_node("Trace");
        xml_node<> * root2 = root_node->first_node("node");

        v = recur(root2,root2->first_attribute("name")->value(), v);
        // <<  <<std::endl;
        return v;
}


private:
        struct node{
               std::string Id;
               float delay;
               float Sig;
             };

        //std::vector<node> collected;
        std::map<uint32_t, std::vector<node>> mcol;  //requires initiialization
        //std::map<std:string, std::vector<node>> mpcol;  //requires initiialization

        Face m_face;
        int id  = 0;
        std::string data_a;
        std::map<uint32_t, steady_clock::time_point> mi;
        std::map<uint32_t, steady_clock::time_point> mr;
        //std::map<uint32_t, clock_t> smapi;
        std::map<uint32_t, std::vector<std::string>> v;


};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{


    std::string p1;
    std::string p2;
    std::string name;
    int life;

    int c ;
    while( ( c = getopt (argc, argv, "n:t:mspcbe") ) != -1 )
    {
        switch(c)
        {
            case 'n':
                name = optarg;
                break;
            case 't':
                life = atoi(optarg);
                break;
            case 'm':
                p1="M";
                break;
            case 's':
                p1="S";
            case 'p':
                p2="01";
                break;
            case 'c':
                p2="10";
                break;
            case 'b':
                p2="11";
                break;
            case 'e':
                p2="00";
                break;
        }
    }

  ndn::examples::User user;
  try {
    user.run(name, life, p1, p2);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
