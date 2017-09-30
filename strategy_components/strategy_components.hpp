  virtual void 
  found(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest, const Data& data);

  virtual void 
  notfound(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest);
  
  virtual void 
  Trace(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry);
  
  virtual void
  Cache_check(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name);
  
  virtual void
  multi_process(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry);
  
  virtual void
  single_process(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry);
  
  virtual void
  S_ForwardIt(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name);
  
  virtual void
  M_ForwardIt(const Face& inFace, const Interest& interest, const shared_ptr<pit::Entry>& pitEntry, const ndn::Name name);
  
  const ndn::Name GetLookupName(const Interest& interest);
  
  private:	
	Forwarder& m_forwarder;
	bool in = true;
	ndn::Name nx = "name";
	int i = 0;
	int j = 0;
