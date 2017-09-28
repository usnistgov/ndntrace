
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "best-route-strategy2.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"

namespace nfd {
namespace fw {

NFD_LOG_INIT("BestRouteStrategy2");
NFD_REGISTER_STRATEGY(BestRouteStrategy2);

const time::milliseconds BestRouteStrategy2::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds BestRouteStrategy2::RETX_SUPPRESSION_MAX(250);

BestRouteStrategy2::BestRouteStrategy2(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , m_forwarder(forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("BestRouteStrategy2 does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "BestRouteStrategy2 does not support version " + std::to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}



                void
        BestRouteStrategy2::notfound(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest){

            // notfound simply stays put
            NFD_LOG_DEBUG("Cs lookup negative");
            in =false;

        }

        void
        BestRouteStrategy2::found(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest, const Data& data){

            // found simply generates a nack CACHE_LOCAL


            NFD_LOG_DEBUG("In the cache");

            lp::NackHeader nackHeader;
            nackHeader.setReason(lp::NackReason::CACHE_LOCAL);

            this->sendNack(pitEntry, inFace, nackHeader);

            this->rejectPendingInterest(pitEntry);


        }


const Name&
BestRouteStrategy2::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/best-route/%FD%05");
  return strategyName;
}

/** \brief determines whether a NextHop is eligible
 *  \param inFace incoming face of current Interest
 *  \param interest incoming Interest
 *  \param nexthop next hop
 *  \param pitEntry PIT entry
 *  \param wantUnused if true, NextHop must not have unexpired out-record
 *  \param now time::steady_clock::now(), ignored if !wantUnused
 */
static inline bool
isNextHopEligible(const Face& inFace, const Interest& interest,
                  const fib::NextHop& nexthop,
                  const shared_ptr<pit::Entry>& pitEntry,
                  bool wantUnused = false,
                  time::steady_clock::TimePoint now = time::steady_clock::TimePoint::min())
{
  const Face& outFace = nexthop.getFace();

  // do not forward back to the same face, unless it is ad hoc
  if (outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC)
    return false;

  // forwarding would violate scope
  if (wouldViolateScope(inFace, interest, outFace))
    return false;

  if (wantUnused) {
    // nexthop must not have unexpired out-record
    pit::OutRecordCollection::iterator outRecord = pitEntry->getOutRecord(outFace);
    if (outRecord != pitEntry->out_end() && outRecord->getExpiry() > now) {
      return false;
    }
  }

  return true;
}

/** \brief pick an eligible NextHop with earliest out-record
 *  \note It is assumed that every nexthop has an out-record.
 */
static inline fib::NextHopList::const_iterator
findEligibleNextHopWithEarliestOutRecord(const Face& inFace, const Interest& interest,
                                         const fib::NextHopList& nexthops,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  fib::NextHopList::const_iterator found = nexthops.end();
  time::steady_clock::TimePoint earliestRenewed = time::steady_clock::TimePoint::max();
  for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
    if (!isNextHopEligible(inFace, interest, *it, pitEntry))
      continue;
    pit::OutRecordCollection::iterator outRecord = pitEntry->getOutRecord(it->getFace());
    BOOST_ASSERT(outRecord != pitEntry->out_end());
    if (outRecord->getLastRenewed() < earliestRenewed) {
      found = it;
      earliestRenewed = outRecord->getLastRenewed();
    }
  }
  return found;
}

void
BestRouteStrategy2::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  RetxSuppression::Result suppression = m_retxSuppression.decide(inFace, interest, *pitEntry);
  if (suppression == RetxSuppression::SUPPRESS) {
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " suppressed");
    return;
  }
  ////////
   std::size_t found3 = interest.getName().toUri().find("Key-TID");
            std::size_t found2 = interest.getName().toUri().find("/Trace");

