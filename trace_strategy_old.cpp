        /*****************************************************************************************************************/       
        void
        YourDefaultStrategyHere::notfound(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest){           
            // notfound simply stays put
            NFD_LOG_DEBUG("Cs lookup negative, procees to forwarding based on S or M");
            int k = (-1)*interest.getName().size();    
            const ndn::Name name = GetLookupName(interest);              
            // if it's a multipath interest check               
            if (interest.getName().at(k+1).toUri()=="S"){
            	S_ForwardIt(inFace, interest, pitEntry, name);
            }else{
            	M_ForwardIt(inFace, interest, pitEntry, name);
	    }
        }
        
        void
        YourDefaultStrategyHere::found(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest, const Data& data){
                
            NFD_LOG_DEBUG("Traced name is cached");
            
            lp::NackHeader nackHeader;
            nackHeader.setReason(lp::NackReason::CACHE_LOCAL);
            
            this->sendNack(pitEntry, inFace, nackHeader);
            this->rejectPendingInterest(pitEntry);          
        }
        
        void 
        YourDefaultStrategyHere::Cache_check(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name){
		// P2 = c
		NFD_LOG_DEBUG("Checking the content store ");                    
		// Access to the Forwarder's FIB
		const Cs& cs = m_forwarder.getCs();                    
		//Cs lookup with name                    
		NFD_LOG_DEBUG("Creating a fake interest for the cs_lookup");                        
		ndn::Interest fi;
		fi.setName(name);
		fi.setInterestLifetime(ndn::time::milliseconds(10000));                    
		cs.find(fi, bind(&YourDefaultStrategyHere::found, this, ref(inFace), pitEntry, _1, _2),
		bind(&YourDefaultStrategyHere::notfound, this, ref(inFace), pitEntry, _1));  
	}   
	
	
	void
	YourDefaultStrategyHere::M_ForwardIt(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name){
                    const Fib& fib = m_forwarder.getFib();                    
                    // FIB lookup with name
                    const fib::Entry& fibEntry = fib.findLongestPrefixMatch(name);                                       
                    // Getting faces for nexthops??
                    std::string face_id = interest.getName().at(-1).toUri(); // Regular multipath request >> has face_id in the end
                    const fib::NextHopList& nexthops = fibEntry.getNextHops();                    
                    for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {                        
                        if(it->getFace().getId()==std::stoi(face_id)){                            
                            if((it->getFace().getScope()== ndn::nfd::FACE_SCOPE_LOCAL)){                             
                                lp::NackHeader nackHeader;
                                nackHeader.setReason(lp::NackReason::PRODUCER_LOCAL);                                                            
                                this->sendNack(pitEntry, inFace, nackHeader);
                                this->rejectPendingInterest(pitEntry);
                                return;                                
                            }else{                                
                                this->sendInterest(pitEntry, it->getFace(), interest);
                                return;
                            }
                        }
                    }                                        
                    return;	
	}
	void       
        YourDefaultStrategyHere::multi_process(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry){
                    NFD_LOG_DEBUG("Received a multipath interest");                    
                    //Constructing name                    
                    int i;
                    int k = (-1)*interest.getName().size();  
                    const ndn::Name c = interest.getName().at(k+3).toUri();
                    const ndn::Name n = c.toUri();                    
                    // actual multipath
                    std::string face_id = interest.getName().at(-1).toUri(); // Regular multipath request >> has face_id in the end
                    ndn::Name v = n.toUri();                    
                    for(i=k+4; i< -2; i++){                        
                        v = v.toUri() + "/" + interest.getName().at(i).toUri();                        
                    }                
                    
                    const ndn::Name name = v.toUri();                                                          
                    if (interest.getName().at(k+2).toUri()== "c"){                    
                        Cache_check(inFace, pitEntry, name);               
                    }else{
                    	M_ForwardIt(inFace, interest, pitEntry, name);
                    } 
                   
                }
	
	void
        YourDefaultStrategyHere::S_ForwardIt(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name){
		// Access to the Forwarder's FIB
		const Fib& fib = m_forwarder.getFib();                    
		// FIB lookup with name
		const fib::Entry& fibEntry = fib.findLongestPrefixMatch(name);                                    
		// Getting faces for nexthops??
		const fib::NextHopList& nexthops = fibEntry.getNextHops();
		/// This part is for sending                    
		for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {                        
			 if (it->getFace().getScope()== ndn::nfd::FACE_SCOPE_LOCAL){                                                       
				  lp::NackHeader nackHeader;
				  nackHeader.setReason(lp::NackReason::PRODUCER_LOCAL);                                                       
				  this->sendNack(pitEntry, inFace, nackHeader);                            
				  this->rejectPendingInterest(pitEntry);
				  return;                           
			 }else{                          
				  this->sendInterest(pitEntry, it->getFace(), interest);                       
				  }
			  return;                        
		}
	}
	
	        
        const ndn::Name
        YourDefaultStrategyHere::GetLookupName(const Interest& interest){                 
		            int i;
		            int k = (-1)*interest.getName().size();
		            const ndn::Name c = interest.getName().at(k+3).toUri();
		            const ndn::Name n = c.toUri() ;                    
		            //const ndn::Name x = n.toUri() + c.toUri
		            ndn::Name v = n.toUri();                   
		            for(i=k+4; i< -1; i++){
		                   v = v.toUri() + "/" + interest.getName().at(i).toUri();
		            }                                 
		            return v.toUri(); 		            
                    }
                    
	void
        YourDefaultStrategyHere::single_process(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry) {        	                  
                    NFD_LOG_DEBUG("**************Single path option****************");                   
                    // Extract name from interest   
                    const ndn::Name name = GetLookupName(interest);   
                    int k = (-1)*interest.getName().size();                                                         
                    if (interest.getName().at(k+2).toUri()== "c"){                    
                           Cache_check(inFace, pitEntry, name);               
                    }else{
                    	S_ForwardIt(inFace, interest, pitEntry, name);
                    } // end of option p2                    
        }// End of if S
	
        void 
        YourDefaultStrategyHere::Trace(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry){
            std::size_t found3 = interest.getName().toUri().find("Key-TID");
            std::size_t found2 = interest.getName().toUri().find("/Trace");
            
            if ((found2!=std::string::npos)&&(found3!=std::string::npos)&&(inFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL)){
                int k = (-1)*interest.getName().size();                
                // if it's a multipath interest check               
                if (interest.getName().at(k+1).toUri()=="M"){
                	multi_process(inFace, interest, pitEntry);
                }
            
            	if (interest.getName().at(k+1).toUri()=="S"){              
        		single_process(inFace, interest, pitEntry);       
            	} 
          }
        }
        
        
