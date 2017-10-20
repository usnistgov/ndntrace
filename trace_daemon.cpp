#include "face.hpp"
#include "security/validator-null.hpp"
#include <stdio.h>
#include <map>
#include <chrono>
#include <ctime>
#include <ratio>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/prettywriter.h> // for stringify JSON
#include <cstdio>

namespace ndn {
namespace examples {

using namespace rapidjson;
using namespace std;


class Client : noncopyable
{
public:
	void
	run(std::string name, int life, std::string p1, std::string p2)
	{
		std::string n = "/Trace/" + p1 + "/" + p2 + name + "/" + "Key-UID" + std::to_string(number());
		ndn::Name nm;
		nm.append(n.c_str());
		ndn::Interest interest;
		interest.setName(n);
		int x = life*1000;
		interest.setInterestLifetime(time::milliseconds(x));
		interest.setMustBeFresh(true);
		m_face.expressInterest(interest,
				bind(&Client::onData, this,  _1, _2),
				bind(&Client::onNack, this, _1, _2),
				bind(&Client::onTimeout, this, _1));
		m_face.processEvents();

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
	getPathDetails(const Interest& interest, std::string data_a){
		mcol[interest.getNonce()]=pars(data_a, mcol[interest.getNonce()]);
		std::vector<node> collected = mcol[interest.getNonce()];
		if(collected.size()==1){
			std::cout << "Traced object is on this node " << std::endl;
			return;
		}else{
			std::cout << "       Localhost to: " << std::endl;
			std::cout << "                     " << std::endl;
			node a = collected[0];
			for (unsigned int index = 1; index < collected.size(); ++index){
				node  k = collected[index];
				float f = (a.delay-k.delay-sum(index, interest.getNonce()));
				std::string pp = collected[index].Id;
				std::cout << "                 " <<index << "   " << pp << "   " << f << " seconds" << std::endl;
			}
			std::cout << "                     " << std::endl;
			return;
		}
	}
	void
	getmultipaths(const Interest& interest, std::string data_a){
		vpath = pars(data_a, vpath);
		if(vpath.size()==2){
			std::cout << "Traced object is on this node " << std::endl;
			return;
		}else{
			std::cout << "       Localhost to: " << std::endl;
			std::cout << "                     " << std::endl;
			node a = vpath[1];
			for (unsigned int index = 2; index < vpath.size(); ++index){
				node  k = vpath[index];
				float f = (a.delay-k.delay-sum(index, interest.getNonce()))*1000;
				std::string pp = vpath[index].Id;
				std::cout << "                 " <<index << "   " << pp << "   " << f << std::endl;
			}
			std::cout << "                     " << std::endl;
			return;
		}

	}
	void
	onData(const Interest& interest, const Data& data)
	{
		int k =(-1)*interest.getName().size();
		if(interest.getName().at(k+1).toUri()=="S"){
			data_a = std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
			getPathDetails(interest,data_a);
		}
		if (interest.getName().at(k+1).toUri()=="M"){
			data_a = std::string(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
			std::vector<std::string> vect;
			std::string temp ="{\"Id\":\"Client\",\"delay\":0.0,\"overhead\":0.0,\"next\":" +getarray(data_a)+ "}";
			vect = m_pars(temp, vect);
			for(std::vector<std::string>::iterator it = vect.begin(); it != vect.end(); ++it) {
				std::cout << " path to decode " << *it << std::endl;
				getmultipaths(interest, *it);
				vpath.clear();
			}
		}
	}

	void
	onNack(const Interest& interest, const lp::Nack& nack)
	{
		std::cout << "received Nack with reason " << nack.getReason() << " for interest " << interest << std::endl;
	}

	void
	onTimeout(const Interest& interest)
	{
		std::cout << "Timeout " << interest << std::endl;
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

	std::string
	getarray(std::string s){
		rapidjson::Document document; //root
		std::string array;
		if ( document.Parse<0>( s.c_str() ).HasParseError() ) {
			array = "Parsing error";			
		}else{
			if(document.HasMember("next")){
				const Value& next = document["next"];
				assert(next.IsArray());
				StringBuffer strbuf;
				Writer<StringBuffer> writer(strbuf);
				next.Accept(writer);
				array = strbuf.GetString();
			}			
		}
		return array;
	}

	std::string createData(std::string id, double delay, double over, std::string s)
	{
		rapidjson::Document document;
		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		Value i;
		char b[100];
		int len = sprintf(b, "%s", id.c_str());
		i.SetString(b, len, document.GetAllocator());
		Value d(delay);
		Value o(over);
		document.AddMember("Id", i , allocator);
		document.AddMember("delay", d, allocator);
		document.AddMember("overhead", o, allocator);
		rapidjson::Document document2;
		if ( document2.Parse<0>( s.c_str() ).HasParseError() ) {
			std::cout << "Parsing error" << std::endl;
		}else{
			assert(document2.IsObject());
			document.AddMember("next", document2.GetObject(), allocator);
		}
		StringBuffer strbuf;
		Writer<StringBuffer> writer(strbuf);
		document.Accept(writer);
		return strbuf.GetString();
	}
	
	struct node{
		std::string Id;
		double delay;
		double Sig;
	};


	std::vector<node>
	pars(std::string s, std::vector<node> vect){

		rapidjson::Document document;
		if ( document.Parse<0>( s.c_str() ).HasParseError() ) {
			std::cout << "Parsing error" << std::endl;
		}else{
			node n;
			assert(document.IsObject());
			assert(document["Id"].IsString());
			n.Id=document["Id"].GetString();
			n.delay=document["delay"].GetDouble();
			n.Sig=document["overhead"].GetDouble();
			vect.push_back(n);
			if(document.HasMember("next")){

				rapidjson::StringBuffer sb;
				rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
				document["next"].Accept( writer );
				vect = pars(sb.GetString(), vect);
			}else{
				return vect;
			}
		}
		return vect;
	}

	std::string
	printNode (node n) {
		std::stringstream s;

		s << "\"Id\":\"" << n.Id <<"\",\"delay\":" << n.delay <<",\"overhead\":" << n.Sig ;
		return s.str ();
	}
	
	std::vector<std::string>
	m_pars(std::string m, std::vector<std::string> val){
		rapidjson::Document document;
		if ( document.Parse<0>( m.c_str() ).HasParseError() ) {
			std::cerr << "Parsing error" << std::endl;
			exit (-1);
		}

		node n;
		n.Id=document["Id"].GetString();
		n.delay=document["delay"].GetDouble();
		n.Sig=document["overhead"].GetDouble();

		stringstream ss;
		ss << "{";
		ss << printNode (n);
		if(document.HasMember("next")){
			const Value& next = document["next"];
			if(next.IsArray()){
				ss << ", \"next\":{";
				for (rapidjson::SizeType i = 0; i < next.Size(); i++) {
					val = m_pars (next[i], val, ss.str (), 0);
				}
			}
		} else {
			ss << "}" << std::endl;
			val.push_back(ss.str ());
		}
		return val;
	}

	std::vector<std::string>
	m_pars(const Value& v, vector<std::string> val, std::string s, int level){
		node n;
		n.Id=v["Id"].GetString();
		n.delay=v["delay"].GetDouble();
		n.Sig=v["overhead"].GetDouble();

		stringstream ss;
		ss << s;

		level++;
		ss << printNode (n);
		if(v.HasMember("next")){
			const Value& next = v["next"];
			if(next.IsArray()){
				ss << ", \"next\":{";
				for (rapidjson::SizeType i = 0; i < next.Size(); i++) {
					val = m_pars (next[i], val, ss.str (), level);
				}
			}
		} else {
			for (int i = 0; i <= level; i++) {
				ss << "}";
			}
			val.push_back(ss.str ());
		}
		return val;

	}

	std::vector<node> collected;
	std::vector<node> vpath;
	std::map<uint32_t, std::vector<node>> mcol;  //requires initiialization
	Face m_face;
	int id  = 0;
	std::string data_a;
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
	int life = 4;
	bool path = false;
	bool dest = false;

	int c ;
	while( ( c = getopt (argc, argv, "n:t:smpcbe") ) != -1 )
	{
		switch(c)
		{
		case 'n':
			name = optarg;
			break;
		case 't':
			life = atoi(optarg);
			break;
		case 's':
			p1="S"; path = true;
		case 'm':
			if (path == false){
				p1="M";
			}
			break;
		case 'p':
			p2="p"; dest = true;
			break;
		case 'c':
			if (dest == false){
				p2="c";
			}
			break;
		case 'b':
			if (dest == false){
				p2="b";
			}
			break;
		case 'e':
			if (dest == false){
				p2="e";
			}
			break;
		}
	}

	ndn::examples::Client Client;
	try {
		Client.run(name, life, p1, p2);
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}