            if ((found2!=std::string::npos)&&(found3!=std::string::npos)&&(inFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL)){

                int s = interest.getName().size();
                int k = (-1)*s;
                // if it's a multipath interest check

                if (interest.getName().at(k+1).toUri()=="M"){

                    NFD_LOG_DEBUG("Received a multipath interest");

                    //Constructing name

                    int i;
                    const ndn::Name c = interest.getName().at(k+3).toUri();
                    const ndn::Name n = c.toUri();

                    // actual multipath
                    std::string face_id = interest.getName().at(-1).toUri(); // Regular multipath request >> has face_id in the end

                    //const ndn::Name x = n.toUri() + c.toUri
                    ndn::Name v = n.toUri();

                    for(i=k+4; i< -2; i++){

                        v = v.toUri() + "/" + interest.getName().at(i).toUri();

                    }
                    //////////////////////// CS Lookup //////////////////////

                    NFD_LOG_DEBUG("Checking the content store ");

                    // Access to the Forwarder's FIB
                    const Cs& cs = m_forwarder.getCs();

                    // Cs lookup with name

                    NFD_LOG_DEBUG("Creating a fake interest for the cs_lookup");

                    ndn::Interest fi;
                    fi.setName(v);
                    fi.setInterestLifetime(ndn::time::milliseconds(10000));  //doesn't matter it's fake
                    cs.find(fi, bind(&BestRouteStrategy2::found, this, ref(inFace), pitEntry, _1, _2),
                    bind(&BestRouteStrategy2::notfound, this, ref(inFace), pitEntry, _1));

                    ////////////////////////////////////////////////////////
                    // No need for a fib lookup just send the interest through the face in the name

                    // Access to the Forwarder's FIB
                    const Fib& fib = m_forwarder.getFib();

                    // FIB lookup with name
                    const fib::Entry& fibEntry = fib.findLongestPrefixMatch(v);


                    // Getting faces for nexthops??
                    const fib::NextHopList& nexthops = fibEntry.getNextHops();

                    for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {

                        if(it->getFace().getId()==std::stoi(face_id)){

                            // forward vs nack
                            if((it->getFace().getScope()== ndn::nfd::FACE_SCOPE_LOCAL)){
                                // Nack it

                                NFD_LOG_DEBUG("Next hop scope multipath local ");

                                lp::NackHeader nackHeader;
                                nackHeader.setReason(lp::NackReason::PRODUCER_LOCAL);

                                NFD_LOG_DEBUG("NAck" << nackHeader.getReason());

                                this->sendNack(pitEntry, inFace, nackHeader);
                                this->rejectPendingInterest(pitEntry);
                                return;

                            }else{

                                NFD_LOG_DEBUG("Sending through the desired face");
                                this->sendInterest(pitEntry, it->getFace(), interest);
                                return;
                            }
                        }
                    }


                    return;  // End if multipath
                }

                // starting /Trace/S/P2/name/Key-ID
                if (interest.getName().at(k+1).toUri()=="S"){

                    NFD_LOG_DEBUG("**************Single path option****************");

                    NFD_LOG_DEBUG("From Traceroute ");

                    // Extract name from interest

                    int i;
                    const ndn::Name c = interest.getName().at(k+3).toUri();
                    const ndn::Name n = c.toUri() ;

                    //const ndn::Name x = n.toUri() + c.toUri
                    ndn::Name v = n.toUri();

                    for(i=k+4; i< -1; i++){
                        v = v.toUri() + "/" + interest.getName().at(i).toUri();
                    }




                    NFD_LOG_DEBUG("single path name to lookup " << v);



                    NFD_LOG_DEBUG("Last component of Trace name is: " << interest.getName().at(-1));

                    NFD_LOG_DEBUG("The component before the last component of Trace name is: " << interest.getName().at(-2));

                    const ndn::Name name = v.toUri();


                    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


                    NFD_LOG_DEBUG("Checking the content store ");

                    // Access to the Forwarder's FIB
                    const Cs& cs = m_forwarder.getCs();

                    // Cs lookup with name

                    NFD_LOG_DEBUG("Creating a fake interest for the cs_lookup");

                    ndn::Interest fi;
                    fi.setName(name);
                    fi.setInterestLifetime(ndn::time::milliseconds(10000));

                    cs.find(fi, bind(&BestRouteStrategy2::found, this, ref(inFace), pitEntry, _1, _2),
                    bind(&BestRouteStrategy2::notfound, this, ref(inFace), pitEntry, _1));


                    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/



                    // Access to the Forwarder's FIB
                    const Fib& fib = m_forwarder.getFib();

                    // FIB lookup with name
                    const fib::Entry& fibEntry = fib.findLongestPrefixMatch(name);

                    // Show corresponding prefix for that entry
                    NFD_LOG_DEBUG("Search Fib returned: " << fibEntry.getPrefix());

                    // Getting faces for nexthops??
                    const fib::NextHopList& nexthops = fibEntry.getNextHops();



                    /////////////////////////////////////////////////////////////

                    /// This part is for sending

                    for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {

                        if (it->getFace().getScope()== ndn::nfd::FACE_SCOPE_LOCAL){

                            NFD_LOG_DEBUG("Next hop scope foo " << it->getFace().getScope());

                            lp::NackHeader nackHeader;
                            nackHeader.setReason(lp::NackReason::PRODUCER_LOCAL);

                            NFD_LOG_DEBUG("NAck" << nackHeader.getReason());

                            this->sendNack(pitEntry, inFace, nackHeader);

                            this->rejectPendingInterest(pitEntry);
                            return;

                        }
                        else{
                            //std::string x = boost::lexical_cast<std::string> (it->getFace().toUri());
                            NFD_LOG_DEBUG("getting face for an iterator: " << it->getFace().getId());

                            // Local or not???
                            NFD_LOG_DEBUG("External name is Local or Not????? " << it->getFace().getScope());

                            this->sendInterest(pitEntry, it->getFace(), interest);
                            NFD_LOG_DEBUG("Sending through the desired face(s)");


                        }
                        return;

                    }

                }// End of if S
            }else {
  ////////

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();
  fib::NextHopList::const_iterator it = nexthops.end();

  if (suppression == RetxSuppression::NEW) {
    // forward to nexthop with lowest cost except downstream
    it = std::find_if(nexthops.begin(), nexthops.end(),
      bind(&isNextHopEligible, cref(inFace), interest, _1, pitEntry,
           false, time::steady_clock::TimePoint::min()));

    if (it == nexthops.end()) {
      NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");

      lp::NackHeader nackHeader;
      nackHeader.setReason(lp::NackReason::NO_ROUTE);
      this->sendNack(pitEntry, inFace, nackHeader);

      this->rejectPendingInterest(pitEntry);
      return;
    }

    Face& outFace = it->getFace();
    this->sendInterest(pitEntry, outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " newPitEntry-to=" << outFace.getId());
    return;
  }

  // find an unused upstream with lowest cost except downstream
  it = std::find_if(nexthops.begin(), nexthops.end(),
                    bind(&isNextHopEligible, cref(inFace), interest, _1, pitEntry,
                         true, time::steady_clock::now()));
  if (it != nexthops.end()) {
    Face& outFace = it->getFace();
    this->sendInterest(pitEntry, outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " retransmit-unused-to=" << outFace.getId());
    return;
  }

  // find an eligible upstream that is used earliest
  it = findEligibleNextHopWithEarliestOutRecord(inFace, interest, nexthops, pitEntry);
  if (it == nexthops.end()) {
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " retransmitNoNextHop");
  }
  else {
    Face& outFace = it->getFace();
    this->sendInterest(pitEntry, outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " retransmit-retry-to=" << outFace.getId());
  }
                        }
}

void
BestRouteStrategy2::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                     const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

} // namespace fw
} // namespace nfd
